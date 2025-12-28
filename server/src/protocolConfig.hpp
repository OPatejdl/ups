#ifndef PROTOCOL_CONFIG_HPP
#define PROTOCOL_CONFIG_HPP

#include <cstdint>
#include <string>

namespace Protocol {
    // --- General setup ---
    inline const std::string PROTOCOL_HEADER = "OP23|";
    inline constexpr int DISCONNECTED_USER_SOCKET = -1;
    inline constexpr int MIN_PARTS = 2;
    inline constexpr int COMMAND_POS = 0;

    // --- LOGIN Responses ---
    inline constexpr int NICK_PARAM_POS = 1;
    inline constexpr int LOGIN_LOGGED = 0;
    inline constexpr int LOGIN_RECONNECT = 1;
    inline constexpr int LOGIN_FULL_SERVER = 2;
    inline constexpr int LOGIN_NICK_DUPLICITY = 3;
    inline constexpr int LOGIN_MAX_NICK_LEN = 4;
    inline constexpr int LOGIN_MIN_NICK_LEN = 5;
}

#endif