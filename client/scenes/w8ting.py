"""
Filename: lobby.py
Author: Ondrej Patejdl
Contact: opatejdl@students.zcu.cz
Date: 2025-10-02
Version: 0.1.0
Description: This script defines lobby scenes of the game
"""

from PyQt6.QtWidgets import (
    QWidget
)


class W8ingScene(QWidget):
    """
    Class representing Waiting scene
    """

    def __init__(self):
        super().__init__()
