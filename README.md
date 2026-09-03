# 🖧 Computer Networks Laboratory

Hands-on networking lab programs implementing core data-link, error-control, routing, and socket-programming concepts in **C** (with a few Python helpers).

---

## 📚 Exercises

| Folder | Topic | Programs |
|---|---|---|
| `ex0` | **Layering** | Layered network architecture demonstration (`layering.c`) |
| `ex1` | **Data Link Layer — Framing** | **Bit stuffing**, **Byte stuffing (BISYNC)**, and **PPP** framing — sender & receiver (`bit_*`, `bisync_*`, `ppp_*`) |
| `ex2` | **Error Detection — Checksum** | Internet **checksum** sender & receiver (`checksum_sender.c`, `checksum_receiver.c`) |
| `ex3` | **Error Correction — Hamming Code** | **Hamming (7,4)** encoding & decoding sender/receiver (`hamming_sender.c`, `hamming_receiver.c`) |
| `ex4` | **Routing** | **Bellman–Ford** distance-vector routing (`bellman-ford.c`) + virtual-lab snapshots |
| `ex5` | **Socket Programming** | Multi-client **TCP client–server** chat (`server1–3.c`, `client1–3.c`) |

Each exercise folder contains its own `README.md` explaining the concept and how to run it, along with sample outputs / screenshots.

## 🛠️ Build & Run

```bash
# C programs
gcc program.c -o program && ./program

# Sockets — run the server first, then clients
gcc server1.c -o server && ./server
gcc client1.c -o client && ./client
```

> Python helpers (`*.py`) run with `python3 <file>.py`.

## 📝 Topics Covered

OSI layering · framing (bit/byte/PPP) · checksum · Hamming error correction · distance-vector routing · TCP sockets & client–server communication

---

<div align="center">
Coursework · <a href="https://github.com/Vijayaraj-IHT">Vijayaraj K P</a> · Mepco Schlenk Engineering College
</div>
