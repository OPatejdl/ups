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
from core.constants import (
    TITLES_SIZE, LETTER_FONT,
    LABEL_SIZE
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

def setAppStyling(app: QApplication):
    """Apply the global stylesheet and fonts to the QApplication."""
    app.setStyleSheet(GLOBAL_STYLESHEET)