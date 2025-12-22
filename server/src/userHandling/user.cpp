#include "user.h"

User::User(int socket_fd, const std::string& nick)
  : fd_socket(socket_fd),
    state(USER_STATE::CONNECTED),
    nickname(nick)
{
  last_active = std::chrono::steady_clock::now();
}
