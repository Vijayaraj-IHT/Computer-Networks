/* Self-Question 2 (Page 17): Palindrome checking server for a TCP client in C */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5001
#define BUF 1024

int is_palindrome(const char *s) {
    int len = strlen(s);
    for (int i = 0, j = len - 1; i < j; i++, j--)
        if (s[i] != s[j]) return 0;
    return 1;
}

int main(void) {
    int srv_fd, cli_fd;
    struct sockaddr_in srv_addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    char buf[BUF], result[BUF];

    srv_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(srv_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = INADDR_ANY;
    srv_addr.sin_port = htons(PORT);

    bind(srv_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    listen(srv_fd, 5);
    printf("Palindrome Server listening on port %d...\n", PORT);

    while (1) {
        cli_fd = accept(srv_fd, (struct sockaddr *)&cli_addr, &cli_len);
        printf("Client connected: %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

        int n;
        while ((n = recv(cli_fd, buf, BUF - 1, 0)) > 0) {
            buf[n] = '\0';
            if (strcmp(buf, "exit") == 0) { printf("Client exited.\n"); break; }
            printf("Received string: %s\n", buf);
            if (is_palindrome(buf))
                snprintf(result, BUF, "\"%s\" IS a palindrome", buf);
            else
                snprintf(result, BUF, "\"%s\" is NOT a palindrome", buf);
            send(cli_fd, result, strlen(result), 0);
            printf("Result sent: %s\n", result);
        }
        close(cli_fd);
        printf("Client disconnected.\n");
    }
    close(srv_fd);
    return 0;
}
