"""
Filename: stylling.py
Author: Ondrej Patejdl
Contact: opatejdl@students.zcu.cz
Date: 2025-10-07
Version: 0.1.0
Description: This script defines general styling used throughout the client
"""

from PyQt6.QtGui import QFont
from PyQt6.QtWidgets import QApplication
from src.core.constants import (
    TITLES_SIZE, LETTER_FONT,
    LABEL_SIZE, GAME_TEXT_SIZE,
    TXT_BOLD
)

# Fonts
TITLE_FONT = QFont(LETTER_FONT, TITLES_SIZE, QFont.Weight.ExtraBold)
LABEL_FONT = QFont(LETTER_FONT, LABEL_SIZE, QFont.Weight.DemiBold)

GLOBAL_STYLESHEET = """
    QPushButton {
        color: white;
        border-radius: 6px;
        padding: 10px 20px;
        font-size: 10px;
        font-weight: bold;
        min-width: 40px;
        min-height: 10px;
        max-width: 150px;
    }
    QPushButton:hover {
        color: #F0F0F0;             /* lighter text color */
    }
    QPushButton:pressed {
        border: 2px solid #004C26;  /**/
        color: #E0E0E0;
    }
"""

BLUE_BTN_STYLE = """
            QPushButton {
                background-color: #007BFF;
            }
            QPushButton:hover {
                background-color: #33A1FF;
            }
            QPushButton:pressed {
                background-color: #0056B3;
            }
        """

RED_BTN_STYLE = """
            QPushButton {
                background-color: #FF3B3B;
            }
            QPushButton:hover {
                background-color: #FF6666;
            }
            QPushButton:pressed {
                background-color: #CC0000;
            }
        """

TILE_STYLE = """
            QPushButton { font-size: 30px; font-weight: bold; background-color: #EEE; border: 2px solid #CCC; }
            QPushButton:hover { background-color: #DDD; }
        """

LOGIN_BTN = """
            QPushButton {
                background-color: #00994C;
            }

            QPushButton:hover {
                background-color: #00B359;      /* light green on hover */
            }

            QPushButton:pressed {
                background-color: #007A3D;      /* darker green on click */
            }
        """


STATUS_MSG_STYLE = f"color: orange; font-weight: {TXT_BOLD};"
GREEN_TXT_STYLE = f"color: green; font-weight: {TXT_BOLD}; font-size: {GAME_TEXT_SIZE}px;"
ORANGE_TXT_STYLE = f"color: orange; font-weight: {TXT_BOLD}; font-size: {GAME_TEXT_SIZE}px;"

def setAppStyling(app: QApplication):
    """Apply the global stylesheet and fonts to the QApplication."""
    app.setStyleSheet(GLOBAL_STYLESHEET)