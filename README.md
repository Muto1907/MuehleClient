# Milly

**C, POSIX, LINUX:** A real‑time, tournament‑grade Nine Men’s Morris (Neunermühle) client that plays autonomously against a university‑hosted game server.

## Goals and Purpose

Our four‑person team built Milly during the LMU Systems Programming practicum as our capstone in low‑level, performance‑critical software. The goal: beat human and bot opponents under hard real‑time constraints while respecting a strict wire protocol. Everything—from network handshake to move generation—runs in C with zero external dependencies.

## Highlights

- **Two‑process architecture**\
  *Connector* (network I/O) ↔ *Thinker* (game AI) using POSIX shared memory, unnamed pipes and `SIGUSR1` for wake‑ups\
  Keeps the critical path for server communication tight while letting the AI think in parallel.

- **Inter‑process communication — shared memory, pipe, signal**\
  Two lightweight Unix primitives keep the twin processes in sync:

  - **Shared memory** – Connector writes the board into a common memory page; Thinker reads it instantly, no copying required.
  - **Unnamed pipe** – Thinker hands its chosen move back through a one‑way pipe; Connector relays it to the server.
  - **Custom Signal** – A small “tap on the shoulder” that wakes Thinker when a fresh board arrives, avoiding busy‑waiting.

- **Event‑driven socket layer**\
  Uses `epoll_wait(2)` to multiplex between the server’s TCP socket and the Thinker pipe. The Connector:

  - completes a three‑step handshake (`VERSION`, `ID`, `PLAYER`);
  - parses ` `‑terminated protocol messages with a small state machine;
  - enforces per‑message deadlines so a slow server never stalls the client.

- **Clock‑aware move generator**\
  Selects a legal move at random (deterministic seed) and writes it back just before the server’s deadline.

## 🚀 Quick Start

### Prerequisites

- **Build tools:** GCC ≥ 9 and `make`
- **Platform:** Linux or any POSIX‑compatible OS with glibc
- **Optional dev tools:** Valgrind, GDB

### Build from source

```bash
git clone https://github.com/Muto1907/MuehleClient.git
cd MuehleClient
make            # builds ./sysprak-client
```

### Join a live game

```bash
./sysprak-client -g <GAME_ID> -p <1|2>
```

- `-g` – the 13‑digit game ID assigned by the server.
- `-p` – your preferred player number (`1` or `2`).

The hostname and port are read from **client.conf** (defaults to `sysprak.priv.lab.nm.ifi.lmu.de:1357`).

> **Note:** The server is only reachable from within the Münchner Wissenschaftsnetz (MWN). If you're off‑campus, connect via the LRZ campus VPN before running the client.

### Use a custom config (optional)

```bash
./sysprak-client -g <GAME_ID> -p 1 -f test.conf
```

## 🕹️ Usage

### Play against the bot in your browser

1. Open the LMU Mühle web interface: [http://sysprak.priv.lab.nm.ifi.lmu.de](http://sysprak.priv.lab.nm.ifi.lmu.de) *(MWN/VPN required).*
2. Create a new match and note the 13‑digit **Game‑ID**.
3. In a terminal, start the client for one side:
   ```bash
   ./sysprak-client -g <GAME_ID> -p 1
   ```
4. Join the same match in the browser as the opposite player and make moves manually—the client replies in real time.

### Let two bots battle each other

```bash
# terminal 1
./sysprak-client -g <GAME_ID> -p 1
# terminal 2
./sysprak-client -g <GAME_ID> -p 2
```

Both instances connect to the same Game‑ID and play autonomously until a win or draw.

> Every board update is rendered as ASCII art in each client’s console, so you can watch the game unfold directly in the terminal.

bash ./sysprak-client -g \<GAME\_ID> -p 1 -f test.conf


## 🤝 Contributing

### Contributors

- Renate
- Meltem
- Charlotte
- Mahmut

### Clone the repo

```bash
git clone https://github.com/Muto1907/MuehleClient.git
cd MuehleClient
```

### Build variants

```bash
make         # release binary (./sysprak-client)
make debug   # adds -g for GDB-friendly symbols
```

### Memory & static analysis (recommended)

```bash
valgrind --leak-check=full ./sysprak-client -g <GAME_ID> -p 1
```

### Clean up artefacts

```bash
make clean
```

### Submit a pull request

Fork the repository and open a PR against **main**. Please:

- keep `-Wall -Werror` clean (no warnings),
- run the Valgrind smoke‑test above,
- describe *why* the change is useful.

---


> Built with 💙 at LMU Munich, 2023.

