#include "user.h"

namespace MyUser{

  User::User(int socket_fd)
    : fd_socket(socket_fd),
      state(USER_STATE::CONNECTED)
  {}

}