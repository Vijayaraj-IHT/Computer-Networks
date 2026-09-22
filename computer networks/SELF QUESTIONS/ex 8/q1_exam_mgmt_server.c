/* Self-Question 1 (Page 26): Concurrent TCP server - Online Examination Management System.
   Multithreaded: one thread per client. Auth (user/pass), retrieve questions, submit
   answers, evaluate score. Mutex protects shared question bank / results file.
   Compile: gcc q1_exam_mgmt_server.c -o exam_server -lpthread */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 7001
#define BUF 2048
#define NQ 5

pthread_mutex_t db_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct { char q[100]; char opts[4][30]; int ans; } Question;
Question bank[NQ] = {
    {"Which layer does TCP operate at?", {"Physical","Transport","Application","Network"}, 1},
    {"What does DNS stand for?", {"Domain Name System","Data Node Service","Direct Net Sync","Dynamic Node Setup"}, 0},
    {"Which protocol is connectionless?", {"TCP","FTP","UDP","HTTP"}, 2},
    {"Default HTTP port?", {"21","23","80","443"}, 2},
    {"Which device operates at Data Link layer?", {"Router","Switch","Hub","Repeater"}, 1}
};

int authenticate(const char *u, const char *p) {
    return (strncmp(u, "student", 7) == 0 && strlen(p) >= 4);
}

void log_it(const char *ip, int port, const char *msg) {
    pthread_mutex_lock(&db_lock);
    time_t now = time(NULL); char ts[64];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", localtime(&now));
    FILE *lf = fopen("exam.log", "a");
    fprintf(lf, "[%s] %s:%d - %s\n", ts, ip, port, msg);
    fclose(lf);
    pthread_mutex_unlock(&db_lock);
}

typedef struct { int fd; struct sockaddr_in addr; } ClientArg;

void *client_thread(void *arg) {
    ClientArg *ca = (ClientArg *)arg;
    int fd = ca->fd;
    char *ip = inet_ntoa(ca->addr.sin_addr);
    int cport = ntohs(ca->addr.sin_port);
    char buf[BUF], resp[BUF];

    printf("[Thread %lu] Client connected: %s:%d\n", pthread_self(), ip, cport);

    char user[50], pass[50];
    int n = recv(fd, user, sizeof(user) - 1, 0); user[n] = '\0';
    n = recv(fd, pass, sizeof(pass) - 1, 0); pass[n] = '\0';

    if (!authenticate(user, pass)) {
        send(fd, "AUTH_FAIL", 9, 0);
        log_it(ip, cport, "Authentication FAILED");
        close(fd); free(ca);
        return NULL;
    }
    send(fd, "AUTH_OK", 7, 0);
    log_it(ip, cport, "Authenticated successfully");
    printf("[Thread %lu] %s authenticated.\n", pthread_self(), user);

    int answers[NQ]; for (int i = 0; i < NQ; i++) answers[i] = -1;

    while ((n = recv(fd, buf, BUF - 1, 0)) > 0) {
        buf[n] = '\0';
        char cmd[20]; sscanf(buf, "%19s", cmd);
        resp[0] = '\0';

        if (strcmp(cmd, "GETQ") == 0) {
            pthread_mutex_lock(&db_lock);
            for (int i = 0; i < NQ; i++) {
                char l[256];
                sprintf(l, "Q%d: %s A)%s B)%s C)%s D)%s\n", i+1, bank[i].q,
                    bank[i].opts[0], bank[i].opts[1], bank[i].opts[2], bank[i].opts[3]);
                strcat(resp, l);
            }
            pthread_mutex_unlock(&db_lock);
        } else if (strcmp(cmd, "ANSWER") == 0) {
            int qn, opt; sscanf(buf, "ANSWER %d %d", &qn, &opt);
            if (qn >= 1 && qn <= NQ) { answers[qn-1] = opt; sprintf(resp, "Recorded answer for Q%d", qn); }
            else sprintf(resp, "ERR: invalid question number");
        } else if (strcmp(cmd, "SUBMIT") == 0) {
            int score = 0;
            for (int i = 0; i < NQ; i++) if (answers[i] == bank[i].ans) score++;
            sprintf(resp, "Exam submitted. Score: %d/%d", score, NQ);
            char logmsg[128]; sprintf(logmsg, "%s submitted exam, score=%d/%d", user, score, NQ);
            log_it(ip, cport, logmsg);
        } else if (strcmp(cmd, "EXIT") == 0) {
            log_it(ip, cport, "Session terminated"); break;
        } else sprintf(resp, "ERR: invalid command");

        send(fd, resp, strlen(resp), 0);
        printf("[Thread %lu] %s -> %s\n", pthread_self(), cmd, resp);
    }
    close(fd);
    printf("[Thread %lu] Client %s:%d disconnected.\n", pthread_self(), ip, cport);
    free(ca);
    return NULL;
}

int main(void) {
    int srv_fd; struct sockaddr_in srv_addr;
    srv_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1; setsockopt(srv_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    srv_addr.sin_family = AF_INET; srv_addr.sin_addr.s_addr = INADDR_ANY; srv_addr.sin_port = htons(PORT);
    bind(srv_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    listen(srv_fd, 10);
    printf("Concurrent Online Examination Server on port %d...\n", PORT);

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
