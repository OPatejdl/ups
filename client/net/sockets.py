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
    connected = pyqtSignal()
    disconnected = pyqtSignal()
    error = pyqtSignal(str)
    loginResult = pyqtSignal(int)

    def __init__(self):
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

        # Authentication check
        if parts[0] == "AUTH" and parts[1] == "1":
            login_msg = f"{self.header}LOGIN{SPLITTER}{nickname}\n"
            self.socket.sendall(login_msg.encode('utf-8'))

        # Response for login
        elif parts[0] == "LOGIN":
            res_code = int(parts[1])
            self.loginResult.emit(res_code)

    def disconnect(self):
        """
        Set its self to disconnected form
        """
        self.running = False
        if (self.socket):
            self.socket.close()
        self.disconnected.emit()