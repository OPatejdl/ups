"""
Filename: login.py
Author: Ondrej Patejdl
Contact: opatejdl@students.zcu.cz
Date: 2025-10-05
Version: 0.1.0
Description: This script defines login scene of the game
"""

from PyQt6.QtCore import (
    Qt, pyqtSignal)
from PyQt6.QtWidgets import (
    QWidget, QLineEdit, QPushButton, QLabel,
    QFormLayout, QVBoxLayout, QHBoxLayout
)
from PyQt6.QtGui import QFont
from core.constants import (
    LETTER_FONT,
    TITLES_SIZE,
    LABEL_SIZE,
    LOGIN_SPACING
)


class LoginScene(QWidget):
    """
    Class representing Login scene

    Signals:
        loginRequested
            - signal emitted, when Login button was clicked
    """

    # Login Scene signals
    loginRequested = pyqtSignal(str, str, int)

    def __init__(self):
        super().__init__()
        # TODO: add input check from user
        self.nickname_box = QLineEdit(self)
        self.nickname_box.setPlaceholderText("User1234")

        self.ip_address_box = QLineEdit(self)
        self.ip_address_box.setPlaceholderText("172.128.27.12")

        self.port_box = QLineEdit(self)
        self.port_box.setPlaceholderText("2222")

        self.login_btn = QPushButton("LOGIN", self)

        self._setupScene()

        self.login_btn.clicked.connect(self._onLoginBtnClick)

    def _setupScene(self):
        """
        Private function, which sets up login scene
            - formats title, form and login button
        """
        # --- Title setup ---
        title_label = QLabel("SERVER LOGIN")
        title_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        title_label.setFont(QFont(LETTER_FONT, TITLES_SIZE,
                                  QFont.Weight.ExtraBold))

        # Labels set up
        nick_label = QLabel("Nickname:")
        ip_label = QLabel("IP Address:")
        port_label = QLabel("Port:")

        label_font = QFont(LETTER_FONT, LABEL_SIZE, QFont.Weight.DemiBold)

        for lbl in (nick_label, ip_label, port_label):
            lbl.setFont(label_font)
            lbl.setAlignment(Qt.AlignmentFlag.AlignRight |
                             Qt.AlignmentFlag.AlignVCenter)

        # --- Form set up ---
        form = QWidget(self)
        form_layout = QFormLayout(form)
        form_layout.addRow(nick_label, self.nickname_box)
        form_layout.addRow(ip_label, self.ip_address_box)
        form_layout.addRow(port_label, self.port_box)

        form.setMaximumWidth(400)
        form.setMinimumWidth(200)

        # Create horizontal layout for centering
        hbox_form = QHBoxLayout()
        hbox_form.addWidget(form)

        # --- Set up button ---
        self.login_btn.setStyleSheet("""
            QPushButton {
                background-color: #00994C;
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
                background-color: #00B359;      /* light green on hover */
                color: #F0F0F0;                 /* lighter text color */
            }

            QPushButton:pressed {
                background-color: #007A3D;      /* darker green on click */
                border: 2px solid #004C26;
                color: #E0E0E0;
            }
        """)

        hbox_btn = QHBoxLayout()
        hbox_btn.addWidget(self.login_btn)

        # Set up login widget layout
        login_layout = QVBoxLayout()

        login_layout.addStretch(3)
        login_layout.addWidget(title_label,
                               alignment=Qt.AlignmentFlag.AlignHCenter)
        login_layout.addSpacing(LOGIN_SPACING)
        login_layout.addLayout(hbox_form)
        login_layout.addSpacing(LOGIN_SPACING)
        login_layout.addLayout(hbox_btn)
        login_layout.addStretch(3)
        self.setLayout(login_layout)

    def _onLoginBtnClick(self):
        """
        function handles reaction on Login btn click

        Emits signal carrying information about input values
        """
        nick = self.nickname_box.text().strip()
        host = self.ip_address_box.text().strip()
        # TODO: add validation if allowed
        if (self.port_box.text() == "" or nick == "" or host == ""):
            print("Empty values!!!")
            return
        port = int(self.port_box.text())
        self.loginRequested.emit(nick, host, port)
