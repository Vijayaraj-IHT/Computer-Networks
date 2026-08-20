# Exercise 1 — Framing Techniques & Bit Manipulation

## Overview

This exercise covers **data-link layer framing protocols** — implementing bit stuffing and byte stuffing (BISYNC & PPP) as sender/receiver pairs. Each protocol has two variants: a basic array-based version and a struct-based version for better data encapsulation. Additionally, several LeetCode bit-manipulation problems are included.

## Framing Programs

### 1. Bit Stuffing

Inserts a `0` after every five consecutive `1`s to prevent the flag pattern `01111110` from appearing in the data.

| File | Description |
|------|-------------|
| `bit_send_basic.c` | Bit stuffing sender (basic arrays) |
| `bit_send_struct.c` | Bit stuffing sender (struct-based) |
| `bit_recv_basic.c` | Bit stuffing receiver (basic arrays) |
| `bit_recv_struct.c` | Bit stuffing receiver (struct-based) |
| `bit_channel.txt` | Channel file used for sender→receiver communication |

**Protocol:** `01111110` + stuffed_data + `01111110`

```bash
gcc -o bit_send bit_send_basic.c && ./bit_send
gcc -o bit_recv bit_recv_basic.c && ./bit_recv
```

### 2. BISYNC Byte Stuffing (IBM Binary Synchronous Communication)

Uses control characters — `SYN`, `STX`, `ETX`, `DLE` — and doubles any literal `DLE` byte (0x10) in the payload.

| File | Description |
|------|-------------|
| `bisync_send_basic.c` | BISYNC byte stuffing sender |
| `bisync_recv_basic.c` | BISYNC byte stuffing receiver |

**Protocol:** `SYN SYN STX` + DLE-stuffed_data + `ETX`

```bash
gcc -o bisync_send bisync_send_basic.c && ./bisync_send
gcc -o bisync_recv bisync_recv_basic.c && ./bisync_recv
```

### 3. PPP Byte Stuffing (Point-to-Point Protocol)

Uses special bytes `FLAG` (0x7E) and `ESC` (0x7D). When FLAG or ESC appears in data, it is replaced with `ESC` + `(byte XOR 0x20)`.

| File | Description |
|------|-------------|
| `ppp_send_basic.c` | PPP byte stuffing sender |
| `ppp_send_struct.c` | PPP byte stuffing sender (struct-based) |
| `ppp_recv_basic.c` | PPP byte stuffing receiver |
| `ppp_recv_struct.c` | PPP byte stuffing receiver (struct-based) |

**Protocol:** `0x7E` + byte-stuffed_data + `0x7E`

```bash
gcc -o ppp_send ppp_send_basic.c && ./ppp_send
gcc -o ppp_recv ppp_recv_basic.c && ./ppp_recv
```

## LeetCode Problems (Bit Manipulation)

| File | Problem | Language |
|------|---------|----------|
| `338.cpp` | [Counting Bits](https://leetcode.com/problems/counting-bits/) — Count 1s in binary representation of 0 to n | C++ |
| `421.c` | [Maximum XOR of Two Numbers](https://leetcode.com/problems/maximum-xor-of-two-numbers-in-an-array/) — Using Trie-based approach | C |
| `476.cpp` | [Number Complement](https://leetcode.com/problems/number-complement/) — Find the complement of a number by flipping bits | C++ |

## Execution Workflow (Sender/Receiver)

```
1. Run sender → enter message → framed output written to channel file
2. Run receiver → reads from channel file → frame detection → unstuff → recover message
```

## Key Concepts

- **Bit stuffing** — Prevents flag pattern collision in HDLC-like framing
- **Byte stuffing** — Uses escape sequences for transparent data transmission
- **BISYNC** — Legacy IBM framing with SYN/STX/ETX control characters
- **PPP** — Modern framing with FLAG/ESC byte substitution
- **Struct-based design** — Cleaner data encapsulation with `BitStream`, `BitFrame` types
