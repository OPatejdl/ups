"""
Filename: login.py
Author: Ondrej Patejdl
Contact: opatejdl@students.zcu.cz
Description: This script defines login scene of the game
"""

from PyQt6.QtCore import (
    Qt, pyqtSignal)
from PyQt6.QtWidgets import (
    QWidget, QLineEdit, QPushButton, QLabel,
    QFormLayout, QVBoxLayout, QHBoxLayout
)
from core.constants import (
    DEFAULT_SPACING
)
from core.styling import (
    TITLE_FONT, LABEL_FONT
)


class LoginScene(QWidget):
    """
    Class representing Login scene

    Signals:
        loginRequest
            - signal emitted, when Login button was clicked
    """

    # Login Scene signals
    loginRequest = pyqtSignal(str, str, int)

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

        self._setupLoginScene()

        self.login_btn.clicked.connect(self._onLoginBtnClick)

    def _setupLoginScene(self):
        """
        Private function, which sets up login scene
            - formats title, form and login button
        """
        # --- Title setup ---
        title_label = QLabel("SERVER LOGIN", self)
        title_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        title_label.setFont(TITLE_FONT)

        # Labels set up
        nick_label = QLabel("Nickname:", self)
        ip_label = QLabel("IP Address:", self)
        port_label = QLabel("Port:", self)

        for lbl in (nick_label, ip_label, port_label):
            lbl.setFont(LABEL_FONT)
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
            }

            QPushButton:hover {
                background-color: #00B359;      /* light green on hover */
            }

            QPushButton:pressed {
                background-color: #007A3D;      /* darker green on click */
            }
        """)
        self.login_btn.setFocusPolicy(Qt.FocusPolicy.NoFocus)

        hbox_btn = QHBoxLayout()
        hbox_btn.addWidget(self.login_btn)

        # Set up login widget layout
        login_layout = QVBoxLayout()

        login_layout.addStretch(3)
        login_layout.addWidget(title_label,
                               alignment=Qt.AlignmentFlag.AlignHCenter)
        login_layout.addSpacing(DEFAULT_SPACING)
        login_layout.addLayout(hbox_form)
        login_layout.addSpacing(DEFAULT_SPACING)
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
        # TODO: add validation
        if (self.port_box.text() == "" or nick == "" or host == ""):
            print("Empty values!!!")
            return
        port = int(self.port_box.text())
        self.loginRequest.emit(nick, host, port)
