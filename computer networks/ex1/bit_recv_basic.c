#include <stdio.h>
#include <string.h>

#define MAXB 4096

int unstuff(char *in, int len, char *out) {
    int i, cnt = 0, o = 0;
    for (i = 0; i < len; i++) {
        out[o++] = in[i];
        if (in[i] == '1') {
            cnt++;
            if (cnt == 5) {
                printf("  [UNSTUFF] 5 consecutive 1s ending at bit %d -> next bit is a stuffed 0\n", i);
                i++;
                if (i < len && in[i] == '0')
                    printf("            removed stuffed 0 at bit %d\n", i);
                else
                    printf("            WARNING: expected stuffed 0 not found!\n");
                cnt = 0;
            }
        } else {
            cnt = 0;
        }
    }
    out[o] = '\0';
    return o;
}

void frombits(char *bits, int len, char *out) {
    int i, j, o = 0;
    for (i = 0; i + 8 <= len; i += 8) {
        int v = 0;
        for (j = 0; j < 8; j++)
            v = v * 2 + (bits[i + j] - '0');
        out[o++] = (char) v;
    }
    out[o] = '\0';
}

int main() {
    char frame[MAXB], payload[MAXB], unst[MAXB], msg[MAXB];
    int flen, plen, ulen;

    printf("=== BIT STUFFING RECEIVER (basic) ===\n\n");

    FILE *ch = fopen("bit_channel.txt", "r");
    if (!ch) { printf("ERROR: bit_channel.txt not found. Run the sender first.\n"); return 1; }
    if (!fgets(frame, MAXB, ch)) { printf("ERROR: channel file empty.\n"); fclose(ch); return 1; }
    fclose(ch);

    flen = strlen(frame);
    if (flen > 0 && frame[flen - 1] == '\n') { frame[flen - 1] = '\0'; flen--; }

    printf("Received framed bitstream (%d bits):\n%s\n\n", flen, frame);

    if (flen < 16 || strncmp(frame, "01111110", 8) != 0 || strncmp(frame + flen - 8, "01111110", 8) != 0) {
        printf("ERROR: flags not found, frame corrupted!\n");
        return 1;
    }
    printf("Start flag 01111110 detected.\n");
    printf("End   flag 01111110 detected.\n\n");

    plen = flen - 16;
    strncpy(payload, frame + 8, plen);
    payload[plen] = '\0';
    printf("Payload after removing flags (%d bits): %s\n\n", plen, payload);

    printf("Bit unstuffing pass:\n");
    ulen = unstuff(payload, plen, unst);
    printf("\nUnstuffed bitstream (%d bits): %s\n\n", ulen, unst);

    frombits(unst, ulen, msg);
    printf("Recovered message: \"%s\"\n", msg);

    return 0;
}
