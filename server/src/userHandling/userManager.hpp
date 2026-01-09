#ifndef USER_MANAGER_HPP
#define USER_MANAGER_HPP

#include <vector>
#include <memory>
#include <chrono>
#include "user.hpp"
#include "../utility/utility.hpp"
#include "../config.hpp"
#include "../protocolConfig.hpp"

/**
 * Static manager class for user lifecycle and session handling.
 *  - Handles authentication, reconnection logic, and session timeouts.
 */
class UserManager {
    public:
        static std::vector<std::shared_ptr<User>> user_list;        /** Global list of all users */

        /**
         * Attempts to add a new user to the system
         * @param fd File descriptor of the new connection
         * @param nick Nickname of new user
         * @return true if added successfully, false if the server is full
         */
        static bool add_new_user(int fd, const std::string& nick);

        /**
         * Removes a user from the system based on their file descriptor
         * @param fd Socket descriptor to remove
         */
        static void remove_user(int fd);

        /**
         * Identifies and removes users in "disconnected mode" 
         *  who failed to reconnect within the timeout
         * @param timeout Maximum allowed duration in disconnected state
         * @return Vector of shared pointers to the users that were purged
         */
        static std::vector<std::shared_ptr<User>> cleanup_users(std::chrono::seconds timeout);

        /**
         * Transitions a user to disconnected mode
         *  - Changes fd_socket to DISCONNECTED_USER_SOCKET and updates activity timestamp
         * @param fd The socket descriptor of the user who lost connection
         */
        static void disconnect_user(int fd);

        /**
         * Finds a user object associated with a specific file descriptor
         * @param fd File descriptor to search for
         * @return Pointer to User object, or nullptr if not found
         */
        static std::shared_ptr<User> get_user_by_fd(int fd);

        /**
         * Handles the complete login protocol
         * @param client_fd The current socket of the connecting client
         * @param nick The nickname provided by the client
         * @return Protocol login code
         */
        static int handle_login(int client_fd, const std::string& nick);

        /**
         * Scans connected users for those who haven't sent data/heartbeats recently
         * @param timeout Maximum allowed duration of inactivity for a connected socket
         * @return Vector of file descriptors that should be disconnected due to timeout
         */
        static std::vector<int> get_timeouted_users(std::chrono::seconds timeout);
};

#endif