#include "server.hpp"

namespace MyServer {

    Server::Server() {
        create_server_socket();
        bind_server();
        server_listen();

        LOG_INFO("Server setup: \n\tPort: " + std::to_string(Utility::PORT) + 
                "\n\tClients: " + std::to_string(Utility::CLIENTS_COUNT) +
                "\n\tRooms: " + std::to_string(Utility::ROOMS_COUNT)
        );

        FD_ZERO(&current_sockets);
        FD_SET(server_socket, &current_sockets);
    }

    Server::~Server() {
        LOG_INFO("Ending server");
        close(server_socket);
    }

    /**
    * Main loop of server 
    */
    void Server::run_server() {
        // Init actual max file descriptor
        int max_fd = server_socket;

        while(Utility::server_running) {
            struct timeval tv;
            tv.tv_sec = Config::SEC_TIME;
            tv.tv_usec = Config::MSEC_TIME;

            ready_sockets = current_sockets;

            return_value = select(max_fd + 1, &ready_sockets, NULL, NULL, &tv);
            
            if (return_value < 0) {
                // Check if error was not caused by Ctrl+C
                if (errno == EINTR) {
                    continue;
                }
                LOG_ERROR("Select failed");
                break;
            }

            // Check users activity
            auto dead_sockets = UserManager::get_timeouted_users(std::chrono::seconds(Config::INACTIVE_ALLOWED_TIME_SEC));
            
            for (int dead_fd : dead_sockets) {
                LOG_WARNING("User timeout detected (no activity) on fd: " + std::to_string(dead_fd));
                handle_disconnection(dead_fd);
            }
            
            // Cleanup process
            auto expired_users = UserManager::cleanup_users(std::chrono::seconds(Config::DISCONNECT_ALLOWED_TIME_SEC));

            if (!expired_users.empty()) {
                for (auto& user : expired_users) {
                    // Check for room
                    auto room = RoomManager::get_room_by_user_fd(user->fd_socket);

                    if (room) {
                        LOG_INFO("Cleaning up room: " + std::to_string(room->id) + " due to user timeout.");

                        // Inform opponent
                        auto opponent = room->get_opponent(user->fd_socket);

                        if (opponent) {
                            opponent->state = USER_STATE::CONNECTED;

                            // Send msg to opponent
                            std::string end_msg = Protocol::PROTOCOL_HEADER + "GAME" +
                                                Protocol::SPLITTER + "ENDED" +
                                                Protocol::PROTOCOL_END;
                            send_all(opponent->fd_socket, end_msg);

                            // To ensure sync send one more SYNC msg
                            std::string sync_msg = Protocol::PROTOCOL_HEADER + "SYNC" + 
                                                Protocol::SPLITTER + "LOBBY" +
                                                Protocol::PROTOCOL_END;
                            send_all(opponent->fd_socket, sync_msg);
                        }
                        RoomManager::remove_room(room->id);
                    }
                }
            }

            cleanup_unauth_sockets();

            if (return_value == 0) {
                continue; 
            }

            for (fd = 0; fd <= max_fd; fd++) {
                if (!FD_ISSET(fd, &ready_sockets)) continue;

                if (fd == server_socket) {
                    addr = sizeof(peer_addr);

                    client_socket = accept(server_socket, (struct sockaddr *) &peer_addr, &addr);
                    if (client_socket < 0) {
                        LOG_WARNING("Error when loading new client!");
                        continue;
                    }
                    
                    if (client_socket > max_fd) {
                        max_fd = client_socket;
                    }

                    new_client_connection();
                }
                else {
                    handle_client_data();
                }
            }
        }
    }

    /////////////////////////////////////////////
    // Private Functions

    // ====================================================
    // --------- Function Needed for Server Init --------

    void Server::create_server_socket() {
        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket < 0) {
            LOG_ERROR("Unable to create server socket");
            throw MyExceptions::ServerException(Utility::ERROR_UNCREATED_SERVER_SOC);
        } else {
            LOG_INFO("Server socket was created");
        }
    }

    void Server::bind_server() {
        memset(&my_addr, 0, sizeof(struct sockaddr_in));
        my_addr.sin_family = AF_INET;
        my_addr.sin_port = htons(Utility::PORT);
        my_addr.sin_addr.s_addr = INADDR_ANY;

        return_value = bind(server_socket, (struct sockaddr *) &my_addr, sizeof(my_addr));
        if (return_value != 0) {
            LOG_ERROR("Binding of server socket failed");
            throw  MyExceptions::ServerException(Utility::ERROR_BINDING);
        } else {
            LOG_INFO("Binding successful");
        }
    }

    void Server::server_listen() {
        return_value = listen(server_socket, Config::BACKLOG_SIZE);
        if (return_value != 0) {
            LOG_ERROR("Listen - FAILED");
            throw MyExceptions::ServerException(Utility::ERROR_LISTEN);
        } else {
            LOG_INFO("Listening successful");
        }
    }

    // ====================================================
    // ----------- Function needed for server run --------

    void Server::new_client_connection() {
        FD_SET(client_socket, &current_sockets);

        unauth_sockets[client_socket].joined_time = std::chrono::steady_clock::now();

        LOG_INFO("New client socket connected on fd: " + std::to_string(client_socket));

        msg = Protocol::PROTOCOL_HEADER + "AUTH|1\n";
        send_all(client_socket, msg);
    }

    void Server::handle_client_data() {
        memset(buffer, 0, Config::MAX_BUFFER_SIZE);
        int bytes_recv = recv(fd, buffer, Config::MAX_BUFFER_SIZE, 0);

        if (bytes_recv <= 0) {
            handle_disconnection(fd);
            return;
        }

        // Store received msg
        std::string* active_buffer;
        auto user = UserManager::get_user_by_fd(fd);
        if (user) {
            active_buffer = &(user->partial_msg);
        } else {
            active_buffer = &(unauth_sockets[fd].buffer);
        }

        active_buffer->append(buffer, bytes_recv);

        // Handle msg
        size_t pos;
        while ((pos = active_buffer->find(Protocol::PROTOCOL_END_CHAR)) != std::string::npos) {
            std::string msg_to_process = active_buffer->substr(0, pos);
            active_buffer->erase(0, pos + 1);

            if (!process_msg(fd, msg_to_process)) {
                break;
            }

            user = UserManager::get_user_by_fd(fd);
            if (user) {
                active_buffer = &(user->partial_msg);
            } else if (unauth_sockets.count(fd)) {
                active_buffer = &(unauth_sockets[fd].buffer);
            } else {
                // Closed socket
                break;
            }
        }
    }

    void Server::handle_disconnection(int fd_disconnected) {

        LOG_INFO("Handling disconnection for fd: " + std::to_string(fd_disconnected));
        
        // Ensure users integrity
        auto user = UserManager::get_user_by_fd(fd_disconnected);

        if (user) {
            if (user->fd_socket != fd_disconnected) {
                LOG_WARNING("Ignoring disconnect logic for old fd: " + std::to_string(fd_disconnected) + 
                            ". User " + user->nickname + " is already active on fd: " + std::to_string(user->fd_socket));
                
                unauth_sockets.erase(fd_disconnected);
                close(fd_disconnected);
                FD_CLR(fd_disconnected, &current_sockets);
                return;
            }
        }

        // Check rooms
        auto room = RoomManager::get_room_by_user_fd(fd_disconnected);
        
        if (room) {
            auto opponent = room->handle_player_disconnect(fd_disconnected);
    
            // Inform opponent if exist
            if (opponent && room->state != ROOM_STATE::FINISHED) {
                std::string msg = Protocol::PROTOCOL_HEADER + "GAME" +
                                Protocol::SPLITTER + "PAUSED" + 
                                Protocol::PROTOCOL_END;
                
                send_all(opponent->fd_socket, msg);
            }

            // Delete room if empty
            if (room->get_players().empty()) {
                RoomManager::remove_room(room->id);
                LOG_INFO("Room " + std::to_string(room->id) + " deleted because it is empty.");
            }
        }

        // Disconnect user
        UserManager::disconnect_user(fd_disconnected);

        // Cleaning
        unauth_sockets.erase(fd_disconnected);
        close(fd_disconnected);
        FD_CLR(fd_disconnected, &current_sockets);
    }

    void Server::remove_client(int client_fd) {
        // Inform client
        std::string err_msg = "Error: Invalid protocol" + Protocol::PROTOCOL_END;
        send_all(client_fd, err_msg);

        // Remove client
        close(client_fd);
        FD_CLR(client_fd, &current_sockets);
        UserManager::remove_user(client_fd);
        unauth_sockets.erase(client_fd);
    };

    void Server::cleanup_unauth_sockets() {
        auto now = std::chrono::steady_clock::now();
        auto timeout = std::chrono::seconds(Config::AUTH_TIMEOUT);

        for (auto it = unauth_sockets.begin(); it != unauth_sockets.end(); ) {
            // Check timeout
            if (now - it->second.joined_time > timeout) {
                int fd_to_close = it->first;
                LOG_WARNING("Anonymous connection timeout on fd: " + std::to_string(fd_to_close));
                
                // Remove socket 
                close(fd_to_close);
                FD_CLR(fd_to_close, &current_sockets);
            
                it = unauth_sockets.erase(it);
            } else {
                ++it;
            }
        }
    }

    bool Server::process_msg(int client_fd, std::string msg) {
        // Validate protocol
        std::string header = Protocol::PROTOCOL_HEADER;

        if (msg.size() < header.size() || msg.substr(0, header.size()) != header) {
            // Remove user in case of invalid protocol
            LOG_WARNING("Invalid protocol header from fd: " + std::to_string(client_fd));
            remove_client(client_fd);
            return false;
        }


        // Remove and split msg
        std::string payload = msg.substr(header.size());
        std::vector<std::string> parts = Utility::split(payload, '|');
        if (parts.size() < Protocol::MIN_PARTS) return false;

        std::shared_ptr<User> user = UserManager::get_user_by_fd(client_fd);
        std::string command = parts[Protocol::COMMAND_POS];

        if (user == nullptr) {
            // Unknown user - only LOGIN|<param>
            if (command == "LOGIN") {
                return handle_login(client_fd, parts);
            }
        } else {
            // Set activity
            user->last_active = std::chrono::steady_clock::now();

            if (command == "FIND") {
                handle_find(client_fd, user);
            }
            else if (command == "MOVE") {
                LOG_INFO("User" + user->nickname + " sent MOVE msg.");
                handle_move(client_fd, user, parts);
            }
            else if (command == "SYNC") {
                LOG_INFO("User" + user->nickname + " sent SYNC msg.");
                handle_sync(client_fd, user);
            }
            else if (command == "PING") {
                LOG_INFO("User" + user->nickname + " sent PING msg.");
                handle_ping(client_fd, user);
            }
            else if (command == "REMATCH") {
                LOG_INFO("User" + user->nickname + " sent REMATCH msg.");
                handle_rematch(client_fd, user);
            } 
            else if (command == "LEAVE") {
                LOG_INFO("User" + user->nickname + " sent LEAVE msg.");
                handle_leave(client_fd, user);
            }
            else if (command == "DISCONNECT") {
                LOG_INFO("User" + user->nickname + " sent DISCONNECT msg.");
                handle_disconnection(client_fd);
                return true;
            }
        } 

        return false;
    }

    bool Server::send_all(int socket_fd, const std::string& data) {
        const char* ptr = data.c_str();
        size_t remaining = data.size();
        ssize_t sent = 0;

        while (remaining > 0) {
            sent = send(socket_fd, ptr, remaining, 0);
            
            if (sent == -1) {
                LOG_ERROR("Sending failed to fd: " + std::to_string(socket_fd));
                return false; // Connection error
            }
            
            ptr += sent;
            remaining -= sent;
        }
        return true; // All went fine
    }

    // ====================================================
    // ------------- Msg Handling Functions --------

    bool Server::handle_login(int client_fd, const std::vector<std::string>& parts) {
        // Validation of parameters
        if (parts.size() <= Protocol::NICK_PARAM_POS) {
            remove_client(client_fd);
            return false;
        }

        int rsp_code = UserManager::handle_login(client_fd, parts[Protocol::NICK_PARAM_POS]);

        if (rsp_code < Protocol::LOGIN_FULL_SERVER) {
            unauth_sockets.erase(client_fd);
        } else {
            unauth_sockets[client_fd].joined_time = std::chrono::steady_clock::now();
        }

        // Send response
        std::string rsp_msg = Protocol::PROTOCOL_HEADER + "LOGIN" +
                            Protocol::SPLITTER + std::to_string(rsp_code) +
                            Protocol::PROTOCOL_END;
        
        send_all(client_fd, rsp_msg);

        return true;
    }

    bool Server::handle_find(int client_fd, std::shared_ptr<User> user) {
        LOG_INFO("User with fd: " + std::to_string(client_fd) + " tries to find a game");
        if (user->state == USER_STATE::IN_GAME) return false;

        auto room = RoomManager::join_waiting_room(user);

        if (!room) {
            std::string err = Protocol::PROTOCOL_HEADER + "ROOM_ERROR" + Protocol::PROTOCOL_END;
            send_all(client_fd, err);
            return true;
        }

        // New game if full room
        if (room->state == ROOM_STATE::PLAYING) {
            auto players = room->get_players();
            
            // MSG to both players - GAME|<start_symbol>|<opponent nick>|<board>
            std::string msg1 = Protocol::PROTOCOL_HEADER + "GAME" + 
                            Protocol::SPLITTER + "START_X" +
                            Protocol::SPLITTER + players[1]->nickname +
                            Protocol::SPLITTER + room->get_board_string() +
                            Protocol::PROTOCOL_END;
            
            send_all(players[0]->fd_socket, msg1);

            std::string msg2 = Protocol::PROTOCOL_HEADER + "GAME" + Protocol::SPLITTER + "START_O" + 
                                Protocol::SPLITTER + players[0]->nickname + 
                                Protocol::SPLITTER + room->get_board_string() + Protocol::PROTOCOL_END;
            send_all(players[1]->fd_socket, msg2);
            
            LOG_INFO("Match started in Room " + std::to_string(room->id));
        } else {
            // Waiting for another player
            std::string wait_msg = Protocol::PROTOCOL_HEADER + "WAITING" + Protocol::PROTOCOL_END;
            send_all(client_fd, wait_msg);
        }

        return true;
    }

    bool Server::handle_move(int client_fd, std::shared_ptr<User> user, const std::vector<std::string>& parts) {
        if (parts.size() < 3) {
            return false;
        }

        auto room = RoomManager::get_room_by_user_fd(client_fd);
        if (!room) return false; // Player not in the room

        try {
            int x = std::stoi(parts[1]);
            int y = std::stoi(parts[2]);

            // Process move - returns game_rsp
            std::string game_response = room->process_move(client_fd, x, y);
            bool is_error = false;

            // Error check for move
            if (game_response.find("TURN") != std::string::npos) {
                is_error = (game_response.find(Protocol::SPLITTER + std::to_string(Protocol::OCCUPIED_FIELD)) != std::string::npos) ||
                                (game_response.find(Protocol::SPLITTER + std::to_string(Protocol::NOT_YOUR_TURN)) != std::string::npos) ||
                                (game_response.find(Protocol::SPLITTER + std::to_string(Protocol::INVALID_MOVE)) != std::string::npos) ||
                                (game_response.find(Protocol::SPLITTER + std::to_string(Protocol::PLAYER_NOT_BELONG)) != std::string::npos) ||
                                (game_response.rfind("ERROR", 0) == 0);
            }

            if (is_error) {
                std::string full_msg = Protocol::PROTOCOL_HEADER + game_response + Protocol::PROTOCOL_END;
                send_all(client_fd, full_msg);
            } 
            else {
                // Valid move or game finished - inform both players
                std::string full_msg;

                if (game_response.find("RESULT") == std::string::npos) {
                    std::string next_turn_sym = std::string(1, room->get_current_turn_symbol());
                    full_msg = Protocol::PROTOCOL_HEADER + game_response +
                            Protocol::SPLITTER + next_turn_sym +
                            Protocol::PROTOCOL_END;
            
                } else {
                    // Game finished
                    full_msg = Protocol::PROTOCOL_HEADER + game_response + Protocol::PROTOCOL_END;
                }

                auto players = room->get_players();
                for (auto& p : players) {
                    send_all(p->fd_socket, full_msg);
                }
            }

            return true;
        } catch (const std::exception& e) {
            LOG_WARNING("Invalid integer format in MOVE command from fd: " + std::to_string(client_fd));
            return false;
        }

    }

    bool Server::handle_sync(int client_fd, std::shared_ptr<User> user) {
        std::string sync_msg;

        switch (user->state) {
            case USER_STATE::WAITING:
                sync_msg = "SYNC" + Protocol::SPLITTER + "WAITING";
                break;

            case USER_STATE::IN_GAME: {
                auto room = RoomManager::get_room_by_user_fd(client_fd);
                if (room) {
                    char my_symbol = (room->get_players()[RoomConfig::FIRST_PLAYER]->fd_socket == client_fd) ? 'X' : 'O';
                    
                    // Get user nick
                    auto opponent = room->get_opponent(client_fd);
                    std::string opponent_nick = (opponent) ? opponent->nickname : "Unknown";

                    sync_msg = "SYNC" + 
                            Protocol::SPLITTER + "GAME" +
                            Protocol::SPLITTER + std::string(1, my_symbol) + 
                            Protocol::SPLITTER + room->get_board_string() +
                            Protocol::SPLITTER + std::string(1, room->get_current_turn_symbol()) +
                            Protocol::SPLITTER + opponent_nick;
                    
                    // Inform opponent
                    for(auto& p : room->get_players()) {
                        if(p->fd_socket != client_fd) {
                            std::string res_msg = Protocol::PROTOCOL_HEADER + "GAME" +
                                                Protocol::SPLITTER + "RESUMED" + 
                                                Protocol::SPLITTER + std::string(1, room->get_current_turn_symbol()) + 
                                                Protocol::PROTOCOL_END;
                            send_all(p->fd_socket, res_msg);
                        }
                    }
                } else {
                    // User IN_GAME but game ended
                    user->state = USER_STATE::CONNECTED;
                    sync_msg = "SYNC" + Protocol::SPLITTER + "LOBBY";
                }
                break;
            }
            
            case USER_STATE::RESULT: {
                auto room = RoomManager::get_room_by_user_fd(client_fd);
                if (room) {
                    // Find opponent nickname
                    auto opponent = room->get_opponent(client_fd);
                    std::string opponent_nick = (opponent) ? opponent->nickname : "Unknown";

                    // SYNC|RESULT|<opponent_nick>|<board>|<winner_nick>
                    sync_msg = "SYNC"+ Protocol::SPLITTER + "RESULT" + 
                            Protocol::SPLITTER + opponent_nick + 
                            Protocol::SPLITTER + room->get_board_string() +
                            Protocol::SPLITTER + room->winner_nickname;
                } else {
                    user->state = USER_STATE::CONNECTED;
                    sync_msg = "SYNC" + Protocol::SPLITTER + "LOBBY";
                }
                break;
            }

            case USER_STATE::CONNECTED:
            default:
                sync_msg = "SYNC" + Protocol::SPLITTER + "LOBBY";
                break;
        }

        // Send msg
        std::string full_msg = Protocol::PROTOCOL_HEADER + sync_msg + Protocol::PROTOCOL_END;
        send_all(client_fd, full_msg);
        return true;
    }

    bool Server::handle_ping(int client_fd, std::shared_ptr<User> user) {
        std::string state_str = "LOBBY";
    
        switch (user->state) {
            case USER_STATE::WAITING:
                state_str = "WAITING";
                break;
            case USER_STATE::IN_GAME:
                state_str = "GAME"; 
                break;
            case USER_STATE::RESULT:
                state_str = "RESULT";
                break;
            case USER_STATE::CONNECTED:
            default:
                state_str = "LOBBY";
                break;
        }
        
        std::string pong = Protocol::PROTOCOL_HEADER + "PONG" +
                        Protocol::SPLITTER + state_str + 
                        Protocol::PROTOCOL_END;
        send_all(client_fd, pong);

        return true;
    }

    bool Server::handle_rematch(int client_fd, std::shared_ptr<User> user) {
        if (user->state != USER_STATE::RESULT) return false;

        auto room = RoomManager::get_room_by_user_fd(client_fd);
        if (!room) return false;

        // Vote for rematch
        room->vote_rematch(client_fd);

        // Rematch check
        if (room->check_rematch_ready()) {
            // Rematch starts
            LOG_INFO("Rematch accepted in Room " + std::to_string(room->id));

            room->reset_game();
            room->state = ROOM_STATE::PLAYING;

            // Sets players state IN_GAME
            auto players = room->get_players();
            for (auto& p : players) p->state = USER_STATE::IN_GAME;
            
            char current_symbol = room->get_current_turn_symbol();
            int turn_index = room->get_turn_index();
            
            for (auto& p : players) {
                char p_sym = (p->fd_socket == players[turn_index]->fd_socket) ? current_symbol : ((current_symbol == 'X') ? 'O' : 'X');
                auto opp = room->get_opponent(p->fd_socket);
                
                // MSG: GAME|START_X|<opp_nick>|<board>
                std::string msg = Protocol::PROTOCOL_HEADER + "GAME" +
                                Protocol::SPLITTER + "START_" + std::string(1, p_sym) +
                                Protocol::SPLITTER + opp->nickname +
                                Protocol::SPLITTER + room->get_board_string() +
                                Protocol::PROTOCOL_END;
                send_all(p->fd_socket, msg);
            }
        } else {
            // Inform about waiting for opponent response
            send_all(client_fd, Protocol::PROTOCOL_HEADER + "GAME" + Protocol::SPLITTER + "REMATCH_WAIT" + Protocol::PROTOCOL_END);
        }
        return true;
    }

    bool Server::handle_leave(int client_fd, std::shared_ptr<User> user) {
        auto room = RoomManager::get_room_by_user_fd(client_fd);
        
        // User left a game
        if (room) {
            LOG_INFO("User " + user->nickname + " left the room explicitly.");

            // Find opponent and inform him about game finish
            auto opponent = room->get_opponent(client_fd);
            if (opponent) {
                opponent->state = USER_STATE::CONNECTED;
                
                std::string end_msg = Protocol::PROTOCOL_HEADER + "GAME" +
                                    Protocol::SPLITTER + "ENDED" +
                                    Protocol::PROTOCOL_END;
                
                send_all(opponent->fd_socket, end_msg);
                
                room->remove_player_by_fd(opponent->fd_socket);
            }

            // Sync client
            user->state = USER_STATE::CONNECTED;
            std::string confirm_msg = Protocol::PROTOCOL_HEADER + "SYNC" +
                                    Protocol::SPLITTER + "LOBBY" +
                                    Protocol::PROTOCOL_END;
            
            send_all(client_fd, confirm_msg);
            
            room->remove_player_by_fd(client_fd);

            // Remove room if possible
            if (room->is_empty()) {
                RoomManager::remove_room(room->id);
            }
        }
        return true;
    }
}