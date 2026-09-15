#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF 1024

int gp(char *s)
{
    int p = atoi(s);

    if (p <= 0 || p > 65535)
    {
        printf("Invalid port number.\n");
        exit(1);
    }

    return p;
}

int ipok(char *s)
{
    struct in_addr a;

    return inet_pton(AF_INET, s, &a) == 1;
}

int mkc(char *ip, int p)
{
    int fd;
    struct sockaddr_in sa;

    fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0)
    {
        perror("socket");
        exit(1);
    }

    memset(&sa, 0, sizeof(sa));

    sa.sin_family = AF_INET;
    sa.sin_port = htons(p);

    if (inet_pton(AF_INET, ip, &sa.sin_addr) <= 0)
    {
        printf("Invalid server IP address.\n");
        close(fd);
        exit(1);
    }

    if (connect(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0)
    {
        perror("connect");
        close(fd);
        exit(1);
    }

    return fd;
}

void lkup(int fd)
{
    char ip[BUF];
    char b[BUF];

    printf("Enter IP address: ");
    fflush(stdout);

    if (fgets(ip, sizeof(ip), stdin) == NULL)
        return;

    ip[strcspn(ip, "\r\n")] = '\0';

    if (!ipok(ip))
    {
        printf("Invalid IP address.\n");
        return;
    }

    if (send(fd, ip, strlen(ip) + 1, 0) < 0)
    {
        perror("send");
        return;
    }

    int n = recv(fd, b, BUF - 1, 0);

    if (n <= 0)
    {
        printf("Server closed the connection.\n");
        return;
    }

    b[n] = '\0';

    printf("\n===== ARP LOOKUP RESULT =====\n");
    printf("%s", b);
    printf("==============================\n");
}

void rcli(int argc, char *argv[])
{
    int fd;
    int p;

    if (argc != 3)
    {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        return;
    }

    p = gp(argv[2]);

    fd = mkc(argv[1], p);

    printf("Connected to ARP server.\n");

    lkup(fd);

    close(fd);
}

int main(int argc, char *argv[])
{
    rcli(argc, argv);
    return 0;
}
