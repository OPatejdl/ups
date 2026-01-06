#ifndef CONFIG_HPP
#define CONFIG_HPP

namespace Config {
    // --- Constrains ---
    inline constexpr int MIN_PORT_VALUE = 1024;
    inline constexpr int MAX_PORT_VALUE = 65535;
    inline constexpr int MIN_CLIENT_COUNT = 2;

    // --- Init values ---
    inline constexpr int PORT_INIT = 10000;
    inline constexpr unsigned int CLIENT_INIT_COUNT = 0U;
    inline constexpr unsigned int ROOMS_INIT_COUNT = 0U;
    inline constexpr size_t MAX_BUFFER_SIZE = 1024;
    inline constexpr int BACKLOG_SIZE = 16;
    inline constexpr int ADDITIONAL_STREAM = 1;

    // --- Server Signal Setup ---
    inline constexpr int END_SERVER = 0;
    inline constexpr int START_SERVER = 1;

    // --- Logger Setup ---
    inline constexpr const char* LOGS_FOLDER = "../logs";
    inline constexpr const char* LOGGER_PATH = "../logs/server.log";

    // --- Allowed Timestemps ---
    inline constexpr int DISCONNECT_ALLOWED_TIME_SEC = 60;
    inline constexpr int AUTH_TIMEOUT = 30;
    inline constexpr int INACTIVE_ALLOWED_TIME_SEC = 6;

    // --- Select Setup ---
    inline constexpr int SEC_TIME = 5;
    inline constexpr int MSEC_TIME = 0;

    // --- Nickname Setup ---
    inline constexpr int MIN_NICK_SIZE = 4;
    inline constexpr int MAX_NICK_SIZE = 12;
}

#endif