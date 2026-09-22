/* Self-Question 1 (Page 17): Echo client for TCP Echo Server */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5000
#define BUF 1024

int main(void) {
    int sock_fd;
    struct sockaddr_in srv_addr;
    char msg[BUF], buf[BUF];

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) { perror("socket"); exit(1); }

    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &srv_addr.sin_addr);

    if (connect(sock_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0) {
        perror("connect"); exit(1);
    }
    printf("Connected to Server.\n");

    while (1) {
        printf("Enter a message: ");
        fgets(msg, BUF, stdin);
        msg[strcspn(msg, "\n")] = '\0';

        send(sock_fd, msg, strlen(msg), 0);
        if (strcmp(msg, "exit") == 0) {
            printf("Closing connection.\n");
            break;
        }

        int n = recv(sock_fd, buf, BUF - 1, 0);
        if (n <= 0) { printf("Server closed connection.\n"); break; }
        buf[n] = '\0';
        printf("Echo from Server: %s\n", buf);
    }
    close(sock_fd);
    return 0;
}
