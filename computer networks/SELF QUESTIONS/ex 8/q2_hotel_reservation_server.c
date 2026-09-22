/* Self-Question 2 (Page 26): Concurrent TCP server - Hotel Reservation System.
   Multiple clients book rooms concurrently. Mutex ensures two clients cannot
   reserve the same room simultaneously. Compile: gcc q2_hotel_reservation_server.c
   -o hotel_server -lpthread */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 7002
#define BUF 2048
#define NROOMS 10

pthread_mutex_t room_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct { int room_no; int booked; char guest[40]; } Room;
Room rooms[NROOMS];

void log_it(const char *msg) {
    time_t now = time(NULL); char ts[64];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", localtime(&now));
    FILE *lf = fopen("hotel.log", "a"); fprintf(lf, "[%s] %s\n", ts, msg); fclose(lf);
}

typedef struct { int fd; struct sockaddr_in addr; } ClientArg;

void *client_thread(void *arg) {
    ClientArg *ca = (ClientArg *)arg;
    int fd = ca->fd;
    char *ip = inet_ntoa(ca->addr.sin_addr);
    int cport = ntohs(ca->addr.sin_port);
    char buf[BUF], resp[BUF];
    printf("[Thread %lu] Client connected: %s:%d\n", pthread_self(), ip, cport);

    int n;
    while ((n = recv(fd, buf, BUF - 1, 0)) > 0) {
        buf[n] = '\0';
        char cmd[20]; sscanf(buf, "%19s", cmd);
        resp[0] = '\0';

        if (strcmp(cmd, "AVAIL") == 0) {
            pthread_mutex_lock(&room_lock);
            for (int i = 0; i < NROOMS; i++)
                if (!rooms[i].booked) { char l[32]; sprintf(l, "Room%d ", rooms[i].room_no); strcat(resp, l); }
            pthread_mutex_unlock(&room_lock);
            if (resp[0] == '\0') strcpy(resp, "No rooms available");
        } else if (strcmp(cmd, "BOOK") == 0) {
            int rno; char guest[40];
            sscanf(buf, "BOOK %d %39s", &rno, guest);
            pthread_mutex_lock(&room_lock); /* critical section: prevent double booking */
            if (rno < 1 || rno > NROOMS) sprintf(resp, "ERR: invalid room number");
            else if (rooms[rno-1].booked) sprintf(resp, "ERR: Room%d already booked", rno);
            else {
                rooms[rno-1].booked = 1;
                strcpy(rooms[rno-1].guest, guest);
                sprintf(resp, "Room%d booked for %s", rno, guest);
                log_it(resp);
            }
            pthread_mutex_unlock(&room_lock);
        } else if (strcmp(cmd, "CANCEL") == 0) {
            int rno; sscanf(buf, "CANCEL %d", &rno);
            pthread_mutex_lock(&room_lock);
            if (rno < 1 || rno > NROOMS || !rooms[rno-1].booked) sprintf(resp, "ERR: room not booked");
            else {
                rooms[rno-1].booked = 0;
                sprintf(resp, "Room%d reservation cancelled", rno);
                log_it(resp);
            }
            pthread_mutex_unlock(&room_lock);
        } else if (strcmp(cmd, "DETAILS") == 0) {
            int rno; sscanf(buf, "DETAILS %d", &rno);
            pthread_mutex_lock(&room_lock);
            if (rno < 1 || rno > NROOMS) sprintf(resp, "ERR: invalid room number");
            else if (!rooms[rno-1].booked) sprintf(resp, "Room%d is not booked", rno);
            else sprintf(resp, "Room%d booked by %s", rno, rooms[rno-1].guest);
            pthread_mutex_unlock(&room_lock);
        } else if (strcmp(cmd, "EXIT") == 0) { log_it("Client session ended"); break; }
        else strcpy(resp, "ERR: invalid command");

        send(fd, resp, strlen(resp), 0);
        printf("[Thread %lu] %s -> %s\n", pthread_self(), cmd, resp);
    }
    close(fd);
    printf("[Thread %lu] Client %s:%d disconnected.\n", pthread_self(), ip, cport);
    free(ca);
    return NULL;
}

int main(void) {
    for (int i = 0; i < NROOMS; i++) { rooms[i].room_no = i + 1; rooms[i].booked = 0; }

    int srv_fd; struct sockaddr_in srv_addr;
    srv_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1; setsockopt(srv_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    srv_addr.sin_family = AF_INET; srv_addr.sin_addr.s_addr = INADDR_ANY; srv_addr.sin_port = htons(PORT);
    bind(srv_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    listen(srv_fd, 10);
    printf("Concurrent Hotel Reservation Server on port %d (%d rooms)...\n", PORT, NROOMS);

    while (1) {
        ClientArg *ca = malloc(sizeof(ClientArg));
        socklen_t alen = sizeof(ca->addr);
        ca->fd = accept(srv_fd, (struct sockaddr *)&ca->addr, &alen);
        if (ca->fd < 0) { free(ca); continue; }
        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, ca);
        pthread_detach(tid);
    }
    close(srv_fd);
    return 0;
}
