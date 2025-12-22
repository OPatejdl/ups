#include "userManager.h"


std::vector<std::shared_ptr<User>> UserManager::user_list;

/**
    Adds user in case there is still space for one more user
    @returns true if adding was successful otherwise false
*/
bool UserManager::add_new_user(int fd) {
    if (user_list.size() < Utility::CLIENTS_COUNT ) {
        std::shared_ptr<User> new_user = std::make_shared<User>(fd); 
        user_list.push_back(new_user);

        return true;
    }

    return false;
}

void UserManager::remove_user(int fd) {
    for (int i = 0; i < user_list.size(); i++) {
        if (user_list[i]->fd_socket == fd) {
            user_list.erase(user_list.begin() + i);
            break;
        }
    }
}

void UserManager::cleanup_users(std::chrono::seconds timeout) {
    auto now = std::chrono::steady_clock::now();
    for (int i = 0; i < user_list.size(); i++) {
        if (user_list[i]->fd_socket == Config::DISCONNECTED_USER_SOCKET) {
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - user_list[i]->last_active);
            if (duration > timeout) {
                LOG_INFO("Inactive timeout exceeded: removing user" + user_list[i]->nickname);
                user_list.erase(user_list.begin() + i);
            }
        }
    }
}

void UserManager::disconnect_user(int fd) {
    for (auto& user : user_list) {
        if (user->fd_socket == fd) {
            user->fd_socket = Config::DISCONNECTED_USER_SOCKET;
            user->last_active = std::chrono::steady_clock::now();
            LOG_INFO("User " + user->nickname + " is now in DISCONNECTED mode");
            return;
        }
    }
}