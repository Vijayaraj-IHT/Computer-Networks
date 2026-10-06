/*
 * sr_server.c
 *
 * Selective Repeat ARQ -- RECEIVER
 *
 *     gcc sr_server.c -o sr_server
 *     ./sr_server <port> [window-size]
 *
 * Unlike Go-Back-N, an out-of-order frame that falls inside the receive
 * window is BUFFERED (not discarded) and acknowledged individually. Once
 * the frame the receiver was actually waiting for (`expected`) finally
 * arrives, it drains every consecutive buffered frame that follows it to
 * the "upper layer" in one go.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSZ     64
#define MAXSEQ    256
#define EOT_SEQ   (-1)

struct Frame
{
    int  seq;
    char data[BUFSZ];
};

void die(char *msg) { perror(msg); exit(1); }

int get_port(int argc, char *argv[])
{
    int p;
    if (argc < 2)
    {
        printf("Usage : %s <port> [window-size]\n", argv[0]);
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

int get_window(int argc, char *argv[])
{
    int w = 4;
    if (argc >= 3) w = atoi(argv[2]);
    if (w <= 0 || w >= MAXSEQ)
    {
        printf("Window size must be between 1 and %d.\n", MAXSEQ - 1);
        exit(1);
    }
    return w;
}

int get_socket(int port)
{
    int sd, on = 1;
    struct sockaddr_in addr;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) die("socket() failed");

    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
        die("setsockopt() failed");

    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0) die("bind() failed");
    if (listen(sd, 5) < 0) die("listen() failed");

    return sd;
}

/* Modular "is seq within w slots forward of expected" test. */
int in_window(int seq, int expected, int w)
{
    int d = (seq - expected + MAXSEQ) % MAXSEQ;
    return d < w;
}

void send_ack(int cd, int seq, char *what)
{
    struct Frame ack;
    ack.seq = seq;
    sprintf(ack.data, "%s", what);
    if (send(cd, &ack, sizeof(ack), 0) < 0)
        printf("send() failed : %s\n", strerror(errno));
}

void show_buffer(int got[], int expected, int w)
{
    int i, s;
    printf("Receiver buffer : ");
    for (i = 0; i < w; i++)
    {
        s = (expected + i) % MAXSEQ;
        if (got[s])            printf("F%d OK   ", s);
        else if (s == expected) printf("F%d ?    ", s);
        else                     printf("F%d --   ", s);
    }
    printf("\n");
}

void summary(char delivered[][64], int count)
{
    int i;
    printf("----------------------------------------\n");
    printf("Frames delivered in order (%d) :\n", count);
    for (i = 0; i < count; i++)
        printf("   [%d] %s\n", i, delivered[i]);
    printf("----------------------------------------\n");
}

/* Returns 1 on a clean end of transmission, 0 when the link closes. */
int handle(int cd, int w)
{
    struct Frame fr;
    char delivered[BUFSZ][64];
    char store[MAXSEQ][64];
    int got[MAXSEQ];
    int rc, expected = 0, count = 0;

    memset(got, 0, sizeof(got));

    printf("========================================\n");
    printf("       SELECTIVE REPEAT  PROTOCOL\n");
    printf("        RECEIVER / SERVER SIDE\n");
    printf("========================================\n");
    printf("\nReceive window size : %d\n", w);
    printf("Expected sequence   : %d\n\n", expected);

    while (1)
    {
        rc = recv(cd, &fr, sizeof(fr), MSG_WAITALL);
        if (rc < 0) { printf("recv() failed : %s\n", strerror(errno)); return 0; }
        if (rc == 0) { printf("\nSender closed the connection.\n"); return 0; }
        if (rc != (int)sizeof(fr))
        {
            printf("Incomplete frame received (%d bytes).\n", rc);
            continue;
        }

        if (fr.seq == EOT_SEQ)
        {
            send_ack(cd, EOT_SEQ, "END-CONFIRMED");
            printf("\nReceived end of transmission.\n");
            summary(delivered, count);
            return 1;
        }

        printf("Frame arrived : seq = %d  data = \"%s\"\n", fr.seq, fr.data);

        if (got[fr.seq])
        {
            /* already buffered: ACK it again, do not deliver twice */
            printf("Frame %d is a duplicate -> ACK %d sent again.\n\n", fr.seq, fr.seq);
            send_ack(cd, fr.seq, "ACK-DUP");
            continue;
        }

        if (fr.seq == expected)
        {
            /* the frame we were waiting for: store, then drain the run */
            got[fr.seq] = 1;
            strncpy(store[fr.seq], fr.data, 63); store[fr.seq][63] = '\0';
            printf("Frame %d is the expected frame -> stored.\n", fr.seq);

            while (got[expected])
            {
                strncpy(delivered[count], store[expected], 63);
                delivered[count][63] = '\0';
                printf("Delivering Frame %d to the upper layer.\n", expected);
                count++;
                got[expected] = 0;
                expected = (expected + 1) % MAXSEQ;
            }
            show_buffer(got, expected, w);
            printf("Sending ACK %d (next expected frame is %d).\n\n", fr.seq, expected);
            send_ack(cd, fr.seq, "ACK");
        }
        else if (in_window(fr.seq, expected, w))
        {
            /* out of order, inside window: BUFFER it (do not discard) */
            got[fr.seq] = 1;
            strncpy(store[fr.seq], fr.data, 63); store[fr.seq][63] = '\0';
            printf("*** Frame %d is OUT OF ORDER (waiting for %d).\n", fr.seq, expected);
            printf("*** Frame %d BUFFERED, not discarded (Selective Repeat).\n", fr.seq);
            show_buffer(got, expected, w);
            printf("Sending individual ACK %d.\n\n", fr.seq);
            send_ack(cd, fr.seq, "ACK");
        }
        else
        {
            printf("*** Frame %d is outside the receive window -> ignored.\n\n", fr.seq);
        }
    }
}

void run(int argc, char *argv[])
{
    int sd, cd, port, w;
    struct sockaddr_in cli;
    socklen_t len = sizeof(cli);

    port = get_port(argc, argv);
    w    = get_window(argc, argv);
    sd   = get_socket(port);

    printf("Server listening on port %d ...\n", port);

    while (1)
    {
        cd = accept(sd, (struct sockaddr *)&cli, &len);
        if (cd < 0) die("accept() failed");

        printf("Sender connected from %s:%d\n\n",
               inet_ntoa(cli.sin_addr), ntohs(cli.sin_port));

        if (handle(cd, w))
        {
            printf("----------------------------------------\n");
            printf("Session finished. Waiting for next sender.\n\n");
        }

        close(cd);
    }

    close(sd);
}

int main(int argc, char *argv[])
{
    run(argc, argv);
    return 0;
}
