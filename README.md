<img width="100%" src="https://capsule-render.vercel.app/api?type=waving&color=0:2b5876,50:2f6f9f,100:4e9acb&height=190&section=header&text=Computer%20Networks%20Lab&fontSize=50&fontColor=ffffff&animation=fadeIn&desc=Framing%20%E2%80%A2%20Error%20Control%20%E2%80%A2%20Routing%20%E2%80%A2%20Sockets&descSize=18&descAlignY=60"/>

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

</td>
<td width="50%" valign="top">

**3️⃣ `ex3` — Error Correction**
**Hamming (7,4)** encode/decode — sender & receiver.

**4️⃣ `ex4` — Routing**
**Bellman–Ford** distance-vector routing (+ VLab snapshots).

**5️⃣ `ex5` — Socket Programming**
Multi-client **TCP client–server** chat (`server1–3.c`, `client1–3.c`).

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
