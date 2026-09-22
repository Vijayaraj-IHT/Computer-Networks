/* Self-Question 4 (Page 17): Client for Student Result Management System */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5003
#define BUF 2048

int main(void) {
    int sock_fd;
    struct sockaddr_in srv_addr;
    char buf[BUF], line[BUF];

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &srv_addr.sin_addr);
    connect(sock_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    printf("Connected to Student Result Management Server.\n");

    while (1) {
        printf("\n1.Add 2.Search 3.Update 4.Delete 5.Display 6.Class Topper 7.Exit\nChoice: ");
        fgets(line, sizeof(line), stdin);
        int choice = atoi(line);

        if (choice == 1) {
            char regno[20], name[50], dept[20]; int sem; float m[5];
            printf("RegNo Name Dept Sem Mark1 Mark2 Mark3 Mark4 Mark5: ");
            fgets(line, sizeof(line), stdin);
            sscanf(line, "%19s %49s %19s %d %f %f %f %f %f", regno, name, dept, &sem, &m[0],&m[1],&m[2],&m[3],&m[4]);
            snprintf(buf, BUF, "ADD %s %s %s %d %.1f %.1f %.1f %.1f %.1f", regno, name, dept, sem, m[0],m[1],m[2],m[3],m[4]);
        } else if (choice == 2) {
            printf("RegNo: "); fgets(line, sizeof(line), stdin); line[strcspn(line,"\n")]=0;
            snprintf(buf, BUF, "SEARCH %s", line);
        } else if (choice == 3) {
            char regno[20]; int field; float val;
            printf("RegNo MarkIndex(0-4) NewValue: ");
            fgets(line, sizeof(line), stdin);
            sscanf(line, "%19s %d %f", regno, &field, &val);
            snprintf(buf, BUF, "UPDATE %s %d %.1f", regno, field, val);
        } else if (choice == 4) {
            printf("RegNo: "); fgets(line, sizeof(line), stdin); line[strcspn(line,"\n")]=0;
            snprintf(buf, BUF, "DELETE %s", line);
        } else if (choice == 5) {
            snprintf(buf, BUF, "DISPLAY");
        } else if (choice == 6) {
            snprintf(buf, BUF, "TOPPER");
        } else if (choice == 7) {
            snprintf(buf, BUF, "EXIT");
            send(sock_fd, buf, strlen(buf), 0);
            printf("Session ended.\n");
            break;
        } else { continue; }

        send(sock_fd, buf, strlen(buf), 0);
        int n = recv(sock_fd, buf, BUF - 1, 0);
        buf[n] = '\0';
        printf("Server: %s\n", buf);
    }
    close(sock_fd);
    return 0;
}
