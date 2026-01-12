#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

namespace Config {
    // --- Constrains ---
    inline constexpr int MIN_PORT_VALUE = 1024;                             /** Maximal value for port */
    inline constexpr int MAX_PORT_VALUE = 65535;                            /** Minimal value for port */
    inline constexpr int MIN_CLIENT_COUNT = 2;                              /** Minimal clients amount */
    inline constexpr int MIN_PARAM_VALUE = 1;                               /** Minimal value for all parameters */

    // --- Init values ---
    inline constexpr int PORT_INIT = 10000;
    inline constexpr unsigned int CLIENT_INIT_COUNT = 0U;
    inline constexpr unsigned int ROOMS_INIT_COUNT = 0U;
    inline const std::string INIT_IP_ADDRESS = "0.0.0.0";
    inline constexpr size_t MAX_BUFFER_SIZE = 1024;                         /** Maximal size of buffer */
    inline constexpr int BACKLOG_SIZE = 16;
    inline constexpr int ADDITIONAL_STREAM = 1;

    // --- Server Signal Setup ---
    inline constexpr int END_SERVER = 0;
    inline constexpr int START_SERVER = 1;

    // --- Logger Setup ---
    inline constexpr const char* LOGS_FOLDER = "../logs";
    inline constexpr const char* LOGGER_PATH = "../logs/server.log";

    // --- Allowed Timestemps ---
    inline constexpr int DISCONNECT_ALLOWED_TIME_SEC = 60;              /** Allowed time for user in disconnected state */
    inline constexpr int AUTH_TIMEOUT = 30;                             /** Time for client to authorized itself */
    inline constexpr int INACTIVE_ALLOWED_TIME_SEC = 6;                 /** Time for client to be inactive */

    // --- Select Setup ---
    inline constexpr int SEC_TIME = 5;
    inline constexpr int MSEC_TIME = 0;

    // --- Nickname Setup ---
    inline constexpr int MIN_NICK_SIZE = 4;                             /** Minimal chars count of nickname */
    inline constexpr int MAX_NICK_SIZE = 12;                            /** Maximal chars count of nickname */
}

#endif