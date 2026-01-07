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
from src.core.protocol import *
from src.core.constants import *
import logging

logger = logging.getLogger(f"{LOG_NAME}.{__name__}")

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
    turnError = pyqtSignal(str)
    gameResult = pyqtSignal(str, str) # result_code, winner
    gameEnded = pyqtSignal() # Lobby return
    rematchWait = pyqtSignal() # Information about rematch request


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
        self.current_port = INIT_PORT

        # Heartbeat setup
        self.last_response_time = INIT_LAST_RESPONSE
        self.heartbeat_timer = QTimer()
        self.heartbeat_timer.timeout.connect(self._checkConnection)
        self.heartbeat_timer.setInterval(HEARTBEAT_TIME)

        # Reconnect setup
        self.reconnect_active: bool = False
        self.reconnect_attempts: int = INIT_RECONNECT_ATTEMPTS
        self.max_reconnect_attempts: int = MAX_ATTEMPTS

    def connectToServer(self, host: str, port: int, nickname: str) -> None:
        """
        Creates socket and connects to a server

        Args:
            host: ip address of server
            port: Port number
            nickname: Nickname of user
        """
        logger.info(f"Connecting to server {host}:{port} as {nickname}...")
        try:
            self.nickname = nickname

            # Recycle - invalid login
            if self.running and self.socket:
                if getattr(self, "current_host", None) == host and getattr(self, "current_port", None) == port:
                    logger.info("Reusing current connection")
                    self.sendLogin(nickname)
                    return

            # End old connection on different server
            if self.running:
                self.disconnect()
            
            # Set up connection
            self.current_host = host
            self.current_port = port

            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

            # Connection attempt
            self.socket.settimeout(CONNECTION_TIMEOUT)
            self.socket.connect((host, port))
            self.socket.settimeout(None)

            self.running = True

            self.last_response_time = time.time()

            # Thread for reading data
            self.thread = threading.Thread(target=self._receiveLoop, args=(self.socket,), daemon=True)
            self.thread.start()
            self.connected.emit()
        
        except socket.gaierror:
            # Invalid format
            logger.error("Connection error - Invalid IP address or Hostname")
            self.error.emit("Invalid IP address or Hostname")

        except ConnectionRefusedError:
            # Server doesn't listen on this port
            logger.error("Refused connection - validate Ip address and hostname")
            self.error.emit("Server refused connection (Validate IP address and Port number)")

        except socket.timeout:
            # Unreachable server
            logger.error("Connection attempt failed - Unreachable server")
            self.error.emit("Connection attempt timed out (Server is unreachable)")

        except Exception as e:
            # Any other exception
            logger.error(f"Connection failed {str(e)}")
            self.error.emit(f"Connection failed: {str(e)}")

    # ==================================================
    # --- Connection Handling and Data Receiving Func ---

    def _checkConnection(self):
        if not self.running or self.reconnect_active:
            return

        if time.time() - self.last_response_time > MAX_HEARTBEAT:
            logger.warning("Heartbeat timeout! - No response from server")

            # Stop timer to refused duplicity reconnection attempt
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
                        logger.warning("Server closed connection.")
                        break
                    
                    # Server is alive
                    self.last_response_time = time.time()

                    data = raw_data.decode("utf-8", errors="replace")

                    buffer += data
                    while PROTOCOL_ENDING in buffer:
                        line, buffer = buffer.split(PROTOCOL_ENDING, 1)
                        self._handleMsg(line)

                except OSError as e:
                    if self.running:
                        logger.error(f"Socket error: {e}")
                    break

        except Exception as e:
            logger.error(f"Critical Loop Error: {e}")
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
        if (len(parts) < MIN_ARGS):
            return

        cmd = parts[CMD_POS]

        # Authentication check
        if cmd == "AUTH" and parts[NUM_INDEX] == "1":
            logger.info("AUTH msg received - authorize your self!")
            self.sendLogin(self.nickname)

        # Response for login
        elif cmd == "LOGIN":
            if (len(parts) != LOGIN_ARGS):
                return

            res_code = int(parts[CODE_POS])

            if res_code in [LOGIN_SUCCESS, LOGIN_RECONNECT]:
                logger.info("Login successful")

                QMetaObject.invokeMethod(
                    self.heartbeat_timer, 
                    "start", 
                    Qt.ConnectionType.QueuedConnection
                )
            self.loginResult.emit(res_code)

        # Waiting room
        elif cmd == "WAITING":
            self.waiting.emit()

        elif cmd == "GAME":
            if len(parts) < GAME_ARGS:
                return

            sub_cmd = parts[GAME_CODE]
            if sub_cmd.startswith("START_"):
                # Format: GAME|<start_symbol>|<opponent_nick>|<board>
                if len(parts) != GAME_START_ARGS:
                    return

                my_symbol = sub_cmd.split("_")[GAME_START_SYM]
                opponent = parts[GAME_START_NICK]
                board = parts[GAME_START_BOARD]

                logger.info(f"New game starts - opponent: {opponent}")
                self.gameStarted.emit(my_symbol, opponent, board)
            
            elif sub_cmd == "PAUSED":
                logger.info("Game paused - opponent disconnected..")
                self.gamePaused.emit()
            
            elif sub_cmd == "RESUMED":
                logger.info("Game resume - opponent reconnected...")
                turn_sym = parts[GAME_RESUME_TURN] if len(parts) == GAME_RESUME_ARGS else ""
                self.gameResumed.emit(turn_sym)

            elif sub_cmd == "ENDED":
                logger.info("Game ended")
                self.gameEnded.emit()

            elif sub_cmd == "REMATCH_WAIT":
                logger.info("Server confirmed rematch request, waiting for opponent.")
                self.rematchWait.emit()

        elif cmd == "TURN":
            # Format: TURN|<code>|<board>|<next_turn>
            if len(parts) < TURN_ARGS:
                return

            code = parts[TURN_CODE]
            board = parts[TURN_BOARD]
            next_turn = parts[TURN_NEXT]

            self.turnUpdate.emit(board, next_turn)

            if code != TURN_VALID:
                # Invalid turn by a player
                logger.info("Made invalid move")
                reasons = {
                    TURN_NOT_YOUR: "It's not your turn!",
                    TURN_INVALID_MOVE: "Invalid move!",
                    TURN_OCCUPIED_FILED: "This field is already occupied!",
                    TURN_NOT_BELONG: "Move rejected: Unauthorized player.",
                    TURN_NOT_RUN: "The game is not currently running."
                }
                msg = reasons.get(code, f"Move rejected (Error {code})")
                self.turnError.emit(msg)
            else:
                logger.info("Made valid move")

        elif cmd == "RESULT":
            # Format: RESULT|WIN|<board>|<winner> OR RESULT|DRAW|<board>
            if len(parts) < RESULT_MIN_ARGS:
                return
            
            res_code = "WIN" if parts[RESULT_RESULT] == RESULT_WINNER_CODE else "DRAW"
            board = parts[RESULT_BOARD]
            winner = parts[RESULT_WINNER] if len(parts) > RESULT_MIN_ARGS else ""

            logger.info(f"Game finished - game status is {res_code}")
            
            # Update board one last time
            self.turnUpdate.emit(board, "-") 
            self.gameResult.emit(res_code, winner)

        elif cmd == "SYNC":
            # Format: SYNC|GAME|<symbol>|<board>|<turn>|<opponent>
            # OR: SYNC|LOBBY ...
            if len(parts) != SYNC_MIN_ARGS:
                return

            state = parts[SYNC_STATE]
            data = {}
            if (state == "GAME"):
                if len(parts) >= SYNC_GAME_ARGS:
                    data = {
                        "symbol": parts[SYNC_GAME_SYM],
                        "board": parts[SYNC_GAME_BOARD],
                        "turn": parts[SYNC_GAME_TURN],
                        "opponent": parts[SYNC_GAME_OPPONENT]
                    }
            
            elif state == "RESULT":
                # SYNC|RESULT|<opponent_nick>|<board>|<winner_nick>
                if len(parts) >= SYNC_RESULT_ARGS:
                    data = {
                        "opponent": parts[SYNC_RESULT_OPPONENT],
                        "board": parts[SYNC_RESULT_BOARD],
                        "winner": parts[SYNC_RESULT_WINNER]
                    }

            logger.info("Synchronizing game...")

            self.stateSync.emit(state, data)

        elif cmd == "PONG":
            return

    def disconnect(self):
        """
        Set its self to disconnected form
        """
        logger.warning("Setting client in to disconnected state")
        self.running = False

        QMetaObject.invokeMethod(self.heartbeat_timer, "stop", Qt.ConnectionType.QueuedConnection)

        if self.socket:
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

                logger.info("Sending request for synchronization")
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

                logger.info("Sending my move on the board")
            except:
                self.error.emit("Failed to send MOVE")

    def sendFindRequest(self):
        """
        Send find request to server
        """
        if self.running:
            try:
                self.socket.sendall(f"{self.header}FIND\n".encode("utf-8"))

                logger.info("Sending request to FIND a game")
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

                logger.info("Checking connection - PING msg")
            except:
                pass

    def sendLogin(self, nickname: str):
        if self.running and self.socket:
            try:
                login_msg = f"{self.header}LOGIN{SPLITTER}{nickname}\n"
                self.socket.sendall(login_msg.encode("utf-8"))

                logger.info(f"Sending login request with nickname {nickname}")
            except:
                pass

    def sendRematch(self):
        """
        Sends status for rematch of game
        """
        # Format REMATCH\n
        if self.running and self.socket:
            try:
                msg = f"{self.header}REMATCH{PROTOCOL_ENDING}"
                self.socket.sendall(msg.encode("utf-8"))

                logger.info("Sending request for rematch")
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

                logger.info("Sending request to get back to lobby")
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

                logger.warning("Disconnecting from server - my decision")
            except:
                pass

    ##########################################################
    # ----- Reconnect Logic -----

    def _startReconnect(self):
        """
        Called to start reconnection process
        """
        self.reconnect_active = True
        self.socket.close()
        
        # Reconnect attempt on the new thread
        threading.Thread(target=self._reconnectLoop, daemon=True).start()

    def _reconnectLoop(self):
        self.reconnect_attempts = INIT_RECONNECT_ATTEMPTS

        logger.info("Reconnecting...")

        while self.reconnect_attempts < self.max_reconnect_attempts:
            self.reconnect_attempts += 1
            logger.info(f"Reconnect attempt {self.reconnect_attempts}/{self.max_reconnect_attempts}")
            
            try:
                # New socket
                new_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

                new_sock.settimeout(CONNECTION_TIMEOUT)
                new_sock.connect((self.current_host, self.current_port))
                new_sock.settimeout(None)
                
                self.socket = new_sock
                self.running = True

                # Create new thread
                self.thread = threading.Thread(target=self._receiveLoop, args=(self.socket,), daemon=True)
                self.thread.start()

                self.reconnect_active = False
                self.reconnect_attempts = INIT_RECONNECT_ATTEMPTS
                self.last_response_time = time.time()

                # Restart heartbeat
                QMetaObject.invokeMethod(self.heartbeat_timer, "start", Qt.ConnectionType.QueuedConnection)

                logger.info("RECONNECTED!")
                return

            except Exception as e:
                logger.warning(f"\tAttempt failed: {e}")
                time.sleep(2)

        # Attempts exceeded
        logger.error("Unable to reconnect to server...")
        self.reconnect_active = False
        self.disconnect()