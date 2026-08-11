#include <stdio.h>
#include <string.h>

#define FLAG 0x7E
#define ESC  0x7D
#define MAXF 512

typedef struct {
    unsigned char byte[MAXF];
    int len;
} ByteBuf;

typedef struct {
    ByteBuf frm;
    ByteBuf pay;
    ByteBuf unst;
} PPPFrame;

void unstuff(ByteBuf *in, ByteBuf *out) {
    int i, o = 0;
    for (i = 0; i < in->len; i++) {
        if (in->byte[i] == ESC) {
            printf("  [UNSTUFF] ESC found at byte %d -> next byte 0x%02X XOR 0x20 -> 0x%02X\n",
                   i, in->byte[i + 1], (unsigned char) (in->byte[i + 1] ^ 0x20));
            i++;
            out->byte[o++] = in->byte[i] ^ 0x20;
        } else {
            out->byte[o++] = in->byte[i];
        }
    }
    out->len = o;
}

int main() {
    PPPFrame f;
    int v, i;

    printf("=== PPP BYTE STUFFING RECEIVER (struct) ===\n\n");

    FILE *ch = fopen("ppp_channel.txt", "r");
    if (!ch) { printf("ERROR: ppp_channel.txt not found. Run the sender first.\n"); return 1; }
    f.frm.len = 0;
    while (f.frm.len < MAXF && fscanf(ch, "%x", &v) == 1) f.frm.byte[f.frm.len++] = (unsigned char) v;
    fclose(ch);

    printf("Received PPP frame (%d bytes): ", f.frm.len);
    for (i = 0; i < f.frm.len; i++) printf("%02X ", f.frm.byte[i]);
    printf("\n\n");

    if (f.frm.len < 2 || f.frm.byte[0] != FLAG || f.frm.byte[f.frm.len - 1] != FLAG) {
        printf("ERROR: flags not found, frame corrupted!\n");
        return 1;
    }
    printf("Start FLAG 0x7E detected.\n");
    printf("End   FLAG 0x7E detected.\n\n");

    f.pay.len = f.frm.len - 2;
    for (i = 0; i < f.pay.len; i++) f.pay.byte[i] = f.frm.byte[i + 1];

    printf("Payload after removing flags (%d bytes):\n", f.pay.len);
    for (i = 0; i < f.pay.len; i++) printf("  [%d] 0x%02X\n", i, f.pay.byte[i]);

    printf("\nByte unstuffing pass:\n");
    unstuff(&f.pay, &f.unst);
    printf("\nUnstuffed payload (%d bytes):\n", f.unst.len);
    for (i = 0; i < f.unst.len; i++) printf("  [%d] 0x%02X\n", i, f.unst.byte[i]);

    printf("\nRecovered message: \"");
    for (i = 0; i < f.unst.len; i++) printf("%c", f.unst.byte[i]);
    printf("\"\n");

    return 0;
}
