#ifndef USER_H
#define USER_H

#include <string>
#include "user.h"

namespace MyUser {
    /*
    -------------------
    -- User's states --
    -------------------
    */
    enum class USER_STATE {
        DISCONNECTED = -1,
        CONNECTED = 0,
        LOBBY = 1,
        WAITING = 2,
    };

    class User {
        public:
            int fd_socket;              /** socket file descriptor of user */
            USER_STATE state;           /** current state of user */
            std::string nickname;       /** nickname of the user */

            User(int socket_fd);        /** Constructor of class User */
    };
}

#endif