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
from core.scene_manager import MainWindow

if __name__ == "__main__":
    app = QApplication(sys.argv)

    window = MainWindow()
    window.show()

    sys.exit(app.exec())
