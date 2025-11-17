#include "user.h"

User::User(int socket_fd, std::string nick)
    : fd_socket(socket_fd),
      state(USER_STATE::LOBBY),
      nickname(nick)
{}