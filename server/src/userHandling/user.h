#ifndef USER_H
#define USER_H

#include <string>
#include <chrono>
#include "user.h"

/*
-------------------
-- User's states --
-------------------
*/
enum class USER_STATE {
    CONNECTED = 0,
    LOBBY = 1,
    WAITING = 2,
};



class User {
    public:
        int fd_socket;              /** socket file descriptor of user */
        USER_STATE state;           /** current state of user */
        std::string nickname;       /** nickname of the user */
        std::chrono::steady_clock::time_point last_active;  /** timestamp of last activity of a user */

        User(int socket_fd);        /** Constructor of class User */
};

#endif