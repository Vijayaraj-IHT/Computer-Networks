/* Self-Question 3 (Page 26): Client for concurrent File Sharing Server */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 7003
#define BUF 4096

int main(void) {
    int sock_fd; struct sockaddr_in srv_addr; char buf[BUF], line[BUF];
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    srv_addr.sin_family = AF_INET; srv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &srv_addr.sin_addr);
    connect(sock_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    printf("Connected to File Sharing Server.\n");

    while (1) {
        printf("\n1.List Files 2.Download File 3.Exit\nChoice: ");
        fgets(line, sizeof(line), stdin);
        int c = atoi(line);
        if (c == 1) {
            send(sock_fd, "LIST", 4, 0);
            int n = recv(sock_fd, buf, BUF - 1, 0); buf[n] = '\0';
            printf("Available files:\n%s\n", buf);
        } else if (c == 2) {
            printf("Filename: "); fgets(line, sizeof(line), stdin); line[strcspn(line, "\n")] = 0;
            snprintf(buf, BUF, "GET %s", line);
            send(sock_fd, buf, strlen(buf), 0);
            int n = recv(sock_fd, buf, BUF - 1, 0); buf[n] = '\0';
            if (strncmp(buf, "ERR", 3) == 0) { printf("%s\n", buf); continue; }
            FILE *fp = fopen("downloaded_out", "wb");
            while ((n = recv(sock_fd, buf, BUF, 0)) > 0) {
                if (n == 8 && memcmp(buf, "EOF_MARK", 8) == 0) break;
                fwrite(buf, 1, n, fp);
            }
            fclose(fp);
            printf("Downloaded as 'downloaded_out'\n");
        } else if (c == 3) {
            send(sock_fd, "EXIT", 4, 0);
            printf("Session closed.\n");
            break;
        }
    }
    close(sock_fd);
    return 0;
}
