#include <stdio.h>
#include <string.h>

#define FLAG 0x7E
#define ESC  0x7D
#define MAXC 200
#define MAXF 512

int stuff(unsigned char *in, int len, unsigned char *out) 
{
    int i, o = 0;
    for (i = 0; i < len; i++) {
        unsigned char b = in[i];
        if (b == FLAG || b == ESC) {
            printf("  [STUFF] byte %d (0x%02X) matches FLAG/ESC -> insert ESC 0x%02X then 0x%02X XOR 0x20 -> 0x%02X\n",
                   i, b, ESC, b, (unsigned char) (b ^ 0x20));
            out[o++] = ESC;
            out[o++] = b ^ 0x20;
        } else {
            out[o++] = b;
        }
    }
    return o;
}

int main() {
    char msg[MAXC];
    unsigned char pay[MAXC], stf[MAXF], frm[MAXF];
    int len, i, slen, flen;

    printf("=== PPP BYTE STUFFING SENDER (basic) ===\n\n");
    printf("Enter message to send: ");
    fgets(msg, MAXC, stdin);
    len = strlen(msg);
    if (len > 0 && msg[len - 1] == '\n') { msg[len - 1] = '\0'; len--; }
    if (len == 0) { printf("Empty message, aborting.\n"); return 1; }

    for (i = 0; i < len; i++) pay[i] = (unsigned char) msg[i];

    printf("\nStep 1: raw payload bytes\n");
    for (i = 0; i < len; i++) printf("  [%d] 0x%02X ('%c')\n", i, pay[i], msg[i]);

    printf("\nStep 2: byte stuffing pass (FLAG 0x7E, ESC 0x7D, XOR 0x20)\n");
    slen = stuff(pay, len, stf);
    printf("\nStuffed payload (%d bytes):\n", slen);
    for (i = 0; i < slen; i++) printf("  [%d] 0x%02X\n", i, stf[i]);

    flen = 0;
    frm[flen++] = FLAG;
    for (i = 0; i < slen; i++) frm[flen++] = stf[i];
    frm[flen++] = FLAG;

    printf("\nStep 3: add start/end FLAG 0x7E\n");
    printf("Final PPP frame (%d bytes): ", flen);
    for (i = 0; i < flen; i++) printf("%02X ", frm[i]);
    printf("\n\n");

    FILE *ch = fopen("ppp_channel.txt", "w");
    if (!ch) { printf("ERROR: could not open ppp_channel.txt\n"); return 1; }
    for (i = 0; i < flen; i++) fprintf(ch, "%02X ", frm[i]);
    fprintf(ch, "\n");
    fclose(ch);

    printf("Frame sent through channel (ppp_channel.txt).\n");
    return 0;
}
