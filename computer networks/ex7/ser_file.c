#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <stdint.h>
#include <errno.h>

#define BUF 1024
#define NAME 256

struct Hdr
{
    uint64_t size;
    uint32_t len;
};

unsigned long asum(unsigned long s,
                      char *b,
                      int n)
{
    for (int i = 0; i < n; i++)
        s += (unsigned char)b[i];

    return s;
}

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

int rall(int fd, void *buf, size_t n)
{
    size_t got = 0;

    while (got < n)
    {
        ssize_t x = recv(fd,
                         (char *)buf + got,
                         n - got,
                         0);

        if (x <= 0)
            return 0;

        got += x;
    }

    return 1;
}

void cnm(char *s)
{
    for (int i = 0; s[i] != '\0'; i++)
    {
        if (s[i] == '/' ||
            s[i] == '\\' ||
            s[i] == ':')
        {
            s[i] = '_';
        }
    }
}

void rfl(int fd)
{
    struct Hdr h;

    char name[NAME];
    char path[NAME + 20];
    char b[BUF];

    unsigned long sum = 0;
    unsigned long remote = 0;

    uint64_t left;
    uint64_t total = 0;

    if (!rall(fd, &h, sizeof(h)))
    {
        printf("Failed to receive file header.\n");
        close(fd);
        return;
    }

    if (h.len == 0 || h.len >= NAME)
    {
        printf("Invalid filename.\n");
        close(fd);
        return;
    }

    if (!rall(fd, name, h.len))
    {
        printf("Failed to receive filename.\n");
        close(fd);
        return;
    }

    name[h.len] = '\0';

    cnm(name);

    snprintf(path, sizeof(path),
             "received_%s", name);

    FILE *fp = fopen(path, "wb");

    if (fp == NULL)
    {
        perror("fopen");
        close(fd);
        return;
    }

    left = h.size;

    while (left > 0)
    {
        size_t want = left > BUF ? BUF : left;

        int n = recv(fd, b, want, 0);

        if (n <= 0)
        {
            printf("\nConnection interrupted.\n");
            fclose(fp);
            close(fd);
            return;
        }

        fwrite(b, 1, n, fp);

        sum = asum(sum, b, n);

        total += n;
        left -= n;

        printf("\rReceived: %llu / %llu bytes",
               (unsigned long long)total,
               (unsigned long long)h.size);

        fflush(stdout);
    }

    if (!rall(fd, &remote, sizeof(remote)))
    {
        printf("\nChecksum not received.\n");
        fclose(fp);
        close(fd);
        return;
    }

    fclose(fp);

    printf("\n\nTransfer complete.\n");
    printf("File     : %s\n", name);
    printf("Size     : %llu bytes\n",
           (unsigned long long)total);

    printf("Checksum : %lu\n", sum);
    printf("Remote   : %lu\n", remote);

    if (sum == remote)
        printf("Integrity: VERIFIED\n");
    else
        printf("Integrity: FAILED\n");

    printf("Saved as : %s\n", path);

    close(fd);
}

void hcli(int fd)
{
    rfl(fd);
}

void rsrv(int argc, char *argv[])
{
    int sf;
    int cf;

    struct sockaddr_in ca;
    socklen_t z;

    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        return;
    }

    sf = mks(gp(argv[1]));

    printf("File transfer server running on port %s...\n",
           argv[1]);

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

        printf("Client connected.\n");

        if (fork() == 0)
        {
            close(sf);

            hcli(cf);

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
