/* Self-Question 1 (Page 14): Go-Back-N Sliding Window Protocol simulation.
   User specifies window size and number of frames. Random frame loss/corruption
   is simulated. On timeout, sender retransmits ALL frames from the lost frame
   onward (characteristic of Go-Back-N). Displays sequence of transmitted,
   acknowledged, and retransmitted frames, and total transmissions required. */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAXF 100

int main(void) {
    int nframes, wsize;
    int acked[MAXF] = {0};
    int total_tx = 0;

    printf("=== Go-Back-N Sliding Window Protocol Simulation ===\n");
    printf("Enter number of frames: ");
    scanf("%d", &nframes);
    printf("Enter window size: ");
    scanf("%d", &wsize);

    srand((unsigned)time(NULL));

    int base = 0;
    int safety = 0;
    while (base < nframes && safety < 200) {
        safety++;
        int window_end = base + wsize;
        if (window_end > nframes) window_end = nframes;

        printf("\n--- Window: sending frames %d to %d ---\n", base, window_end - 1);
        int lost_at = -1;

        for (int i = base; i < window_end; i++) {
            total_tx++;
            int lost = (rand() % 5 == 0); /* ~20% loss probability */
            if (lost && lost_at == -1) {
                printf("Frame %d transmitted -> LOST/CORRUPTED in transit\n", i);
                lost_at = i;
            } else if (lost_at != -1) {
                printf("Frame %d transmitted -> discarded (comes after lost frame %d)\n", i, lost_at);
            } else {
                printf("Frame %d transmitted -> ACK %d received\n", i, i);
                acked[i] = 1;
            }
        }

        if (lost_at != -1) {
            printf("Timeout on frame %d. Go-Back-N: retransmitting frames %d to %d\n",
                   lost_at, lost_at, window_end - 1);
            base = lost_at; /* go back and resend from lost frame */
        } else {
            base = window_end;
        }
    }

    printf("\n=== Transmission Summary ===\n");
    for (int i = 0; i < nframes; i++)
        printf("Frame %d: %s\n", i, acked[i] ? "ACKED" : "PENDING");
    printf("Total frame transmissions (including retransmissions): %d\n", total_tx);
    printf("Total data frames required: %d\n", nframes);
    printf("Retransmission overhead: %d extra transmissions\n", total_tx - nframes);
    return 0;
}
