#include <stdio.h>
#include <string.h>

#define MAXC 200
#define MAXB (MAXC*8+50)
#define MAXS (MAXB + MAXB/5 + 50)
#define MAXF (MAXS + 20)

typedef struct {
    char bit[MAXB];
    int len;
} BitStream;

typedef struct {
    char bit[MAXS];
    int len;
} BitStreamBig;

typedef struct {
    char bit[MAXF];
    int len;
} BitStreamFrame;

typedef struct {
    BitStream raw;
    BitStreamBig stf;
    BitStreamFrame frm;
} BitFrame;

void tobits(unsigned char c, char *o) {
    int i;
    for (i = 7; i >= 0; i--)
        o[7 - i] = ((c >> i) & 1) ? '1' : '0';
    o[8] = '\0';
}

void stuff(BitStream *in, BitStreamBig *out) {
    int i, cnt = 0, o = 0;
    for (i = 0; i < in->len; i++) {
        out->bit[o++] = in->bit[i];
        if (in->bit[i] == '1') {
            cnt++;
            if (cnt == 5) {
                printf("  [STUFF] 5 consecutive 1s ending at bit %d -> inserting extra 0\n", i);
                out->bit[o++] = '0';
                cnt = 0;
            }
        } else {
            cnt = 0;
        }
    }
    out->bit[o] = '\0';
    out->len = o;
}

void mkframe(BitFrame *f) {
    sprintf(f->frm.bit, "01111110%s01111110", f->stf.bit);
    f->frm.len = (int) strlen(f->frm.bit);
}

int main() {
    char msg[MAXC];
    BitFrame f;
    int len, i;

    printf("=== BIT STUFFING SENDER (struct) ===\n\n");
    printf("Enter message to send: ");
    fgets(msg, MAXC, stdin);
    len = strlen(msg);
    if (len > 0 && msg[len - 1] == '\n') { msg[len - 1] = '\0'; len--; }
    if (len == 0) { printf("Empty message, aborting.\n"); return 1; }

    f.raw.bit[0] = '\0';
    printf("\nStep 1: convert each char to 8 bits\n");
    for (i = 0; i < len; i++) {
        char o[9];
        tobits((unsigned char) msg[i], o);
        strcat(f.raw.bit, o);
        printf("  '%c' -> %s\n", msg[i], o);
    }
    f.raw.len = len * 8;
    printf("\nRaw bitstream (%d bits): %s\n\n", f.raw.len, f.raw.bit);

    printf("Step 2: bit stuffing pass (insert 0 after every five consecutive 1s)\n");
    stuff(&f.raw, &f.stf);
    printf("\nStuffed bitstream (%d bits): %s\n\n", f.stf.len, f.stf.bit);

    printf("Step 3: add start/end flag 01111110\n");
    mkframe(&f);
    printf("Final framed bitstream (%d bits):\n%s\n\n", f.frm.len, f.frm.bit);

    FILE *ch = fopen("bit_channel.txt", "w");
    if (!ch) { printf("ERROR: could not open bit_channel.txt\n"); return 1; }
    fprintf(ch, "%s\n", f.frm.bit);
    fclose(ch);

    printf("Frame sent through channel (bit_channel.txt).\n");
    return 0;
}
