#include <stdio.h>
#include <string.h>

#define MAXC 200
#define MAXB (MAXC*8+50)
#define MAXS (MAXB + MAXB/5 + 50)
#define MAXF (MAXS + 20)

void tobits(unsigned char c, char *o) {
    int i;
    for (i = 7; i >= 0; i--)
        o[7 - i] = ((c >> i) & 1) ? '1' : '0';
    o[8] = '\0';
}

int stuff(char *in, int len, char *out) {
    int i, cnt = 0, o = 0;
    for (i = 0; i < len; i++) {
        out[o++] = in[i];
        if (in[i] == '1') {
            cnt++;
            if (cnt == 5) {
                printf("  [STUFF] 5 consecutive 1s ending at bit %d -> inserting extra 0\n", i);
                out[o++] = '0';
                cnt = 0;
            }
        } else {
            cnt = 0;
        }
    }
    out[o] = '\0';
    return o;
}

int main() {
    char msg[MAXC];
    char bits[MAXB], stf[MAXS], frame[MAXF];
    int len, i, blen, slen;

    printf("=== BIT STUFFING SENDER (basic) ===\n\n");
    printf("Enter message to send: ");
    fgets(msg, MAXC, stdin);
    len = strlen(msg);
    if (len > 0 && msg[len - 1] == '\n') { msg[len - 1] = '\0'; len--; }
    if (len == 0) { printf("Empty message, aborting.\n"); return 1; }

    bits[0] = '\0';
    printf("\nStep 1: convert each char to 8 bits\n");
    for (i = 0; i < len; i++) {
        char o[9];
        tobits((unsigned char) msg[i], o);
        strcat(bits, o);
        printf("  '%c' -> %s\n", msg[i], o);
    }
    blen = len * 8;
    printf("\nRaw bitstream (%d bits): %s\n\n", blen, bits);

    printf("Step 2: bit stuffing pass (insert 0 after every five consecutive 1s)\n");
    slen = stuff(bits, blen, stf);
    printf("\nStuffed bitstream (%d bits): %s\n\n", slen, stf);

    sprintf(frame, "01111110%s01111110", stf);
    printf("Step 3: add start/end flag 01111110\n");
    printf("Final framed bitstream (%d bits):\n%s\n\n", (int) strlen(frame), frame);

    FILE *ch = fopen("bit_channel.txt", "w");
    if (!ch) { printf("ERROR: could not open bit_channel.txt\n"); return 1; }
    fprintf(ch, "%s\n", frame);
    fclose(ch);

    printf("Frame sent through channel (bit_channel.txt).\n");
    return 0;
}
