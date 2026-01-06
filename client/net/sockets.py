"""
Filename: sockets.py
Author: Ondrej Patejdl
Contact: opatejdl@students.zcu.cz
Description: This script is used to handle the network connection of the game
"""
import socket
import threading
import time
from PyQt6.QtCore import (
    QObject, pyqtSignal, QTimer,
    Qt, QMetaObject
)
from core.protocol import *
from typing import Literal

class NetworkClient(QObject):
    """
    Class representing the network connection
    """

    # --- Signals ---
    connected = pyqtSignal()
    disconnected = pyqtSignal()
    error = pyqtSignal(str)

    # Login & Status
    loginResult = pyqtSignal(int) # login_code
    waiting = pyqtSignal()
    stateSync = pyqtSignal(str, dict)

    # Game
    gameStarted = pyqtSignal(str, str, str) # symbol, opponent, board
    gamePaused = pyqtSignal()
    gameResumed = pyqtSignal(str)
    turnUpdate = pyqtSignal(str, str) # board, next_turn
    gameResult = pyqtSignal(str, str) # result_code, winner
    gameEnded = pyqtSignal() # Lobby return


    # --- Functions ---
    def __init__(self):
        """
        Constructor of NetworkClient class
        """
        super().__init__()
        self.socket = None
        self.running = False
        self.header = PROTOCOL_HEADER

        # Client Identifier
        self.nickname = ""
        self.current_host = ""
        self.current_port = 0

        # Heartbeat setup
        self.last_response_time = 0
        self.heartbeat_timer = QTimer()
        self.heartbeat_timer.timeout.connect(self._checkConnection)
        self.heartbeat_timer.setInterval(2000)

        # Reconnect setup
        self.reconnect_active: bool = False
        self.reconnect_attempts: int = 0
        self.max_reconnect_attempts: int = 10

    def connectToServer(self, host: str, port: int, nickname: str) -> None:
        """
        Creates socket and connects to a server

        Args:
            host: ip address of server
            port: Port number
            nickname: Nickname of user
        """
        try:
            self.nickname = nickname
            # Recycle - invalid login
            if self.running and self.socket:
                if getattr(self, "current_host", None) == host and getattr(self, "current_port", None) == port:
                    print("Reusing existing connection...")
                    self.sendLogin(nickname)
                    return

            # end old connection on different server
            if self.running:
                self.disconnect()
            
            # Set up connection
            self.current_host = host
            self.current_port = port

            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect((host, port))
            self.running = True

            # Start heartbeat
            self.last_response_time = time.time()
            self.heartbeat_timer.start()

            # Thread for reading data
            self.thread = threading.Thread(target=self._receiveLoop, args=(self.socket,), daemon=True)
            self.thread.start()
            self.connected.emit()
        
        except Exception as e:
            self.error.emit(str(e))

    # ==================================================
    # --- Connection Handling and Data Receiving Func ---

    def _checkConnection(self):
        if not self.running or self.reconnect_active:
            return

        if time.time() - self.last_response_time > 6.0:
            print("Heartbeat timeout! Server neodpovídá.")
            # Zastavíme timer okamžitě, aby se nespouštěl reconnect duplicitně
            QMetaObject.invokeMethod(self.heartbeat_timer, "stop", Qt.ConnectionType.QueuedConnection)
            
            self.error.emit("Connection timed out (Lost connection).")
            self._startReconnect() 
            return

        self.sendPing()

    def _receiveLoop(self, sock):
        """
        Main loop for receiving messages
        """
        buffer = ""
        try:
            while self.running:

                # Ensure socket change (reconnect)
                if sock != self.socket:
                    break

                try:
                    raw_data = self.socket.recv(MAX_BUFFER_SIZE)
                    if not raw_data:
                        print("Server closed connection.")
                        break
                    
                    # Server is alive
                    self.last_response_time = time.time()

                    data = raw_data.decode("utf-8", errors="replace")

                    buffer += data
                    while "\n" in buffer:
                        line, buffer = buffer.split("\n", 1)
                        self._handleMsg(line)

                except OSError as e:
                    if self.running:
                        print(f"Socket error: {e}")
                    break

        except Exception as e:
            print(f"Critical Loop Error: {e}")
        finally:
            if not self.reconnect_active:
                self.disconnect()

    def _handleMsg(self, msg: str):
        """
        Message handling function
        """
        if not msg.startswith(self.header):
            return

        payload = msg[len(self.header):]
        parts = payload.split(SPLITTER)
        cmd = parts[0]

        # Authentication check
        if cmd == "AUTH" and parts[1] == "1":
            self.sendLogin(self.nickname)

        # Response for login
        elif cmd == "LOGIN":
            res_code = int(parts[1])
            self.loginResult.emit(res_code)

        # Waiting room
        elif cmd == "WAITING":
            self.waiting.emit()

        elif cmd == "GAME":
            sub_cmd = parts[1]
            if sub_cmd.startswith("START_"):
                # Format: GAME|<start_symbol>|<opponent_nick>|<board>
                my_symbol = sub_cmd.split("_")[1]
                opponent = parts[2]
                board = parts[3] if len(parts) > 3 else " "*9
                self.gameStarted.emit(my_symbol, opponent, board)
            
            elif sub_cmd == "PAUSED":
                self.gamePaused.emit()
            
            elif sub_cmd == "RESUMED":
                turn_sym = parts[2] if len(parts) > 2 else ""
                self.gameResumed.emit(turn_sym)

            elif sub_cmd == "ENDED":
                self.gameEnded.emit()

        elif cmd == "TURN":
            # Format: TURN|VALID_MOVE|<board>|<next_turn>
            code = parts[1]
            if code == "VALID_MOVE" or code == "0": 
                board = parts[2]
                next_turn = parts[3]
                self.turnUpdate.emit(board, next_turn)

        elif cmd == "RESULT":
            # Format: RESULT|WIN|<board>|<winner> OR RESULT|DRAW|<board>
            res_code = "WIN" if parts[1] == "0" else "DRAW"
            board = parts[2]
            winner = parts[3] if len(parts) > 3 else ""
            
            # Update board one last time
            self.turnUpdate.emit(board, "-") 
            self.gameResult.emit(res_code, winner)

        elif cmd == "SYNC":
            # Format: SYNC|GAME|<symbol>|<board>|<turn>|<opponent>
            # OR: SYNC|LOBBY or SYNC|WAITING
            state = parts[1]
            data = {}
            if (state == "GAME"):
                data = {
                    "symbol": parts[2],
                    "board": parts[3],
                    "turn": parts[4],
                    "opponent": parts[5] if len(parts) > 5 else "Unknown"
                }
            
            elif (state == "RESULT"):
                # SYNC|RESULT|<opponent_nick>|<board>|<winner_nick>
                if len(parts) >= 5:
                    data = {
                        "opponent": parts[2],
                        "board": parts[3],
                        "winner": parts[4]
                    }
            
            self.stateSync.emit(state, data)

        elif (cmd == "PONG"):
            return

    def disconnect(self):
        """
        Set its self to disconnected form
        """
        self.running = False
        QMetaObject.invokeMethod(self.heartbeat_timer, "stop", Qt.ConnectionType.QueuedConnection)
        if (self.socket):
            try:
                self.socket.close()
            except:
                pass
            self.socket = None
        self.disconnected.emit()

    # ==================================
    # ----- Function fro MSG send -----

    def sendSync(self):
        """
        Sends sync msg to server to find out the current state of user
        """
        if self.running and self.socket:
            try:
                msg = f"{self.header}SYNC\n"
                self.socket.sendall(msg.encode("utf-8"))
            except:
                self.error.emit("Failed to send SYNC")

    def sendMove(self, x, y):
        """
        Sends move msg to server

        Args:
            x: Coordinate of X on the game board
            y: Coordinate of Y on the game board
        """
        if self.running and self.socket:
            try:
                msg = f"{self.header}MOVE|{x}|{y}\n"
                self.socket.sendall(msg.encode("utf-8"))
            except:
                self.error.emit("Failed to send MOVE")

    def sendFindRequest(self):
        """
        Sent find request to server
        """
        if self.running:
            try:
                self.socket.sendall(f"{self.header}FIND\n".encode("utf-8"))
            except:
                pass

    def sendPing(self):
        """
        Function sends PING msg to server
        """
        if self.running and self.socket:
            try:
                msg = f"{self.header}PING\n"
                self.socket.sendall(msg.encode("utf-8"))
            except:
                pass

    def sendLogin(self, nickname: str):
        if self.running and self.socket:
            try:
                login_msg = f"{self.header}LOGIN{SPLITTER}{nickname}\n"
                self.socket.sendall(login_msg.encode("utf-8"))
            except:
                pass

    def sendRematch(self):
        """
        Sends status for rematch of game
        """
        # Format REMATCH|<status>\n
        if self.running and self.socket:
            try:
                msg = f"{self.header}REMATCH{PROTOCOL_ENDING}"
                self.socket.sendall(msg.encode("utf-8"))
            except:
                self.error.emit("Failed to send REMATCH")

    def sendLeave(self):
        """
        Sends request to get back to lobby
        """
        if self.running and self.socket:
            try:
                msg = f"{self.header}LEAVE{PROTOCOL_ENDING}"
                self.socket.sendall(msg.encode("utf-8"))
            except:
                pass

    def sendDisconnect(self):
        """
        Disconnects client from server and leads to login scene
        """
        if self.running and self.socket:
            try:
                msg = f"{self.header}DISCONNECT{PROTOCOL_ENDING}"
                self.socket.sendall(msg.encode("utf-8"))
            except:
                pass

    # Reconnect Logic
    def _startReconnect(self):
        self.reconnect_active = True
        self.socket.close()
        
        # Spustíme reconnect ve vlákně, aby nezamrzlo GUI
        threading.Thread(target=self._reconnectLoop, daemon=True).start()

    def _reconnectLoop(self):
        self.reconnect_attempts = 0
        while self.reconnect_attempts < self.max_reconnect_attempts:
            self.reconnect_attempts += 1
            print(f"🔄 Pokus o reconnect {self.reconnect_attempts}/{self.max_reconnect_attempts}")
            
            try:
                # New socket
                new_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                new_sock.settimeout(5) # Nedovolíme connectu viset věčně
                new_sock.connect((self.current_host, self.current_port))
                new_sock.settimeout(None)
                
                self.socket = new_sock
                self.running = True

                # Create new thread
                self.thread = threading.Thread(target=self._receiveLoop, args=(self.socket,), daemon=True)
                self.thread.start()

                self.sendLogin(self.nickname)
                
                self.reconnect_active = False
                self.reconnect_attempts = 0
                self.last_response_time = time.time()

                # Restartujeme heartbeat timer
                QMetaObject.invokeMethod(self.heartbeat_timer, "start", Qt.ConnectionType.QueuedConnection)

                print("✅ Reconnect úspěšný!")
                return

            except Exception as e:
                print(f"Attempt failed: {e}")
                time.sleep(2)

        # Attempts exceeded
        print("Unable to reconnect to server.")
        self.reconnect_active = False
        self.disconnect()