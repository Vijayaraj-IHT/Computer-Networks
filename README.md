# Computer-Networks
# Computer Networks

A collection of laboratory exercises and implementations covering fundamental Computer Networking concepts — from layered network simulation and data-link framing techniques, through error detection/correction, routing algorithms, and TCP socket programming.

## Repository Structure

| Directory | Topic | Description |
|-----------|-------|-------------|
| [ex0](computer%20networks/ex0/) | **Network Layering Simulation** | 4-layer OSI network simulator demonstrating address resolution (DNS/IP/MAC), packetization, and framing — all visualized in binary |
| [ex1](computer%20networks/ex1/) | **Framing Techniques** | Bit stuffing and byte stuffing (BISYNC & PPP) sender/receiver implementations + LeetCode bit-manipulation problems |
| [ex2](computer%20networks/ex2/) | **Checksum Error Detection** | OSI-layered sender/receiver using checksum verification and byte-count protocol + binary arithmetic problems |
| [ex3](computer%20networks/ex3/) | **Hamming Code Error Correction** | OSI-layered sender/receiver with Hamming single-error-correcting codes + Hamming distance problems |
| [ex4](computer%20networks/ex4/) | **Routing — Bellman-Ford Algorithm** | Bellman-Ford shortest-path implementation with negative-cycle detection + VLab routing simulation screenshots |
| [ex5](computer%20networks/ex5/) | **TCP Socket Programming** | Client–server programs over TCP sockets: echo service, string comparison, and VRC error detection over a network |

## Prerequisites

- **C compiler** — `gcc` (or any C99-compatible compiler)
- **C++ compiler** — `g++` (for ex1 LeetCode problems)
- **Python 3** — (for ex2/ex3 LeetCode problems)
- **Linux/POSIX environment** — socket programs (ex5) use POSIX sockets (`<sys/socket.h>`)

## How to Compile & Run

```bash
# Compile a C program
gcc -o program program.c
./program

# Compile a C++ program
g++ -o program program.cpp
./program

# Run a Python script
python3 script.py
```

For socket programs (ex5), run the server in one terminal and the client in another:

```bash
# Terminal 1 — Server
gcc -o server server1.c
./server 8080

# Terminal 2 — Client
gcc -o client client1.c
./client 127.0.0.1 8080 "Hello"
```

## Concepts Covered

- 🌐 **OSI Layer Model** — Application, Transport, Network, Data Link layers
- 📡 **Address Resolution** — URL → IP (DNS), IP → MAC (ARP), binary representations
- 🗃️ **Framing** — Character count, bit stuffing, byte stuffing (BISYNC, PPP)
- ✅ **Error Detection** — Checksum (modulo-256), VRC (Vertical Redundancy Check)
- 🔧 **Error Correction** — Hamming codes (single-bit error correction)
- 🛤️ **Routing** — Bellman-Ford algorithm with negative-cycle detection
- 🔌 **Socket Programming** — TCP client/server communication using POSIX sockets

## Author

**Vijayaraj** — [Vijayaraj-IHT](https://github.com/Vijayaraj-IHT)

## License

This repository is intended for educational purposes.
