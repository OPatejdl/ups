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
        struct timeval tv;
        tv.tv_sec = Config::SEC_TIME;
        tv.tv_usec = Config::MSEC_TIME;

        while(Utility::server_running) {
            ready_sockets = current_sockets;

            return_value = select(FD_SETSIZE, &ready_sockets, NULL, NULL, &tv);
            if (return_value < 0) {
                
                // Check if error was not caused by Ctrl+C
                if (errno == EINTR) {
                    continue;
                }

                LOG_ERROR("Select failed");
                break;
            } else {
                // Check cleanups
                UserManager::cleanup_users(std::chrono::seconds(Config::ALLOWED_TIME_SEC));
                cleanup_unauth_sockets();
            }

            for (fd = 0; fd < FD_SETSIZE; fd++) {
                if (!FD_ISSET(fd, &ready_sockets)) continue;

                if (fd == server_socket) {
                    addr = sizeof(peer_addr);

                    client_socket = accept(server_socket, (struct sockaddr *) &peer_addr, &addr);
                    if (client_socket < 0) {
                        LOG_WARNING("Error when loading new client!");
                        continue;
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
    void Server::create_server_socket() {
        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket < 0) {
            LOG_ERROR("Unable to create server socket\n");
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

    void Server::new_client_connection() {
        FD_SET(client_socket, &current_sockets);

        unauth_sockets[client_socket].joined_time = std::chrono::steady_clock::now();

        LOG_INFO("New client socket connected on fd: " + std::to_string(client_socket));

        msg = Protocol::PROTOCOL_HEADER + "AUTH|1\n";
        send(client_socket, msg.c_str(), msg.size(), 0);
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
        while ((pos = active_buffer->find('\n')) != std::string::npos) {
            std::string msg_to_process = active_buffer->substr(0, pos);
            active_buffer->erase(0, pos + 1);

            if (!process_msg(fd, msg_to_process)) {
                break;
            }
        }
    }

    void Server::handle_disconnection(int fd_disconnected) {

        LOG_INFO("Handling disconnection for fd: " + std::to_string(fd_disconnected));

        // Check rooms
        auto room = RoomManager::get_room_by_user_fd(fd_disconnected);
        
        if (room) {
            auto opponent = room->handle_player_disconnect(fd_disconnected);
    
            // Inform opponent if exist
            if (opponent && room->state != ROOM_STATE::FINISHED) {
                std::string msg = Protocol::PROTOCOL_HEADER + "GAME|PAUSED|Opponent disconnected\n";
                send(opponent->fd_socket, msg.c_str(), msg.size(), 0);
            }

            // Delete room if empty
            if (room->get_players().empty()) {
                RoomManager::remove_room(room->id);
                LOG_INFO("Room " + std::to_string(room->id) + " deleted because it is empty.");
            }
        }

        UserManager::disconnect_user(fd_disconnected);
        unauth_sockets.erase(fd_disconnected);

        close(fd_disconnected);
        FD_CLR(fd_disconnected, &current_sockets);
    }

    void Server::remove_client(int client_fd) {
        // Inform client
        std::string err_msg = "Error: Invalid protocol\n";
        send(client_fd, err_msg.c_str(), err_msg.size(), 0);

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
            } else if (command == "MOVE") {
                handle_move(client_fd, user, parts);
            } else if (command == "PING") {
                handle_ping(client_fd, user);
            }
        } 

        return false;
    }

    // Function for certain types of msg handling
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
        std::string rsp_msg = Protocol::PROTOCOL_HEADER + "LOGIN|" + std::to_string(rsp_code) + "\n";
        send(client_fd, rsp_msg.c_str(), rsp_msg.size(), 0);

        return true;
    }

    bool Server::handle_find(int client_fd, std::shared_ptr<User> user) {
        LOG_INFO("User with fd: " + std::to_string(client_fd) + " tries to find a game");
        if (user->state == USER_STATE::IN_GAME) return false;

        auto room = RoomManager::join_waiting_room(user);

        if (!room) {
            std::string err = Protocol::PROTOCOL_HEADER + "ROOM_ERROR\n";
            send(client_fd, err.c_str(), err.size(), 0);
            return true;
        }

        // New game if full room
        if (room->state == ROOM_STATE::PLAYING) {
            auto players = room->get_players();
            
            // MSG to both players - GAME|<start_symbol>|<opponent nick>|<board>
            std::string msg1 = Protocol::PROTOCOL_HEADER + "GAME|START_X|" + players[1]->nickname + "|" + room->get_board_string() + "\n";
            send(players[0]->fd_socket, msg1.c_str(), msg1.size(), 0);

            std::string msg2 = Protocol::PROTOCOL_HEADER + "GAME|START_O|" + players[0]->nickname + "|" + room->get_board_string() + "\n";
            send(players[1]->fd_socket, msg2.c_str(), msg2.size(), 0);
            
            LOG_INFO("Match started in Room " + std::to_string(room->id));
        } else {
            // Waiting for another player
            std::string wait_msg = Protocol::PROTOCOL_HEADER + "WAITING\n";
            send(client_fd, wait_msg.c_str(), wait_msg.size(), 0);
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
            
            // Error check
            bool is_error = (game_response.find("|" + std::to_string(Protocol::OCCUPIED_FIELD)) != std::string::npos) ||
                            (game_response.find("|" + std::to_string(Protocol::NOT_YOUR_TURN)) != std::string::npos) ||
                            (game_response.find("|" + std::to_string(Protocol::INVALID_MOVE)) != std::string::npos) ||
                            (game_response.find("|" + std::to_string(Protocol::PLAYER_NOT_BELONG)) != std::string::npos) ||
                            (game_response.rfind("ERROR", 0) == 0);

            if (is_error) {
                // Msg only for sender
                std::string full_msg = Protocol::PROTOCOL_HEADER + game_response + "\n";
                send(client_fd, full_msg.c_str(), full_msg.size(), 0);
            } 
            else {
                // Valid move or game finished - inform both players
                std::string full_msg;

                if (game_response.find("RESULT") == std::string::npos) {
                    std::string next_turn_sym = std::string(1, room->get_current_turn_symbol());
                    full_msg = Protocol::PROTOCOL_HEADER + game_response + "|" + next_turn_sym + "\n";
                } else {
                    // Game finished
                    full_msg = Protocol::PROTOCOL_HEADER + game_response + "\n";
                }

                auto players = room->get_players();
                for (auto& p : players) {
                    send(p->fd_socket, full_msg.c_str(), full_msg.size(), 0);
                }
            }

            return true;
        } catch (const std::exception& e) {
            LOG_WARNING("Invalid integer format in MOVE command from fd: " + std::to_string(client_fd));
            return false;
        }

    }

    bool Server::handle_ping(int client_fd, std::shared_ptr<User> user) {
        std::string pong_msg;

        switch (user->state) {
            case USER_STATE::WAITING:
                pong_msg = "PONG|WAITING";
                break;

            case USER_STATE::IN_GAME: {
                auto room = RoomManager::get_room_by_user_fd(client_fd);
                if (room) {
                    char my_symbol = (room->get_players()[RoomConfig::FIRST_PLAYER]->fd_socket == client_fd) ? 'X' : 'O';
                    
                    // Get user nick
                    std::string opponent_nick = "Unknown";
                    for (auto& p : room->get_players()) {
                        if (p->fd_socket != client_fd) opponent_nick = p->nickname;
                    }

                    pong_msg = "PONG|GAME|" 
                            + std::string(1, my_symbol) + "|" 
                            + room->get_board_string() + "|"
                            + std::string(1, room->get_current_turn_symbol()) + "|"
                            + opponent_nick;
                    
                    // Inform opponent
                    for(auto& p : room->get_players()) {
                        if(p->fd_socket != client_fd) {
                            std::string res_msg = Protocol::PROTOCOL_HEADER + "GAME|RESUMED\n";
                            send(p->fd_socket, res_msg.c_str(), res_msg.size(), 0);
                        }
                    }
                } else {
                    // User IN_GAME but game ended
                    user->state = USER_STATE::CONNECTED;
                    pong_msg = "PONG|LOBBY";
                }
                break;
            }

            case USER_STATE::CONNECTED:
            default:
                pong_msg = "PONG|LOBBY";
                break;
        }

        // Send msg
        std::string full_msg = Protocol::PROTOCOL_HEADER + pong_msg + "\n";
        send(client_fd, full_msg.c_str(), full_msg.size(), 0);
        return true;
        }
}