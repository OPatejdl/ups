# Tic-Tac-Toe over TCP

A multiplayer **Tic-Tac-Toe** game built on a custom text protocol over **TCP/IP**.
It has two parts: a **C++ server** that runs the game and serves many clients at once, and a **Python (PyQt6) desktop client**.

> Semestral work for the **UPS** (Introduction to Computer Networks) course.
> Full documentation is in [doc/ticTacToe_doc.pdf](doc/ticTacToe_doc.pdf) - only in czech.

---

## Features

- **Online 1v1 matches.** Players queue up and are paired into game rooms automatically.
- **Many clients at once.** The server handles all connections with `select()`, and you choose the client and room limits at startup.
- **Reconnects.** A player who drops out has **60 s** to come back and continue the same game. The opponent sees the game as *paused* until then.
- **Heartbeat.** The client sends `PING` every 2 s. The server treats a client as inactive after 6 s of silence.
- **Rematch.** After a game, both players can vote for a rematch without going back to the queue.
- **Protocol validation.** Every message must start with the `OP23|` header. A client that sends invalid or malformed data is disconnected.
- **Logging.** Both the server and the client write log files, which makes debugging easier.

---

## Project Structure

```
.
├── server/                 # C++17 game server
│   ├── makefile
│   └── src/
│       ├── main.cpp
│       ├── config.hpp          # timeouts, limits, defaults
│       ├── protocolConfig.hpp  # protocol constants & response codes
│       ├── server/             # socket handling, command dispatch
│       ├── roomHandling/       # rooms, game logic, matchmaking
│       ├── userHandling/       # users, login, reconnects
│       ├── logger/  exceptions/  utility/
├── client/                 # Python PyQt6 GUI client
│   ├── run_client.py           # launcher (creates venv + installs deps)
│   ├── pack_requirements.txt
│   └── src/
│       ├── client.py           # entry point
│       ├── core/               # protocol constants, scene manager, styling
│       ├── net/sockets.py      # networking, heartbeat, reconnect
│       └── scenes/             # login, lobby, waiting, game screens
└── doc/
    └── ticTacToe_doc.pdf   # full project documentation
```

---

## Requirements

| Component | Requirements |
|-----------|--------------|
| **Server** | Linux / POSIX system (or WSL), `g++` with C++17 support, `make` |
| **Client** | Python **3.9+** (PyQt6 is installed automatically), Windows or Linux |

---

## Quick Start

### 1. Build and run the server

```bash
cd server
make
cd bin
./server.exe -p 10000 -c 10 -r 5
```

`make` creates:
- **`bin/`**, which holds the server executable (`server.exe`)
- **`obj/`**, which holds the compiled object files

Run `make clean` to remove both.

#### Server parameters

| Flag | Required | Description | Constraints |
|------|:--------:|-------------|-------------|
| `-p <port>` | yes | Port the server listens on | `1024`–`65535` |
| `-c <clients>` | yes | Maximum number of connected clients | at least `2` |
| `-r <rooms>` | yes | Maximum number of game rooms | at least `1` |
| `-a <ip>` | no | IPv4 address to bind to | valid IPv4, default `0.0.0.0` |

```bash
# Example: bind only to a specific interface
./server.exe -p 2222 -c 20 -r 10 -a 192.168.1.50
```

Stop the server with **Ctrl+C**. Logs are written to `server/logs/server.log`.

### 2. Run the client

**Linux only:** first install the system libraries that Qt needs:

```bash
sudo apt update && sudo apt install -y libxcb-cursor0 libxcb-xinerama0 libxcb-icccm4 \
  libxcb-image0 libxcb-keysyms1 libxcb-render-util0 libxcb-shape0
```

Then start the client from the `client` folder:

| OS | Command |
|----|---------|
| **Windows** | `python run_client.py` |
| **Linux** | `python3 ./run_client.py` |

The first launch creates a virtual environment in `client/.venv` and installs the dependencies from `pack_requirements.txt`. Later launches start straight away.

---

## How to Play

1. **Log in.** Enter:
   - **Nickname:** 4–12 characters. It must be unique on the server.
   - **IP Address:** the address of the machine running the server (e.g. `127.0.0.1` for local play).
   - **Port:** the port the server was started with (`-p`).
2. **Lobby.** Click **FIND GAME** to join the matchmaking queue, or **EXIT** to disconnect.
3. **Waiting.** The client waits for a second player. **Back to lobby** cancels the search.
4. **Game.**
   - The first player plays **X** and the second plays **O**.
   - The status bar shows whose turn it is. Click an empty tile to place your symbol.
   - Get three in a row horizontally, vertically or diagonally to win.
5. **Result.** Choose one of these:
   - **REMATCH** plays again with the same opponent. The new game starts once both players agree.
   - **BACK TO LOBBY** leaves the room.

> **Tip:** To try it on one machine, start the server and then open **two** clients that both connect to `127.0.0.1`.

### Connection issues
- If the connection drops, the client tries to reconnect by itself (up to 10 attempts, 2 s apart).
- If you reconnect within **60 s**, you return to the exact state you left: the waiting queue, a running game or the result screen.
- If you leave a game, your opponent is sent back to the lobby.

---

## Protocol Overview

The client and server exchange plain-text messages over TCP:

```
OP23|<COMMAND>|<param1>|<param2>|...\n
```

- Header: `OP23|`
- Separator: `|`
- Terminator: `\n`
- The board is sent as a 9-character string, read row by row: `X`, `O`, or a space for an empty tile.

### Client → Server

| Message | Meaning |
|---------|---------|
| `LOGIN\|<nickname>` | Log in, or reconnect with an existing nickname |
| `FIND` | Join matchmaking |
| `MOVE\|<x>\|<y>` | Place a symbol at column `x`, row `y` (`0`–`2`) |
| `REMATCH` | Vote for a rematch |
| `LEAVE` | Leave the current room |
| `SYNC` | Ask for the current state (after a reconnect) |
| `PING` | Heartbeat |
| `DISCONNECT` | Log out gracefully |

### Server → Client

| Message | Meaning |
|---------|---------|
| `LOGIN\|<code>` | `0` logged in · `1` reconnected · `2` server full · `3` nickname taken · `4` nickname too long · `5` nickname too short |
| `WAITING\|<code>` | `0` in queue · `1` all rooms are full |
| `GAME\|START_<X/O>\|<opponent>\|<board>` | Game started, with your symbol |
| `GAME\|PAUSED` / `GAME\|RESUMED\|<turn>` | Opponent disconnected / came back |
| `GAME\|REMATCH_WAIT` | Waiting for the opponent's rematch vote |
| `GAME\|ENDED` | Opponent left, back to the lobby |
| `TURN\|<code>\|<board>\|<next>` | `0` valid · `1` not your turn · `2` invalid move · `3` occupied · `4` not your game · `5` game not running |
| `RESULT\|<code>\|<board>[\|<winner>]` | `0` win (with the winner's nickname) · `1` draw |
| `SYNC\|LOBBY` · `SYNC\|WAITING` · `SYNC\|GAME\|...` · `SYNC\|RESULT\|...` | State restore after a reconnect |
| `PONG\|<state>` | Heartbeat reply with the player's current state |

See [doc/ticTacToe_doc.pdf](doc/ticTacToe_doc.pdf) for the full protocol and state diagrams.

---

## Tech Stack

- **Server:** C++17, POSIX sockets, `select()` multiplexing, Make
- **Client:** Python 3, PyQt6

## Author

**Ondřej Patejdl**
