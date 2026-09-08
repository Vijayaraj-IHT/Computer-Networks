# Exercise 7 — Concurrent TCP Socket Programming

## Overview

This exercise implements **concurrent client–server programs using TCP sockets** (POSIX) in C. Building on the single-client servers of Exercise 5, these servers handle **multiple clients at the same time** using `fork()` and `pthread`: a multi-client chat, an ARP simulation with an aging cache, and a reliable file transfer with integrity checking.

## Programs

### 1. Multi-Client TCP Chat (fork)

A chat server that **forks a child process per client**, so several clients can hold independent conversations with the server simultaneously. Messages are timestamped, and both sides support the `/help`, `/stats` and `/quit` commands.

| File | Description |
|------|-------------|
| `ser_chat.c` | Chat server — `accept()` loop + `fork()` per client, per-client statistics, `SO_REUSEADDR` |
| `cli_chat.c` | Chat client — receives a client-number banner, sends messages, local message counters |

**Commands:** `/help` — list commands · `/stats` — messages sent/received · `/quit` — end the chat

**Usage:**
```bash
# Terminal 1 — Start server
gcc -o ser_chat ser_chat.c
./ser_chat 8080

# Terminals 2..n — connect clients
gcc -o cli_chat cli_chat.c
./cli_chat 127.0.0.1 8080
```

**Output:**
```
# Client:
Connected to cht server at 127.0.0.1:8080
Type /help for commands.

You are Client 1

You: hello from client 1
[23:16:34] Message #1 sent
[23:16:34] Message #1 | You(Server): welcome!

You: /quit
===== CHAT STATISTICS =====
Messages sent     : 2
Messages received : 1
============================

# Server:
TCP Chat Server listening on port 8080...
Waiting for clients...

Client 1 connected.
Client 1: hello from client 1
You(Server): Client 1 has left the chat.
Client 2 connected.
```

### 2. ARP Simulation Server/Client (threads)

The server maintains a **simulated ARP cache**: a 10-slot hash table with linear probing, seeded with `192.168.1.1`–`192.168.1.4`. For every lookup it serves a hit from the table or auto-generates a MAC address (`AA:BB:xx:xx:xx:xx`) for unknown IPs — mimicking an ARP request/reply. Entries **age out** after 5 lookups, and the server tracks hit/miss statistics. Each client is handled by a **POSIX thread**, with a mutex protecting the shared table. The server also dumps the real system ARP cache via `ip neigh`.

| File | Description |
|------|-------------|
| `ser_arp.c` | ARP server — pthread per client, hash table + linear probing, entry aging, hit/miss stats, `ip neigh` dump |
| `cli_arp.c` | ARP client — sends one IP address, prints the MAC lookup result |

**Usage:**
```bash
# Terminal 1 — Start server (needs pthreads)
gcc -o ser_arp ser_arp.c -lpthread
./ser_arp 8081

# Terminal 2 — Run client
gcc -o cli_arp cli_arp.c
./cli_arp 127.0.0.1 8081
```

**Output:**
```
# Client (entry already in table):
Enter IP address: 192.168.1.2

===== ARP LOOKUP RESULT =====
IP: 192.168.1.2
MAC: AA:BB:CC:DD:EE:02
Status: Entry found in ARP table.
==============================

# Client (unknown IP — new entry created):
Enter IP address: 10.0.0.5

===== ARP LOOKUP RESULT =====
IP: 10.0.0.5
MAC: AA:BB:0A:00:00:05
Status: New entry created.
==============================

# Server:
========== SIMULATED ARP TABLE ==========
Slot  IP                 MAC                  Age
[01]  192.168.1.1        AA:BB:CC:DD:EE:01    2
[02]  192.168.1.2        AA:BB:CC:DD:EE:02    1
[05]  10.0.0.5           AA:BB:0A:00:00:05    0
...

========== ARP STATISTICS ==========
Table entries : 5
Free slots    : 5
Lookups       : 2
Hits          : 1
Misses        : 1
====================================
```

### 3. TCP File Transfer with Checksum

A reliable file transfer over TCP. The client streams a **header** (file size + filename length), the filename, the raw bytes (with a live progress indicator), and finally a **byte-sum checksum**. The server sanitizes the filename, writes the file as `received_<name>`, and verifies the checksum — reporting `Integrity: VERIFIED` on success.

**Wire format:**
```
| uint64 size | uint32 name_len | filename ... | file bytes ... | uint64 checksum |
```

| File | Description |
|------|-------------|
| `ser_file.c` | Receiver — `fork()` per connection, header parsing, filename sanitising, checksum verification |
| `cli_file.c` | Sender — reads a path from the user, streams the file with progress and computes the checksum |

**Usage:**
```bash
# Terminal 1 — Start server
gcc -o ser_file ser_file.c
./ser_file 8082

# Terminal 2 — Send a file
gcc -o cli_file cli_file.c
./cli_file 127.0.0.1 8082
# Enter path of file to send: sample.txt
```

**Output:**
```
# Client:
Enter path of file to send: sample.txt
Sent: 88 / 88 bytes

Transfer complete.
File     : sample.txt
Size     : 88 bytes
Checksum : 7949

# Server:
File transfer server running on port 8082...
Client connected.
Received: 88 / 88 bytes

Transfer complete.
File     : sample.txt
Size     : 88 bytes
Checksum : 7949
Remote   : 7949
Integrity: VERIFIED
Saved as : received_sample.txt
```

## Key Concepts

- **Concurrent servers** — `fork()` per connection (chat, file transfer) vs `pthread_create()` per connection (ARP)
- **`SO_REUSEADDR`** — Avoids "Address already in use" on quick server restarts
- **ARP cache behaviour** — Hash table + linear probing, entry aging/expiry, hit/miss statistics
- **Mutex synchronization** — `pthread_mutex_lock` protecting the shared ARP table across threads
- **Application protocols** — Text commands (`/help`, `/stats`, `/quit`) and a binary header (size, name length) + checksum
- **Reliable transfer** — `send()`/`recv()` loops that handle partial reads/writes, plus byte-sum integrity checking
- **Safe file handling** — Filename sanitising (`/`, `\`, `:` → `_`) and `received_` prefix to avoid overwrites
