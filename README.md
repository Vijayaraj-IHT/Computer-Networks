<img width="100%" src="https://capsule-render.vercel.app/api?type=waving&color=0:2b5876,50:2f6f9f,100:4e9acb&height=190&section=header&text=Computer%20Networks%20Lab&fontSize=50&fontColor=ffffff&animation=fadeIn&desc=Framing%20%E2%80%A2%20Error%20Control%20%E2%80%A2%20Routing%20%E2%80%A2%20TCP%2FUDP%20Servers&descSize=18&descAlignY=60"/>

<div align="center">

Hands-on networking programs implementing data-link framing, error detection/correction, routing, and client–server sockets — in **C** with a few **Python** helpers.

<br/>

![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)
![Python](https://img.shields.io/badge/Python-3776AB?style=for-the-badge&logo=python&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)
![TCP](https://img.shields.io/badge/TCP-Sockets-4169E1?style=for-the-badge)

<br/>

![Type](https://img.shields.io/badge/Type-Lab%20Coursework-6366F1?style=flat-square)
![College](https://img.shields.io/badge/Mepco%20Schlenk-Engg.%20College-0EA5E9?style=flat-square)

</div>

---

## 📚 Experiments

<table>
<tr>
<td width="50%" valign="top">

**0️⃣ `ex0` — Layering**
Layered network-architecture demo (`layering.c`).

**1️⃣ `ex1` — Data-Link Framing**
**Bit stuffing**, **byte stuffing (BISYNC)** and **PPP** framing — sender & receiver.

**2️⃣ `ex2` — Error Detection**
Internet **checksum** sender & receiver.

**3️⃣ `ex3` — Error Correction**
**Hamming (7,4)** encode/decode — sender & receiver.

</td>
<td width="50%" valign="top">

**4️⃣ `ex4` — Routing**
**Bellman–Ford** distance-vector routing (+ VLab snapshots).

**5️⃣ `ex5` — Socket Programming**
Multi-client **TCP client–server** chat (`server1–3.c`, `client1–3.c`).

**6️⃣ `ex6` — UDP Socket Programming**
UDP **chat**, **DNS resolver** (hash table) & **DHCP-style** subnet/IP allocator.

**7️⃣ `ex7` — Concurrent TCP Servers**
Multi-client chat (`fork`), **ARP simulation** (threads) & **file transfer** with checksum.

</td>
</tr>
</table>

> 📖 Every experiment folder has its own `README.md` with the concept, sample output, and screenshots.

---

## 🧠 Concepts Covered

<p align="center">
<img src="https://img.shields.io/badge/OSI%20Layering-00599C?style=flat-square"/>
<img src="https://img.shields.io/badge/Bit%20%2F%20Byte%20Stuffing-00599C?style=flat-square"/>
<img src="https://img.shields.io/badge/PPP%20Framing-00599C?style=flat-square"/>
<img src="https://img.shields.io/badge/Checksum-00599C?style=flat-square"/>
<img src="https://img.shields.io/badge/Hamming%20Code-00599C?style=flat-square"/>
<img src="https://img.shields.io/badge/Bellman--Ford-00599C?style=flat-square"/>
<img src="https://img.shields.io/badge/TCP%20Sockets-00599C?style=flat-square"/>
<img src="https://img.shields.io/badge/UDP%20Sockets-00599C?style=flat-square"/>
<img src="https://img.shields.io/badge/DNS-00599C?style=flat-square"/>
<img src="https://img.shields.io/badge/DHCP-00599C?style=flat-square"/>
<img src="https://img.shields.io/badge/ARP-00599C?style=flat-square"/>
<img src="https://img.shields.io/badge/File%20Transfer-00599C?style=flat-square"/>
<img src="https://img.shields.io/badge/Fork%20%26%20Threads-00599C?style=flat-square"/>
</p>

---

## 🛠️ Build & Run

<details open>
<summary><b>▶️ Compile a C program</b></summary>

```bash
gcc program.c -o program
./program
```
</details>

<details>
<summary><b>▶️ Run the multi-client chat (ex5)</b></summary>

```bash
# Terminal 1 — start a server first
gcc server1.c -o server && ./server
# Terminals 2..n — connect clients
gcc client1.c -o client && ./client
```
</details>

<details>
<summary><b>▶️ Run a UDP service (ex6)</b></summary>

```bash
# Terminal 1 — start a server (chat / DNS / DHCP)
gcc ser_dns.c -o ser_dns && ./ser_dns 5353
# Terminal 2 — connect a client
gcc cli_dns.c -o cli_dns && ./cli_dns 127.0.0.1 5353
```
</details>

<details>
<summary><b>▶️ Run a concurrent TCP server (ex7)</b></summary>

```bash
# ARP server needs pthreads
gcc ser_arp.c -o ser_arp -lpthread && ./ser_arp 8081
gcc cli_arp.c -o cli_arp && ./cli_arp 127.0.0.1 8081
```
</details>

<details>
<summary><b>🐍 Run a Python helper</b></summary>

```bash
python3 filename.py
```
</details>

---

<div align="center">

**Coursework · [Vijayaraj K P](https://github.com/Vijayaraj-IHT)** · Mepco Schlenk Engineering College

</div>

<img width="100%" src="https://capsule-render.vercel.app/api?type=waving&color=0:4e9acb,50:2f6f9f,100:2b5876&height=110&section=footer"/>
