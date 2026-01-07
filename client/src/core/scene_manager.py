"""
Filename: scene_manager.py
Author: Ondrej Patejdl
Contact: opatejdl@students.zcu.cz
Description: This script managing user data and scene content
"""
from PyQt6.QtWidgets import (
    QMainWindow, QStackedWidget)
from PyQt6.QtCore import pyqtSlot
from core.constants import (
    WINDOW_NAME,
    DEFAULT_X_POS, DEFAULT_Y_POS,
    DEFAULT_HEIGH, DEFAULT_WIDTH,
    LOG_NAME
)
from scenes.login import LoginScene
from scenes.lobby import LobbyScene
from scenes.waiting import WaitingScene
from scenes.game import GameScene
from net.sockets import NetworkClient
from core.protocol import *
import logging

logger = logging.getLogger(f"{LOG_NAME}.{__name__}")

class MainWindow(QMainWindow):

    def __init__(self):
        super().__init__()
        logger.info("Setting up UI")

        # Network setup
        self.network = NetworkClient()
        self.network.loginResult.connect(self.onLoginResult)

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
        self.waiting_scene.leaveRequest.connect(self.onLeaveWaiting)

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
        self.network.turnError.connect(self.game_scene.displayTurnError)
        self.network.rematchWait.connect(self.game_scene.showRematchWait)

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

    def onLoginResult(self, code: int):
        """
        Handles LOGIN msg from server

        Args:
            code - Specifies response on login request
        """
        # Set up msg
        if code < LOGIN_FULL_SERVER:
            self.login_scene.setErrorMsg("")
        else:
            reasons = {
                LOGIN_FULL_SERVER: "Server is full",
                LOGIN_NICK_DUPLICITY: "Nickname already taken.",
                LOGIN_MAX_NICK_LEN: "Nickname is too long.",
                LOGIN_MIN_NICK_LEN: "Nickname is too short."
            }

            msg = reasons.get(code, f"Login failed (Error {code})")
            self.login_scene.setErrorMsg(msg)

        if code == LOGIN_SUCCESS:
            self.lobby_scene.updateInfo(self.current_nick, f"{self.current_ip}:{self.current_port}")
            self.scene_manager.setCurrentWidget(self.lobby_scene)
        elif code == LOGIN_RECONNECT:
            # Reconnect successful
            logger.info("Reconnected! Sending SYNC to sync state...")
            self.lobby_scene.updateInfo(self.current_nick, f"{self.current_ip}:{self.current_port}")
            self.network.sendSync()
        else:
            logger.error(f"Login failed with code {code}")

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

    def onGameStarted(self, symbol: str, opponent: str, board: str):
        """
        Called when match starts

        Args
            symbol - symbol of the player
            opponent - nickname of the opponent
            board - game board representation
        """
        self.game_scene.initializeGame(symbol, opponent, board)
        self.scene_manager.setCurrentWidget(self.game_scene)

    def onStateSync(self, state: str, data: dict):
        """
        Called on SYNC response from server

        Args
            state - defining current state of client
            data - specifies data if any needed
        """
        # Setup info and clean error msg
        self.lobby_scene.updateInfo(self.current_nick, f"{self.current_ip}:{self.current_port}")
        self.lobby_scene.setConnectionError("", is_err=False)
        self.waiting_scene.setConnectionError("")
    
        # LOBBY reconnect
        if state == "LOBBY":
            self.scene_manager.setCurrentWidget(self.lobby_scene)
        
        # Waiting reconnect
        elif state == "WAITING":
            self.scene_manager.setCurrentWidget(self.waiting_scene)
            self.network.sendFindRequest()
        
        # Playing game reconnect
        elif state == "GAME":
            self.game_scene.syncPlayingGame(
                data["symbol"], 
                data["board"], 
                data["turn"], 
                data["opponent"]
            )
            self.scene_manager.setCurrentWidget(self.game_scene)

        # Finished game reconnect
        elif state == "RESULT":
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

        elif current_scene == self.game_scene:
            self.game_scene.setLocalConnectionErr(is_reconnecting=True)

        elif current_scene == self.lobby_scene:
            self.lobby_scene.setConnectionError(msg)

        elif current_scene == self.waiting_scene:
            self.waiting_scene.setConnectionError(msg)
        
        logger.error(f"Network error handled in UI: {err_msg}")

    def onLeaveWaiting(self):
        """
        Handles request to leave the waiting room
        """
        logger.info("User is leaving the waiting queue")
        self.network.sendLeave()
        self.scene_manager.setCurrentWidget(self.lobby_scene)