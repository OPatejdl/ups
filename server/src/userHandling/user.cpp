#include "user.h"

User::User(int socket_fd)
  : fd_socket(socket_fd),
    state(USER_STATE::CONNECTED)
{
  last_active = std::chrono::steady_clock::now();
}
