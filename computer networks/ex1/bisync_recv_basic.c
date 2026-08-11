#include <stdio.h>
#include <string.h>

#define SYN 0x16
#define STX 0x02
#define ETX 0x03
#define DLE 0x10
#define MAXF 512

int unstuff(unsigned char *in, int len, unsigned char *out) {
    int i, o = 0;
    for (i = 0; i < len; i++) {
        if (in[i] == DLE) {
            printf("  [UNSTUFF] DLE found at byte %d -> dropping it, next byte 0x%02X kept as data\n",
                   i, in[i + 1]);
            i++;
            out[o++] = in[i];
        } else {
            out[o++] = in[i];
        }
    }
    return o;
}

int main() {
    unsigned char frm[MAXF], pay[MAXF], unst[MAXF];
    int flen = 0, v, i;

    printf("=== BISYNC BYTE STUFFING RECEIVER (basic) ===\n\n");

    FILE *ch = fopen("bisync_channel.txt", "r");
    if (!ch) { printf("ERROR: bisync_channel.txt not found. Run the sender first.\n"); return 1; }
    while (flen < MAXF && fscanf(ch, "%x", &v) == 1) frm[flen++] = (unsigned char) v;
    fclose(ch);

    printf("Received BISYNC frame (%d bytes): ", flen);
    for (i = 0; i < flen; i++) printf("%02X ", frm[i]);
    printf("\n\n");

    if (flen < 4 || frm[0] != SYN || frm[1] != SYN || frm[2] != STX || frm[flen - 1] != ETX) {
        printf("ERROR: SYN SYN STX ... ETX header/trailer not found, frame corrupted!\n");
        return 1;
    }
    printf("SYN SYN STX header detected.\n");
    printf("ETX trailer detected.\n\n");

    int plen = flen - 4;
    for (i = 0; i < plen; i++) pay[i] = frm[i + 3];

    printf("Payload after removing SYN/SYN/STX/ETX (%d bytes):\n", plen);
    for (i = 0; i < plen; i++) printf("  [%d] 0x%02X\n", i, pay[i]);

    printf("\nDLE unstuffing pass:\n");
    int ulen = unstuff(pay, plen, unst);
    printf("\nUnstuffed payload (%d bytes):\n", ulen);
    for (i = 0; i < ulen; i++) printf("  [%d] 0x%02X\n", i, unst[i]);

    printf("\nRecovered message: \"");
    for (i = 0; i < ulen; i++) printf("%c", unst[i]);
    printf("\"\n");

    return 0;
}
