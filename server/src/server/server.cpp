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
        while(Utility::server_running) {
            ready_sockets = current_sockets;

            return_value = select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL);
            if (return_value < 0) {
                
                // Check if error was not caused by Ctrl+C
                if (errno == EINTR) {
                    continue;
                }

                LOG_ERROR("Select failed");
                break;
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
                    LOG_INFO("Work in progress");
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
        if (UserManaging::UserManager::add_new_user(client_socket)) {
            // add successfully
            FD_SET(client_socket, &current_sockets);
            LOG_INFO("New client connected: " + std::to_string(client_socket));

            msg = "Welcome to server!";
            send(client_socket, msg.c_str(), msg.size(), 0);
        } else {
            // Excited total amount of clients
            LOG_WARNING("Full Server \n\t Unable to add new client:" + std::to_string(client_socket) + "was not accepted!");
                        
            msg = "Server is currently full. Try again later";
            
            send(client_socket, msg.c_str(), msg.size(), 0);
            close(client_socket);
        }
    }
}