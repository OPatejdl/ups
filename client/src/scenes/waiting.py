"""
Filename: waiting.py
Author: Ondrej Patejdl
Contact: opatejdl@students.zcu.cz
Description: This script defines waiting scenes of the game
"""

from PyQt6.QtWidgets import (
    QWidget, QLabel, QVBoxLayout,
    QPushButton, QHBoxLayout
)
from PyQt6.QtCore import (
    Qt, pyqtSignal, QTimer
)
from core.styling import (
    TITLE_FONT, LABEL_FONT,
    STATUS_MSG_STYLE, RED_BTN_STYLE
)
from core.constants import (
    DEFAULT_SPACING, SPINNER_SPACE_UP,
    SPINNER_SPACE_DOWN, LEAVE_BTN_WIDTH,
    SPINNER_DOTS_OFFSET, SPINNER_DOTS_DIVIDE,
    SPINNER_TIMER, SPINNER_DOTS_INIT
)

class WaitingScene(QWidget):
    """
    Class representing Waiting scene
    """

    leaveRequest = pyqtSignal()

    def __init__(self):
        super().__init__()
        self.status_label = QLabel("", self)
        self.leave_btn = QPushButton("Back to lobby", self)

        # Spinner animation
        self.spinner_dots = SPINNER_DOTS_INIT
        self.spinner_timer = QTimer()
        self.spinner_timer.timeout.connect(self._animate_spinner)

        self._setupUI()

    def _setupUI(self):
        """
        Sets up UI for waiting scene
        """
        layout = QVBoxLayout()

        # -- Title --
        title = QLabel("WAITING FOR OPPONENT", self)
        title.setFont(TITLE_FONT)
        title.setAlignment(Qt.AlignmentFlag.AlignCenter)

        # -- Info --
        info = QLabel("Please wait until a second player joins...", self)
        info.setFont(LABEL_FONT)
        info.setAlignment(Qt.AlignmentFlag.AlignCenter)
        info.setStyleSheet("color: grey;")

        # -- Loader --
        self.spinner = QLabel("... ... ...", self)
        self.spinner.setFont(TITLE_FONT)
        self.spinner.setAlignment(Qt.AlignmentFlag.AlignCenter)

        # -- Status --
        self.status_label.setFont(LABEL_FONT)
        self.status_label.setAlignment(Qt.AlignmentFlag.AlignCenter)

        # -- Leave btn --
        self.leave_btn.setStyleSheet(RED_BTN_STYLE)
        self.leave_btn.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.leave_btn.setFixedWidth(LEAVE_BTN_WIDTH)
        self.leave_btn.clicked.connect(self.leaveRequest.emit)

        layout.addStretch()
        layout.addWidget(title)
        layout.addSpacing(DEFAULT_SPACING)
        layout.addWidget(info)

        layout.addSpacing(SPINNER_SPACE_UP)
        layout.addWidget(self.status_label)

        layout.addSpacing(SPINNER_SPACE_DOWN)
        layout.addWidget(self.spinner)

        layout.addSpacing(DEFAULT_SPACING)
        btn_layout = QHBoxLayout()
        btn_layout.addStretch()
        btn_layout.addWidget(self.leave_btn)
        btn_layout.addStretch()
        layout.addLayout(btn_layout)

        layout.addStretch()

        self.setLayout(layout)

    def setConnectionError(self, msg: str):
        """
        Shows connection error msg
        """
        self.status_label.setText(msg)
        self.status_label.setStyleSheet(STATUS_MSG_STYLE)

    def _animate_spinner(self):
        """
        Animation of hte spinner
        """
        self.spinner_dots = (self.spinner_dots + SPINNER_DOTS_OFFSET) % SPINNER_DOTS_DIVIDE
        dots = ". " * self.spinner_dots
        self.spinner.setText(dots.strip())

    def showEvent(self, event):
        """
        Starts animation when scene shown
        """
        super().showEvent(event)
        self.spinner_timer.start(SPINNER_TIMER)

    def hideEvent(self, event):
        """
        Stops animation when scene disappeared
        """
        super().hideEvent(event)
        self.spinner_timer.stop()