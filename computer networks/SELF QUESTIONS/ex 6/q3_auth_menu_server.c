/* Self-Question 3 (Page 17): Authenticated multi-service TCP client-server.
   Menu: upload file, download file, server date/time, server system info, terminate.
   Logs client activity with timestamps to server.log */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/utsname.h>
#include <arpa/inet.h>

#define PORT 5002
#define BUF 4096
#define USER "admin"
#define PASS "admin123"
#define LOGFILE "server.log"

FILE *logfp;

void log_activity(const char *ip, int port, const char *action) {
    time_t now = time(NULL);
    char ts[64];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", localtime(&now));
    fprintf(logfp, "[%s] %s:%d - %s\n", ts, ip, port, action);
    fflush(logfp);
}

int authenticate(int fd) {
    char user[BUF], pass[BUF];
    int n;
    send(fd, "USERNAME:", 9, 0);
    n = recv(fd, user, BUF - 1, 0); user[n] = '\0';
    send(fd, "PASSWORD:", 9, 0);
    n = recv(fd, pass, BUF - 1, 0); pass[n] = '\0';
    return (strcmp(user, USER) == 0 && strcmp(pass, PASS) == 0);
}

void handle_upload(int fd) {
    char fname[256], buf[BUF];
    int n = recv(fd, fname, sizeof(fname) - 1, 0); fname[n] = '\0';
    FILE *fp = fopen(fname, "wb");
    if (!fp) { send(fd, "ERR: cannot open file\n", 23, 0); return; }
    send(fd, "READY", 5, 0);
    while ((n = recv(fd, buf, BUF, 0)) > 0) {
        if (n == 3 && memcmp(buf, "EOF", 3) == 0) break;
        fwrite(buf, 1, n, fp);
    }
    fclose(fp);
    send(fd, "Upload complete\n", 17, 0);
    printf("File uploaded: %s\n", fname);
}

void handle_download(int fd) {
    char fname[256], buf[BUF];
    int n = recv(fd, fname, sizeof(fname) - 1, 0); fname[n] = '\0';
    FILE *fp = fopen(fname, "rb");
    if (!fp) { send(fd, "ERR: file not found", 20, 0); return; }
    size_t r;
    while ((r = fread(buf, 1, BUF, fp)) > 0)
        send(fd, buf, r, 0);
    fclose(fp);
    send(fd, "EOF", 3, 0);
    printf("File sent: %s\n", fname);
}

void handle_datetime(int fd) {
    time_t now = time(NULL);
    char ts[128];
    strftime(ts, sizeof(ts), "Server Date/Time: %Y-%m-%d %H:%M:%S", localtime(&now));
    send(fd, ts, strlen(ts), 0);
}

void handle_sysinfo(int fd) {
    struct utsname u;
    uname(&u);
    char info[512];
    snprintf(info, sizeof(info), "System: %s | Node: %s | Release: %s | Machine: %s",
              u.sysname, u.nodename, u.release, u.machine);
    send(fd, info, strlen(info), 0);
}

int main(void) {
    logfp = fopen(LOGFILE, "a");
    int srv_fd, cli_fd;
    struct sockaddr_in srv_addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    char buf[BUF];

    srv_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(srv_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = INADDR_ANY;
    srv_addr.sin_port = htons(PORT);
    bind(srv_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    listen(srv_fd, 5);
    printf("Multi-Service Auth Server listening on port %d...\n", PORT);

    while (1) {
        cli_fd = accept(srv_fd, (struct sockaddr *)&cli_addr, &cli_len);
        char *ip = inet_ntoa(cli_addr.sin_addr);
        int cport = ntohs(cli_addr.sin_port);
        printf("Connection from %s:%d\n", ip, cport);

        if (!authenticate(cli_fd)) {
            send(cli_fd, "AUTH_FAIL", 9, 0);
            log_activity(ip, cport, "Authentication FAILED");
            close(cli_fd);
            continue;
        }
        send(cli_fd, "AUTH_OK", 7, 0);
        printf("Client authenticated. IP: %s Port: %d\n", ip, cport);
        log_activity(ip, cport, "Authenticated successfully");

        int n, choice;
        while ((n = recv(cli_fd, buf, BUF - 1, 0)) > 0) {
            buf[n] = '\0';
            choice = atoi(buf);
            switch (choice) {
                case 1: handle_upload(cli_fd); log_activity(ip, cport, "Uploaded file"); break;
                case 2: handle_download(cli_fd); log_activity(ip, cport, "Downloaded file"); break;
                case 3: handle_datetime(cli_fd); log_activity(ip, cport, "Requested date/time"); break;
                case 4: handle_sysinfo(cli_fd); log_activity(ip, cport, "Requested sysinfo"); break;
                case 5: log_activity(ip, cport, "Session terminated"); goto done;
                default: send(cli_fd, "Invalid choice", 15, 0);
            }
        }
    done:
        close(cli_fd);
        printf("Client %s:%d disconnected.\n", ip, cport);
    }
    fclose(logfp);
    close(srv_fd);
    return 0;
}
