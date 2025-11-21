#include "utility/utility.h"
#include "config.h"
#include "logger/logger.h"
#include "userHandling/userManager.h"

#include <iostream>
#include <string.h>
#include <cstdlib>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>



// exe cmd: server.exe -p (PORT) -C (MAX_CLIENT) -r (MAX_ROOMS)
int main(int argc, char *argv[]) {
    // server's socket
    int server_socket, return_value;

    // new client
    int client_socket, fd;

    // buffer for received data
    char buffer[MAX_BUFFER_SIZE];

    // Vars for server's address setup and connected user's address
    struct sockaddr_in my_addr, peer_addr;
    socklen_t addr;

    fd_set current_sockets, ready_sockets;

    std::string msg ;
    
    // Check parameters
    if (argc < MIN_ARG || argc > MAX_ARG) {
        LOG_ERROR("Invalid arguments count.\n"
                "\tNeed to run starting command using format: ./main <-p <PORT>> -c <MAX_CLIENT> -r <MAX_ROOMS>");
        return ERROR_INVALID_PARAM;
    };

    // Set Parameters
    handle_params(argc, argv);

    // Check if rooms and client set
    if (CLIENTS_COUNT == CLIENT_INIT_COUNT || ROOMS_COUNT == ROOMS_INIT_COUNT) {
        LOG_ERROR("Unset rooms or clients count\n"
                "\tNeed to run starting command in format: ./main -c <MAX_CLIENT> -r <MAX_ROOMS>");
        return ERROR_UNSET_PARAMETERS;
    }

    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        LOG_ERROR("Unable to create server socket\n");
        return ERROR_UNCREATED_SERVER_SOC;
    } else {
        LOG_INFO("Server socket was created");
    }

    // Bind PORT and IP
    memset(&my_addr, 0, sizeof(struct sockaddr_in));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(PORT);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    return_value = bind(server_socket, (struct sockaddr *) &my_addr, sizeof(my_addr));
    if (return_value != 0) {
        LOG_ERROR("Binding of server socket failed");
        close(server_socket);
        return ERROR_BINDING;
    } else {
        LOG_INFO("Binding successful");
    }

    // Set up listening
    return_value = listen(server_socket, BACKLOG_SIZE);
    if (return_value != 0) {
        LOG_ERROR("Listen - FAILED");
        close(server_socket);
        return ERROR_LISTEN;
    } else {
        LOG_INFO("Listening successful");
    }

    LOG_INFO("Server setup: \n\tPort: " + std::to_string(PORT) + 
            "\n\tClients: " + std::to_string(CLIENTS_COUNT) +
            "\n\tRooms: " + std::to_string(ROOMS_COUNT)
    );

    // Init set
    FD_ZERO(&current_sockets);
    FD_SET(server_socket, &current_sockets);

    for(;;) {
        ready_sockets = current_sockets;

        return_value = select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL);
        if (return_value < 0) {
            LOG_ERROR("Select failed");
            break;
        }

        for (fd = 0; fd < FD_SETSIZE; fd++) {
            if (!FD_ISSET(fd, &ready_sockets)) continue;

            if (fd == server_socket) {
                // possible new connection
                addr = sizeof(peer_addr);

                client_socket = accept(server_socket, (struct sockaddr *) &peer_addr, &addr);
                if (client_socket < 0) {
                    LOG_WARNING("Error when loading new client!");
                    continue;
                }

                if (UserManager::addUser(client_socket)) {
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
                
            } else {
                LOG_INFO("Work in progress...");
            }
        }
    }

    LOG_INFO("Server ends");
    close(server_socket);
    return EXIT_SUCCESS;
}