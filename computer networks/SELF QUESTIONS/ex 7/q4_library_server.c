/* Self-Question 4 (Page 23): Iterative TCP server - College Library Management System.
   Search by title/author, check availability, issue, return, view issued details.
   DB: books.dat, issues.dat. Logs to library.log */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>

#define PORT 6004
#define BUF 2048
#define MAXBK 100
#define MAXIS 200

typedef struct { char id[10], title[40], author[30], category[20]; int avail; } Book;
typedef struct { char id[10], bookid[10], member[40]; char status[10]; } Issue;

Book books[MAXBK]; int bcount = 0;
Issue issues[MAXIS]; int icount = 0;

void log_it(const char *msg) {
    time_t now = time(NULL); char ts[64];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", localtime(&now));
    FILE *lf = fopen("library.log", "a"); fprintf(lf, "[%s] %s\n", ts, msg); fclose(lf);
}

void load(void) {
    FILE *fp = fopen("books.dat", "r");
    if (fp) { while (fscanf(fp,"%9s %39s %29s %19s %d",books[bcount].id,books[bcount].title,books[bcount].author,books[bcount].category,&books[bcount].avail)==5) bcount++; fclose(fp); }
    if (bcount == 0) {
        strcpy(books[0].id,"B1"); strcpy(books[0].title,"Computer_Networks"); strcpy(books[0].author,"Tanenbaum"); strcpy(books[0].category,"CS"); books[0].avail=1;
        strcpy(books[1].id,"B2"); strcpy(books[1].title,"Operating_Systems"); strcpy(books[1].author,"Galvin"); strcpy(books[1].category,"CS"); books[1].avail=1;
        bcount = 2;
    }
    fp = fopen("issues.dat", "r");
    if (fp) { while (fscanf(fp,"%9s %9s %39s %9s",issues[icount].id,issues[icount].bookid,issues[icount].member,issues[icount].status)==4) icount++; fclose(fp); }
}
void save_books(void){ FILE *fp=fopen("books.dat","w"); for(int i=0;i<bcount;i++) fprintf(fp,"%s %s %s %s %d\n",books[i].id,books[i].title,books[i].author,books[i].category,books[i].avail); fclose(fp);}
void save_issues(void){ FILE *fp=fopen("issues.dat","w"); for(int i=0;i<icount;i++) fprintf(fp,"%s %s %s %s\n",issues[i].id,issues[i].bookid,issues[i].member,issues[i].status); fclose(fp);}

int find_book_id(const char *id){ for(int i=0;i<bcount;i++) if(strcmp(books[i].id,id)==0) return i; return -1; }
int find_issue(const char *id){ for(int i=0;i<icount;i++) if(strcmp(issues[i].id,id)==0) return i; return -1; }

void serve_client(int fd) {
    char buf[BUF], resp[BUF]; int n;
    while ((n = recv(fd, buf, BUF - 1, 0)) > 0) {
        buf[n] = '\0';
        char cmd[20]; sscanf(buf,"%19s",cmd);
        resp[0]='\0';

        if (strcmp(cmd,"SEARCH")==0) {
            char key[40]; sscanf(buf,"SEARCH %39s",key);
            for (int i=0;i<bcount;i++) if (strstr(books[i].title,key) || strstr(books[i].author,key)) {
                char l[128]; sprintf(l,"%s %s by %s [%s] avail=%d\n",books[i].id,books[i].title,books[i].author,books[i].category,books[i].avail); strcat(resp,l);
            }
            if (resp[0]=='\0') strcpy(resp,"No matching books found");
        } else if (strcmp(cmd,"AVAIL")==0) {
            char id[10]; sscanf(buf,"AVAIL %9s",id); int idx=find_book_id(id);
            if (idx<0) sprintf(resp,"ERR: book not found"); else sprintf(resp,"%s availability: %s",id,books[idx].avail?"AVAILABLE":"ISSUED");
        } else if (strcmp(cmd,"ISSUE")==0) {
            char id[10], member[40]; sscanf(buf,"ISSUE %9s %39s",id,member);
            int idx=find_book_id(id);
            if (idx<0) sprintf(resp,"ERR: invalid book");
            else if (!books[idx].avail) sprintf(resp,"ERR: book already issued");
            else {
                books[idx].avail = 0;
                sprintf(issues[icount].id,"I%04d",icount+1000);
                strcpy(issues[icount].bookid,id); strcpy(issues[icount].member,member); strcpy(issues[icount].status,"ISSUED");
                icount++;
                save_books(); save_issues();
                sprintf(resp,"Book issued. IssueID=%s to %s",issues[icount-1].id,member);
                log_it(resp);
            }
        } else if (strcmp(cmd,"RETURN")==0) {
            char id[10]; sscanf(buf,"RETURN %9s",id); int idx=find_issue(id);
            if (idx<0) sprintf(resp,"ERR: issue record not found");
            else {
                strcpy(issues[idx].status,"RETURNED");
                int bidx=find_book_id(issues[idx].bookid); if (bidx>=0) books[bidx].avail = 1;
                save_books(); save_issues();
                sprintf(resp,"Book returned for issue %s",id); log_it(resp);
            }
        } else if (strcmp(cmd,"DETAILS")==0) {
            char id[10]; sscanf(buf,"DETAILS %9s",id); int idx=find_issue(id);
            if (idx<0) sprintf(resp,"ERR: issue record not found");
            else sprintf(resp,"IssueID:%s Book:%s Member:%s Status:%s",issues[idx].id,issues[idx].bookid,issues[idx].member,issues[idx].status);
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
    printf("Iterative Library Management Server on port %d (%d books loaded)\n", PORT, bcount);

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
