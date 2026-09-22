/* Self-Question 4 (Page 26): Concurrent TCP Chat Notification Server.
   Multiple clients connect simultaneously; a message from one client is
   broadcast to all other connected clients. Detects disconnects and removes
   inactive clients. Compile: gcc q4_chat_broadcast_server.c -o chat_server -lpthread */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 7004
#define BUF 1024
#define MAXCLIENTS 50

typedef struct { int fd; struct sockaddr_in addr; char name[32]; int active; } ClientEntry;

ClientEntry clients[MAXCLIENTS];
int nclients = 0;
pthread_mutex_t clients_lock = PTHREAD_MUTEX_INITIALIZER;

void broadcast(const char *msg, int sender_fd) {
    pthread_mutex_lock(&clients_lock);
    for (int i = 0; i < nclients; i++) {
        if (clients[i].active && clients[i].fd != sender_fd)
            send(clients[i].fd, msg, strlen(msg), 0);
    }
    pthread_mutex_unlock(&clients_lock);
}

void remove_client(int fd) {
    pthread_mutex_lock(&clients_lock);
    for (int i = 0; i < nclients; i++) {
        if (clients[i].fd == fd) {
            clients[i].active = 0;
            printf("Removed inactive client: %s\n", clients[i].name);
            break;
        }
    }
    pthread_mutex_unlock(&clients_lock);
}

void *client_thread(void *arg) {
    int fd = *(int *)arg;
    free(arg);
    char name[32], buf[BUF], out[BUF + 64];

    send(fd, "Enter your name: ", 18, 0);
    int n = recv(fd, name, sizeof(name) - 1, 0);
    if (n <= 0) { close(fd); return NULL; }
    name[n] = '\0';

    pthread_mutex_lock(&clients_lock);
    clients[nclients].fd = fd;
    strcpy(clients[nclients].name, name);
    clients[nclients].active = 1;
    nclients++;
    pthread_mutex_unlock(&clients_lock);

    snprintf(out, sizeof(out), "*** %s joined the chat ***\n", name);
    printf("%s", out);
    broadcast(out, fd);

    while ((n = recv(fd, buf, BUF - 1, 0)) > 0) {
        buf[n] = '\0';
        if (strcmp(buf, "exit") == 0) break;
        snprintf(out, sizeof(out), "%s: %s\n", name, buf);
        printf("Broadcast: %s", out);
        broadcast(out, fd);
    }

    remove_client(fd);
    close(fd);
    snprintf(out, sizeof(out), "*** %s left the chat ***\n", name);
    printf("%s", out);
    broadcast(out, fd);
    return NULL;
}

int main(void) {
    int srv_fd; struct sockaddr_in srv_addr;
    srv_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1; setsockopt(srv_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    srv_addr.sin_family = AF_INET; srv_addr.sin_addr.s_addr = INADDR_ANY; srv_addr.sin_port = htons(PORT);
    bind(srv_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    listen(srv_fd, MAXCLIENTS);
    printf("Concurrent Chat Notification Server on port %d...\n", PORT);

    while (1) {
        struct sockaddr_in cli_addr;
        socklen_t alen = sizeof(cli_addr);
        int *cfd = malloc(sizeof(int));
        *cfd = accept(srv_fd, (struct sockaddr *)&cli_addr, &alen);
        if (*cfd < 0) { free(cfd); continue; }
        printf("New connection: %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, cfd);
        pthread_detach(tid);
    }
    close(srv_fd);
    return 0;
}
