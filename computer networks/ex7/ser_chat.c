#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
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

int mks(int p)
{
    int fd;
    int op = 1;
    struct sockaddr_in sa;

    fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0)
    {
        perror("socket");
        exit(1);
    }

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
               &op, sizeof(op));

    memset(&sa, 0, sizeof(sa));

    sa.sin_family = AF_INET;
    sa.sin_port = htons(p);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0)
    {
        perror("bind");
        close(fd);
        exit(1);
    }

    if (listen(fd, 5) < 0)
    {
        perror("listen");
        close(fd);
        exit(1);
    }

    return fd;
}

void gt(char *s)
{
    time_t t = time(NULL);
    struct tm *x = localtime(&t);

    sprintf(s, "%02d:%02d:%02d",
            x->tm_hour, x->tm_min, x->tm_sec);
}

void sinf(int fd, int no)
{
    char b[BUF];

    sprintf(b, "You are Client %d", no);

    send(fd, b, strlen(b), 0);
}

void shlp(int fd)
{
    char b[BUF];

    strcpy(b,
           "\nCommands:\n"
           "/help  - Show available commands\n"
           "/stats - Show message statistics\n"
           "/quit  - End the chat\n\n");

    send(fd, b, strlen(b), 0);
}

void hcht(int fd, int no)
{
    char b[BUF];
    char r[BUF];
    char t[32];

    int rx = 0;
    int tx = 0;

    sinf(fd, no);

    while (1)
    {
        int n = recv(fd, b, BUF - 1, 0);

        if (n <= 0)
            break;

        b[n] = '\0';
        b[strcspn(b, "\r\n")] = '\0';

        if (strcmp(b, "/help") == 0)
        {
            shlp(fd);
            continue;
        }

        if (strcmp(b, "/stats") == 0)
        {
            sprintf(r,
                    "\n===== CLIENT %d STATISTICS =====\n"
                    "Messages received : %d\n"
                    "Messages sent     : %d\n"
                    "=================================\n\n",
                    no, rx, tx);

            send(fd, r, strlen(r), 0);
            continue;
        }

        if (strcmp(b, "quit") == 0 ||
            strcmp(b, "/quit") == 0)
        {
            break;
        }

        rx++;

        printf("Client %d: %s\n", no, b);

        printf("You(Server): ");

        if (fgets(r, BUF, stdin) == NULL)
            strcpy(r, "/quit");

        r[strcspn(r, "\r\n")] = '\0';

        if (strcmp(r, "/help") == 0)
        {
            shlp(fd);
            continue;
        }

        gt(t);

        tx++;

        char out[BUF];

        snprintf(out, sizeof(out),
                 "[%s] Message #%d | You(Server): %s",
                 t, tx, r);

        if (send(fd, out, strlen(out), 0) < 0)
            break;

        if (strcmp(r, "quit") == 0 ||
            strcmp(r, "/quit") == 0)
        {
            break;
        }
    }

    printf("Client %d has left the chat.\n", no);

    close(fd);
}

void rsrv(int argc, char *argv[])
{
    int sf;
    int cf;
    int no = 0;

    struct sockaddr_in ca;
    socklen_t z;

    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        return;
    }

    int p = gp(argv[1]);

    sf = mks(p);

    printf("TCP Chat Server listening on port %d...\n", p);
    printf("Waiting for clients...\n\n");

    while (1)
    {
        z = sizeof(ca);

        cf = accept(sf,
                    (struct sockaddr *)&ca,
                    &z);

        if (cf < 0)
        {
            perror("accept");
            continue;
        }

        no++;

        printf("Client %d connected.\n", no);

        if (fork() == 0)
        {
            close(sf);

            hcht(cf, no);

            exit(0);
        }

        close(cf);

        waitpid(-1, NULL, WNOHANG);
    }

    close(sf);
}

int main(int argc, char *argv[])
{
    rsrv(argc, argv);
    return 0;
}
