#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include <vector>
#include <memory>
#include <chrono>
#include "user.h"
#include "../utility/utility.h"
#include "../config.h"
#include "../protocolConfig.hpp"


class UserManager {
    public:
        static std::vector<std::shared_ptr<User>> user_list;
        static bool add_new_user(int fd, const std::string& nick);
        static void remove_user(int fd);
        static void cleanup_users(std::chrono::seconds timeout);
        static void disconnect_user(int fd);
        static std::shared_ptr<User> get_user_by_fd(int fd);
        static int handle_login(int client_fd, const std::string& nick);
};

#endif