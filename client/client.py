"""
Filename: client.py
Author: Ondrej Patejdl
Contact: opatejdl@students.zcu.cz
Date: 2025-09-28
Version: 0.1.0
Description: This script defines entry point of application
            (TIC-TAC-TOE client)
"""

import sys
from PyQt6.QtWidgets import QApplication
from src.logger.logger import client_logger
from src.core.scene_manager import MainWindow
from src.core.styling import setAppStyling

if __name__ == "__main__":
    client_logger.info("--- Starting Tic-Tac-Toe Client App")

    app = QApplication(sys.argv)
    setAppStyling(app)

    window = MainWindow()
    window.show()

    sys.exit(app.exec())
