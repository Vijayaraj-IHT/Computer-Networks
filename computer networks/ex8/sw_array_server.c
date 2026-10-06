/*
 * sw_array_server.c
 *
 * Stop-and-Wait ARQ -- RECEIVER
 *
 *     gcc sw_array_server.c -o sw_array_server
 *     ./sw_array_server <port>
 *
 * Accepts one sender, then loops: receive a frame, print it, send back an
 * ACK carrying the SAME sequence number. Because the sender never advances
 * past an unacknowledged frame, the receiver never needs to deal with
 * out-of-order delivery -- it just echoes the seq number back as the ACK.
 * A frame with sequence number EOT_SEQ ends the session.
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

/* Receive frames one at a time and ACK each by its own seq number. */
int handle(int cd)
{
    struct Frame fr, ack;
    int rc, count = 0;

    printf("========================================\n");
    printf("   STOP-AND-WAIT ARQ  (array queue)\n");
    printf("        RECEIVER / SERVER SIDE\n");
    printf("========================================\n\n");

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
            ack.seq = EOT_SEQ;
            sprintf(ack.data, "ALL-%d-FRAMES-RECEIVED", count);
            send(cd, &ack, sizeof(ack), 0);
            printf("\nReceived end of transmission. Total frames : %d\n", count);
            return 1;
        }

        printf("Frame arrived : seq = %d  data = \"%s\"\n", fr.seq, fr.data);
        count++;

        ack.seq = fr.seq;
        sprintf(ack.data, "ACK-%d", fr.seq);
        printf("Sending ACK %d\n\n", fr.seq);
        if (send(cd, &ack, sizeof(ack), 0) < 0)
            printf("send() failed : %s\n", strerror(errno));
    }
}

void run(int argc, char *argv[])
{
    int sd, cd, port;
    struct sockaddr_in cli;
    socklen_t len = sizeof(cli);

    port = get_port(argc, argv);
    sd   = get_socket(port);

    printf("Server listening on port %d ...\n", port);

    while (1)
    {
        cd = accept(sd, (struct sockaddr *)&cli, &len);
        if (cd < 0) die("accept() failed");

        printf("Sender connected from %s:%d\n\n",
               inet_ntoa(cli.sin_addr), ntohs(cli.sin_port));

        if (handle(cd))
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
