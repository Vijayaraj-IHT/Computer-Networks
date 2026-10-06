/*
 * sw_array_client.c
 *
 * Stop-and-Wait ARQ -- SENDER (array-queue version)
 *
 *     gcc sw_array_client.c -o sw_array_client
 *     ./sw_array_client <port>
 *
 * Frames sit in a circular array queue. One frame is dequeued, sent, and
 * the sender then BLOCKS on select() until either its ACK arrives or a
 * TIMEOUT fires. Only after a matching ACK is the next frame dequeued, so
 * at most one frame is ever outstanding.
 *
 * Loss simulation: the user may mark exactly one frame as "lost". On that
 * frame's FIRST transmission attempt the send() is skipped entirely (the
 * server never sees it, so the timeout that follows is a genuine timeout).
 * The retransmission that the timeout triggers goes through for real.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSZ     64          /* payload size of one frame            */
#define MAXQ      32          /* capacity of the array queue          */
#define EOT_SEQ   (-1)        /* sequence number of the "all done" msg */
#define TIMEOUT   2           /* ACK waiting time in seconds          */

struct Frame
{
    int  seq;
    char data[BUFSZ];
};

/* ---------------- array based circular FIFO queue ---------------- */

struct Queue
{
    struct Frame q[MAXQ];
    int front;
    int rear;
    int cnt;
};

void initq(struct Queue *qu)
{
    qu->front = 0;
    qu->rear  = -1;
    qu->cnt   = 0;
}

int isEmpty(struct Queue *qu) { return qu->cnt == 0; }
int isFull(struct Queue *qu)  { return qu->cnt == MAXQ; }

int enqueue(struct Queue *qu, struct Frame fr)
{
    if (isFull(qu))
    {
        printf("*** Queue overflow : frame %d cannot be inserted.\n", fr.seq);
        return 0;
    }
    qu->rear = (qu->rear + 1) % MAXQ;
    qu->q[qu->rear] = fr;
    qu->cnt++;
    return 1;
}

int dequeue(struct Queue *qu, struct Frame *fr)
{
    if (isEmpty(qu))
    {
        printf("*** Queue underflow : nothing to dequeue.\n");
        return 0;
    }
    *fr = qu->q[qu->front];
    qu->front = (qu->front + 1) % MAXQ;
    qu->cnt--;
    return 1;
}

void display(struct Queue *qu)
{
    int i, k;

    printf("   Queue : ");
    if (isEmpty(qu)) { printf("(empty)\n"); return; }
    for (i = 0; i < qu->cnt; i++)
    {
        k = (qu->front + i) % MAXQ;
        printf("[F%d] ", qu->q[k].seq);
    }
    printf("   (%d frame(s) waiting)\n", qu->cnt);
}

/* ---------------------------- socket ----------------------------- */

void die(char *msg) { perror(msg); exit(1); }

int get_port(int argc, char *argv[])
{
    int p;
    if (argc != 2)
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
    p = atoi(argv[1]);
    if (p <= 0 || p > 65535)
    {
        printf("Invalid port : %s\n", argv[1]);
        exit(1);
    }
    return p;
}

int connect_server(int port)
{
    int sd;
    struct sockaddr_in addr;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) die("socket() failed");

    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        die("connect() failed - is the server running?");

    return sd;
}

/* --------------------------- protocol ---------------------------- */

/*
 * Send one frame and wait for its ACK, retransmitting on timeout.
 * `lost` is 1 only for the frame chosen for simulated loss; `drop`
 * starts equal to `lost` and is cleared after its first use, so the
 * send is skipped exactly once and the retransmission after the
 * resulting timeout actually reaches the server.
 */
void stopwait(int sd, struct Frame fr, int lost)
{
    struct Frame ack;
    fd_set rset;
    struct timeval tv;
    int rc, tries = 1;
    int drop = lost;

    while (1)
    {
        if (drop)
        {
            drop = 0;
            printf("Sending Frame %d ... *** FRAME %d LOST ***\n", fr.seq, fr.seq);
        }
        else
        {
            printf("Sending Frame %d (data \"%s\") ...\n", fr.seq, fr.data);
            if (send(sd, &fr, sizeof(fr), 0) != (int)sizeof(fr))
                die("send() failed");
        }

        FD_ZERO(&rset);
        FD_SET(sd, &rset);
        tv.tv_sec  = TIMEOUT;
        tv.tv_usec = 0;

        rc = select(sd + 1, &rset, NULL, NULL, &tv);
        if (rc < 0) die("select() failed");

        if (rc == 0)
        {
            printf("----------------------------------------\n");
            printf("TIMEOUT : no ACK for Frame %d\n", fr.seq);
            printf("----------------------------------------\n");
            printf("Retransmitting Frame %d (attempt %d)\n", fr.seq, ++tries);
            continue;                     /* same frame goes again */
        }

        if (recv(sd, &ack, sizeof(ack), MSG_WAITALL) != (int)sizeof(ack))
            die("recv() failed");

        if (ack.seq != fr.seq)
        {
            printf("Unexpected ACK %d for Frame %d, retransmitting.\n", ack.seq, fr.seq);
            continue;
        }

        printf("ACK %d received -> Frame %d delivered.\n\n", ack.seq, fr.seq);
        return;                           /* move to next frame    */
    }
}

void transmit(int sd, struct Queue *qu, int lost)
{
    struct Frame fr, eot;

    printf("Sending frames one at a time, waiting for each ACK :\n\n");

    while (!isEmpty(qu))
    {
        display(qu);
        if (!dequeue(qu, &fr)) break;
        stopwait(sd, fr, (fr.seq == lost));
    }

    eot.seq = EOT_SEQ;
    strcpy(eot.data, "END-OF-TRANSMISSION");
    send(sd, &eot, sizeof(eot), 0);

    if (recv(sd, &eot, sizeof(eot), MSG_WAITALL) != (int)sizeof(eot))
        die("recv() failed while waiting for final ACK");

    printf("Server reply : %s\n", eot.data);
}

/* ------------------------------ input ---------------------------- */

void build_queue(struct Queue *qu, int n)
{
    struct Frame fr;
    char line[BUFSZ];
    int i;

    for (i = 0; i < n; i++)
    {
        printf("Data for Frame %d (blank = auto) : ", i);
        fflush(stdout);
        if (fgets(line, sizeof(line), stdin) == NULL) line[0] = '\0';
        line[strcspn(line, "\n")] = '\0';

        fr.seq = i;
        if (line[0] == '\0') sprintf(fr.data, "DATA-%d", i);
        else { strncpy(fr.data, line, BUFSZ - 1); fr.data[BUFSZ - 1] = '\0'; }

        if (!enqueue(qu, fr))
        {
            printf("Only %d frames could be queued.\n", i);
            break;
        }
    }
}

void run(int argc, char *argv[])
{
    int sd, port, n, lost = -1;
    char line[64];
    struct Queue qu;

    port = get_port(argc, argv);
    sd   = connect_server(port);

    printf("========================================\n");
    printf("   STOP-AND-WAIT ARQ  (array queue)\n");
    printf("        SENDER / CLIENT SIDE\n");
    printf("========================================\n");

    printf("Number of frames to send : ");
    fflush(stdout);
    if (fgets(line, sizeof(line), stdin) == NULL) line[0] = '\0';
    n = atoi(line);
    if (n <= 0 || n > MAXQ)
    {
        printf("Invalid frame count (1 - %d).\n", MAXQ);
        close(sd);
        return;
    }

    initq(&qu);
    build_queue(&qu, n);

    printf("\nFrame to simulate as lost (0..%d, -1 = none) : ", n - 1);
    fflush(stdout);
    if (fgets(line, sizeof(line), stdin) == NULL) line[0] = '\0';
    lost = atoi(line);
    if (lost < 0 || lost >= n) lost = -1;

    printf("\nQueue built using ARRAY :\n");
    display(&qu);
    printf("\n");

    transmit(sd, &qu, lost);

    printf("----------------------------------------\n");
    printf("Stop-and-Wait transmission complete.\n");
    close(sd);
}

int main(int argc, char *argv[])
{
    run(argc, argv);
    return 0;
}
