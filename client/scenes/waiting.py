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
    TITLE_FONT, LABEL_FONT
)


class WaitingScene(QWidget):
    """
    Class representing Waiting scene
    """

    def __init__(self):
        super().__init__()
        self._setupUI()

    def _setupUI(self):
        """
        Sets up UI for waiting scene
        """
        layout = QVBoxLayout()

        # Title setup
        title = QLabel("WAITING FOR OPPONENT", self)
        title.setFont(TITLE_FONT)
        title.setAlignment(Qt.AlignmentFlag.AlignCenter)
        # title.setStyleSheet("color: white;")

        # Info
        info = QLabel("Please wait until a second player joins...", self)
        info.setFont(LABEL_FONT)
        info.setAlignment(Qt.AlignmentFlag.AlignCenter)
        info.setStyleSheet("color: grey;")

        # Loader
        spinner = QLabel("... ... ...", self)
        spinner.setFont(TITLE_FONT)
        spinner.setAlignment(Qt.AlignmentFlag.AlignCenter)
        # spinner.setStyleSheet("color: white;")

        layout.addStretch()
        layout.addWidget(title)
        layout.addSpacing(20)
        layout.addWidget(info)
        layout.addSpacing(40)
        layout.addWidget(spinner)
        layout.addStretch()

        self.setLayout(layout)