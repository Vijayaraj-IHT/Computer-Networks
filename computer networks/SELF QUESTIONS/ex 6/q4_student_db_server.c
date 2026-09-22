/* Self-Question 4 (Page 17): TCP client-server Student Result Management System.
   Operations: search, add, update, delete, display, class topper.
   Records persisted in students.dat (text file). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>

#define PORT 5003
#define BUF 2048
#define DBFILE "students.dat"
#define MAXREC 200

typedef struct {
    char regno[20];
    char name[50];
    char dept[20];
    int sem;
    float marks[5];
    char grade;
    float cgpa;
} Student;

Student db[MAXREC];
int count = 0;

void log_txn(const char *action) {
    time_t now = time(NULL);
    char ts[64];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", localtime(&now));
    FILE *lf = fopen("txn.log", "a");
    fprintf(lf, "[%s] %s\n", ts, action);
    fclose(lf);
}

void load_db(void) {
    FILE *fp = fopen(DBFILE, "r");
    if (!fp) return;
    count = 0;
    while (fscanf(fp, "%19s %49s %19s %d %f %f %f %f %f %c %f",
        db[count].regno, db[count].name, db[count].dept, &db[count].sem,
        &db[count].marks[0], &db[count].marks[1], &db[count].marks[2],
        &db[count].marks[3], &db[count].marks[4], &db[count].grade,
        &db[count].cgpa) == 11) count++;
    fclose(fp);
}

void save_db(void) {
    FILE *fp = fopen(DBFILE, "w");
    for (int i = 0; i < count; i++)
        fprintf(fp, "%s %s %s %d %.2f %.2f %.2f %.2f %.2f %c %.2f\n",
            db[i].regno, db[i].name, db[i].dept, db[i].sem,
            db[i].marks[0], db[i].marks[1], db[i].marks[2], db[i].marks[3], db[i].marks[4],
            db[i].grade, db[i].cgpa);
    fclose(fp);
}

int find_index(const char *regno) {
    for (int i = 0; i < count; i++)
        if (strcmp(db[i].regno, regno) == 0) return i;
    return -1;
}

void fmt_record(char *out, Student *s) {
    sprintf(out, "RegNo:%s Name:%s Dept:%s Sem:%d Marks:[%.1f,%.1f,%.1f,%.1f,%.1f] Grade:%c CGPA:%.2f",
        s->regno, s->name, s->dept, s->sem, s->marks[0], s->marks[1], s->marks[2],
        s->marks[3], s->marks[4], s->grade, s->cgpa);
}

int main(void) {
    load_db();
    int srv_fd, cli_fd;
    struct sockaddr_in srv_addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    char buf[BUF], resp[BUF];

    srv_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(srv_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = INADDR_ANY;
    srv_addr.sin_port = htons(PORT);
    bind(srv_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    listen(srv_fd, 5);
    printf("Student Result Management Server on port %d... (%d records loaded)\n", PORT, count);

    while (1) {
        cli_fd = accept(srv_fd, (struct sockaddr *)&cli_addr, &cli_len);
        printf("Client connected: %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

        int n;
        while ((n = recv(cli_fd, buf, BUF - 1, 0)) > 0) {
            buf[n] = '\0';
            char cmd[20]; sscanf(buf, "%19s", cmd);
            printf("Request: %s\n", buf);
            resp[0] = '\0';

            if (strcmp(cmd, "ADD") == 0) {
                Student s;
                sscanf(buf, "ADD %19s %49s %19s %d %f %f %f %f %f",
                    s.regno, s.name, s.dept, &s.sem,
                    &s.marks[0], &s.marks[1], &s.marks[2], &s.marks[3], &s.marks[4]);
                float tot = s.marks[0]+s.marks[1]+s.marks[2]+s.marks[3]+s.marks[4];
                s.cgpa = tot / 50.0f;
                s.grade = s.cgpa >= 9 ? 'O' : s.cgpa >= 8 ? 'A' : s.cgpa >= 7 ? 'B' : s.cgpa >= 6 ? 'C' : 'F';
                db[count++] = s;
                save_db();
                sprintf(resp, "Added record %s (CGPA %.2f, Grade %c)", s.regno, s.cgpa, s.grade);
                log_txn(resp);
            } else if (strcmp(cmd, "SEARCH") == 0) {
                char regno[20]; sscanf(buf, "SEARCH %19s", regno);
                int idx = find_index(regno);
                if (idx < 0) sprintf(resp, "ERR: %s not found", regno);
                else fmt_record(resp, &db[idx]);
            } else if (strcmp(cmd, "UPDATE") == 0) {
                char regno[20]; int field; float val;
                sscanf(buf, "UPDATE %19s %d %f", regno, &field, &val);
                int idx = find_index(regno);
                if (idx < 0) sprintf(resp, "ERR: %s not found", regno);
                else {
                    db[idx].marks[field] = val;
                    float tot = 0; for (int k=0;k<5;k++) tot += db[idx].marks[k];
                    db[idx].cgpa = tot / 50.0f;
                    save_db();
                    sprintf(resp, "Updated %s mark[%d]=%.1f new CGPA=%.2f", regno, field, val, db[idx].cgpa);
                    log_txn(resp);
                }
            } else if (strcmp(cmd, "DELETE") == 0) {
                char regno[20]; sscanf(buf, "DELETE %19s", regno);
                int idx = find_index(regno);
                if (idx < 0) sprintf(resp, "ERR: %s not found", regno);
                else {
                    for (int k = idx; k < count - 1; k++) db[k] = db[k+1];
                    count--;
                    save_db();
                    sprintf(resp, "Deleted record %s", regno);
                    log_txn(resp);
                }
            } else if (strcmp(cmd, "DISPLAY") == 0) {
                resp[0] = '\0';
                for (int i = 0; i < count; i++) {
                    char line[BUF];
                    fmt_record(line, &db[i]);
                    strcat(resp, line);
                    strcat(resp, "\n");
                }
                if (count == 0) sprintf(resp, "No records found");
            } else if (strcmp(cmd, "TOPPER") == 0) {
                if (count == 0) sprintf(resp, "No records found");
                else {
                    int best = 0;
                    for (int i = 1; i < count; i++)
                        if (db[i].cgpa > db[best].cgpa) best = i;
                    sprintf(resp, "Class Topper -> ");
                    char line[BUF]; fmt_record(line, &db[best]);
                    strcat(resp, line);
                }
            } else if (strcmp(cmd, "EXIT") == 0) {
                log_txn("Client terminated session");
                break;
            } else {
                sprintf(resp, "ERR: Invalid command");
            }
            send(cli_fd, resp, strlen(resp), 0);
        }
        close(cli_fd);
        printf("Client disconnected.\n");
    }
    close(srv_fd);
    return 0;
}
