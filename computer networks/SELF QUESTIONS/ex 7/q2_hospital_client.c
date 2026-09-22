/* Self-Question 2 (Page 23): Client for Hospital Appointment Management Server */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 6002
#define BUF 2048

int main(void) {
    int sock_fd; struct sockaddr_in srv_addr; char buf[BUF], line[BUF];
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    srv_addr.sin_family = AF_INET; srv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &srv_addr.sin_addr);
    connect(sock_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    printf("Connected to Hospital Appointment Server.\n");

    while (1) {
        printf("\n1.List Doctors 2.Check Availability 3.Book 4.Cancel 5.History 6.Exit\nChoice: ");
        fgets(line, sizeof(line), stdin);
        int c = atoi(line);
        if (c==1) strcpy(buf,"DOCTORS");
        else if (c==2) { printf("Doctor ID: "); fgets(line,sizeof(line),stdin); line[strcspn(line,"\n")]=0; snprintf(buf,BUF,"AVAIL %s",line); }
        else if (c==3) { char id[10],p[40],t[20]; printf("DoctorID Patient Timing: "); fgets(line,sizeof(line),stdin); sscanf(line,"%9s %39s %19s",id,p,t); snprintf(buf,BUF,"BOOK %s %s %s",id,p,t); }
        else if (c==4) { printf("Appointment ID: "); fgets(line,sizeof(line),stdin); line[strcspn(line,"\n")]=0; snprintf(buf,BUF,"CANCEL %s",line); }
        else if (c==5) { printf("Patient name: "); fgets(line,sizeof(line),stdin); line[strcspn(line,"\n")]=0; snprintf(buf,BUF,"HISTORY %s",line); }
        else if (c==6) { strcpy(buf,"EXIT"); send(sock_fd,buf,strlen(buf),0); printf("Session closed.\n"); break; }
        else continue;

        send(sock_fd, buf, strlen(buf), 0);
        int n = recv(sock_fd, buf, BUF-1, 0); buf[n]='\0';
        printf("Server:\n%s\n", buf);
    }
    close(sock_fd);
    return 0;
}
