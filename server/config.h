#ifndef CONFIG_H
#define CONFIG_H

namespace Config {
    // --- Constrains ---
    inline constexpr int MIN_PORT_VALUE = 1024;
    inline constexpr int MAX_PORT_VALUE = 65535;
    inline constexpr int MIN_CLIENT_COUNT = 2;

    // --- Init values ---
    // Používá se pro počáteční hodnoty
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
    inline constexpr const char* LOGS_FOLDER = "logs";
    inline constexpr const char* LOGGER_PATH = "logs/server.log";
}

#endif