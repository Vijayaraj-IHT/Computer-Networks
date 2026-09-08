#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <libgen.h>
#include <sys/stat.h>
#include <stdint.h>

#define BUF 1024

struct Hdr
{
    uint64_t size;
    uint32_t len;
};

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

unsigned long asum(unsigned long s,
                      char *b,
                      int n)
{
    for (int i = 0; i < n; i++)
        s += (unsigned char)b[i];

    return s;
}

void sall(int fd, void *buf, size_t n)
{
    size_t sent = 0;

    while (sent < n)
    {
        ssize_t x = send(fd,
                         (char *)buf + sent,
                         n - sent,
                         0);

        if (x <= 0)
        {
            perror("send");
            exit(1);
        }

        sent += x;
    }
}

void sfl(int fd, char *path)
{
    FILE *fp;
    struct stat st;
    struct Hdr h;

    char b[BUF];
    char *name;

    unsigned long sum = 0;
    uint64_t total = 0;

    fp = fopen(path, "rb");

    if (fp == NULL)
    {
        perror("fopen");
        return;
    }

    if (stat(path, &st) < 0)
    {
        perror("stat");
        fclose(fp);
        return;
    }

    name = basename(path);

    h.size = st.st_size;
    h.len = strlen(name);

    if (h.len >= 256)
    {
        printf("Filename is too long.\n");
        fclose(fp);
        return;
    }

    sall(fd, &h, sizeof(h));
    sall(fd, name, h.len);

    int n;

    while ((n = fread(b, 1, BUF, fp)) > 0)
    {
        sall(fd, b, n);

        sum = asum(sum, b, n);
        total += n;

        printf("\rSent: %llu / %llu bytes",
               (unsigned long long)total,
               (unsigned long long)h.size);

        fflush(stdout);
    }

    fclose(fp);

    sall(fd, &sum, sizeof(sum));

    printf("\n\nTransfer complete.\n");
    printf("File     : %s\n", name);
    printf("Size     : %llu bytes\n",
           (unsigned long long)total);
    printf("Checksum : %lu\n", sum);
}

void rcli(int argc, char *argv[])
{
    char path[256];

    if (argc != 3)
    {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        return;
    }

    printf("Enter path of file to send: ");

    if (fgets(path, sizeof(path), stdin) == NULL)
        return;

    path[strcspn(path, "\r\n")] = '\0';

    int fd = mkc(argv[1], gp(argv[2]));

    sfl(fd, path);

    close(fd);
}

int main(int argc, char *argv[])
{
    rcli(argc, argv);
    return 0;
}
