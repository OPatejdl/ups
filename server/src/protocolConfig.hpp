#ifndef PROTOCOL_CONFIG_HPP
#define PROTOCOL_CONFIG_HPP

#include <cstdint>
#include <string>

namespace Protocol {
    // --- General setup ---
    inline const std::string PROTOCOL_HEADER = "OP23|";         /** Header of the protocol */
    inline constexpr int DISCONNECTED_USER_SOCKET = -1;         /** Value of the fd for DISCONNECTED user */
    inline constexpr int MIN_PARTS = 1;                         /** Minimal parts of protocol msg */
    inline constexpr int COMMAND_POS = 0;                       /** Command position in the protocol msg */
    inline const std::string SPLITTER = "|";                    /** Spliter of protocol parts */
    inline const char SPLITTER_CH = '|';                        
    inline const std::string PROTOCOL_END = "\n";               /** End sign of the protocol */
    inline const char PROTOCOL_END_CHAR = '\n';

    // --- LOGIN Responses ---
    inline constexpr int NICK_PARAM_POS = 1;                    /** Position of nickname parameter for login */
    inline constexpr int LOGIN_LOGGED = 0;                      /** Response code if login successful */
    inline constexpr int LOGIN_RECONNECT = 1;                   /** Response code if login successful and reconnect */
    inline constexpr int LOGIN_FULL_SERVER = 2;                 /** Response code if server is full */
    inline constexpr int LOGIN_NICK_DUPLICITY = 3;              /** Response code if nickname is already taken */
    inline constexpr int LOGIN_MAX_NICK_LEN = 4;                /** Response code if nickname is too long */
    inline constexpr int LOGIN_MIN_NICK_LEN = 5;                /** Response code if nickname is too short */

    // --- ROOM Responses ---
    // Turn response
    inline constexpr int VALID_MOVE = 0;                        /** Response code if move is valid */
    inline constexpr int NOT_YOUR_TURN = 1;                     /** Response code if user made move out of his turn */
    inline constexpr int INVALID_MOVE = 2;                      /** Response code if user made invalid move */
    inline constexpr int OCCUPIED_FIELD = 3;                    /** Response code if user made move to occupied field */
    inline constexpr int PLAYER_NOT_BELONG = 4;                 /** Response code if user tried to make a move within a different game */
    inline constexpr int GAME_NOT_RUN = 5;                      /** Response code if user tried to make move in not running game */

    // Result response
    inline constexpr int WIN = 0;                               /** Response code for game which doesn't end as DRAW */
    inline constexpr int DRAW = 1;                              /** Response code for game which ends as DRAW */

    // --- MOVE ---
    inline constexpr int MOVE_ARGS = 3;                         /** Amount of needed args for move command */
    inline constexpr int MOVE_X_POS = 1;                        /** Index of X coordinate value */
    inline constexpr int MOVE_Y_POS = 2;                        /** Index of Y coordinate value */

    inline const char ST_PLAYER_CHAR = 'X';                     /** Character of first player */
    inline const char ND_PLAYER_CHAR = 'O';                     /** Character of second player */

    // --- WAITING ---
    inline constexpr int VALID_WAITING = 0;                     /** Code for validation of move to waiting queue */
    inline constexpr int INVALID_WAITING = 1;                   /** Code for inform user about full rooms */
}

#endif