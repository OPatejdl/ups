"""
Filename: game.py
Author: Ondrej Patejdl
Contact: opatejdl@students.zcu.cz
Description: This script defines game scene of the game
"""

from PyQt6.QtWidgets import (
    QWidget, QGridLayout, QPushButton, QLabel, QVBoxLayout,
    QHBoxLayout
)
from PyQt6.QtCore import Qt, pyqtSignal
from core.styling import (
    TITLE_FONT, LABEL_FONT,
    BLUE_BTN_STYLE, RED_BTN_STYLE
)


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
        self.my_symbol = ""
        self.opponent_nick = "Unknown"
        self.is_my_turn = False
        self.board_enabled = False

        # UI Components
        self.status_label = QLabel("Waiting for game...", self)
        self.turn_label = QLabel("", self)
        self.opponent_label = QLabel("", self)
        self.board_buttons = [] # 2D array [y][x]
        
        # Back btn
        self.back_btn = QPushButton("BACK TO LOBBY", self)
        self.back_btn.clicked.connect(self.backToLobbyRequest.emit)
        
        # Rematch btn
        self.rematch_btn = QPushButton("REMATCH", self)
        self.rematch_btn.clicked.connect(self.rematchRequest.emit)
        
        # Btn container
        self.end_game_container = QWidget()
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
        layout.addSpacing(20)

        # Board (3X3)
        grid_layout = QGridLayout()
        grid_layout.setSpacing(10)

        for y in range(3):
            row = []
            for x in range(3):
                btn = QPushButton("")
                btn.setFixedSize(80, 80)
                btn.setFocusPolicy(Qt.FocusPolicy.NoFocus)
                btn.setStyleSheet("""
                    QPushButton { font-size: 30px; font-weight: bold; background-color: #EEE; border: 2px solid #CCC; }
                    QPushButton:hover { background-color: #DDD; }
                """)
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
        button_layout.addStretch(1)
        button_layout.addWidget(self.back_btn)
        button_layout.addSpacing(20)
        button_layout.addWidget(self.rematch_btn)
        button_layout.addStretch(1)

        layout.addWidget(self.end_game_container)
        layout.addStretch()

        self.setLayout(layout)

    def initializeGame(self, my_symbol, opponent_nick, board_str=" "*9):
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
        self.end_game_container.hide()
        self.board_enabled = True
        
        # Initial draw (X starts)
        self.updateBoard(board_str, 'X')

    def syncPlayingGame(self, my_symbol: str, board_str: str, turn_symbol: str, opponent_nick: str):
        """
        Called after RECONNECT to restore game's state

        Args:
            my_symbol: Player's symbol
            board_str: String representing the game board
            turn_symbol: Symbol of the player whose turn it is
        """
        self.my_symbol = my_symbol
        self.opponent_nick = opponent_nick
        self.status_label.setText(f"RECONNECTED as: {my_symbol}")
        self.opponent_label.setText(f"Opponent: {opponent_nick}")
        self.back_btn.hide()
        self.board_enabled = True
        
        self.updateBoard(board_str, turn_symbol)

    def updateBoard(self, board_str, turn_symbol):
        """
        Updates game board based on the given string

        Args:
            board_str: String representing the game board
            turn_symbol: Symbol of the player whose turn it is
        """
        for i, char in enumerate(board_str):
            y = i // 3
            x = i % 3
            if y < 3 and x < 3: # Safety check
                btn = self.board_buttons[y][x]
                
                if char != ' ':
                    btn.setText(char)
                    color = "red" if char == 'X' else "blue"
                    btn.setStyleSheet(f"color: {color}; font-size: 30px; font-weight: bold;")
                    btn.setEnabled(False) 
                else:
                    btn.setText("")
                    btn.setEnabled(True)

        self._updateTurnInfo(turn_symbol)

    def _updateTurnInfo(self, turn_symbol):
        """
        Updates label and locks the game board, if it isn't player's turn

        Args:
            turn_symbol: Symbol of the player whose turn it is
        """
        if turn_symbol == '-':
            return

        self.is_my_turn = (turn_symbol == self.my_symbol)
        
        if self.is_my_turn:
            self.turn_label.setText("YOUR TURN!")
            self.turn_label.setStyleSheet("color: green; font-weight: bold; font-size: 16px;")
        else:
            self.turn_label.setText(f"Waiting for {self.opponent_nick}...")
            self.turn_label.setStyleSheet("color: orange; font-weight: bold; font-size: 16px;")

    def setPaused(self):
        """
        Pauses game, when opponent disconnects
        """
        self.board_enabled = False
        self.status_label.setText("GAME PAUSED")
        self.turn_label.setText("Opponent disconnected. Waiting...")
        self.turn_label.setStyleSheet("color: red;")
        self._setGridEnabled(False)

    def setResumed(self):
        """
        Activates game, when opponent reconnect
        """
        self.board_enabled = True
        self.status_label.setText(f"You are playing as: {self.my_symbol}")
        self.turn_label.setText("Game Resumed!")
        self._setGridEnabled(True)

    def handleResult(self, result_code, winner_nick):
        """
        Handles result state of the game

        Args:
            result_code: Code defining result of the game
            winner_nick: Nickname of the winner
        """
        self.board_enabled = False
        self._setGridEnabled(False)
        self.end_game_container.show()

        if result_code == "WIN":
            if winner_nick == self.opponent_nick:
                self.turn_label.setText("YOU LOST!")
                self.turn_label.setStyleSheet("color: red; font-size: 20px;")
            else:
                self.turn_label.setText("YOU WON!")
                self.turn_label.setStyleSheet("color: green; font-size: 20px;")
        elif result_code == "DRAW":
            self.turn_label.setText("IT'S A DRAW!")
            self.turn_label.setStyleSheet("color: gray; font-size: 20px;")

    def _onTileClick(self, x, y):
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
            enabled: Sets the tile as enabled or disabled  
        """
        for row in self.board_buttons:
            for btn in row:
                if btn.text() == "":
                    btn.setEnabled(enabled)


    def syncFinishedGame(self, opponent_nick: str, board_str: str, winner_nick:str):
        """
        Shows layout for finished game
        """
        self.opponent_nick = opponent_nick
        self.opponent_label.setText(f"Opponent: {opponent_nick}")
        
        self.updateBoard(board_str, "-")
        
        result_code = "WIN" if winner_nick else "DRAW"
        self.handleResult(result_code, winner_nick)
        
        self.end_game_container.show()
        self.back_btn.show()