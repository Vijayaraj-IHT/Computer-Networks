/* Self-Question 2 (Page 23): Iterative TCP server - Hospital Appointment Management System.
   Register, book, cancel, search doctor availability, view appointment history.
   DB: doctors.dat, appts.dat. Logs to hospital.log */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>

#define PORT 6002
#define BUF 2048
#define MAXD 30
#define MAXA 200

typedef struct { char id[10], name[40], dept[30]; int slots; } Doctor;
typedef struct { char aid[10], docid[10], patient[40], timing[20]; char status[10]; } Appt;

Doctor docs[MAXD]; int dcount = 0;
Appt appts[MAXA]; int acount = 0;

void log_it(const char *msg) {
    time_t now = time(NULL); char ts[64];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", localtime(&now));
    FILE *lf = fopen("hospital.log", "a"); fprintf(lf, "[%s] %s\n", ts, msg); fclose(lf);
}

void load(void) {
    FILE *fp = fopen("doctors.dat", "r");
    if (fp) { while (fscanf(fp, "%9s %39s %29s %d", docs[dcount].id, docs[dcount].name, docs[dcount].dept, &docs[dcount].slots) == 4) dcount++; fclose(fp); }
    if (dcount == 0) {
        strcpy(docs[0].id,"D1"); strcpy(docs[0].name,"Dr_Karthik"); strcpy(docs[0].dept,"Cardiology"); docs[0].slots=10;
        strcpy(docs[1].id,"D2"); strcpy(docs[1].name,"Dr_Priya"); strcpy(docs[1].dept,"Orthopedics"); docs[1].slots=8;
        dcount = 2;
    }
    fp = fopen("appts.dat", "r");
    if (fp) { while (fscanf(fp, "%9s %9s %39s %19s %9s", appts[acount].aid, appts[acount].docid, appts[acount].patient, appts[acount].timing, appts[acount].status) == 5) acount++; fclose(fp); }
}
void save_docs(void) { FILE *fp = fopen("doctors.dat", "w"); for (int i=0;i<dcount;i++) fprintf(fp,"%s %s %s %d\n",docs[i].id,docs[i].name,docs[i].dept,docs[i].slots); fclose(fp); }
void save_appts(void) { FILE *fp = fopen("appts.dat", "w"); for (int i=0;i<acount;i++) fprintf(fp,"%s %s %s %s %s\n",appts[i].aid,appts[i].docid,appts[i].patient,appts[i].timing,appts[i].status); fclose(fp); }

int find_doc(const char *id) { for (int i=0;i<dcount;i++) if (strcmp(docs[i].id,id)==0) return i; return -1; }
int find_appt(const char *aid) { for (int i=0;i<acount;i++) if (strcmp(appts[i].aid,aid)==0) return i; return -1; }

void serve_client(int fd) {
    char buf[BUF], resp[BUF]; int n;
    while ((n = recv(fd, buf, BUF - 1, 0)) > 0) {
        buf[n] = '\0';
        char cmd[20]; sscanf(buf, "%19s", cmd);
        resp[0] = '\0';

        if (strcmp(cmd,"DOCTORS")==0) {
            for (int i=0;i<dcount;i++) { char l[128]; sprintf(l,"%s %s %s slots=%d\n",docs[i].id,docs[i].name,docs[i].dept,docs[i].slots); strcat(resp,l); }
        } else if (strcmp(cmd,"AVAIL")==0) {
            char id[10]; sscanf(buf,"AVAIL %9s",id); int idx=find_doc(id);
            if (idx<0) sprintf(resp,"ERR: doctor not found"); else sprintf(resp,"%s has %d slots available",id,docs[idx].slots);
        } else if (strcmp(cmd,"BOOK")==0) {
            char id[10], patient[40], timing[20]; sscanf(buf,"BOOK %9s %39s %19s",id,patient,timing);
            int idx=find_doc(id);
            if (idx<0) sprintf(resp,"ERR: invalid doctor");
            else if (docs[idx].slots<=0) sprintf(resp,"ERR: no slots available");
            else {
                docs[idx].slots--;
                sprintf(appts[acount].aid,"A%04d",acount+1000);
                strcpy(appts[acount].docid,id); strcpy(appts[acount].patient,patient); strcpy(appts[acount].timing,timing);
                strcpy(appts[acount].status,"BOOKED"); acount++;
                save_docs(); save_appts();
                sprintf(resp,"Appointment booked. ID=%s with %s at %s",appts[acount-1].aid,id,timing);
                log_it(resp);
            }
        } else if (strcmp(cmd,"CANCEL")==0) {
            char aid[10]; sscanf(buf,"CANCEL %9s",aid); int idx=find_appt(aid);
            if (idx<0) sprintf(resp,"ERR: appointment not found");
            else {
                strcpy(appts[idx].status,"CANCELLED");
                int did=find_doc(appts[idx].docid); if (did>=0) docs[did].slots++;
                save_docs(); save_appts();
                sprintf(resp,"Appointment %s cancelled",aid); log_it(resp);
            }
        } else if (strcmp(cmd,"HISTORY")==0) {
            char patient[40]; sscanf(buf,"HISTORY %39s",patient);
            for (int i=0;i<acount;i++) if (strcmp(appts[i].patient,patient)==0) {
                char l[128]; sprintf(l,"%s Doc:%s Time:%s Status:%s\n",appts[i].aid,appts[i].docid,appts[i].timing,appts[i].status); strcat(resp,l);
            }
            if (resp[0]=='\0') strcpy(resp,"No appointment history found");
        } else if (strcmp(cmd,"EXIT")==0) { log_it("Client session ended"); break; }
        else strcpy(resp,"ERR: invalid command");

        send(fd, resp, strlen(resp), 0);
        printf("Processed: %s -> %s\n", cmd, resp);
    }
}

int main(void) {
    load();
    int srv_fd, cli_fd; struct sockaddr_in srv_addr, cli_addr; socklen_t cli_len=sizeof(cli_addr);
    srv_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt=1; setsockopt(srv_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    srv_addr.sin_family=AF_INET; srv_addr.sin_addr.s_addr=INADDR_ANY; srv_addr.sin_port=htons(PORT);
    bind(srv_fd,(struct sockaddr*)&srv_addr,sizeof(srv_addr));
    listen(srv_fd,5);
    printf("Iterative Hospital Appointment Server on port %d (%d doctors loaded)\n", PORT, dcount);

    while (1) {
        cli_fd = accept(srv_fd,(struct sockaddr*)&cli_addr,&cli_len);
        printf("Client connected: %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
        serve_client(cli_fd);
        close(cli_fd);
        printf("Client finished; ready for next client.\n");
    }
    close(srv_fd);
    return 0;
}
