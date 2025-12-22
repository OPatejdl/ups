#ifndef __SERVER__
#define __SERVER__

#include "../utility/utility.h"
#include "../config.h"
#include "../logger/logger.h"
#include "../userHandling/userManager.h"
#include "../userHandling/user.h"
#include "../exceptions/exceptions.h"
#include "../protocolConfig.hpp"

#include <iostream>
#include <string.h>
#include <cstdlib>
#include <chrono>
#include <map>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace MyServer {
    struct UnauthInfo {
        std::chrono::steady_clock::time_point joined_time;
        std::string buffer;
    };

    class Server {
        public:
            ~Server();
            Server();

            void run_server();

            Server(const Server&) = delete;
            Server& operator = (const Server&) = delete;

        private:
            int server_socket, return_value;
            int client_socket, fd;
            char buffer[Config::MAX_BUFFER_SIZE];
            struct sockaddr_in my_addr, peer_addr;
            socklen_t addr;
            fd_set current_sockets, ready_sockets;
            std::string msg;
            std::map<int, UnauthInfo> unauth_sockets;

            // Init functions
            void create_server_socket();
            void bind_server();
            void server_listen();

            // Function for server run
            void new_client_connection();
            void handle_client_data();
            void handle_disconnection(int fd_disconnected);
            bool process_msg(int client_fd, std::string msg);

            // Cleanup functions
            void cleanup_unauth_sockets();
    };

};

#endif