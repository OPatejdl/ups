"""
Filename: scene_manager.py
Author: Ondrej Patejdl
Contact: opatejdl@students.zcu.cz
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
from scenes.waiting import WaitingScene
from scenes.game import GameScene
from net.sockets import NetworkClient

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
        self.lobby_scene.findGameRequest.connect(self.findGameRequestHandler)
        self.lobby_scene.exitRequest.connect(self.exitRequestHandler)

        # Waiting Scene
        self.waiting_scene = WaitingScene()
        self.scene_manager.addWidget(self.waiting_scene)

        # Game Scene
        self.game_scene = GameScene()
        self.scene_manager.addWidget(self.game_scene)
        self.game_scene.moveRequest.connect(self.network.sendMove)
        self.game_scene.backToLobbyRequest.connect(self.network.sendLeave)
        self.game_scene.rematchRequest.connect(self.network.sendRematch)

        # Network Logic Connections
        self.network.waiting.connect(self.onWaiting)
        self.network.gameStarted.connect(self.onGameStarted)
        self.network.stateSync.connect(self.onStateSync)
        self.network.gameEnded.connect(self.onBackToLobby)
        self.network.disconnected.connect(lambda: self.onDisconnected("Disconnected from server"))
        self.network.error.connect(self.onNetworkError)

        # In-Game Updates
        self.network.turnUpdate.connect(self.game_scene.updateBoard)
        self.network.gamePaused.connect(self.game_scene.setPaused)
        self.network.gameResumed.connect(self.game_scene.setResumed)
        self.network.gameResult.connect(self.game_scene.handleResult)

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
        """
        Handles LOGIN msg from server

        Args:
            code - Specifies response on login request
        """
        # Set up msg
        if (code < 2):
            self.login_scene.setErrorMsg("")
        else:
            reasons = {
                2: "Server is full",
                3: "Nickname already taken.",
                4: "Nickname is too long.",
                5: "Nickname is too short."
            }

            msg = reasons.get(code, f"Login failed (Error {code})")
            self.login_scene.setErrorMsg(msg)

        if (code == 0):
            self.lobby_scene.updateInfo(self.current_nick, f"{self.current_ip}:{self.current_port}")
            self.scene_manager.setCurrentWidget(self.lobby_scene)
        elif (code == 1):
            # Reconnect successful
            print("Reconnected! Sending SYNC to sync state...")
            self.lobby_scene.updateInfo(self.current_nick, f"{self.current_ip}:{self.current_port}")
            self.network.sendSync()
        else:
            print(f"Login failed with code {code}")

    @pyqtSlot()
    def exitRequestHandler(self):
        """
        Handles exitRequest signal emitted by exit_btn in the Lobby scene
        """
        self.network.sendDisconnect()
        self.network.disconnect()
        self.scene_manager.setCurrentWidget(self.login_scene)

    @pyqtSlot()
    def findGameRequestHandler(self):
        """
        Handles findGameRequest signal emitted by findGame_btn
        """
        self.network.sendFindRequest()

    def onWaiting(self):
        """
        Called when server puts user in waiting room
        """
        self.scene_manager.setCurrentWidget(self.waiting_scene)

    def onGameStarted(self, symbol, opponent, board):
        """
        Called when match starts
        """
        self.game_scene.initializeGame(symbol, opponent, board)
        self.scene_manager.setCurrentWidget(self.game_scene)

    def onStateSync(self, state, data):
        """
        Called on SYNC response from server
        """
        # Setup info and clean error msg
        self.lobby_scene.updateInfo(self.current_nick, f"{self.current_ip}:{self.current_port}")
        self.lobby_scene.setConnectionError("", is_err=False)
        self.waiting_scene.setConnectionError("")
    
        # LOBBY reconnect
        if (state == "LOBBY"):
            self.scene_manager.setCurrentWidget(self.lobby_scene)
        
        # Waiting reconnect
        elif (state == "WAITING"):
            self.scene_manager.setCurrentWidget(self.waiting_scene)
            self.network.sendFindRequest()
        
        # Playing game reconnect
        elif (state == "GAME"):
            self.game_scene.syncPlayingGame(
                data["symbol"], 
                data["board"], 
                data["turn"], 
                data["opponent"]
            )
            self.scene_manager.setCurrentWidget(self.game_scene)

        # Finished game reconnect
        elif (state == "RESULT"):
            self.game_scene.syncFinishedGame(
                data["opponent"],
                data["board"],
                data["winner"]
            )
            self.scene_manager.setCurrentWidget(self.game_scene)

    def onBackToLobby(self):
        self.scene_manager.setCurrentWidget(self.lobby_scene)

    def onDisconnected(self, reason: str):
        """
        Moves user to login page
        Called as a reaction on disconnected signal
        """
        self.login_scene.setErrorMsg(reason)
        self.scene_manager.setCurrentWidget(self.login_scene)

    def onNetworkError(self, err_msg: str):
        """
        Reaction on network error during app run
        """
        current_scene = self.scene_manager.currentWidget()
        msg = "CONNECTION LOST - Reconnecting..."

        if current_scene == self.login_scene:
            self.login_scene.setErrorMsg(err_msg)
        if current_scene == self.game_scene:
            self.game_scene.setLocalConnectionErr(is_reconnecting=True)
        elif current_scene == self.lobby_scene:
            self.lobby_scene.setConnectionError(msg)
        elif current_scene == self.waiting_scene:
            self.waiting_scene.setConnectionError(msg)
        
        print(f"Network error handled in UI: {err_msg}")