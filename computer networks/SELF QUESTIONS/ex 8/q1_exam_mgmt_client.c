/* Self-Question 1 (Page 26): Client for concurrent Online Examination Server */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 7001
#define BUF 2048

int main(void) {
    int sock_fd; struct sockaddr_in srv_addr; char buf[BUF], line[BUF];
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    srv_addr.sin_family = AF_INET; srv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &srv_addr.sin_addr);
    connect(sock_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));

    printf("Username: "); fgets(line, sizeof(line), stdin); line[strcspn(line,"\n")]=0;
    send(sock_fd, line, strlen(line), 0);
    printf("Password: "); fgets(line, sizeof(line), stdin); line[strcspn(line,"\n")]=0;
    send(sock_fd, line, strlen(line), 0);

    int n = recv(sock_fd, buf, BUF - 1, 0); buf[n] = '\0';
    if (strcmp(buf, "AUTH_FAIL") == 0) { printf("Login failed.\n"); close(sock_fd); return 1; }
    printf("Login successful.\n");

    while (1) {
        printf("\n1.Get Questions 2.Answer 3.Submit 4.Exit\nChoice: ");
        fgets(line, sizeof(line), stdin);
        int c = atoi(line);
        if (c==1) strcpy(buf,"GETQ");
        else if (c==2) { int q,o; printf("QuestionNo OptionIndex(0-3): "); fgets(line,sizeof(line),stdin); sscanf(line,"%d %d",&q,&o); snprintf(buf,BUF,"ANSWER %d %d",q,o); }
        else if (c==3) strcpy(buf,"SUBMIT");
        else if (c==4) { strcpy(buf,"EXIT"); send(sock_fd,buf,strlen(buf),0); printf("Exiting.\n"); break; }
        else continue;

        send(sock_fd, buf, strlen(buf), 0);
        n = recv(sock_fd, buf, BUF - 1, 0); buf[n] = '\0';
        printf("Server:\n%s\n", buf);
    }
    close(sock_fd);
    return 0;
}
