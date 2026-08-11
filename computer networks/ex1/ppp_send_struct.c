#include <stdio.h>
#include <string.h>

#define FLAG 0x7E
#define ESC  0x7D
#define MAXC 200
#define MAXF 512

typedef struct {
    unsigned char byte[MAXF];
    int len;
} ByteBuf;

typedef struct {
    ByteBuf pay;
    ByteBuf stf;
    ByteBuf frm;
} PPPFrame;

void stuff(ByteBuf *in, ByteBuf *out) {
    int i, o = 0;
    for (i = 0; i < in->len; i++) {
        unsigned char b = in->byte[i];
        if (b == FLAG || b == ESC) {
            printf("  [STUFF] byte %d (0x%02X) matches FLAG/ESC -> insert ESC 0x%02X then 0x%02X XOR 0x20 -> 0x%02X\n",
                   i, b, ESC, b, (unsigned char) (b ^ 0x20));
            out->byte[o++] = ESC;
            out->byte[o++] = b ^ 0x20;
        } else {
            out->byte[o++] = b;
        }
    }
    out->len = o;
}

void mkframe(PPPFrame *f) {
    int o = 0, i;
    f->frm.byte[o++] = FLAG;
    for (i = 0; i < f->stf.len; i++) f->frm.byte[o++] = f->stf.byte[i];
    f->frm.byte[o++] = FLAG;
    f->frm.len = o;
}

int main() {
    char msg[MAXC];
    PPPFrame f;
    int len, i;

    printf("=== PPP BYTE STUFFING SENDER (struct) ===\n\n");
    printf("Enter message to send: ");
    fgets(msg, MAXC, stdin);
    len = strlen(msg);
    if (len > 0 && msg[len - 1] == '\n') { msg[len - 1] = '\0'; len--; }
    if (len == 0) { printf("Empty message, aborting.\n"); return 1; }

    for (i = 0; i < len; i++) f.pay.byte[i] = (unsigned char) msg[i];
    f.pay.len = len;

    printf("\nStep 1: raw payload bytes\n");
    for (i = 0; i < f.pay.len; i++) printf("  [%d] 0x%02X ('%c')\n", i, f.pay.byte[i], msg[i]);

    printf("\nStep 2: byte stuffing pass (FLAG 0x7E, ESC 0x7D, XOR 0x20)\n");
    stuff(&f.pay, &f.stf);
    printf("\nStuffed payload (%d bytes):\n", f.stf.len);
    for (i = 0; i < f.stf.len; i++) printf("  [%d] 0x%02X\n", i, f.stf.byte[i]);

    printf("\nStep 3: add start/end FLAG 0x7E\n");
    mkframe(&f);
    printf("Final PPP frame (%d bytes): ", f.frm.len);
    for (i = 0; i < f.frm.len; i++) printf("%02X ", f.frm.byte[i]);
    printf("\n\n");

    FILE *ch = fopen("ppp_channel.txt", "w");
    if (!ch) { printf("ERROR: could not open ppp_channel.txt\n"); return 1; }
    for (i = 0; i < f.frm.len; i++) fprintf(ch, "%02X ", f.frm.byte[i]);
    fprintf(ch, "\n");
    fclose(ch);

    printf("Frame sent through channel (ppp_channel.txt).\n");
    return 0;
}
