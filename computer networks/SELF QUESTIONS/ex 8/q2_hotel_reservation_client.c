/* Self-Question 2 (Page 26): Client for concurrent Hotel Reservation Server */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 7002
#define BUF 2048

int main(void) {
    int sock_fd; struct sockaddr_in srv_addr; char buf[BUF], line[BUF];
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    srv_addr.sin_family = AF_INET; srv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &srv_addr.sin_addr);
    connect(sock_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    printf("Connected to Hotel Reservation Server.\n");

    while (1) {
        printf("\n1.View Availability 2.Book Room 3.Cancel 4.Room Details 5.Exit\nChoice: ");
        fgets(line, sizeof(line), stdin);
        int c = atoi(line);
        if (c==1) strcpy(buf,"AVAIL");
        else if (c==2) { int r; char g[40]; printf("RoomNo Guest: "); fgets(line,sizeof(line),stdin); sscanf(line,"%d %39s",&r,g); snprintf(buf,BUF,"BOOK %d %s",r,g); }
        else if (c==3) { int r; printf("RoomNo: "); fgets(line,sizeof(line),stdin); sscanf(line,"%d",&r); snprintf(buf,BUF,"CANCEL %d",r); }
        else if (c==4) { int r; printf("RoomNo: "); fgets(line,sizeof(line),stdin); sscanf(line,"%d",&r); snprintf(buf,BUF,"DETAILS %d",r); }
        else if (c==5) { strcpy(buf,"EXIT"); send(sock_fd,buf,strlen(buf),0); printf("Session closed.\n"); break; }
        else continue;

        send(sock_fd, buf, strlen(buf), 0);
        int n = recv(sock_fd, buf, BUF-1, 0); buf[n]='\0';
        printf("Server: %s\n", buf);
    }
    close(sock_fd);
    return 0;
}
