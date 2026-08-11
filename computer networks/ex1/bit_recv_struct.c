#include <stdio.h>
#include <string.h>

#define MAXB 4096

typedef struct {
    char bit[MAXB];
    int len;
} BitStream;

typedef struct {
    BitStream frm;
    BitStream pay;
    BitStream unst;
} BitFrame;

void unstuff(BitStream *in, BitStream *out) {
    int i, cnt = 0, o = 0;
    for (i = 0; i < in->len; i++) {
        out->bit[o++] = in->bit[i];
        if (in->bit[i] == '1') {
            cnt++;
            if (cnt == 5) {
                printf("  [UNSTUFF] 5 consecutive 1s ending at bit %d -> next bit is a stuffed 0\n", i);
                i++;
                if (i < in->len && in->bit[i] == '0')
                    printf("            Removed stuffed 0 at bit %d\n", i);
                else
                    printf("            WARNING: expected stuffed 0 not found!\n");
                cnt = 0;
            }
        } else {
            cnt = 0;
        }
    }
    out->bit[o] = '\0';
    out->len = o;
}

void frombits(BitStream *in, char *out) {
    int i, j, o = 0;
    for (i = 0; i + 8 <= in->len; i += 8) {
        int v = 0;
        for (j = 0; j < 8; j++)
            v = v * 2 + (in->bit[i + j] - '0');
        out[o++] = (char) v;
    }
    out[o] = '\0';
}

int main() {
    BitFrame f;
    char msg[MAXB];

    printf("=== BIT STUFFING RECEIVER (struct) ===\n\n");

    FILE *ch = fopen("bit_channel.txt", "r");
    if (!ch) { printf("ERROR: bit_channel.txt not found. Run the sender first.\n"); return 1; }
    if (!fgets(f.frm.bit, MAXB, ch)) { printf("ERROR: channel file empty.\n"); fclose(ch); return 1; }
    fclose(ch);

    f.frm.len = strlen(f.frm.bit);
    if (f.frm.len > 0 && f.frm.bit[f.frm.len - 1] == '\n') { f.frm.bit[f.frm.len - 1] = '\0'; f.frm.len--; }

    printf("Received framed bitstream (%d bits):\n%s\n\n", f.frm.len, f.frm.bit);

    if (f.frm.len < 16 || strncmp(f.frm.bit, "01111110", 8) != 0 ||
        strncmp(f.frm.bit + f.frm.len - 8, "01111110", 8) != 0) {
        printf("ERROR: flags not found, frame corrupted!\n");
        return 1;
    }
    printf("Start flag 01111110 detected.\n");
    printf("End   flag 01111110 detected.\n\n");

    f.pay.len = f.frm.len - 16;
    strncpy(f.pay.bit, f.frm.bit + 8, f.pay.len);
    f.pay.bit[f.pay.len] = '\0';
    printf("Payload after removing flags (%d bits): %s\n\n", f.pay.len, f.pay.bit);

    printf("Bit unstuffing pass:\n");
    unstuff(&f.pay, &f.unst);
    printf("\nUnstuffed bitstream (%d bits): %s\n\n", f.unst.len, f.unst.bit);

    frombits(&f.unst, msg);
    printf("Recovered message: \"%s\"\n", msg);

    return 0;
}
