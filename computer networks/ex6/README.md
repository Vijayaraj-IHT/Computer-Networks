# Exercise 6 — UDP Socket Programming

## Overview

This exercise implements **client–server programs using UDP sockets** (`SOCK_DGRAM`, POSIX) in C. Three applications demonstrate **connectionless** communication: a UDP chat, a DNS resolver backed by a hash table, and a DHCP-style subnet calculator with IP address allocation.

Unlike the TCP programs in Exercise 5, UDP has **no connection setup** — every message is a self-contained datagram sent with `sendto()` and received with `recvfrom()`, with the sender's address attached to each packet so the server always knows where to reply.

## Programs

### 1. UDP Chat Server/Client

A request–reply chat over datagrams — the client sends a line, the server echoes back a reply. Any number of clients can contact the server; each datagram carries the client's address, which the server uses to route its reply.

| File | Description |
|------|-------------|
| `ser_chat.c` | UDP chat server — binds a port, receives datagrams from any client, replies from stdin |
| `cli_chat.c` | UDP chat client — sends a message, waits for the reply, `exit` ends the session |

**Usage:**
```bash
# Terminal 1 — Start server
gcc -o ser_chat ser_chat.c
./ser_chat 9000

# Terminal 2 — Run client
gcc -o cli_chat cli_chat.c
./cli_chat 127.0.0.1 9000
```

**Output:**
```
# Client:
Connected to chat server at 127.0.0.1:9000
Type 'exit' to end the chat.

You: hello server
Server: hi client!
You: exit
You ended the chat.

# Server:
UDP Chat Server listening on port 9000...
Server is ready for multiple clients.

Client [127.0.0.1:47178]: hello server
You: Client [127.0.0.1:47178]: exit
Client ended the chat.
Waiting for another client...
```

### 2. DNS Lookup Server/Client (Hash Table)

The server acts as a mini **DNS resolver**: it preloads 10 domain → IP mappings (google.com, github.com, …) into a **10-bucket hash table with separate chaining** (sum-of-characters hash). The client sends a domain name and receives the resolved IP address or `Domain not found`.

| File | Description |
|------|-------------|
| `ser_dns.c` | DNS server — hash-table lookup of domain names, replies with the IP address |
| `cli_dns.c` | DNS client — sends domain names entered by the user, prints the response |

**Usage:**
```bash
# Terminal 1 — Start server
gcc -o ser_dns ser_dns.c
./ser_dns 5353

# Terminal 2 — Run client
gcc -o cli_dns cli_dns.c
./cli_dns 127.0.0.1 5353
```

**Output:**
```
# Client:
Connected to DNS Server 127.0.0.1:5353
Type 'exit' to stop.

Enter Domain Name: google.com
DNS Server Response:
Domain: google.com
IP Address: 142.250.195.14

Enter Domain Name: notadomain.xyz
DNS Server Response:
Domain not found

# Server:
DNS UDP Server started on port 5353
Hash table size: 10
Server is ready for multiple clients.

Client [127.0.0.1:47086] requested: google.com
Result: Domain: google.com
IP Address: 142.250.195.14
```

### 3. DHCP Subnet & IP Allocation Server/Client

A **DHCP-style** address manager over UDP. The client supplies an IP block (e.g. `192.168.1.0/24`) and the number of subnets required; the server computes the **borrowed bits**, the new prefix, and every subnet's network / first-host / last-host / broadcast addresses and usable-host count. The client can then request allocation of free host IPs from any subnet.

**Protocol:**

```
SETUP <ip-block>/<prefix> <num-subnets>   → subnet plan
ALLOCATE <subnet-number> <count>          → free host IPs
EXIT                                     → disconnect
```

| File | Description |
|------|-------------|
| `ser_dhcp.c` | DHCP server — subnet math (borrowed bits, new prefix, ranges) + free-IP allocation with a used-host bitmap per subnet |
| `cli_dhcp.c` | DHCP client — sends SETUP/ALLOCATE/EXIT requests built from user input |

**Usage:**
```bash
# Terminal 1 — Start server
gcc -o ser_dhcp ser_dhcp.c
./ser_dhcp 6767

# Terminal 2 — Run client
gcc -o cli_dhcp cli_dhcp.c
./cli_dhcp 127.0.0.1 6767
```

**Output:**
```
# Client:
Enter IP block: 192.168.1.0/24
Enter number of subnets: 4

Subnet calculation:

Subnet 1: 192.168.1.0/26
Network: 192.168.1.0
First Host: 192.168.1.1
Last Host: 192.168.1.62
Broadcast: 192.168.1.63
Usable Hosts: 62

Subnet 2: 192.168.1.64/26
Network: 192.168.1.64
First Host: 192.168.1.65
Last Host: 192.168.1.126
Broadcast: 192.168.1.127
Usable Hosts: 62
...

Enter subnet number (0 to exit): 1
Enter number of IP addresses to allot: 2

DHCP Server Response:
IP addresses allotted successfully:
192.168.1.1
192.168.1.2

# Server:
Client [127.0.0.1:55547]
IP Block: 192.168.1.0/24
Required Subnets: 4
Borrowed Bits: 2
New Prefix: /26
```

## Key Concepts

- **UDP sockets** — Connectionless `SOCK_DGRAM` communication, no handshake, message boundaries preserved
- **`sendto()` / `recvfrom()`** — Address-carrying datagram API; the server replies to whatever client address arrived
- **Hash table with separate chaining** — Domain → IP storage with collision handling via linked lists
- **Subnetting math** — Borrowed bits, new prefix length, network/broadcast/host ranges, usable-host count
- **DHCP allocation model** — Server-side pool with a used/free bitmap per subnet
- **Connectionless session handling** — Server serves multiple clients from a single socket without per-connection state
