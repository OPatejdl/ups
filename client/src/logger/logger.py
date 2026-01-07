"""
Filename: logger.py
Author: Ondrej Patejdl
Contact: opatejdl@students.zcu.cz
Description: This script defines logger logic of the client's app
"""

import logging
import os
from datetime import datetime
from src.core.constants import (
    LOG_DIR, LOG_NAME, LOG_FILE_NAME
    )

def setupLogger():
    if not os.path.exists(LOG_DIR):
        os.makedirs(LOG_DIR)

    logger = logging.getLogger(LOG_NAME)
    logger.setLevel(logging.DEBUG)

    # Formats
    file_formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - [%(threadName)s] - %(message)s')
    console_formatter = logging.Formatter('%(levelname)s: %(message)s')

    # File handler - stores all
    file_handler = logging.FileHandler(
        f"{LOG_DIR}/{LOG_FILE_NAME}_{datetime.now().strftime('%Y-%m-%d')}.log", 
        encoding='utf-8'
    )
    file_handler.setLevel(logging.DEBUG)
    file_handler.setFormatter(file_formatter)

    # Console - INFO and higher priority
    console_handler = logging.StreamHandler()
    console_handler.setLevel(logging.INFO)
    console_handler.setFormatter(console_formatter)

    logger.addHandler(file_handler)
    logger.addHandler(console_handler)

    return logger

# Init logger
client_logger = setupLogger()
