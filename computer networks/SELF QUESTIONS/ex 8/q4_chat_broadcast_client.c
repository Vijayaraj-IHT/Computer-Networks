/* Self-Question 4 (Page 26): Client for Chat Notification Server.
   Uses a background thread to receive broadcasts while the main thread
   reads user input. Compile: gcc q4_chat_broadcast_client.c -o chat_client -lpthread */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 7004
#define BUF 1024

int sock_fd;

void *receiver(void *arg) {
    (void)arg;
    char buf[BUF];
    int n;
    while ((n = recv(sock_fd, buf, BUF - 1, 0)) > 0) {
        buf[n] = '\0';
        printf("%s", buf);
        fflush(stdout);
    }
    return NULL;
}

int main(void) {
    struct sockaddr_in srv_addr;
    char buf[BUF], name[32];

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    srv_addr.sin_family = AF_INET; srv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &srv_addr.sin_addr);
    connect(sock_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));

    recv(sock_fd, buf, BUF - 1, 0); /* "Enter your name: " */
    printf("Enter your name: ");
    fgets(name, sizeof(name), stdin); name[strcspn(name, "\n")] = '\0';
    send(sock_fd, name, strlen(name), 0);

    pthread_t tid;
    pthread_create(&tid, NULL, receiver, NULL);
    pthread_detach(tid);

    printf("Connected. Type messages ('exit' to quit).\n");
    while (1) {
        fgets(buf, BUF, stdin);
        buf[strcspn(buf, "\n")] = '\0';
        send(sock_fd, buf, strlen(buf), 0);
        if (strcmp(buf, "exit") == 0) break;
    }
    close(sock_fd);
    return 0;
}
