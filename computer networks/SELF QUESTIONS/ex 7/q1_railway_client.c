/* Self-Question 1 (Page 23): Client for Iterative Railway Reservation Server */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 6001
#define BUF 2048

int main(void) {
    int sock_fd; struct sockaddr_in srv_addr; char buf[BUF], line[BUF];

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    srv_addr.sin_family = AF_INET; srv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &srv_addr.sin_addr);
    connect(sock_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    printf("Connected to Railway Reservation Server.\n");

    while (1) {
        printf("\n1.Search Trains 2.Check Availability 3.Book Ticket 4.Cancel 5.Booking Status 6.Exit\nChoice: ");
        fgets(line, sizeof(line), stdin);
        int c = atoi(line);
        if (c == 1) strcpy(buf, "SEARCH");
        else if (c == 2) { printf("Train ID: "); fgets(line,sizeof(line),stdin); line[strcspn(line,"\n")]=0; snprintf(buf,BUF,"AVAIL %s",line); }
        else if (c == 3) { char id[10],name[40]; int s; printf("TrainID Passenger Seats: "); fgets(line,sizeof(line),stdin); sscanf(line,"%9s %39s %d",id,name,&s); snprintf(buf,BUF,"BOOK %s %s %d",id,name,s); }
        else if (c == 4) { printf("PNR: "); fgets(line,sizeof(line),stdin); line[strcspn(line,"\n")]=0; snprintf(buf,BUF,"CANCEL %s",line); }
        else if (c == 5) { printf("PNR: "); fgets(line,sizeof(line),stdin); line[strcspn(line,"\n")]=0; snprintf(buf,BUF,"STATUS %s",line); }
        else if (c == 6) { strcpy(buf, "EXIT"); send(sock_fd, buf, strlen(buf), 0); printf("Session closed.\n"); break; }
        else continue;

        send(sock_fd, buf, strlen(buf), 0);
        int n = recv(sock_fd, buf, BUF - 1, 0);
        buf[n] = '\0';
        printf("Server:\n%s\n", buf);
    }
    close(sock_fd);
    return 0;
}
