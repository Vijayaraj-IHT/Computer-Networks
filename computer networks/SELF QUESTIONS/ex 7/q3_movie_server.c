/* Self-Question 3 (Page 23): Iterative TCP server - Movie Ticket Booking System.
   View movie list, check seats, book, cancel, display booking details.
   DB: movies.dat, mbookings.dat. Logs to movie.log */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>

#define PORT 6003
#define BUF 2048
#define MAXM 20
#define MAXB 200

typedef struct { char id[10], title[40], showtime[20]; int seats, price; } Movie;
typedef struct { char bid[10], movieid[10], customer[40]; int seats; char status[10]; } MBooking;

Movie movies[MAXM]; int mcount = 0;
MBooking bks[MAXB]; int bcount = 0;

void log_it(const char *msg) {
    time_t now = time(NULL); char ts[64];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", localtime(&now));
    FILE *lf = fopen("movie.log", "a"); fprintf(lf, "[%s] %s\n", ts, msg); fclose(lf);
}

void load(void) {
    FILE *fp = fopen("movies.dat", "r");
    if (fp) { while (fscanf(fp,"%9s %39s %19s %d %d",movies[mcount].id,movies[mcount].title,movies[mcount].showtime,&movies[mcount].seats,&movies[mcount].price)==5) mcount++; fclose(fp); }
    if (mcount == 0) {
        strcpy(movies[0].id,"M1"); strcpy(movies[0].title,"Interstellar_Return"); strcpy(movies[0].showtime,"18:00"); movies[0].seats=60; movies[0].price=200;
        strcpy(movies[1].id,"M2"); strcpy(movies[1].title,"Vikram_2"); strcpy(movies[1].showtime,"21:00"); movies[1].seats=80; movies[1].price=180;
        mcount = 2;
    }
    fp = fopen("mbookings.dat", "r");
    if (fp) { while (fscanf(fp,"%9s %9s %39s %d %9s",bks[bcount].bid,bks[bcount].movieid,bks[bcount].customer,&bks[bcount].seats,bks[bcount].status)==5) bcount++; fclose(fp); }
}
void save_movies(void){ FILE *fp=fopen("movies.dat","w"); for(int i=0;i<mcount;i++) fprintf(fp,"%s %s %s %d %d\n",movies[i].id,movies[i].title,movies[i].showtime,movies[i].seats,movies[i].price); fclose(fp);}
void save_bks(void){ FILE *fp=fopen("mbookings.dat","w"); for(int i=0;i<bcount;i++) fprintf(fp,"%s %s %s %d %s\n",bks[i].bid,bks[i].movieid,bks[i].customer,bks[i].seats,bks[i].status); fclose(fp);}

int find_movie(const char *id){ for(int i=0;i<mcount;i++) if(strcmp(movies[i].id,id)==0) return i; return -1; }
int find_bk(const char *bid){ for(int i=0;i<bcount;i++) if(strcmp(bks[i].bid,bid)==0) return i; return -1; }

void serve_client(int fd) {
    char buf[BUF], resp[BUF]; int n;
    while ((n = recv(fd, buf, BUF - 1, 0)) > 0) {
        buf[n] = '\0';
        char cmd[20]; sscanf(buf,"%19s",cmd);
        resp[0]='\0';

        if (strcmp(cmd,"MOVIES")==0) {
            for (int i=0;i<mcount;i++) { char l[128]; sprintf(l,"%s %s %s seats=%d price=%d\n",movies[i].id,movies[i].title,movies[i].showtime,movies[i].seats,movies[i].price); strcat(resp,l); }
        } else if (strcmp(cmd,"AVAIL")==0) {
            char id[10]; sscanf(buf,"AVAIL %9s",id); int idx=find_movie(id);
            if (idx<0) sprintf(resp,"ERR: movie not found"); else sprintf(resp,"%s available seats: %d",id,movies[idx].seats);
        } else if (strcmp(cmd,"BOOK")==0) {
            char id[10], cust[40]; int ns;
            sscanf(buf,"BOOK %9s %39s %d",id,cust,&ns);
            int idx=find_movie(id);
            if (idx<0) sprintf(resp,"ERR: invalid movie");
            else if (movies[idx].seats<ns) sprintf(resp,"ERR: only %d seats available",movies[idx].seats);
            else {
                movies[idx].seats -= ns;
                sprintf(bks[bcount].bid,"B%04d",bcount+1000);
                strcpy(bks[bcount].movieid,id); strcpy(bks[bcount].customer,cust); bks[bcount].seats=ns; strcpy(bks[bcount].status,"BOOKED");
                bcount++;
                save_movies(); save_bks();
                sprintf(resp,"Booking confirmed. ID=%s total=%d",bks[bcount-1].bid,movies[idx].price*ns);
                log_it(resp);
            }
        } else if (strcmp(cmd,"CANCEL")==0) {
            char bid[10]; sscanf(buf,"CANCEL %9s",bid); int idx=find_bk(bid);
            if (idx<0) sprintf(resp,"ERR: booking not found");
            else {
                strcpy(bks[idx].status,"CANCELLED");
                int mid=find_movie(bks[idx].movieid); if (mid>=0) movies[mid].seats += bks[idx].seats;
                save_movies(); save_bks();
                sprintf(resp,"Booking %s cancelled",bid); log_it(resp);
            }
        } else if (strcmp(cmd,"DETAILS")==0) {
            char bid[10]; sscanf(buf,"DETAILS %9s",bid); int idx=find_bk(bid);
            if (idx<0) sprintf(resp,"ERR: booking not found");
            else sprintf(resp,"BookingID:%s Movie:%s Customer:%s Seats:%d Status:%s",bks[idx].bid,bks[idx].movieid,bks[idx].customer,bks[idx].seats,bks[idx].status);
        } else if (strcmp(cmd,"EXIT")==0) { log_it("Client session ended"); break; }
        else strcpy(resp,"ERR: invalid command");

        send(fd, resp, strlen(resp), 0);
        printf("Processed: %s -> %s\n", cmd, resp);
    }
}

int main(void) {
    load();
    int srv_fd, cli_fd; struct sockaddr_in srv_addr, cli_addr; socklen_t cli_len=sizeof(cli_addr);
    srv_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt=1; setsockopt(srv_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    srv_addr.sin_family=AF_INET; srv_addr.sin_addr.s_addr=INADDR_ANY; srv_addr.sin_port=htons(PORT);
    bind(srv_fd,(struct sockaddr*)&srv_addr,sizeof(srv_addr));
    listen(srv_fd,5);
    printf("Iterative Movie Ticket Booking Server on port %d (%d movies loaded)\n", PORT, mcount);

    while (1) {
        cli_fd = accept(srv_fd,(struct sockaddr*)&cli_addr,&cli_len);
        printf("Client connected: %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
        serve_client(cli_fd);
        close(cli_fd);
        printf("Client finished; ready for next client.\n");
    }
    close(srv_fd);
    return 0;
}
