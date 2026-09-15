#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

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

void gt(char *s)
{
    time_t t = time(NULL);
    struct tm *x = localtime(&t);

    sprintf(s, "%02d:%02d:%02d",
            x->tm_hour, x->tm_min, x->tm_sec);
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

void shlp()
{
    printf("\nCommands:\n");
    printf("/help  - Show available commands\n");
    printf("/stats - Show message statistics\n");
    printf("/quit  - End the cht\n\n");
}

void cht(int fd)
{
    char b[BUF];
    char t[32];
    int tx = 0;
    int rx = 0;

    int n = recv(fd, b, BUF - 1, 0);

    if (n <= 0)
    {
        printf("Server disconnected.\n");
        return;
    }

    b[n] = '\0';

    printf("\n%s\n\n", b);

    while (1)
    {
        printf("You: ");

        if (fgets(b, BUF, stdin) == NULL)
            strcpy(b, "/quit");

        b[strcspn(b, "\r\n")] = '\0';

        if (strcmp(b, "/help") == 0)
        {
            shlp();
            continue;
        }

        if (strcmp(b, "/stats") == 0)
        {
            printf("\n===== CHAT STATISTICS =====\n");
            printf("Messages sent     : %d\n", tx);
            printf("Messages received : %d\n", rx);
            printf("============================\n\n");
            continue;
        }

        if (send(fd, b, strlen(b), 0) < 0)
        {
            perror("send");
            break;
        }

        tx++;

        gt(t);

        printf("[%s] Message #%d sent\n", t, tx);

        if (strcmp(b, "quit") == 0 ||
            strcmp(b, "/quit") == 0)
        {
            printf("You ended the cht.\n");
            break;
        }

        n = recv(fd, b, BUF - 1, 0);

        if (n <= 0)
        {
            printf("Server disconnected.\n");
            break;
        }

        b[n] = '\0';
        rx++;

        printf("%s\n\n", b);

        if (strcmp(b, "quit") == 0 ||
            strcmp(b, "/quit") == 0)
        {
            printf("Server ended the cht.\n");
            break;
        }
    }
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

    printf("Connected to cht server at %s:%d\n",
           argv[1], p);

    printf("Type /help for commands.\n");

    cht(fd);

    close(fd);
}

int main(int argc, char *argv[])
{
    rcli(argc, argv);
    return 0;
}
