#include "server.h"

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

        unauth_sockets[client_socket] = std::chrono::steady_clock::now();

        LOG_INFO("New client socket connected on fd: " + std::to_string(client_socket));

        msg = PROTOCOL_HEADER + "AUTH|1";
        send(client_socket, msg.c_str(), msg.size(), 0);
        // if (UserManager::add_new_user(client_socket)) {
        //     // add successfully
        //     FD_SET(client_socket, &current_sockets);
        //     LOG_INFO("New client connected: " + std::to_string(client_socket));

        //     msg = "Welcome to server!";
        //     send(client_socket, msg.c_str(), msg.size(), 0);
        // } else {
        //     // Excited total amount of clients
        //     LOG_WARNING("Full Server \n\t Unable to add new client:" + std::to_string(client_socket) + "was not accepted!");
            
        //     msg = "Server is currently full. Try again later";
            
        //     send(client_socket, msg.c_str(), msg.size(), 0);
        //     close(client_socket);
        // }
    }

    void Server::handle_client_data() {
        memset(buffer, 0, Config::MAX_BUFFER_SIZE);
        int bytes_recv = recv(fd, buffer, Config::MAX_BUFFER_SIZE, 0);

        if (bytes_recv <= 0) {
            handle_disconnection();
            return;
        }

        std::string data(buffer, bytes_recv);
        std::string_view header = Config::PROTOCOL_HEADER;

        // Check invalid msg
        if (data.size() < header.size() || data.substr(0, header.size()) != header) {
            // Remove user in case of invalid protocol
            LOG_WARNING("Invalid protocol header from fd: " + std::to_string(fd));

            std::string err_msg = "Error: Invalid protocol \n";

            send(fd, err_msg.c_str(), err_msg.size(), 0);
            close(fd);
            FD_CLR(fd, &current_sockets);
            UserManager::remove_user(fd);
            return;
        }

        std::string rest_msg = data.substr(header.size());
    }
}

void Server::handle_disconnection() {
    UserManager::disconnect_user(fd);
    close(fd);
    FD_CLR(fd, &current_sockets);
}

void Server::cleanup_unauth_sockets() {
    auto now = std::chrono:steady_clock::now();
    auto timeout = std::chrono:seconds(Config::AUTH_TIMEOUT);

    for (auto it = unauth_sockets.begin(); it != unauth_sockets.end(); ) {
        // Check timeout
        if (now - it->second > timeout) {
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
