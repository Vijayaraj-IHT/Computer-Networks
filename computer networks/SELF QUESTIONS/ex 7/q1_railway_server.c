/* Self-Question 1 (Page 23): Iterative TCP server - Online Railway Reservation System.
   Serves ONE client fully (search train, check seats, book, cancel, view status)
   before accepting the next. DB: trains.dat, bookings.dat. Logs to railway.log */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>

#define PORT 6001
#define BUF 2048
#define MAXTR 50
#define MAXBK 200

typedef struct { char id[10], name[40], route[40]; int seats, fare; } Train;
typedef struct { char pnr[12], trainid[10], passenger[40]; int seats; char status[10]; } Booking;

Train trains[MAXTR]; int tcount = 0;
Booking bookings[MAXBK]; int bcount = 0;

void log_it(const char *msg) {
    time_t now = time(NULL); char ts[64];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", localtime(&now));
    FILE *lf = fopen("railway.log", "a");
    fprintf(lf, "[%s] %s\n", ts, msg);
    fclose(lf);
}

void load(void) {
    FILE *fp = fopen("trains.dat", "r");
    if (fp) {
        while (fscanf(fp, "%9s %39s %39s %d %d", trains[tcount].id, trains[tcount].name,
                trains[tcount].route, &trains[tcount].seats, &trains[tcount].fare) == 5) tcount++;
        fclose(fp);
    }
    if (tcount == 0) {
        strcpy(trains[0].id,"T100"); strcpy(trains[0].name,"Chennai_Exp"); strcpy(trains[0].route,"CHN-MDU"); trains[0].seats=50; trains[0].fare=450;
        strcpy(trains[1].id,"T101"); strcpy(trains[1].name,"Vaigai_Exp"); strcpy(trains[1].route,"MDU-CHN"); trains[1].seats=40; trains[1].fare=380;
        tcount = 2;
    }
    fp = fopen("bookings.dat", "r");
    if (fp) {
        while (fscanf(fp, "%11s %9s %39s %d %9s", bookings[bcount].pnr, bookings[bcount].trainid,
                bookings[bcount].passenger, &bookings[bcount].seats, bookings[bcount].status) == 5) bcount++;
        fclose(fp);
    }
}

void save_trains(void) {
    FILE *fp = fopen("trains.dat", "w");
    for (int i = 0; i < tcount; i++)
        fprintf(fp, "%s %s %s %d %d\n", trains[i].id, trains[i].name, trains[i].route, trains[i].seats, trains[i].fare);
    fclose(fp);
}
void save_bookings(void) {
    FILE *fp = fopen("bookings.dat", "w");
    for (int i = 0; i < bcount; i++)
        fprintf(fp, "%s %s %s %d %s\n", bookings[i].pnr, bookings[i].trainid, bookings[i].passenger, bookings[i].seats, bookings[i].status);
    fclose(fp);
}

int find_train(const char *id) {
    for (int i = 0; i < tcount; i++) if (strcmp(trains[i].id, id) == 0) return i;
    return -1;
}
int find_booking(const char *pnr) {
    for (int i = 0; i < bcount; i++) if (strcmp(bookings[i].pnr, pnr) == 0) return i;
    return -1;
}

void serve_client(int fd) {
    char buf[BUF], resp[BUF];
    int n;
    while ((n = recv(fd, buf, BUF - 1, 0)) > 0) {
        buf[n] = '\0';
        char cmd[20]; sscanf(buf, "%19s", cmd);
        resp[0] = '\0';

        if (strcmp(cmd, "SEARCH") == 0) {
            for (int i = 0; i < tcount; i++) {
                char line[128];
                sprintf(line, "%s %s %s seats=%d fare=%d\n", trains[i].id, trains[i].name, trains[i].route, trains[i].seats, trains[i].fare);
                strcat(resp, line);
            }
            if (tcount == 0) strcpy(resp, "No trains available");
        } else if (strcmp(cmd, "AVAIL") == 0) {
            char id[10]; sscanf(buf, "AVAIL %9s", id);
            int idx = find_train(id);
            if (idx < 0) sprintf(resp, "ERR: train not found");
            else sprintf(resp, "%s available seats: %d", id, trains[idx].seats);
        } else if (strcmp(cmd, "BOOK") == 0) {
            char id[10], name[40]; int nseats;
            sscanf(buf, "BOOK %9s %39s %d", id, name, &nseats);
            int idx = find_train(id);
            if (idx < 0) sprintf(resp, "ERR: invalid train");
            else if (trains[idx].seats < nseats) sprintf(resp, "ERR: only %d seats available", trains[idx].seats);
            else {
                trains[idx].seats -= nseats;
                sprintf(bookings[bcount].pnr, "PNR%04d", bcount + 1000);
                strcpy(bookings[bcount].trainid, id);
                strcpy(bookings[bcount].passenger, name);
                bookings[bcount].seats = nseats;
                strcpy(bookings[bcount].status, "CONFIRMED");
                bcount++;
                save_trains(); save_bookings();
                sprintf(resp, "Booking confirmed. PNR=%s fare=%d", bookings[bcount-1].pnr, trains[idx].fare * nseats);
                log_it(resp);
            }
        } else if (strcmp(cmd, "CANCEL") == 0) {
            char pnr[12]; sscanf(buf, "CANCEL %11s", pnr);
            int idx = find_booking(pnr);
            if (idx < 0) sprintf(resp, "ERR: PNR not found");
            else {
                strcpy(bookings[idx].status, "CANCELLED");
                int tid = find_train(bookings[idx].trainid);
                if (tid >= 0) trains[tid].seats += bookings[idx].seats;
                save_trains(); save_bookings();
                sprintf(resp, "Booking %s cancelled, seats released", pnr);
                log_it(resp);
            }
        } else if (strcmp(cmd, "STATUS") == 0) {
            char pnr[12]; sscanf(buf, "STATUS %11s", pnr);
            int idx = find_booking(pnr);
            if (idx < 0) sprintf(resp, "ERR: PNR not found");
            else sprintf(resp, "PNR:%s Train:%s Passenger:%s Seats:%d Status:%s",
                bookings[idx].pnr, bookings[idx].trainid, bookings[idx].passenger, bookings[idx].seats, bookings[idx].status);
        } else if (strcmp(cmd, "EXIT") == 0) {
            log_it("Client session ended");
            break;
        } else sprintf(resp, "ERR: invalid command");

        send(fd, resp, strlen(resp), 0);
        printf("Processed: %s -> %s\n", cmd, resp);
    }
}

int main(void) {
    load();
    int srv_fd, cli_fd;
    struct sockaddr_in srv_addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);

    srv_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(srv_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    srv_addr.sin_family = AF_INET; srv_addr.sin_addr.s_addr = INADDR_ANY; srv_addr.sin_port = htons(PORT);
    bind(srv_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    listen(srv_fd, 5);
    printf("Iterative Railway Reservation Server on port %d (%d trains loaded)\n", PORT, tcount);

    while (1) {
        cli_fd = accept(srv_fd, (struct sockaddr *)&cli_addr, &cli_len);
        printf("Client connected: %s:%d (serving iteratively)\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
        serve_client(cli_fd);
        close(cli_fd);
        printf("Client finished; ready for next client.\n");
    }
    close(srv_fd);
    return 0;
}
