"""
Filename: sockets.py
Author: Ondrej Patejdl
Contact: opatejdl@students.zcu.cz
Date: 2025-12-28
Version: 0.1.0
Description: TODO
"""
import socket
import threading
from PyQt6.QtCore import QObject, pyqtSignal
from core.protocol import *

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
    gameResumed = pyqtSignal()
    turnUpdate = pyqtSignal(str, str) # board, next_turn
    gameResult = pyqtSignal(str, str) # result_code, winner


    # --- Functions ---
    def __init__(self):
        """
        Constructor of NetworkClient class
        """
        super().__init__()
        self.socket = None
        self.running = False
        self.header = HEADER

    def connectToServer(self, host: str, port: int, nickname: str) -> None:
        """
        Creates socket and connects to a server

        Args:
            host: ip address of server
            port: Port number
            nickname: Nickname of user
        """
        try:
            # Set up connection
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect((host, port))
            self.running = True

            # Thread for reading data
            self.thread = threading.Thread(target=self._receiveLoop, args=(nickname,), daemon=True)
            self.thread.start()
            self.connected.emit()
        except Exception as e:
            self.error.emit(str(e))

    def _receiveLoop(self, nickname):
        """
        Main loop for receiving messages
        """
        buffer = ""
        try:
            while self.running:
                data = self.socket.recv(MAX_BUFFER_SIZE).decode('utf-8')
                if not data:
                    break
                
                buffer += data
                while "\n" in buffer:
                    line, buffer = buffer.split("\n", 1)
                    self._handleMsg(line, nickname)
        except Exception as e:
            print(f"Network error: {e}")
        finally:
            self.disconnect()

    def _handleMsg(self, msg, nickname):
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
            login_msg = f"{self.header}LOGIN{SPLITTER}{nickname}\n"
            self.socket.sendall(login_msg.encode('utf-8'))

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
                self.gameResumed.emit()

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

        elif cmd == "PONG":
            # Format: PONG|GAME|<symbol>|<board>|<turn>|<opponent>
            # OR: PONG|LOBBY or PONG|WAITING
            state = parts[1]
            data = {}
            if state == "GAME":
                data = {
                    "symbol": parts[2],
                    "board": parts[3],
                    "turn": parts[4],
                    "opponent": parts[5] if len(parts) > 5 else "Unknown"
                }
            self.stateSync.emit(state, data)

    def sendPing(self):
        """
        Sends ping msg to server to find out the current state of user
        """
        if self.running and self.socket:
            try:
                msg = f"{self.header}PING\n"
                self.socket.sendall(msg.encode('utf-8'))
            except:
                self.error.emit("Failed to send PING")

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
                self.socket.sendall(msg.encode('utf-8'))
            except:
                self.error.emit("Failed to send MOVE")

    def disconnect(self):
        """
        Set its self to disconnected form
        """
        self.running = False
        if (self.socket):
            self.socket.close()
        self.disconnected.emit()

    def sentFindRequest(self):
        """
        Sent find request to server
        """
        if self.running:
            self.socket.sendall(f"{self.header}FIND\n".encode('utf-8'))