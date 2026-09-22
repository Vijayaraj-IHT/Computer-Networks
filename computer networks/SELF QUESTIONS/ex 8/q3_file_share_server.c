/* Self-Question 3 (Page 26): Concurrent TCP File Sharing Server.
   List available files; each client is served on its own thread and can
   download a selected file. Compile: gcc q3_file_share_server.c -o
   file_share_server -lpthread */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 7003
#define BUF 4096
#define SHAREDIR "./shared_files"

typedef struct { int fd; struct sockaddr_in addr; } ClientArg;

void list_files(char *resp) {
    DIR *d = opendir(SHAREDIR);
    resp[0] = '\0';
    if (!d) { strcpy(resp, "No shared_files directory found"); return; }
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (ent->d_name[0] == '.') continue;
        strcat(resp, ent->d_name);
        strcat(resp, "\n");
    }
    closedir(d);
    if (resp[0] == '\0') strcpy(resp, "No files available");
}

void *client_thread(void *arg) {
    ClientArg *ca = (ClientArg *)arg;
    int fd = ca->fd;
    char *ip = inet_ntoa(ca->addr.sin_addr);
    int cport = ntohs(ca->addr.sin_port);
    printf("[Thread %lu] Client connected: %s:%d\n", pthread_self(), ip, cport);

    char buf[BUF], resp[BUF];
    int n;
    while ((n = recv(fd, buf, BUF - 1, 0)) > 0) {
        buf[n] = '\0';
        char cmd[20]; sscanf(buf, "%19s", cmd);

        if (strcmp(cmd, "LIST") == 0) {
            list_files(resp);
            send(fd, resp, strlen(resp), 0);
            printf("[Thread %lu] Sent file list to %s:%d\n", pthread_self(), ip, cport);
        } else if (strcmp(cmd, "GET") == 0) {
            char fname[256], path[300];
            sscanf(buf, "GET %255s", fname);
            snprintf(path, sizeof(path), "%s/%s", SHAREDIR, fname);
            FILE *fp = fopen(path, "rb");
            if (!fp) { send(fd, "ERR:NOTFOUND", 12, 0); continue; }
            send(fd, "OK", 2, 0);
            usleep(50000); /* small pause so client's OK/EOF framing is clean */
            size_t r;
            while ((r = fread(buf, 1, BUF, fp)) > 0) send(fd, buf, r, 0);
            fclose(fp);
            send(fd, "EOF_MARK", 8, 0);
            printf("[Thread %lu] Sent file %s to %s:%d\n", pthread_self(), fname, ip, cport);
        } else if (strcmp(cmd, "EXIT") == 0) {
            break;
        } else {
            send(fd, "ERR: invalid command", 21, 0);
        }
    }
    close(fd);
    printf("[Thread %lu] Client %s:%d disconnected.\n", pthread_self(), ip, cport);
    free(ca);
    return NULL;
}

int main(void) {
    system("mkdir -p " SHAREDIR);
    int srv_fd; struct sockaddr_in srv_addr;
    srv_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1; setsockopt(srv_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    srv_addr.sin_family = AF_INET; srv_addr.sin_addr.s_addr = INADDR_ANY; srv_addr.sin_port = htons(PORT);
    bind(srv_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    listen(srv_fd, 10);
    printf("Concurrent File Sharing Server on port %d (dir: %s)\n", PORT, SHAREDIR);

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
