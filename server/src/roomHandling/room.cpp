#include "room.hpp"

Room::Room(int room_id) : id(room_id), state(ROOM_STATE::WAITING_FOR_PLAYER), turn_index(0) {
    // Initialize 3x3 board with empty spaces
    board.resize(RoomConfig::BOARD_SIZE, ' '); 
}

// --- Player Management ---

bool Room::add_player(std::shared_ptr<User> user) {
    // Check capacity
    if (players.size() >= RoomConfig::PLAYERS_AMOUNT) return false;

    players.push_back(user);
    user->state = USER_STATE::WAITING; // User is now waiting for game start

    // If room reaches 2 players, start the game
    if (players.size() == RoomConfig::PLAYERS_AMOUNT) {
        state = ROOM_STATE::PLAYING;
        
        // Update users's state to IN_GAME
        for (auto& p : players) {
            p->state = USER_STATE::IN_GAME; 
        }
        
        // Player at index 0 always starts
        turn_index = RoomConfig::FIRST_PLAYER;
        reset_game();
        LOG_INFO("Room " + std::to_string(id) + " -> Game started between " + players[RoomConfig::FIRST_PLAYER]->nickname + " and " + players[RoomConfig::SECOND_PLAYER]->nickname);
    }
    return true;
}

bool Room::has_player(int fd) {
    for (const auto& p : players) {
        if (p->fd_socket == fd) return true;
    }
    return false;
}

bool Room::is_full() const {
    return players.size() == RoomConfig::PLAYERS_AMOUNT;
}

bool Room::is_empty() const {
    return players.empty();
}

// --- Game Logic ---

std::string Room::process_move(int fd, int x, int y) {
    // Check if game is running
    if (state != ROOM_STATE::PLAYING) {
        return response("TURN", Protocol::GAME_NOT_RUN);
    }
    
    // Identify player and check if they belong to this room
    int player_idx = -1;
    if (players[RoomConfig::FIRST_PLAYER]->fd_socket == fd) player_idx = RoomConfig::FIRST_PLAYER;
    else if (players[RoomConfig::SECOND_PLAYER]->fd_socket == fd) player_idx = RoomConfig::SECOND_PLAYER;
    else {
        LOG_WARNING("Room: " + std::to_string(id) + " -> User with fd: " + std::to_string(fd) + " tried to make a move in different room");
        return response("TURN", Protocol::PLAYER_NOT_BELONG);
    }

    // Check if it is this player's turn
    if (player_idx != turn_index) {
        LOG_WARNING("Room: " + std::to_string(id) + "-> player tried to make move even though it is not his turn");
        return response("TURN", Protocol::NOT_YOUR_TURN);
    }

    // Check coordinates boundaries and availability
    int board_idx = y * RoomConfig::Y_CONVERTOR + x; 
    
    if (board_idx < RoomConfig::BOARD_START || board_idx > (RoomConfig::BOARD_SIZE - 1)) {
        LOG_INFO("Room: " + std::to_string(id) + "-> player tried to make invalid move");
        return response("TURN", Protocol::INVALID_MOVE);
    }

    if (board[board_idx] != ' ') {
        LOG_INFO("Room: " + std::to_string(id) + "-> player tried to make move to the occupied field");
        return response("TURN", Protocol::OCCUPIED_FIELD);
    }

    // Update board state
    char symbol = get_current_turn_symbol();
    board[board_idx] = symbol;

    // Check for Win/Draw conditions
    if (check_win(symbol)) {
        state = ROOM_STATE::FINISHED;
        std::string winner_nick = players[player_idx]->nickname;
        LOG_INFO("Room: " + std::to_string(id) + "-> game finished - WINNER is " + winner_nick +"!");
        return response("RESULT", Protocol::WIN) + "|" + get_board_string() + "|" + winner_nick; 
    } else if (check_draw()) {
        state = ROOM_STATE::FINISHED;
        LOG_INFO("Room: " + std::to_string(id) + "-> game finished - DRAW!");
        return response("RESULT", Protocol::DRAW) + "|" + get_board_string();
    }

    // Switch turn to the other player
    turn_index = (turn_index + 1) % 2;
    
    // Return success response with updated board
    LOG_INFO("Room: " + std::to_string(id) + "-> player made valid move");
    return response("TURN", Protocol::VALID_MOVE) + "|" + get_board_string();
}

std::string Room::get_board_string() {
    return std::string(board.begin(), board.end());
}

void Room::reset_game() {
    std::fill(board.begin(), board.end(), ' ');
}

char Room::get_current_turn_symbol() {
    return (turn_index == RoomConfig::FIRST_PLAYER) ? 'X' : 'O';
}

std::shared_ptr<User> Room::handle_player_disconnect(int fd) {
    // Game doesn't start
    if (state == ROOM_STATE::WAITING_FOR_PLAYER) {
        remove_player_by_fd(fd);
        LOG_INFO("Room " + std::to_string(id) + " -> Waiting player disconnected. Removed from room.");
        return nullptr;
    }

    // Game runs
    if (state == ROOM_STATE::PLAYING) {

        std::shared_ptr<User> opponent = nullptr;
        for (const auto& p : players) {
            if (p->fd_socket != fd) {
                opponent = p;
                break;
            }
        }

        LOG_INFO("Room " + std::to_string(id) + " -> Player disconnected. Game PAUSED.");
        return opponent;
    }

    return nullptr;
}

void Room::remove_player_by_fd(int fd) {
    for (auto it = players.begin(); it != players.end(); ++it) {
        if ((*it)->fd_socket == fd) {
            players.erase(it);
            break; 
        }
    }
}

// --- Private Helpers ---

bool Room::check_win(char s) {
    for (int i = 0; i < 3; i++) {
        // Row check
        if (board[i * 3] == s && board[(i * 3) + 1] == s && board[(i * 3) + 2] == s) {
            return true;
        }
        // Column check
        if (board[i] == s && board[i + 3] == s && board[i + 6] == s) {
            return true;
        }
    }

    // Diagonals check
    if (board[0] == s && board[4] == s && board[8] == s) return true;
    if (board[2] == s && board[4] == s && board[6] == s) return true;

    return false;
}

bool Room::check_draw() {
    for (char c : board) {
        if (c == ' ') return false; 
    }
    return true;
}

std::string Room::response(std::string tag, int code) {
    return tag + "|" + std::to_string(code);
}