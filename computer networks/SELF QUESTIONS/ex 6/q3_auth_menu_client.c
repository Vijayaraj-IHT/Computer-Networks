/* Self-Question 3 (Page 17): Client for authenticated multi-service TCP server */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5002
#define BUF 4096

int main(void) {
    int sock_fd;
    struct sockaddr_in srv_addr;
    char buf[BUF], line[256];

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &srv_addr.sin_addr);
    connect(sock_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));

    recv(sock_fd, buf, BUF - 1, 0); /* USERNAME: */
    printf("Enter username: ");
    fgets(line, sizeof(line), stdin); line[strcspn(line, "\n")] = '\0';
    send(sock_fd, line, strlen(line), 0);

    recv(sock_fd, buf, BUF - 1, 0); /* PASSWORD: */
    printf("Enter password: ");
    fgets(line, sizeof(line), stdin); line[strcspn(line, "\n")] = '\0';
    send(sock_fd, line, strlen(line), 0);

    int n = recv(sock_fd, buf, BUF - 1, 0); buf[n] = '\0';
    if (strcmp(buf, "AUTH_FAIL") == 0) {
        printf("Authentication failed.\n");
        close(sock_fd);
        return 1;
    }
    printf("Login successful!\n");

    int choice;
    while (1) {
        printf("\n1. Upload file\n2. Download file\n3. Server date/time\n4. Server sysinfo\n5. Terminate\nChoice: ");
        fgets(line, sizeof(line), stdin);
        choice = atoi(line);
        snprintf(buf, BUF, "%d", choice);
        send(sock_fd, buf, strlen(buf), 0);

        if (choice == 1) {
            printf("Filename to upload: ");
            fgets(line, sizeof(line), stdin); line[strcspn(line, "\n")] = '\0';
            send(sock_fd, line, strlen(line), 0);
            recv(sock_fd, buf, BUF - 1, 0); /* READY */
            FILE *fp = fopen(line, "rb");
            if (!fp) { printf("Local file not found.\n"); continue; }
            size_t r;
            while ((r = fread(buf, 1, BUF, fp)) > 0) send(sock_fd, buf, r, 0);
            fclose(fp);
            send(sock_fd, "EOF", 3, 0);
            n = recv(sock_fd, buf, BUF - 1, 0); buf[n] = '\0';
            printf("%s\n", buf);
        } else if (choice == 2) {
            printf("Filename to download: ");
            fgets(line, sizeof(line), stdin); line[strcspn(line, "\n")] = '\0';
            send(sock_fd, line, strlen(line), 0);
            FILE *fp = fopen("downloaded_out", "wb");
            while ((n = recv(sock_fd, buf, BUF, 0)) > 0) {
                if (n == 3 && memcmp(buf, "EOF", 3) == 0) break;
                fwrite(buf, 1, n, fp);
            }
            fclose(fp);
            printf("Downloaded and saved as downloaded_out\n");
        } else if (choice == 3 || choice == 4) {
            n = recv(sock_fd, buf, BUF - 1, 0); buf[n] = '\0';
            printf("%s\n", buf);
        } else if (choice == 5) {
            printf("Session terminated.\n");
            break;
        } else {
            n = recv(sock_fd, buf, BUF - 1, 0); buf[n] = '\0';
            printf("%s\n", buf);
        }
    }
    close(sock_fd);
    return 0;
}
