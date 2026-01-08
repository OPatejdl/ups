"""
Filename: game.py
Author: Ondrej Patejdl
Contact: opatejdl@students.zcu.cz
Description: This script defines game scene of the client's app
"""

from PyQt6.QtWidgets import (
    QWidget, QGridLayout, QPushButton, QLabel, QVBoxLayout,
    QHBoxLayout
)
from PyQt6.QtCore import (
    Qt, pyqtSignal, QTimer
)
from src.core.styling import (
    TITLE_FONT, LABEL_FONT,
    BLUE_BTN_STYLE, RED_BTN_STYLE,
    TILE_STYLE, ORANGE_TXT_STYLE, GREEN_TXT_STYLE,
    STATUS_MSG_STYLE
)
from src.core.constants import *


class GameScene(QWidget):
    """
    Class representing Game scene

    Signals:
        moveRequest(int, int) - Emitted when user clicks a tile (x, y)
        backToLobbyRequest() - Emitted when game ens and user wants to leave
    """
    moveRequest = pyqtSignal(int, int)
    backToLobbyRequest = pyqtSignal()
    rematchRequest = pyqtSignal()

    def __init__(self):
        super().__init__()

        # Game State
        self.my_symbol: str = ""
        self.opponent_nick: str = "Unknown"
        self.is_my_turn: bool = False
        self.board_enabled: bool = False

        # UI Components
        self.status_label: QLabel = QLabel("Waiting for game...", self)
        self.turn_label: QLabel = QLabel("", self)
        self.opponent_label: QLabel = QLabel("", self)
        self.board_buttons: list[list[QPushButton]] = [] # 2D array [y][x]
        
        # Back btn
        self.back_btn: QPushButton = QPushButton("BACK TO LOBBY", self)
        self.back_btn.clicked.connect(self.backToLobbyRequest.emit)
        
        # Rematch btn
        self.rematch_btn: QPushButton = QPushButton("REMATCH", self)
        self.rematch_btn.clicked.connect(self._onRematchClick)
        
        # Btn container
        self.end_game_container: QWidget = QWidget()
        self.end_game_container.hide()

        self._setupUI()

    def _setupUI(self):
        """
        Sets up game scene
        """
        layout = QVBoxLayout()

        # Header
        self.status_label.setFont(TITLE_FONT)
        self.status_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        
        self.opponent_label.setFont(LABEL_FONT)
        self.opponent_label.setAlignment(Qt.AlignmentFlag.AlignCenter)

        self.turn_label.setFont(LABEL_FONT)
        self.turn_label.setStyleSheet("color: #007BFF; font-weight: bold;")
        self.turn_label.setAlignment(Qt.AlignmentFlag.AlignCenter)

        header_layout = QVBoxLayout()
        header_layout.addWidget(self.status_label)
        header_layout.addWidget(self.opponent_label)
        header_layout.addWidget(self.turn_label)
        
        layout.addLayout(header_layout)
        layout.addSpacing(DEFAULT_SPACING)

        # Board (3X3)
        grid_layout = QGridLayout()
        grid_layout.setSpacing(BOARD_SPACING)

        for y in range(BOARD_SIZE):
            row = []
            for x in range(BOARD_SIZE):
                btn = QPushButton("")
                btn.setFixedSize(BOARD_TILE_SIZE, BOARD_TILE_SIZE)
                btn.setFocusPolicy(Qt.FocusPolicy.NoFocus)
                btn.setStyleSheet(TILE_STYLE)
                # Connect click with coordinates
                btn.clicked.connect(lambda _, r=x, c=y: self._onTileClick(r, c))
                
                grid_layout.addWidget(btn, y, x)
                row.append(btn)
            self.board_buttons.append(row)

        layout.addLayout(grid_layout)
        layout.addStretch()

        # Btn setup
        self.rematch_btn.setStyleSheet(BLUE_BTN_STYLE)
        self.back_btn.setStyleSheet(RED_BTN_STYLE)

        button_layout = QHBoxLayout(self.end_game_container)
        button_layout.addStretch(GAME_BTN_SPACING)
        button_layout.addWidget(self.back_btn)
        button_layout.addSpacing(DEFAULT_SPACING)
        button_layout.addWidget(self.rematch_btn)
        button_layout.addStretch(GAME_BTN_SPACING)

        layout.addWidget(self.end_game_container)
        layout.addStretch()

        self.setLayout(layout)

    def initializeGame(self, my_symbol: str, opponent_nick: str, board_str=" "*TOTAL_TILES):
        """
        Initialize a new game

        Args:
            my_symbol: Player's symbol
            opponent_nick: Nickname of the opponent
            board_str String: representing the game board
        """
        self.my_symbol = my_symbol
        self.opponent_nick = opponent_nick
        self.status_label.setText(f"You are playing as: {my_symbol}")
        self.opponent_label.setText(f"Opponent: {opponent_nick}")
        self.rematch_btn.setText("Rematch")
        self.end_game_container.hide()
        self.board_enabled = True
        
        # Initial draw (X starts)
        self.updateBoard(board_str, 'X')
        self.rematch_btn.setEnabled(True)
        self.back_btn.setEnabled(True)

    def syncPlayingGame(self, my_symbol: str, board_str: str, turn_symbol: str, opponent_nick: str):
        """
        Called after RECONNECT to restore game's state

        Args:
            my_symbol - Player's symbol
            board_str - String representing the game board
            turn_symbol - Symbol of the player whose turn it is
            opponent_nick - Nickname of the opponent
        """
        self.my_symbol = my_symbol
        self.opponent_nick = opponent_nick
        self.status_label.setText(f"RECONNECTED as: {my_symbol}")
        self.opponent_label.setText(f"Opponent: {opponent_nick}")
        self.end_game_container.hide()
        self.board_enabled = True
        
        self.updateBoard(board_str, turn_symbol)

    def updateBoard(self, board_str: str, turn_symbol: str):
        """
        Updates game board based on the given string

        Args:
            board_str - String representing the game board
            turn_symbol - Symbol of the player whose turn it is
        """
        for i, char in enumerate(board_str):
            y = i // BOARD_SIZE
            x = i % BOARD_SIZE

            # Safety check
            if y < BOARD_SIZE and x < BOARD_SIZE:
                btn = self.board_buttons[y][x]
                
                if char != ' ':
                    btn.setText(char)
                    color = "red" if char == 'X' else "blue"
                    btn.setStyleSheet(f"color: {color}; font-size: 30px; font-weight: bold;")
                    btn.setEnabled(False) 
                else:
                    btn.setText("")
                    btn.setEnabled(True)
                    btn.setStyleSheet(TILE_STYLE)

        self._updateTurnInfo(turn_symbol)

    def _setTurnText(self, text: str, style: str):
        """
        Sets up turn text of the game

        Args:
            text - Text of the turn info
            style - Style of the text
        """
        self.turn_label.setText(text)
        self.turn_label.setStyleSheet(style)

    def _updateTurnInfo(self, turn_symbol: str, resume_flag: bool = False):
        """
        Updates label and locks the game board, if it isn't player's turn

        Args:
            turn_symbol - Symbol of the player whose turn it is
        """
        if turn_symbol == '-':
            return

        self.is_my_turn = (turn_symbol == self.my_symbol)

        if resume_flag:
            if self.is_my_turn:
                self._setTurnText("Game Resume - YOUR TURN!", GREEN_TXT_STYLE)
            else:
                self._setTurnText("Game Resume - OPPONENT TURN!", ORANGE_TXT_STYLE)
        else:
            if self.is_my_turn:
                self._setTurnText("YOUR TURN!", GREEN_TXT_STYLE)
            else:
                self._setTurnText(f"Waiting for {self.opponent_nick}...", ORANGE_TXT_STYLE)

    def setPaused(self):
        """
        Pauses game, when opponent disconnects
        """
        self.board_enabled = False
        self.status_label.setText("GAME PAUSED")
        self.turn_label.setText("Opponent disconnected. Waiting...")
        self.turn_label.setStyleSheet("color: red;")
        self._setGridEnabled(False)

    def setResumed(self, turn_symbol: str):
        """
        Activates game, when opponent reconnects
        """
        self.board_enabled = True
        self.status_label.setText(f"You are playing as: {self.my_symbol}")
        
        if turn_symbol:
            self._updateTurnInfo(turn_symbol, True)

        self._setGridEnabled(True)

    def handleResult(self, game_result: str, winner_nick: str):
        """
        Handles result state of the game

        Args:
            game_result: Code defining result of the game
            winner_nick: Nickname of the winner
        """
        self.board_enabled = False
        self._setGridEnabled(False)
        self.end_game_container.show()

        if game_result == "WIN":
            if winner_nick == self.opponent_nick:
                self.turn_label.setText("YOU LOST!")
                self.turn_label.setStyleSheet("color: red; font-size: 20px;")
            else:
                self.turn_label.setText("YOU WON!")
                self.turn_label.setStyleSheet("color: green; font-size: 20px;")
        elif game_result == "DRAW":
            self.turn_label.setText("IT'S A DRAW!")
            self.turn_label.setStyleSheet("color: gray; font-size: 20px;")

    def _onTileClick(self, x: int, y: int):
        """
        Handles click on a tile

        Args:
            x: Coordinate of X on the game board
            y: Coordinate of Y on the game board
        """
        if not self.board_enabled: return
        if not self.is_my_turn: return 
        
        self.board_buttons[y][x].setEnabled(False)

        # Send to server
        self.moveRequest.emit(x, y)

    def _setGridEnabled(self, enabled: bool):
        """
        Setups tiles of game board

        Args:
            enabled - Sets the tile as enabled or disabled  
        """
        for row in self.board_buttons:
            for btn in row:
                if btn.text() == "":
                    btn.setEnabled(enabled)

    def syncFinishedGame(self, opponent_nick: str, board_str: str, winner_nick: str):
        """
        Shows layout for finished game

        Args:
            opponent_nick - nickname of the opponent
            board_str - string representing game board
            winner_nick - nickname of the winner
        """
        self.opponent_nick = opponent_nick
        self.opponent_label.setText(f"Opponent: {opponent_nick}")
        
        self.updateBoard(board_str, "-")
        
        game_result = "WIN" if winner_nick else "DRAW"
        self.handleResult(game_result, winner_nick)
        
        self.end_game_container.show()

    def setLocalConnectionErr(self, is_reconnecting: bool):
        """
        Informs user about local connection error

        Args:
            is_reconnecting - value indicating reconnection
        """
        if is_reconnecting:
            self.status_label.setText("CONNECTION LOST!")
            self.turn_label.setText("Trying to reconnect...")
            self.turn_label.setStyleSheet(ORANGE_TXT_STYLE)
        self._setGridEnabled(False)
        self.board_enabled = False

    def displayTurnError(self, message: str):
        """
        Temporarily shows turn error msg

        Args
            message - msg to show
        """
        original_text = self.turn_label.text()
        original_style = self.turn_label.styleSheet()
        
        self.turn_label.setText(message)
        self.turn_label.setStyleSheet(STATUS_MSG_STYLE)
        
        QTimer.singleShot(ERROR_MSG_TIME, lambda: self._restoreTurnLabel(original_text, original_style))

    def _restoreTurnLabel(self, text: str, style: str):
        """
        Restores text to its previous msg

        text:
            text - original text of turn_label
            style - original style of turn_label
        """
        self.turn_label.setText(text)
        self.turn_label.setStyleSheet(style)

    def _onRematchClick(self):
        """
        Called after click on the rematch btn
        """

        self.rematch_btn.setEnabled(False)
        self.rematch_btn.setText("WAITING...")
        self.turn_label.setText("Waiting for opponent's response...")
        self.turn_label.setStyleSheet(STATUS_MSG_STYLE)
        self.rematchRequest.emit()

    def showRematchWait(self):
        """
        Called after the server approved the rematch request
        """
        self.turn_label.setText("Request sent! Waiting for opponent...")