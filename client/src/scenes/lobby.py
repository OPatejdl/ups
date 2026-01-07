"""
Filename: lobby.py
Author: Ondrej Patejdl
Contact: opatejdl@students.zcu.cz
Description: This script defines lobby scenes of the game
"""

from PyQt6.QtCore import (
    pyqtSignal, Qt)
from PyQt6.QtWidgets import (
    QWidget, QPushButton,
    QLabel, QVBoxLayout,
    QHBoxLayout
)
from src.core.constants import (
    DEFAULT_SPACING, INFO_LAYOUT_SPACE,
    LOBBY_STRETCH_AVG, LOBBY_STRETCH_BOTTOM
)
from src.core.styling import (
    TITLE_FONT, LABEL_FONT, 
    BLUE_BTN_STYLE, RED_BTN_STYLE
)


class LobbyScene(QWidget):
    """
    Class representing Lobby scene

    Signals:
        findGameRequest
            - signal emitted, when FIND GAME btn clicked
        exitRequest
            - signal emitted, when EXIT btn clicked
    """
    findGameRequest = pyqtSignal()
    exitRequest = pyqtSignal()

    def __init__(self):
        super().__init__()
        self.findGame_btn = QPushButton("FIND GAME", self)
        self.exit_btn = QPushButton("EXIT", self)

        self.usernameInfo_label = QLabel(self)
        self.connectionInfo_label = QLabel(self)
        self.status_label = QLabel("", self)

        self._setupLobbyScene()

        self.findGame_btn.clicked.connect(self._findGameBtnClicked)
        self.exit_btn.clicked.connect(self._exitBtnClicked)

    def _setupLobbyScene(self):
        """
        Private function, which sets up login scene
            - creates title and info labels
            - formats FIND GAME and EXIT button
        """
        # -- Title setup --
        title_label = QLabel("LOBBY", self)
        title_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        title_label.setFont(TITLE_FONT)

         # -- User info setup --
        username_label = QLabel("Username:", self)
        username_label.setFont(LABEL_FONT)

        connection_label = QLabel("Connected to:", self)
        connection_label.setFont(LABEL_FONT)

        # -- Information layout setup --
        info_layout = QVBoxLayout()
        info_layout.setAlignment(Qt.AlignmentFlag.AlignHCenter)

        # -- Username line --
        username_layout = QHBoxLayout()
        username_layout.addWidget(username_label)
        username_layout.addSpacing(INFO_LAYOUT_SPACE)
        username_layout.addWidget(self.usernameInfo_label)

        # -- Connection info Line --
        connection_layout = QHBoxLayout()
        connection_layout.addWidget(connection_label)
        connection_layout.addSpacing(INFO_LAYOUT_SPACE)
        connection_layout.addWidget(self.connectionInfo_label)

        info_layout.addLayout(username_layout)
        info_layout.addLayout(connection_layout)

        # -- Status label --
        self.status_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.status_label.setFont(LABEL_FONT)

        # --- Buttons Set up ---
        self.findGame_btn.setStyleSheet(BLUE_BTN_STYLE)
        self.findGame_btn.setFocusPolicy(Qt.FocusPolicy.NoFocus)

        self.exit_btn.setStyleSheet(RED_BTN_STYLE)
        self.exit_btn.setFocusPolicy(Qt.FocusPolicy.NoFocus)

        button_layout = QHBoxLayout()
        button_layout.addStretch(LOBBY_STRETCH_AVG)
        button_layout.addWidget(self.exit_btn)
        button_layout.addSpacing(DEFAULT_SPACING)
        button_layout.addWidget(self.findGame_btn)
        button_layout.addStretch(LOBBY_STRETCH_AVG)

        # --- Main Lobby layout ---
        lobby_layout = QVBoxLayout()
        lobby_layout.addStretch(LOBBY_STRETCH_AVG)
        lobby_layout.addWidget(title_label,
                               alignment=Qt.AlignmentFlag.AlignHCenter)
        lobby_layout.addSpacing(DEFAULT_SPACING)
        lobby_layout.addLayout(info_layout)

        lobby_layout.addSpacing(INFO_LAYOUT_SPACE)
        lobby_layout.addWidget(self.status_label)

        lobby_layout.addSpacing(DEFAULT_SPACING)
        lobby_layout.addLayout(button_layout)
        lobby_layout.addStretch(LOBBY_STRETCH_BOTTOM)

        self.setLayout(lobby_layout)

        self.setFocusPolicy(Qt.FocusPolicy.StrongFocus)

    def _exitBtnClicked(self):
        """
        Function called when EXIT btn clicked

        Emits:
        exitRequest signal
        """
        self.exit_btn.setEnabled(False)
        self.exitRequest.emit()

    def _findGameBtnClicked(self):
        """
        Function called when FIND GAME btn clicked

        Emits:
        findGameRequest signal
        """
        self.findGame_btn.setEnabled(False)
        self.findGameRequest.emit()

    def updateInfo(self, username: str, connectionInfo: str):
        self.usernameInfo_label.setText(username)
        self.connectionInfo_label.setText(connectionInfo)

    def setConnectionError(self, msg: str, is_err: bool = True):
        self.status_label.setText(msg)
        color = "orange" if is_err else "transparent" # choose color
        self.status_label.setStyleSheet(f"color: {color}; font-weight: bold;")
        
        self.findGame_btn.setEnabled(not is_err)