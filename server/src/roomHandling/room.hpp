#ifndef ROOM_HPP
#define ROOM_HPP

#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include "../userHandling/user.hpp"
#include "../protocolConfig.hpp"
#include "../utility/utility.hpp"
#include "roomConfig.hpp"

/**
 * Represents the state of a game room.
 */
enum class ROOM_STATE {
    WAITING_FOR_PLAYER, /** Room has 1 player, waiting for opponent */
    PLAYING,            /** Game is currently active */
    FINISHED            /** Game ended (win or draw) */
};

/**
 * Class representing a single game room for Tic-Tac-Toe.
 * Manages players, game board state, and game logic.
 */
class Room {
public:
    int id;                 /** Unique identifier of the room */
    ROOM_STATE state;       /** Current state of the game/room */
    std::string winner_nickname = "";           /** Nickname of the winner (empty if still playing or DRAW) */
    
    /**
     * Constructor to initialize a new room.
     * @param id Unique ID for the room.
     */
    Room(int id);

    /**
     * Tries to add a user to the room.
     * Starts the game automatically if the room becomes full.
     * * @param user Shared pointer to the user to be added.
     * @return true if user was added, false if room is full.
     */
    bool add_player(std::shared_ptr<User> user);

    /**
     * Checks if a specific user is currently in this room.
     * @param fd File descriptor of the user.
     * @return true if user is in the room, otherwise false.
     */
    bool has_player(int fd);

    /**
     * Checks if the room has reached maximum capacity (2 players).
     * @return true if full.
     */
    bool is_full() const;

    /**
     * Checks if the room is completely empty.
     * @return true if no players are in the room.
     */
    bool is_empty() const;
    
    /**
     * Processes a game move from a specific player.
     * Validates the move, updates the board, and checks for win/draw conditions.
     * * @param fd File descriptor of the player making the move.
     * @param x X coordinate.
     * @param y Y coordinate.
     * @return Protocol response string to be sent to the client (e.g., VALID, RESULT, ERROR).
     */
    std::string process_move(int fd, int x, int y);
    
    /**
     * returns the current board state as a string.
     * @return String of length 9 representing the board (e.g., "X..O..X..").
     */
    std::string get_board_string();

    /**
     * Resets the game board and state for a new match.
     */
    void reset_game();
    
    /**
     * Gives information of the current player's symbol
     * @return the symbol ('X' or 'O') of the player who is currently on turn
     */
    char get_current_turn_symbol();
    
    // Getters for players
    std::vector<std::shared_ptr<User>> get_players() const { return players; }

    /**
     * Handles the logic when a player disconnects unexpectedly.
     * If the game was running, it sets the state to FINISHED.
     * @param fd The file descriptor of the disconnected player.
     * @return A shared_ptr to the OPPONENT (the winner) if the game was running, otherwise nullptr.
     */
    std::shared_ptr<User> handle_player_disconnect(int fd);

    /**
     * Gets the opponent of the player with given fd.
     * @param fd File descriptor of the player.
     * @return Shared pointer to the opponent user, or nullptr if not found/only one player.
     */
    std::shared_ptr<User> get_opponent(int fd);

    void remove_player_by_fd(int fd);

private:
    std::vector<std::shared_ptr<User>> players; /** List of players in the room (max 2) */
    std::vector<char> board;                    /** Linear representation of 3x3 board */
    int turn_index;                             /** Index of the player currently on turn */
    
    /**
     * Checks if the given symbol has won the game.
     * @param symbol Character to check ('X' or 'O').
     * @return true if the symbol has a winning line.
     */
    bool check_win(char symbol);

    /**
     * Checks if the game is a draw (board full, no winner).
     * @return true if draw condition is met.
     */
    bool check_draw();

    /**
     * Creates response for the move
     * @param tag String representation of response tag msg
     * @param code Integer representing code response of the tag
     * @return formatted response of server
     */
    std::string response(std::string tag, int code);
};

#endif