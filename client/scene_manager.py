"""
Filename: scene_manager.py
Author: Ondrej Patejdl
Contact: opatejdl@students.zcu.cz
Date: 2025-09-28
Version: 0.1.0
Description: This script managing user data and scene content
"""
from PyQt6.QtWidgets import QMainWindow
from constants import *

class User:
    """
    Class representing a user of a game
        - Stores user's nick and state
    """
    ...

class MainWindow(QMainWindow):

    def __init__(self):
        super().__init__()
        self._setUI()

    def _setUI(self):
        """
        Set up the UI

        Return:
            None
        """

        # Set up Title, Size and Layout
        self.setWindowTitle(WINDOW_NAME)
        self.setGeometry(DEFAULT_X_POS, DEFAULT_Y_POS,
                         DEFAULT_WIDTH, DEFAULT_HEIGH)
