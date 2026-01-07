#ifndef ROOM_CONFIG_HPP
#define ROOM_CONFIG_HPP

namespace RoomConfig {
    inline const int BOARD_SIZE = 9;                /** Board size */
    inline const int BOARD_START = 0;               /** Start index of board */
    inline const int BOARD_OFFSET = 1;              /** Offset for setting last index of board   */
    inline const int Y_CONVERTOR = 3;
    inline const char EMPTY_TILE = ' ';             /** Char representing empty tile */

    inline const int TILES_IN_LINE = 3;             /** Amount of tiles in a direction */
    inline const int ROW_ND_OFFSET = 1;             /** Offset of second tile in a row */
    inline const int ROW_TH_OFFSET = 2;             /** Offset of third tile in a row */

    inline const int COLUMN_ND_OFFSET = 3;          /** Offset of second tile in a column */
    inline const int COLUMN_TH_OFFSET = 6;           /** Offset of third tile in a column */

    inline const int PLAYERS_AMOUNT = 2;            /** Amount of players */
    inline const int FIRST_PLAYER = 0;              /** Index of first player */
    inline const int SECOND_PLAYER = 1;             /** Index of second player */

    inline const int INIT_ROOM_ID = 1;              /** Init value for room id */

    inline const int NEXT_TURN = 1;                 /** Next turn offset */

    /** Indexes of diagonals */
    inline const int ST_DIAGONAL_ST = 0;
    inline const int ST_DIAGONAL_ND = 4;
    inline const int ST_DIAGONAL_TH = 8;

    inline const int ND_DIAGONAL_ST = 2;
    inline const int ND_DIAGONAL_ND = 4;
    inline const int ND_DIAGONAL_TH = 6;
}

#endif