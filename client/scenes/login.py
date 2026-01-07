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
    TITLE_FONT, LABEL_FONT, 
    STATUS_MSG_STYLE, LOGIN_BTN
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

        # Nickname Box
        self.nickname_box = QLineEdit(self)
        self.nickname_box.setPlaceholderText("User1234")

        # IP addr Box
        self.ip_address_box = QLineEdit(self)
        self.ip_address_box.setPlaceholderText("172.128.27.12")

        # Port Box
        self.port_box = QLineEdit(self)
        self.port_box.setPlaceholderText("2222")

        # Login Btn
        self.login_btn = QPushButton("LOGIN", self)

        # Error msg
        self.status_msg = QLabel("", self)

        self._setupLoginScene()

        self.login_btn.clicked.connect(self._onLoginBtnClick)

    def _setupLoginScene(self):
        """
        Private function, which sets up login scene
            - formats title, form and login button
        """
        # -- Title setup --
        title_label = QLabel("SERVER LOGIN", self)
        title_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        title_label.setFont(TITLE_FONT)

        # -- Labels set up --
        nick_label = QLabel("Nickname:", self)
        ip_label = QLabel("IP Address:", self)
        port_label = QLabel("Port:", self)

        for lbl in (nick_label, ip_label, port_label):
            lbl.setFont(LABEL_FONT)
            lbl.setAlignment(Qt.AlignmentFlag.AlignRight |
                             Qt.AlignmentFlag.AlignVCenter)

        # -- Form set up --
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

        # -- Login button --
        self.login_btn.setStyleSheet(LOGIN_BTN)
        self.login_btn.setFocusPolicy(Qt.FocusPolicy.NoFocus)

        hbox_btn = QHBoxLayout()
        hbox_btn.addWidget(self.login_btn)

        # -- Status msg --
        self.status_msg.setStyleSheet(STATUS_MSG_STYLE)
        self.status_msg.setAlignment(Qt.AlignmentFlag.AlignCenter)

        # Set up login widget layout
        login_layout = QVBoxLayout()

        login_layout.addStretch(3)
        login_layout.addWidget(title_label,
                               alignment=Qt.AlignmentFlag.AlignHCenter)
        login_layout.addWidget(self.status_msg)
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
        port_raw = self.port_box.text().strip()

        # Empty values check
        if (self.port_box.text() == "" or nick == "" or host == ""):
            self.setErrorMsg("All fields are required!")
            return

        # Nick length check
        if len(nick) < 4 or len(nick) > 12:
            self.setErrorMsg("Nickname must be 4-12 characters.")
            return
        
        # Port validation
        try:
            port = int(port_raw)
            if not (1024 <= port <= 65535):
                raise ValueError()
        except ValueError:
            self.setErrorMsg("Port must be a number between 1024-65535.")
            return

        self.loginRequest.emit(nick, host, port)

    def setErrorMsg(self, msg: str):
        """
        Shows error msg
        """
        self.status_msg.setText(msg)