#include "room.hpp"

Room::Room(int room_id) : 
    id(room_id), 
    state(ROOM_STATE::WAITING_FOR_PLAYER), 
    turn_index(RoomConfig::FIRST_PLAYER) {
    // Initialize 3x3 board with empty spaces
    board.resize(RoomConfig::BOARD_SIZE, RoomConfig::EMPTY_TILE); 
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
        
        // Player at index 0 starts
        reset_game();
        swap_players();
        turn_index = RoomConfig::FIRST_PLAYER;
        
        LOG_INFO("Room " + std::to_string(id) + " -> Game started between " + 
            players[RoomConfig::FIRST_PLAYER]->nickname + " and " + 
            players[RoomConfig::SECOND_PLAYER]->nickname);
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
        LOG_WARNING("Room: " + std::to_string(id) + " -> User with fd: " + 
            std::to_string(fd) + " tried to make a move in different room");
        return response("TURN", Protocol::PLAYER_NOT_BELONG);
    }

    // Check if it is this player's turn
    if (player_idx != turn_index) {
        LOG_WARNING("Room: " + std::to_string(id) + 
            " -> player tried to make move even though it is not his turn");
        return response("TURN", Protocol::NOT_YOUR_TURN);
    }

    // Check coordinates boundaries and availability
    int board_idx = y * RoomConfig::Y_CONVERTOR + x; 
    
    if (board_idx < RoomConfig::BOARD_START || board_idx > (RoomConfig::BOARD_SIZE - RoomConfig::BOARD_OFFSET)) {
        LOG_INFO("Room: " + std::to_string(id) + "-> player tried to make invalid move");
        return response("TURN", Protocol::INVALID_MOVE);
    }

    if (board[board_idx] != RoomConfig::EMPTY_TILE) {
        LOG_INFO("Room: " + std::to_string(id) + "-> player tried to make move to the occupied field");
        return response("TURN", Protocol::OCCUPIED_FIELD);
    }

    // Update board state
    char symbol = get_current_turn_symbol();
    board[board_idx] = symbol;

    // Check for Win/Draw conditions
    if (check_win(symbol)) {
        state = ROOM_STATE::FINISHED;

        // Set winner and users result's state
        this->winner_nickname = players[player_idx]->nickname;
        for (auto& p : players) p->state = USER_STATE::RESULT;

        LOG_INFO("Room: " + std::to_string(id) + "-> game finished - WINNER is " + this->winner_nickname +"!");
        return response("RESULT", Protocol::WIN) +
                Protocol::SPLITTER + this->winner_nickname;

    } else if (check_draw()) {
        state = ROOM_STATE::FINISHED;

        // Set winner empty and make user's state to RESULT
        this->winner_nickname = "";
        for (auto& p : players) p->state = USER_STATE::RESULT;

        LOG_INFO("Room: " + std::to_string(id) + "-> game finished - DRAW!");
        return response("RESULT", Protocol::DRAW);
    }

    // Switch turn to the other player
    turn_index = (turn_index + RoomConfig::NEXT_TURN) % RoomConfig::PLAYERS_AMOUNT;
    
    // Return success response with updated board
    LOG_INFO("Room: " + std::to_string(id) + "-> player made valid move");
    return response("TURN", Protocol::VALID_MOVE);
}

std::string Room::get_board_string() {
    return std::string(board.begin(), board.end());
}

void Room::reset_game() {
    // Clear field
    std::fill(board.begin(), board.end(), RoomConfig::EMPTY_TILE);
    winner_nickname = "";
    clear_votes();

    // Swap players
    swap_players();
    turn_index = RoomConfig::FIRST_PLAYER;
}

char Room::get_current_turn_symbol() {
    return (turn_index == RoomConfig::FIRST_PLAYER) ? Protocol::ST_PLAYER_CHAR : Protocol::ND_PLAYER_CHAR;
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
        std::shared_ptr<User> opponent = get_opponent(fd);
        LOG_INFO("Room " + std::to_string(id) + " -> Player disconnected. Game PAUSED.");
        return opponent;
    }

    return nullptr;
}

std::shared_ptr<User> Room::get_opponent(int fd) {
    for (const auto& p : players) {
        if (p->fd_socket != fd) {
            return p;
        }
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

void Room::vote_rematch(int fd) {
    if (std::find(rematch_votes.begin(), rematch_votes.end(), fd) == rematch_votes.end()) {
        rematch_votes.push_back(fd);
    }
}

bool Room::check_rematch_ready() {
    return rematch_votes.size() == players.size() && players.size() == RoomConfig::PLAYERS_AMOUNT;
}

void Room::clear_votes() {
    rematch_votes.clear();
}

// --- Private Helpers ---

bool Room::check_win(char s) {
    int till = RoomConfig::TILES_IN_LINE;

    for (int i = 0; i < till; i++) {
        // Row check
        if (board[i * till] == s && 
            board[(i * till) + RoomConfig::ROW_ND_OFFSET] == s && 
            board[(i * till) + RoomConfig::ROW_TH_OFFSET] == s) {
            return true;
        }
        // Column check
        if (board[i] == s && 
            board[i + RoomConfig::COLUMN_ND_OFFSET] == s && 
            board[i + RoomConfig::COLUMN_TH_OFFSET] == s) {
            return true;
        }
    }

    // Diagonals check
    if (board[RoomConfig::ST_DIAGONAL_ST] == s && 
        board[RoomConfig::ST_DIAGONAL_ND] == s && 
        board[RoomConfig::ST_DIAGONAL_TH] == s) return true;
    
    if (board[RoomConfig::ND_DIAGONAL_ST] == s && 
        board[RoomConfig::ND_DIAGONAL_ND] == s && 
        board[RoomConfig::ND_DIAGONAL_TH] == s) return true;

    return false;
}

bool Room::check_draw() {
    for (char c : board) {
        if (c == RoomConfig::EMPTY_TILE) return false; 
    }
    return true;
}

std::string Room::response(std::string tag, int code) {
    return tag +
            Protocol::SPLITTER + std::to_string(code) +
            Protocol::SPLITTER + get_board_string();
}

void Room::swap_players() {
    if (players.size() == RoomConfig::PLAYERS_AMOUNT) {
        std::iter_swap(players.begin(), players.begin() + 1);
    }
}