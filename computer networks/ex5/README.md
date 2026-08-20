# Exercise 5 — TCP Socket Programming

## Overview

This exercise implements **client–server programs** using **TCP sockets** (POSIX) in C. Three different applications demonstrate socket programming concepts: a TCP echo service, a string comparison service, and a Vertical Redundancy Check (VRC) error detection system over a network.

## Programs

### 1. TCP Echo Server/Client

A simple echo service — the client sends a message, and the server sends it back.

| File | Description |
|------|-------------|
| `server1.c` | TCP echo server — accepts connections, reads message, sends it back |
| `client1.c` | TCP echo client — sends a message via command-line args, prints the echo |

**Usage:**
```bash
# Terminal 1 — Start server
gcc -o server1 server1.c
./server1 8080

# Terminal 2 — Run client
gcc -o client1 client1.c
./client1 127.0.0.1 8080 "Hello, World!"
```

**Output:**
```
# Server:
TCP Echo Server running on port 8080...
Client: Hello, World!

# Client:
Echo from server: Hello, World!
```

### 2. String Comparison Server/Client

The client sends two strings to the server; the server compares them character-by-character and returns the result.

| File | Description |
|------|-------------|
| `server2.c` | String comparison server — receives two strings, compares them, sends result |
| `client2.c` | String comparison client — reads two strings from user, sends to server, displays result |

**Usage:**
```bash
# Terminal 1 — Start server (port 8080)
gcc -o server2 server2.c
./server2

# Terminal 2 — Run client
gcc -o client2 client2.c
./client2
```

**Output:**
```
# Server:
Received String 1: "hello"
Received String 2: "hello"
Comparison Result: Strings are EQUAL (All 5 characters matched successfully)

# Client:
Server Response:
Strings are EQUAL (All 5 characters matched successfully)
```

### 3. VRC (Vertical Redundancy Check) Error Detection over Network

The client computes even parity on binary data and sends the codeword over TCP. The server verifies the parity to detect errors.

| File | Description |
|------|-------------|
| `server3.c` | VRC verification server — receives codeword, checks even parity, reports error status |
| `client3.c` | VRC codeword generator client — reads binary data, computes parity bit, sends codeword |

**Usage:**
```bash
# Terminal 1 — Start server (port 5001)
gcc -o server3 server3.c
./server3

# Terminal 2 — Run client
gcc -o client3 client3.c
./client3
```

**Output:**
```
# Client:
Enter binary data (e.g. 1011001): 1011001
Original Data       : 1011001
Number of 1s in Data: 4
VRC Parity Bit      : 0 (Even Parity)
Codeword Generated  : 10110010

# Server:
Received Codeword : 10110010
Total 1s Count    : 4
Result            : No Error Detected (Even Parity Satisfied)
```

## Key Concepts

- **TCP sockets** — Reliable, connection-oriented communication using `SOCK_STREAM`
- **Socket API** — `socket()`, `bind()`, `listen()`, `accept()`, `connect()`, `send()`, `recv()`, `close()`
- **Client-server model** — Server waits for connections; client initiates
- **VRC (Even Parity)** — Appends a parity bit so the total number of 1s is even
- **POSIX networking** — `struct sockaddr_in`, `inet_pton()`, `INADDR_ANY`
- **Byte-level data transfer** — Sending binary strings and structured data over TCP
