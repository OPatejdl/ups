#ifndef USER_MANAGER_HPP
#define USER_MANAGER_HPP

#include <vector>
#include <memory>
#include <chrono>
#include "user.hpp"
#include "../utility/utility.hpp"
#include "../config.hpp"
#include "../protocolConfig.hpp"


class UserManager {
    public:
        static std::vector<std::shared_ptr<User>> user_list;
        static bool add_new_user(int fd, const std::string& nick);
        static void remove_user(int fd);
        static std::vector<std::shared_ptr<User>> cleanup_users(std::chrono::seconds timeout);
        static void disconnect_user(int fd);
        static std::shared_ptr<User> get_user_by_fd(int fd);
        static int handle_login(int client_fd, const std::string& nick);
        static std::vector<int> get_timeouted_users(std::chrono::seconds timeout);
};

#endif