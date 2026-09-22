/* Self-Question 2 (Page 14): Selective Repeat Sliding Window Protocol simulation.
   User specifies window size and number of frames; random frame loss is
   simulated. Receiver acknowledges only correctly received frames; sender
   retransmits ONLY the lost/corrupted frames (not the entire window), unlike
   Go-Back-N. Displays per-frame status and compares transmission efficiency. */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAXF 100

int main(void) {
    int nframes, wsize;
    int acked[MAXF] = {0};
    int total_tx = 0;
    int retransmissions = 0;

    printf("=== Selective Repeat Sliding Window Protocol Simulation ===\n");
    printf("Enter number of frames: ");
    scanf("%d", &nframes);
    printf("Enter window size: ");
    scanf("%d", &wsize);

    srand((unsigned)time(NULL));

    int base = 0;
    while (base < nframes) {
        int window_end = base + wsize;
        if (window_end > nframes) window_end = nframes;

        printf("\n--- Window: sending frames %d to %d ---\n", base, window_end - 1);
        for (int i = base; i < window_end; i++) {
            if (acked[i]) continue; /* already correctly received earlier */
            total_tx++;
            int lost = (rand() % 5 == 0); /* ~20% loss probability, independent per frame */
            if (lost) {
                printf("Frame %d transmitted -> LOST/CORRUPTED (will retransmit individually)\n", i);
            } else {
                printf("Frame %d transmitted -> ACK %d received (buffered at receiver)\n", i, i);
                acked[i] = 1;
            }
        }

        /* Selective Repeat: retransmit ONLY unacknowledged frames within the current window */
        int all_acked_in_window = 1;
        for (int i = base; i < window_end; i++) if (!acked[i]) all_acked_in_window = 0;

        if (!all_acked_in_window) {
            printf("Timeout: selectively retransmitting only unacknowledged frames in window...\n");
            for (int i = base; i < window_end; i++) {
                if (!acked[i]) {
                    retransmissions++;
                    total_tx++;
                    int lost = (rand() % 5 == 0);
                    if (!lost) { printf("  Retransmit frame %d -> ACK %d received\n", i, i); acked[i] = 1; }
                    else printf("  Retransmit frame %d -> LOST again, will retry next round\n", i);
                }
            }
        }

        /* slide window forward while the base frame(s) are acknowledged */
        while (base < nframes && acked[base]) base++;
    }

    printf("\n=== Transmission Summary ===\n");
    for (int i = 0; i < nframes; i++)
        printf("Frame %d: %s\n", i, acked[i] ? "ACKED" : "PENDING");
    printf("Total frame transmissions (including retransmissions): %d\n", total_tx);
    printf("Total data frames required: %d\n", nframes);
    printf("Individual frame retransmissions: %d\n", retransmissions);
    printf("Efficiency note: Selective Repeat retransmits only lost frames,\n");
    printf("unlike Go-Back-N which resends the whole window after a loss.\n");
    return 0;
}
