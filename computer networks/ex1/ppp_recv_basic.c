#include <stdio.h>
#include <string.h>

#define FLAG 0x7E
#define ESC  0x7D
#define MAXF 512

int unstuff(unsigned char *in, int len, unsigned char *out) {
    int i, o = 0;
    for (i = 0; i < len; i++) {
        if (in[i] == ESC) {
            printf("  [UNSTUFF] ESC found at byte %d -> next byte 0x%02X XOR 0x20 -> 0x%02X\n",
                   i, in[i + 1], (unsigned char) (in[i + 1] ^ 0x20));
            i++;
            out[o++] = in[i] ^ 0x20;
        } else {
            out[o++] = in[i];
        }
    }
    return o;
}

int main() {
    unsigned char frm[MAXF], pay[MAXF], unst[MAXF];
    int flen = 0, v, i;

    printf("=== PPP BYTE STUFFING RECEIVER (basic) ===\n\n");

    FILE *ch = fopen("ppp_channel.txt", "r");
    if (!ch) { printf("ERROR: ppp_channel.txt not found. Run the sender first.\n"); return 1; }
    while (flen < MAXF && fscanf(ch, "%x", &v) == 1) frm[flen++] = (unsigned char) v;
    fclose(ch);

    printf("Received PPP frame (%d bytes): ", flen);
    for (i = 0; i < flen; i++) printf("%02X ", frm[i]);
    printf("\n\n");

    if (flen < 2 || frm[0] != FLAG || frm[flen - 1] != FLAG) {
        printf("ERROR: flags not found, frame corrupted!\n");
        return 1;
    }
    printf("Start FLAG 0x7E detected.\n");
    printf("End   FLAG 0x7E detected.\n\n");

    int plen = flen - 2;
    for (i = 0; i < plen; i++) pay[i] = frm[i + 1];

    printf("Payload after removing flags (%d bytes):\n", plen);
    for (i = 0; i < plen; i++) printf("  [%d] 0x%02X\n", i, pay[i]);

    printf("\nByte unstuffing pass:\n");
    int ulen = unstuff(pay, plen, unst);
    printf("\nUnstuffed payload (%d bytes):\n", ulen);
    for (i = 0; i < ulen; i++) printf("  [%d] 0x%02X\n", i, unst[i]);

    printf("\nRecovered message: \"");
    for (i = 0; i < ulen; i++) printf("%c", unst[i]);
    printf("\"\n");

    return 0;
}
