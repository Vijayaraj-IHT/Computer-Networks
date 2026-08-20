# Exercise 3 — Hamming Code Error Correction & Hamming Distance

## Overview

This exercise implements an **OSI-layered sender/receiver** using **Hamming codes** for single-bit error correction. Each frame is encoded with parity bits at the Data Link Layer, allowing the receiver to detect and correct any single-bit error that occurs during transmission.

Additionally, several LeetCode problems related to Hamming distance and bit manipulation are included.

## Hamming Code Implementation

### How It Works

1. **Sender** encapsulates each character through 4 OSI layers:
   - **Application Layer** — Character → 8-bit binary
   - **Transport Layer** — Prepends 16-bit source port + 16-bit destination port
   - **Network Layer** — Prepends 32-bit destination IP + 32-bit source IP
   - **Data Link Layer** — Prepends 48-bit destination MAC + 48-bit source MAC → resulting in a 200-bit frame → applies Hamming encoding

2. **Hamming Encoding:**
   - Data bits are placed in non-parity positions (positions not a power of 2)
   - Parity bits are computed at positions 1, 2, 4, 8, 16, ... using syndrome calculation
   - Result: 200 data bits + 8 parity bits = **208-bit codeword**

3. **Receiver** performs Hamming decoding:
   - Computes the syndrome (XOR of all positions with value `1`)
   - **Syndrome = 0** → No error detected
   - **Syndrome ≤ 208** → Single-bit error at that position → flip it to correct
   - **Syndrome > 208** → Uncorrectable error → drop frame
   - Extracts data bits by removing parity positions

### Frame Structure

```
Original: | DST MAC (48) | SRC MAC (48) | DST IP (32) | SRC IP (32) | SRC PORT (16) | DST PORT (16) | DATA (8) |
           = 200 bits

Encoded:  | Parity bits + Data bits | = 208-bit Hamming codeword
```

## Files

| File | Description |
|------|-------------|
| `hamming_sender.c` | Full OSI sender with Hamming encoding at the Data Link Layer |
| `hamming_receiver.c` | Full OSI receiver with Hamming decoding and single-bit error correction |
| `461.c` | LeetCode — [Hamming Distance](https://leetcode.com/problems/hamming-distance/) (C) |
| `477.c` | LeetCode — [Total Hamming Distance](https://leetcode.com/problems/total-hamming-distance/) (C) |
| `2220.py` | LeetCode — [Minimum Bit Flips to Convert Number](https://leetcode.com/problems/minimum-bit-flips-to-convert-number/) (Python) |
| `717.py` | LeetCode — [1-bit and 2-bit Characters](https://leetcode.com/problems/1-bit-and-2-bit-characters/) (Python) |

## How to Run

```bash
# Step 1: Compile and run sender
gcc -o hamming_sender hamming_sender.c
./hamming_sender
# Enter a message when prompted
# Frames written to hamming_channel.txt

# Step 2: Compile and run receiver
gcc -o hamming_receiver hamming_receiver.c
./hamming_receiver
# Reads from hamming_channel.txt, detects and corrects errors
```

## Sample Output

**Sender:**
```
[HAMMING]     Data bits:   200
[HAMMING]     Parity bits: 8
[HAMMING]     Codeword:    208 bits
[HAMMING]     Verification syndrome: 0
```

**Receiver (no error):**
```
[HAMMING]     Syndrome value: 0
[HAMMING]     No errors detected.
```

**Receiver (single-bit error corrected):**
```
[HAMMING]     Syndrome value: 42
[HAMMING]     Single-bit error at position 42.
[HAMMING]     Error was in a data bit.
[HAMMING]     Bit flipped back. Error corrected.
[HAMMING]     Recheck syndrome: 0
```

## LeetCode Problems

```bash
# Hamming Distance
gcc -o 461 461.c && ./461

# Total Hamming Distance
gcc -o 477 477.c && ./477

# Minimum Bit Flips
python3 2220.py

# 1-bit and 2-bit Characters
python3 717.py
```

## Key Concepts

- **Hamming (n, k) codes** — Single-error-correcting codes using parity bits at power-of-2 positions
- **Syndrome decoding** — XOR of all 1-positions identifies the error location
- **Error correction** — Flip the bit at the syndrome position to restore the original data
- **OSI simulation** — Full 4-layer binary encapsulation with Hamming codes at the Data Link Layer
- **Hamming distance** — Number of bit positions in which two binary strings differ
