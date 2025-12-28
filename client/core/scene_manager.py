"""
Filename: scene_manager.py
Author: Ondrej Patejdl
Contact: opatejdl@students.zcu.cz
Date: 2025-10-08
Version: 0.1.0
Description: This script managing user data and scene content
"""
from PyQt6.QtWidgets import (
    QMainWindow, QStackedWidget)
from PyQt6.QtCore import (
    Qt, pyqtSlot)
from core.constants import (
    WINDOW_NAME,
    DEFAULT_X_POS, DEFAULT_Y_POS,
    DEFAULT_HEIGH, DEFAULT_WIDTH,
)
from scenes.login import LoginScene
from scenes.lobby import LobbyScene
from net.sockets import NetworkClient


class User:
    """
    Class representing a user of a game
        - Stores user's nick and state
    """
    ...


class MainWindow(QMainWindow):

    def __init__(self):
        super().__init__()

        # Network setup
        self.network = NetworkClient()
        self.network.loginResult.connect(self.onLoginResult)
        self.network.error.connect(lambda err: print(f"Connection Error: {err}"))

        self.scene_manager = QStackedWidget(self)

        # Login Scene 
        self.login_scene = LoginScene()
        self.scene_manager.addWidget(self.login_scene)
        self.login_scene.loginRequest.connect(self.loginRequestHandler)

        # Lobby Scene
        self.lobby_scene = LobbyScene()
        self.scene_manager.addWidget(self.lobby_scene)
        # self.lobby_scene.findGameRequest.connect(self.findGameRequestHandler)
        self.lobby_scene.exitRequest.connect(self.exitRequestHandler)

        self._setUI()

    def _setUI(self):
        """
        Set up the UI
        """

        # Set up Title, Size and Layout
        self.setWindowTitle(WINDOW_NAME)
        self.setGeometry(DEFAULT_X_POS, DEFAULT_Y_POS,
                         DEFAULT_WIDTH, DEFAULT_HEIGH)

        self.setCentralWidget(self.scene_manager)

        self.scene_manager.setCurrentWidget(self.login_scene)

    @pyqtSlot(str, str, int)
    def loginRequestHandler(self, username: str, ip: str, port: int):
        """
        Function handles exitRequest signal emitted by login_btn in the Login scene

        Args:
            username: 
                str; represents name of user
            ip: 
                str; represents IP address to which user want to connect
            port: 
                int; represent PORT, to which user want to connect
        """
        self.current_nick = username
        self.current_ip = ip
        self.current_port = port
        self.network.connectToServer(ip, port, username)

    def onLoginResult(self, code):
        if (code == 0 or code == 1):
            self.lobby_scene.updateInfo(self.current_nick, f"{self.current_ip}:{self.current_port}")
            self.scene_manager.setCurrentWidget(self.lobby_scene)
        else:
            print(f"Login failed with code {code}")

    @pyqtSlot()
    def exitRequestHandler(self):
        """
        Function handles exitRequest signal emitted by exit_btn in the Lobby scene
        """
        self.scene_manager.setCurrentWidget(self.login_scene)
