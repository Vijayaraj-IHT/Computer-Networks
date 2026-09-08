#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUF 1024
#define TAB 10
#define MAX_AGE 5

struct Ent
{
    char ip[20];
    char mac[20];
    int used;
    int age;
};

struct Ent tbl[TAB];

pthread_mutex_t lck = PTHREAD_MUTEX_INITIALIZER;

int lookups = 0;
int hits = 0;
int misses = 0;

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

int hsh(char *ip)
{
    int v = 0;

    for (int i = 0; ip[i] != '\0'; i++)
        v += ip[i];

    return v % TAB;
}

void pent(char *ip, char *mac)
{
    int i = hsh(ip);
    int st = i;

    while (tbl[i].used)
    {
        if (strcmp(tbl[i].ip, ip) == 0)
        {
            strcpy(tbl[i].mac, mac);
            tbl[i].age = 0;
            return;
        }

        i = (i + 1) % TAB;

        if (i == st)
        {
            printf("ARP table is full.\n");
            return;
        }
    }

    strcpy(tbl[i].ip, ip);
    strcpy(tbl[i].mac, mac);

    tbl[i].used = 1;
    tbl[i].age = 0;
}

int gent(char *ip, char *mac)
{
    int i = hsh(ip);
    int st = i;

    while (tbl[i].used)
    {
        if (strcmp(tbl[i].ip, ip) == 0)
        {
            if (tbl[i].age >= MAX_AGE)
            {
                tbl[i].used = 0;
                return 0;
            }

            strcpy(mac, tbl[i].mac);
            tbl[i].age = 0;

            return 1;
        }

        i = (i + 1) % TAB;

        if (i == st)
            break;
    }

    return 0;
}

void mmac(char *ip, char *mac)
{
    int a, b, c, d;

    if (sscanf(ip, "%d.%d.%d.%d",
               &a, &b, &c, &d) == 4)
    {
        sprintf(mac,
                "AA:BB:%02X:%02X:%02X:%02X",
                a, b, c, d);
    }
    else
    {
        strcpy(mac, "00:00:00:00:00:00");
    }
}

void atbl()
{
    for (int i = 0; i < TAB; i++)
    {
        if (tbl[i].used)
            tbl[i].age++;
    }
}

void stbl()
{
    printf("\n========== SIMULATED ARP TABLE ==========\n");
    printf("%-5s %-18s %-20s %-5s\n",
           "Slot", "IP", "MAC", "Age");

    for (int i = 0; i < TAB; i++)
    {
        if (tbl[i].used)
        {
            printf("[%02d]  %-18s %-20s %-5d\n",
                   i,
                   tbl[i].ip,
                   tbl[i].mac,
                   tbl[i].age);
        }
        else
        {
            printf("[%02d]  %-18s %-20s %-5s\n",
                   i, "[Empty]", "None", "-");
        }
    }

    printf("==========================================\n");
}

void ssys()
{
    FILE *fp;
    char b[BUF];
    int found = 0;

    printf("\n========== SYSTEM ARP CACHE ==========\n");

    fp = popen("ip neigh", "r");

    if (fp == NULL)
    {
        perror("popen");
        return;
    }

    while (fgets(b, sizeof(b), fp))
    {
        printf("%s", b);
        found = 1;
    }

    if (!found)
        printf("(No entries returned by ip neigh)\n");

    pclose(fp);

    printf("=======================================\n");
}

void sstat()
{
    int used = 0;

    for (int i = 0; i < TAB; i++)
    {
        if (tbl[i].used)
            used++;
    }

    printf("\n========== ARP STATISTICS ==========\n");
    printf("Table entries : %d\n", used);
    printf("Free slots    : %d\n", TAB - used);
    printf("Lookups       : %d\n", lookups);
    printf("Hits          : %d\n", hits);
    printf("Misses        : %d\n", misses);
    printf("====================================\n");
}

void *hdl(void *arg)
{
    int fd = *(int *)arg;

    free(arg);

    char ip[BUF];
    char mac[20];
    char out[BUF];

    int n = recv(fd, ip, sizeof(ip) - 1, 0);

    if (n > 0)
    {
        ip[n] = '\0';
        ip[strcspn(ip, "\r\n")] = '\0';

        struct in_addr a;

        if (inet_pton(AF_INET, ip, &a) != 1)
        {
            strcpy(out, "Invalid IP address.\n");
            send(fd, out, strlen(out), 0);
        }
        else
        {
            pthread_mutex_lock(&lck);

            lookups++;

            atbl();

            if (gent(ip, mac))
            {
                hits++;

                sprintf(out,
                        "IP: %s\n"
                        "MAC: %s\n"
                        "Status: Entry found in ARP table.\n",
                        ip, mac);
            }
            else
            {
                misses++;

                mmac(ip, mac);
                pent(ip, mac);

                sprintf(out,
                        "IP: %s\n"
                        "MAC: %s\n"
                        "Status: New entry created.\n",
                        ip, mac);
            }

            stbl();
            sstat();

            pthread_mutex_unlock(&lck);

            send(fd, out, strlen(out), 0);
        }
    }

    close(fd);

    return NULL;
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

void sd()
{
    pent("192.168.1.1", "AA:BB:CC:DD:EE:01");
    pent("192.168.1.2", "AA:BB:CC:DD:EE:02");
    pent("192.168.1.3", "AA:BB:CC:DD:EE:03");
    pent("192.168.1.4", "AA:BB:CC:DD:EE:04");
}

void rsrv(int argc, char *argv[])
{
    int sf;
    int cf;
    int p;

    struct sockaddr_in ca;
    socklen_t z;

    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        return;
    }

    p = gp(argv[1]);

    sd();

    sf = mks(p);

    printf("ARP lookup server running on port %d...\n", p);

    stbl();
    ssys();

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

        int *fd = malloc(sizeof(int));

        if (fd == NULL)
        {
            perror("malloc");
            close(cf);
            continue;
        }

        *fd = cf;

        pthread_t th;

        if (pthread_create(&th, NULL, hdl, fd) != 0)
        {
            perror("pthread_create");
            free(fd);
            close(cf);
            continue;
        }

        pthread_detach(th);
    }

    close(sf);
}

int main(int argc, char *argv[])
{
    rsrv(argc, argv);
    return 0;
}
