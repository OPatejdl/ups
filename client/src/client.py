"""
Filename: client.py
Author: Ondrej Patejdl
Contact: opatejdl@students.zcu.cz
Description: This script defines entry point of application
            (TIC-TAC-TOE client)
"""

import sys
from PyQt6.QtWidgets import QApplication
from logger.logger import client_logger
from core.scene_manager import MainWindow
from core.styling import setAppStyling

if __name__ == "__main__":
    client_logger.info("--- Starting Tic-Tac-Toe Client App")

    app = QApplication(sys.argv)
    setAppStyling(app)

    window = MainWindow()
    window.show()

    sys.exit(app.exec())
