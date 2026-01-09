# UPS - Semestral ork

## Topic
Create **Tic-Tac-Toe** game using **TCP/IP protocol**

## Protocol
The application uses a custom text-based protocol over TCP.
All valid messages starts with header: **OP23**

## Server
The server is written in **C++** and handles game logic and multiple client connections

### Compilation
Ensure you have `make` and `g++` installed. In the `server` folder, run:
```bash
make
```
This will create:
- **bin/**: Contains the executable server file
- **obj/**: Contains compiled object files

### Start the server
Navigate to the `bin` folder in the server folder and run this command with given parameters:
```bash
./server.exe -p <port-number> -c <number-of-clients> -r <number-of-rooms>
```

## Client
The client is written in Python using the PyQt6 framework.

### System dependencies
Before running the client on Linux, you must install the required graphical libraries:

```bash
sudo apt update && sudo apt install -y libxcb-cursor0 libxcb-xinerama0 libxcb-icccm4 libxcb-image0 libxcb-keysyms1 libxcb-render-util0 libxcb-shape0
```
### Start the client
Navigate to the client folder and run command:
- **Windows:** ``python run_client.py``
- **Linux:** ``python3 ./run_client.py``