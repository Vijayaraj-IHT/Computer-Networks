# Exercise 2 — Checksum Error Detection & Binary Arithmetic

## Overview

This exercise implements an **OSI-layered sender/receiver** that uses a **checksum (modulo-256)** for error detection and a **byte-count protocol** for frame size verification. Data is encapsulated through all layers (Application → Transport → Network → Data Link) in binary, transmitted through a channel file, and then decapsulated and verified by the receiver.

Additionally, several LeetCode binary arithmetic problems are included.

## Checksum Protocol

### How It Works

1. **Sender** encapsulates each character through 4 OSI layers:
   - **Application Layer** — Character → 8-bit binary
   - **Transport Layer** — Prepends 16-bit source port + 16-bit destination port
   - **Network Layer** — Prepends 32-bit destination IP + 32-bit source IP
   - **Data Link Layer** — Prepends 48-bit destination MAC + 48-bit source MAC, calculates an 8-bit checksum over the entire frame, and prepends a 16-bit byte-count field

2. **Receiver** decapsulates in reverse order:
   - Verifies the byte count matches the actual frame size
   - Recalculates the checksum and compares it with the received checksum
   - Strips headers layer by layer, recovering the original message

### Frame Structure

```
| COUNT (16 bits) | DST MAC (48) | SRC MAC (48) | DST IP (32) | SRC IP (32) | SRC PORT (16) | DST PORT (16) | DATA (8) | CHECKSUM (8) |
```

## Files

| File | Description |
|------|-------------|
| `checksum_sender.c` | Full OSI sender with checksum calculation and byte-count framing |
| `checksum_receiver.c` | Full OSI receiver with checksum verification and byte-count validation |
| `#67.py` | LeetCode — [Add Binary](https://leetcode.com/problems/add-binary/) |
| `#693.py` | LeetCode — [Binary Number with Alternating Bits](https://leetcode.com/problems/binary-number-with-alternating-bits/) |
| `#898.py` | LeetCode — [Bitwise ORs of Subarrays](https://leetcode.com/problems/bitwise-ors-of-subarrays/) |

## How to Run

```bash
# Step 1: Compile and run sender
gcc -o checksum_sender checksum_sender.c
./checksum_sender
# Enter a message when prompted

# Step 2: Compile and run receiver
gcc -o checksum_receiver checksum_receiver.c
./checksum_receiver
# Reads from bytecount_channel.txt automatically
```

## Sample Output

**Sender:**
```
[APP LAYER]   Character 'H' -> 01001000
[TRANS LAYER] Added source/destination ports: 40 bits
[NET LAYER]   Added destination/source IPs: 104 bits
[LINK LAYER]  Added MAC headers: 200 bits
[LINK LAYER]  Calculated 8-bit Checksum: 10110101 (181)
[LINK LAYER]  Byte count: 26 bytes
```

**Receiver:**
```
[LINK LAYER]  Byte-count verification: PASS
[LINK LAYER]  Received Checksum:   10110101 (181)
[LINK LAYER]  Calculated Checksum: 10110101 (181)
[LINK LAYER]  Checksum verification: PASS
Final reassembled message: "Hello"
```

## LeetCode Problems

Run with Python 3:
```bash
python3 '#67.py'    # Binary string addition
python3 '#693.py'   # Alternating bits check
python3 '#898.py'   # Subarray bitwise ORs count
```

## Key Concepts

- **Checksum (modulo-256)** — Sum all bytes, keep lowest 8 bits
- **Byte-count protocol** — 16-bit length field for frame size verification
- **OSI encapsulation/decapsulation** — Full 4-layer binary pipeline
- **Error detection** — Receiver detects corrupted frames via checksum mismatch
- **Binary address encoding** — IP, MAC, and port addresses in binary
