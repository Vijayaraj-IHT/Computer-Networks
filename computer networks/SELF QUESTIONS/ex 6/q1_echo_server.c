/* Self-Question 1 (Page 17): Implement Echo server using TCP Socket in C */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5000
#define BUF 1024

int main(void) {
    int srv_fd, cli_fd;
    struct sockaddr_in srv_addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    char buf[BUF];

    srv_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (srv_fd < 0) { perror("socket"); exit(1); }

    int opt = 1;
    setsockopt(srv_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = INADDR_ANY;
    srv_addr.sin_port = htons(PORT);

    if (bind(srv_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0) {
        perror("bind"); exit(1);
    }
    if (listen(srv_fd, 5) < 0) { perror("listen"); exit(1); }

    printf("Echo Server waiting for client on port %d...\n", PORT);

    while (1) {
        cli_fd = accept(srv_fd, (struct sockaddr *)&cli_addr, &cli_len);
        if (cli_fd < 0) { perror("accept"); continue; }
        printf("Client connected: %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

        int n;
        while ((n = recv(cli_fd, buf, BUF - 1, 0)) > 0) {
            buf[n] = '\0';
            printf("Message from Client: %s\n", buf);
            if (strcmp(buf, "exit") == 0) {
                printf("Client requested exit.\n");
                break;
            }
            send(cli_fd, buf, n, 0);
            printf("Echo sent to Client: %s\n", buf);
        }
        printf("Client disconnected.\n");
        close(cli_fd);
    }
    close(srv_fd);
    return 0;
}
