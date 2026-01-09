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
#include "../roomHandling/roomConfig.hpp"

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

    /**
     * Helper structure to track connections that have not yet performed a LOGIN
     * Used for enforcing authentication timeouts and buffering initial data
     */
    struct UnauthInfo {
        std::chrono::steady_clock::time_point joined_time;  /** Timestamp of the connection establishment */
        std::string buffer;                                 /** Buffer for partial messages before user is authorized */
    };

    /**
     * Main Server class
     * - Manages the network connection (client connections, message routing, synchronization)
     */
    class Server {
        public:
            ~Server();      /** Destructor */
            
            /**
             * Constructor
             * @throws MyExceptions::ServerException if socket creation or binding fails.
             */
            Server();

            /**
             * The primary execution loop of the server
             * - Handles socket multiplexing (select), processes new connections, 
             *   handles incoming data, and performs periodic maintenance
             */
            void run_server();

            // Disable copy constructor and assignment operator for safety
            Server(const Server&) = delete;
            Server& operator = (const Server&) = delete;

        private:
            int server_socket;                              /** Main server socket used for listening to incoming connections */
            int return_value;                               /** Stores return value from system calls for error checking */
            int client_socket;                              /** Tmp file descriptor for a accepted client socket */
            int fd;                                         /** Interator used to process active file descriptors in the main loop */
            char buffer[Config::MAX_BUFFER_SIZE];           /** Buffer for receiving raw data from sockets */
            struct sockaddr_in my_addr;                     /** Address structure for the server's own IP and port */
            struct sockaddr_in peer_addr;                   /** Address structure for remote client */
            socklen_t addr;                                 /** Stores the size of addr structure required by accept */
            fd_set current_sockets;                         /** Set containing currently monitored file descriptors */
            fd_set ready_sockets;                           /** Indicates sockets, which are ready for I/O */
            std::string msg;                                /** Helper string used to construct protocol message */
            std::map<int, UnauthInfo> unauth_sockets;       /** Map tracking unauthorized clients */

            // ======================
            // --- Init functions ---

            /** 
             * Creates the main server listener socket
             */
            void create_server_socket();

            /** 
             * Binds the server socket to the IP address and port
             */
            void bind_server();

            /** 
             * Puts the server socket into a listening state
             */
            void server_listen();
            
            // ====================================
            // --- Runtime handling functions ---

            /**
             * Handles a new incoming connection
             */
            void new_client_connection();

            /**
             * Reads incoming data from a client socket
             */
            void handle_client_data();

            /**
             * Manages the disconnection of a client
             * @param fd_disconnected The file descriptor of the disconnected client
             */
            void handle_disconnection(int fd_disconnected);

            /**
             * Removal of a client, usually due to a protocol violation
             * @param fd The file descriptor of the client to be removed
             */
            void remove_client(int fd);

            /**
             * Main message handler
             *   - Validates the protocol header and gives 
             * commands to their respective handlers based on user state.
             * @param client_fd Socket of the sender
             * @param msg The raw message string extracted from the buffer
             * @return true if the connection should remain open, false if it should be closed
             */
            bool process_msg(int client_fd, std::string msg);

            /**
             * Ensures that the entire data string is sent over the TCP socket
             * @param socket_fd Destination socket
             * @param data String to be transmitted
             * @return true on success, otherwise false
             */
            bool send_all(int socket_fd, const std::string& data);

            // ===============================
            // --- Msg Handling Functions ---

            /**
             * Processes a LOGIN request
             * @param client_fd Socket of the sender
             * @param parts Split protocol message containing the nickname
             * @return true on success
             */
            bool handle_login(int client_fd, const std::vector<std::string>& parts);

            /**
             * Processes a FIND request to put a user into a matchmaking queue/room
             * @param client_fd Socket of the sender
             * @param user Pointer to the user object
             */
            bool handle_find(int client_fd, std::shared_ptr<User> user);

            /**
             * Processes a MOVE request from a player during a match
             *  - Validates coordinates and updates both players on the outcome
             * @param client_fd Socket of the sender
             * @param user Pointer to the user object
             * @param parts Split protocol message containing X and Y coordinates
             */
            bool handle_move(int client_fd, std::shared_ptr<User> user, const std::vector<std::string>& parts);

            /**
             * Process synchronization request
             *  - Sends the current state to a client (Reconnect)
             * @param client_fd Socket of the sender
             * @param user Pointer to the user object
             */
            bool handle_sync(int client_fd, std::shared_ptr<User> user);

                        /**
             * Handles heartbeat/PING messages to keep the connection alive
             * @param client_fd Socket of the sender
             * @param user Pointer to the user object
             */
            bool handle_ping(int client_fd, std::shared_ptr<User> user);

            /**
             * Handles a request to play again after a game has finished
             *   - Requires agreement from both players in the room
             * @param client_fd Socket of the sender
             * @param user Pointer to the user object
             */
            bool handle_rematch(int client_fd, std::shared_ptr<User> user);

            /**
             * Processes an LEAVE command, allowing a user to return to the lobby
             * @param client_fd Socket of the sender
             * @param user Pointer to the user object
             */
            bool handle_leave(int client_fd, std::shared_ptr<User> user);

            // ==============================
            // --- Maintenance functions ---

            /**
             * Iterates through unauthenticated sockets and closes those 
             * that failed to log in within the configured time limit.
             */
            void cleanup_unauth_sockets();
    };

};

#endif