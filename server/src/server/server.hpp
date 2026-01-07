#ifndef __SERVER__HPP
#define __SERVER__HPP

#include "../utility/utility.hpp"
#include "../config.hpp"
#include "../logger/logger.hpp"
#include "../userHandling/userManager.hpp"
#include "../userHandling/user.hpp"
#include "../exceptions/exceptions.hpp"
#include "../protocolConfig.hpp"
#include "../roomHandling/roomManager.hpp"
#include "../roomHandling/room.hpp"

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
            void remove_client(int fd);
            bool process_msg(int client_fd, std::string msg);
            bool send_all(int socket_fd, const std::string& data);

            // Function for certain types of msg handling
            bool handle_login(int client_fd, const std::vector<std::string>& parts);
            bool handle_find(int client_fd, std::shared_ptr<User> user);
            bool handle_move(int client_fd, const std::vector<std::string>& parts);
            bool handle_sync(int client_fd, std::shared_ptr<User> user);
            bool handle_rematch(int client_fd, std::shared_ptr<User> user);
            bool handle_leave(int client_fd, std::shared_ptr<User> user);

            /**
             * Handles heartbeat logic for server
             */
            bool handle_ping(int client_fd, std::shared_ptr<User> user);

            // Cleanup functions
            void cleanup_unauth_sockets();
    };

};

#endif