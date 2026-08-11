#include <stdio.h>
#include <string.h>

#define SYN 0x16
#define STX 0x02
#define ETX 0x03
#define DLE 0x10

#define MAXC 200
#define MAXF 512

int stuff(unsigned char *in, int len, unsigned char *out) {
    int i, o = 0;
    for (i = 0; i < len; i++) {
        unsigned char b = in[i];
        if (b == DLE) {
            printf("  [STUFF] byte %d is DLE (0x10) -> inserting extra DLE before it\n", i);
            out[o++] = DLE;
        }
        out[o++] = b;
    }
    return o;
}

int main() {
    char msg[MAXC];
    unsigned char pay[MAXC], stf[MAXF], frm[MAXF];
    int len, i, slen, flen;

    printf("=== BISYNC BYTE STUFFING SENDER (basic) ===\n\n");
    printf("Enter message to send: ");
    fgets(msg, MAXC, stdin);
    len = strlen(msg);
    if (len > 0 && msg[len - 1] == '\n') { msg[len - 1] = '\0'; len--; }
    if (len == 0) { printf("Empty message, aborting.\n"); return 1; }

    for (i = 0; i < len; i++) pay[i] = (unsigned char) msg[i];

    printf("\nStep 1: raw payload bytes\n");
    for (i = 0; i < len; i++) printf("  [%d] 0x%02X ('%c')\n", i, pay[i], msg[i]);

    printf("\nStep 2: DLE stuffing pass (double any literal DLE byte in data)\n");
    slen = stuff(pay, len, stf);
    printf("\nStuffed payload (%d bytes):\n", slen);
    for (i = 0; i < slen; i++) printf("  [%d] 0x%02X\n", i, stf[i]);

    flen = 0;
    frm[flen++] = SYN;
    frm[flen++] = SYN;
    frm[flen++] = STX;
    for (i = 0; i < slen; i++) frm[flen++] = stf[i];
    frm[flen++] = ETX;

    printf("\nStep 3: build frame  SYN SYN STX <data> ETX\n");
    printf("  SYN 0x%02X, SYN 0x%02X, STX 0x%02X ... ETX 0x%02X\n", SYN, SYN, STX, ETX);
    printf("Final BISYNC frame (%d bytes): ", flen);
    for (i = 0; i < flen; i++) printf("%02X ", frm[i]);
    printf("\n\n");

    FILE *ch = fopen("bisync_channel.txt", "w");
    if (!ch) { printf("ERROR: could not open bisync_channel.txt\n"); return 1; }
    for (i = 0; i < flen; i++) fprintf(ch, "%02X ", frm[i]);
    fprintf(ch, "\n");
    fclose(ch);

    printf("Frame sent through channel (bisync_channel.txt).\n");
    return 0;
}
