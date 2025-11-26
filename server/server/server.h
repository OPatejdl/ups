#ifndef __SERVER__
#define __SERVER__

#include "../utility/utility.h"
#include "../config.h"
#include "../logger/logger.h"
#include "../userHandling/userManager.h"
#include "../exceptions/serverException.h"

#include <iostream>
#include <string.h>
#include <cstdlib>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace MyServer {

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
            char buffer[MAX_BUFFER_SIZE];
            struct sockaddr_in my_addr, peer_addr;
            socklen_t addr;
            fd_set current_sockets, ready_sockets;
            std::string msg;

            // Init functions
            void create_server_socket();
            void bind_server();
            void server_listen();

            // Function for server run
            void new_client_connection();
    };

};

#endif