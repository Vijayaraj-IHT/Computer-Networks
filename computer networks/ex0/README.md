# Exercise 0 — 4-Layer Network Simulator

## Overview

A comprehensive C program that simulates the **4-layer network model** (Application, Transport, Network, Data Link) by encapsulating a text message from `message.txt` into binary frames — visualizing every step of the process.

## What It Does

1. **Address Resolution** — Maintains a hash table mapping URLs → IP addresses → MAC addresses (simulating DNS + ARP). Users input source and destination URLs, and the program resolves them to IP/MAC pairs displayed in both decimal and binary.

2. **Application Layer** — Reads a message from `message.txt`, converts each character to its 8-bit ASCII binary representation.

3. **Transport Layer** — Assigns random source/destination port numbers (16-bit binary) to create a transport segment.

4. **Network Layer** — Splits the bitstream into 16-bit packets, prepending source (32-bit) and destination (32-bit) IP addresses.

5. **Data Link Layer** — Divides each packet into 8-bit frames, adding source (48-bit) and destination (48-bit) MAC addresses plus an 8-bit trailer.

## Files

| File | Description |
|------|-------------|
| `layering.c` | Main simulator program |
| `output.txt` | Sample output demonstrating the full encapsulation pipeline |

## How to Run

```bash
gcc -o layering layering.c
./layering
```

You will be prompted to enter:
- **Source URL** (e.g., `google.com`)
- **Destination URL** (e.g., `youtube.com`)

The program reads the message from a file named `message.txt` in the same directory. Create one before running:

```bash
echo "Hello from NWL!" > message.txt
./layering
```

## Sample Output

```
=============== APPLICATION LAYER ===============
Message from file: "Hello from NWL!"
  'H'  ->  01001000
  'e'  ->  01100101
  ...
Full bitstream (120 bits): 0100100001100101...

=============== TRANSPORT LAYER ===============
Source Port      : 49683
Destination Port : 40116

=============== NETWORK LAYER ===============
Total packets: 8

=============== DATA LINK LAYER ===============
Total frames: 16
```

## Key Concepts

- **Hash table** with linear probing for URL → IP → MAC resolution
- **Binary conversion** of IP (32-bit), MAC (48-bit), and port (16-bit) addresses
- **Packetization** (16-bit packets) and **framing** (8-bit frames)
- **Bit-level visualization** of the entire encapsulation process
