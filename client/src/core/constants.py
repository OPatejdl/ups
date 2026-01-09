"""
Filename: constants.py
Author: Ondrej Patejdl
Contact: opatejdl@students.zcu.cz
Description: This script defines constants used throughout the python scripts
"""

#######################
# Titles and text
WINDOW_NAME = "Tic-Tac-Toe"
LOG_DIR = "logs"
LOG_NAME = "MyClientLogger"
LOG_FILE_NAME = "clientLog"
INIT_PORT = 0

########################
# Layout Constants
DEFAULT_X_POS = 250
DEFAULT_Y_POS = 250
DEFAULT_WIDTH = 400
DEFAULT_HEIGH = 350

########################
# Scenes
LETTER_FONT = "Arial"
TITLES_SIZE = 20
LABEL_SIZE = 12

# --- Login scene ---
MIN_WIDTH = 400
MAX_WIDTH = 200
LOGIN_STRETCH = 3

# --- Lobby scene ---
INFO_LAYOUT_SPACE = 10
LOBBY_STRETCH_AVG = 1
LOBBY_STRETCH_BOTTOM = 2

# --- Waiting scene ---
SPINNER_SPACE_UP = 10
SPINNER_SPACE_DOWN = 30
LEAVE_BTN_WIDTH = 150
SPINNER_DOTS_OFFSET = 1
SPINNER_DOTS_DIVIDE = 4
SPINNER_DOTS_INIT = 0
SPINNER_TIMER = 500

# --- Game scene --
GAME_TEXT_SIZE = 16
BOARD_SPACING = 10
BOARD_TILE_SIZE = 80
ERROR_MSG_TIME = 2000
GAME_BTN_SPACING = 1

DEFAULT_SPACING = 20
TXT_BOLD = "bold"
########################
# Game constants (3X3 board)
BOARD_SIZE = 3
TOTAL_TILES = 9

########################
# Reconnect and heartbeat
INIT_RECONNECT_ATTEMPTS = 0
MAX_ATTEMPTS = 10
WAIT_TIME = 2
CONNECTION_TIMEOUT = 5

HEARTBEAT_TIME = 2000
INIT_LAST_RESPONSE = 0
MAX_HEARTBEAT = 6.0

########################
# Validation
MIN_PORT = 1024
MAX_PORT = 65535
MIN_NICK_LEN = 4
MAX_NICK_LEN = 12

SHOWN_MSG = 20