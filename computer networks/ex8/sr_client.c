/*
 * sr_client.c
 *
 * Selective Repeat ARQ -- SENDER
 *
 *     gcc sr_client.c -o sr_client
 *     ./sr_client <port>
 *
 * A whole window of frames may be in flight at once. Every frame is
 * acknowledged INDIVIDUALLY (acked[] per sequence number), so a frame can
 * be confirmed while an earlier frame is still missing. The window slides
 * forward only over a CONSECUTIVE run of acknowledged frames starting at
 * `base`.
 *
 * On timeout, ONLY the single oldest unacknowledged frame (`base`) is
 * retransmitted -- never the rest of the window -- because later frames
 * may already be individually acknowledged and buffered by the receiver.
 *
 * Loss simulation: identical one-shot `drop` trick as the Stop-and-Wait
 * client -- the chosen frame's first send() is skipped so its timeout is
 * genuine, and the retransmission that follows actually goes through.
 */

#define _DEFAULT_SOURCE   /* usleep() needs POSIX prototypes hidden by -std=c99 */

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
#define MAXF      64          /* maximum number of frames             */
#define EOT_SEQ   (-1)        /* sequence number of the "all done" msg */
#define TIMEOUT   2           /* retransmission timer in seconds      */

struct Frame
{
    int  seq;
    char data[BUFSZ];
};

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

/* ------------------------- window display ------------------------ */

void show_window(int base, int w, int total, int acked[])
{
    int i;
    printf("Current Window : ");
    for (i = base; i < base + w && i < total; i++)
        printf(acked[i] ? "(%d) " : "[%d] ", i);
    for (i = base + w; i < total; i++)
        printf(" %d ", i);
    printf("   base = %d\n", base);
}

/* ------------------------- frame handling ------------------------ */

/* `drop` is cleared on first use, so only the chosen frame's very first
 * transmission attempt is withheld -- every later attempt sends for real. */
void send_frame(int sd, struct Frame *buf, int i, int lost, int *drop)
{
    struct Frame *fr = &buf[i];

    if (lost == i && *drop)
    {
        *drop = 0;
        printf("Sending Frame %d ... *** FRAME %d LOST ***\n", i, i);
        return;
    }

    printf("Sending Frame %d (data \"%s\") ...\n", i, fr->data);
    if (send(sd, fr, sizeof(*fr), 0) != (int)sizeof(*fr))
        die("send() failed");
}

/* Retransmit the single lowest unacknowledged frame -- never the rest. */
void retransmit_one(int sd, struct Frame *buf, int seq, int lost, int *drop)
{
    printf("----------------------------------------\n");
    printf("TIMEOUT : no ACK for Frame %d\n", seq);
    printf("Retransmitting Frame %d ONLY (Selective Repeat)\n", seq);
    printf("----------------------------------------\n");
    send_frame(sd, buf, seq, lost, drop);
}

/* --------------------------- protocol ---------------------------- */

void transmit(int sd, struct Frame *buf, int total, int w, int lost)
{
    struct Frame ack, eot;
    fd_set rset;
    struct timeval tv;
    int base = 0, next = 0, rc, drop = 1, i, n, moved;
    int acked[MAXF];

    memset(acked, 0, sizeof(acked));

    printf("Window Size  : %d\n", w);
    printf("Total Frames : %d\n\n", total);
    show_window(base, w, total, acked);
    printf("\n");

    while (base < total)
    {
        /* send as many new frames as the window currently allows */
        while (next < total && next < base + w)
        {
            send_frame(sd, buf, next, lost, &drop);
            next++;
            usleep(150000);
        }

        FD_ZERO(&rset);
        FD_SET(sd, &rset);
        tv.tv_sec  = TIMEOUT;
        tv.tv_usec = 0;

        rc = select(sd + 1, &rset, NULL, NULL, &tv);
        if (rc < 0) die("select() failed");

        if (rc == 0)
        {
            /* TIMEOUT: resend ONLY the oldest unacked frame (`base`) */
            retransmit_one(sd, buf, base, lost, &drop);
            continue;
        }

        n = recv(sd, &ack, sizeof(ack), MSG_WAITALL);
        if (n != (int)sizeof(ack))
        {
            printf("Connection lost while waiting for an ACK.\n");
            return;
        }

        if (ack.seq >= 0 && ack.seq < total)
        {
            if (acked[ack.seq])
                printf("ACK %d is a duplicate -> already acknowledged.\n", ack.seq);
            else
                printf("Individual ACK %d received.\n", ack.seq);
            acked[ack.seq] = 1;
        }

        /* slide base past every CONSECUTIVE acked frame */
        i = base;
        while (base < total && acked[base]) base++;
        moved = base - i;

        if (moved > 0)
        {
            printf("Window slid forward by %d to base = %d\n", moved, base);
            show_window(base, w, total, acked);
            printf("\n");
        }
    }

    eot.seq = EOT_SEQ;
    strcpy(eot.data, "END-OF-TRANSMISSION");
    send(sd, &eot, sizeof(eot), 0);

    if (recv(sd, &ack, sizeof(ack), MSG_WAITALL) != (int)sizeof(ack))
        die("recv() failed while waiting for final ACK");
    printf("Server reply : %s\n", ack.data);
}

/* ------------------------------ input ---------------------------- */

void build_frames(struct Frame *buf, int n)
{
    char line[BUFSZ];
    int i;

    for (i = 0; i < n; i++)
    {
        printf("Data for Frame %d (blank = auto) : ", i);
        fflush(stdout);
        if (fgets(line, sizeof(line), stdin) == NULL) line[0] = '\0';
        line[strcspn(line, "\n")] = '\0';

        buf[i].seq = i;
        if (line[0] == '\0') sprintf(buf[i].data, "DATA-%d", i);
        else { strncpy(buf[i].data, line, BUFSZ - 1); buf[i].data[BUFSZ - 1] = '\0'; }
    }
}

void run(int argc, char *argv[])
{
    int sd, port, n, w, lost = -1;
    char line[64];
    struct Frame buf[MAXF];

    port = get_port(argc, argv);
    sd   = connect_server(port);

    printf("========================================\n");
    printf("       SELECTIVE REPEAT  PROTOCOL\n");
    printf("        SENDER / CLIENT SIDE\n");
    printf("========================================\n");

    printf("Number of frames to send : ");
    fflush(stdout);
    if (fgets(line, sizeof(line), stdin) == NULL) line[0] = '\0';
    n = atoi(line);
    if (n <= 0 || n > MAXF)
    {
        printf("Invalid frame count (1 - %d).\n", MAXF);
        close(sd);
        return;
    }

    printf("Window size : ");
    fflush(stdout);
    if (fgets(line, sizeof(line), stdin) == NULL) line[0] = '\0';
    w = atoi(line);
    if (w <= 0 || w > n)
    {
        printf("Window size must be between 1 and %d.\n", n);
        close(sd);
        return;
    }

    build_frames(buf, n);

    printf("\nFrame to simulate as lost (0..%d, -1 = none) : ", n - 1);
    fflush(stdout);
    if (fgets(line, sizeof(line), stdin) == NULL) line[0] = '\0';
    lost = atoi(line);
    if (lost < 0 || lost >= n) lost = -1;

    printf("\n");
    transmit(sd, buf, n, w, lost);

    printf("----------------------------------------\n");
    printf("Selective Repeat transmission complete.\n");
    close(sd);
}

int main(int argc, char *argv[])
{
    run(argc, argv);
    return 0;
}
