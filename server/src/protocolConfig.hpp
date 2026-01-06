#ifndef PROTOCOL_CONFIG_HPP
#define PROTOCOL_CONFIG_HPP

#include <cstdint>
#include <string>

namespace Protocol {
    // --- General setup ---
    inline const std::string PROTOCOL_HEADER = "OP23|";
    inline constexpr int DISCONNECTED_USER_SOCKET = -1;
    inline constexpr int MIN_PARTS = 1;
    inline constexpr int COMMAND_POS = 0;
    inline const std::string SPLITTER = "|";
    inline const char SPLITTER_CH = '|';
    inline const std::string PROTOCOL_END = "\n";
    inline const char PROTOCOL_END_CHAR = '\n';

    // --- LOGIN Responses ---
    inline constexpr int NICK_PARAM_POS = 1;
    inline constexpr int LOGIN_LOGGED = 0;
    inline constexpr int LOGIN_RECONNECT = 1;
    inline constexpr int LOGIN_FULL_SERVER = 2;
    inline constexpr int LOGIN_NICK_DUPLICITY = 3;
    inline constexpr int LOGIN_MAX_NICK_LEN = 4;
    inline constexpr int LOGIN_MIN_NICK_LEN = 5;

    // --- ROOM Responses ---
    // Turn response
    inline constexpr int VALID_MOVE = 0;
    inline constexpr int NOT_YOUR_TURN = 1;
    inline constexpr int INVALID_MOVE = 2;
    inline constexpr int OCCUPIED_FIELD = 3;
    inline constexpr int PLAYER_NOT_BELONG = 4;
    inline constexpr int GAME_NOT_RUN = 5;

    // Result response
    inline constexpr int WIN = 0;
    inline constexpr int DRAW = 1;
}

#endif