"""
Filename: waiting.py
Author: Ondrej Patejdl
Contact: opatejdl@students.zcu.cz
Description: This script defines waiting scenes of the game
"""

from PyQt6.QtWidgets import (
    QWidget, QLabel, QVBoxLayout
)
from PyQt6.QtCore import Qt
from core.styling import (
    TITLE_FONT, LABEL_FONT,
    STATUS_MSG_STYLE
)


class WaitingScene(QWidget):
    """
    Class representing Waiting scene
    """

    def __init__(self):
        super().__init__()
        self.status_label = QLabel("", self)
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
        spinner = QLabel("... ... ...", self)
        spinner.setFont(TITLE_FONT)
        spinner.setAlignment(Qt.AlignmentFlag.AlignCenter)

        # -- Status --
        self.status_label.setFont(LABEL_FONT)
        self.status_label.setAlignment(Qt.AlignmentFlag.AlignCenter)

        layout.addStretch()
        layout.addWidget(title)
        layout.addSpacing(20)
        layout.addWidget(info)

        layout.addSpacing(10)
        layout.addWidget(self.status_label)

        layout.addSpacing(30)
        layout.addWidget(spinner)
        layout.addStretch()

        self.setLayout(layout)

    def setConnectionError(self, msg: str):
        """
        Shows connection error msg
        """
        self.status_label.setText(msg)
        self.status_label.setStyleSheet(STATUS_MSG_STYLE)