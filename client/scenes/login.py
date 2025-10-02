"""
Filename: login.py
Author: Ondrej Patejdl
Contact: opatejdl@students.zcu.cz
Date: 2025-10-02
Version: 0.1.0
Description: This script defines login scene of the game
"""
from PyQt6.QtWidgets import (
    QWidget,
    QLineEdit, QPushButton, QLabel,
    QFormLayout
)
from PyQt6.QtGui import QFont


class LoginScene(QWidget):
    """
    Class representing Login scene

    Atributes:
    """
    def __init__(self):
        super().__init__()

        self.nickname_box = QLineEdit("User1234", self)
        self.ip_address_box = QLineEdit("172.128.27.12", self)
        self.port_box = QLineEdit("2222", self)

        self.login_btn = QPushButton("LOGIN", self)

        self._setupScene()

    def _setupScene(self):
        """
        Function setting up Login scene of the game
        """

        # set fonts

        # Set up labels and texts
        scene_label = QLabel("LOGIN")
        nick_label = QLabel("Nickname")
        ip_label = QLabel("IP")
        port_label = QLabel("Port")

        # Set up layout
        layout = QFormLayout(self)

        layout.addRow(nick_label, self.nickname_box)
        layout.addRow(ip_label, self.ip_address_box)
        layout.addRow(port_label, self.port_box)
        layout.addRow(self.login_btn)

        self.setLayout(layout)
