#include "userManager.hpp"


std::vector<std::shared_ptr<User>> UserManager::user_list;

/**
    Adds user in case there is still space for one more user
    @returns true if adding was successful otherwise false
*/
bool UserManager::add_new_user(int fd, const std::string& nick) {
    if (user_list.size() < Utility::CLIENTS_COUNT ) {
        std::shared_ptr<User> new_user = std::make_shared<User>(fd, nick); 
        user_list.push_back(new_user);

        return true;
    }

    return false;
}

void UserManager::remove_user(int fd) {
    for (size_t i = 0; i < user_list.size(); i++) {
        if (user_list[i]->fd_socket == fd) {
            user_list.erase(user_list.begin() + i--);
            break;
        }
    }
}

std::vector<std::shared_ptr<User>> UserManager::cleanup_users(std::chrono::seconds timeout) {
    auto now = std::chrono::steady_clock::now();
    std::vector<std::shared_ptr<User>> removed_users;

    for (auto it = user_list.begin(); it != user_list.end(); ) {

        if ((*it)->fd_socket == Protocol::DISCONNECTED_USER_SOCKET) {
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - (*it)->last_active);
            
            if (duration > timeout) {
                LOG_INFO("Inactive timeout exceeded: removing user " + (*it)->nickname);
                
                // Store user
                removed_users.push_back(*it);

                // Remove user from list
                it = user_list.erase(it); 
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }

    return removed_users;
}

void UserManager::disconnect_user(int fd) {
    std::shared_ptr<User> user = get_user_by_fd(fd);
    if (user) {
        user->fd_socket = Protocol::DISCONNECTED_USER_SOCKET;
        user->last_active = std::chrono::steady_clock::now();
        LOG_INFO("User " + user->nickname + " is now in DISCONNECTED mode");
    }
}

std::shared_ptr<User> UserManager::get_user_by_fd(int fd) {
    for (auto& user : user_list) {
        if (user->fd_socket == fd) { 
            return user;
        }
    }
    return nullptr;
}

int UserManager::handle_login(int client_fd, const std::string& nick) {
    // Nickname check (Size)
    if (nick.size() < Config::MIN_NICK_SIZE) {
        LOG_WARNING("Too short size of nickname setted by "+ std::to_string(client_fd));
        return Protocol::LOGIN_MIN_NICK_LEN;
    } else if (nick.size() > Config::MAX_NICK_SIZE) {
        LOG_WARNING("Too long size of nickname setted by "+ std::to_string(client_fd));
        return Protocol::LOGIN_MAX_NICK_LEN;
    }

    // Reconnect and duplicity name check
    for (auto& user : user_list) {
        // Reconnect
        if (user->fd_socket == Protocol::DISCONNECTED_USER_SOCKET
        && user->nickname == nick) {
            LOG_INFO("Reconnecting user: " + nick + "... new fd is " + std::to_string(client_fd));
            user->fd_socket = client_fd;
            user->last_active = std::chrono::steady_clock::now();
            return Protocol::LOGIN_RECONNECT;
        }
        // Duplicity name
        else if (user->nickname == nick) {
            LOG_WARNING("Client: " + std::to_string(client_fd) + " tried to use duplicated nickname");
            return Protocol::LOGIN_NICK_DUPLICITY;
        }
    }

    // Add new user
    if (!add_new_user(client_fd, nick)) {
        LOG_WARNING("Full server.... unable to add new user");
        return Protocol::LOGIN_FULL_SERVER;
    } 

    LOG_INFO("New user added: \n\tNickname: " + nick + "\n\tFD: " + std::to_string(client_fd));
    return Protocol::LOGIN_LOGGED;
}

std::vector<int> UserManager::get_timeouted_users(std::chrono::seconds timeout) {
    std::vector<int> timeouted_fds;
    auto now = std::chrono::steady_clock::now();

    for (const auto& user : user_list) {

        // Check all users, who appear connected
        if (user->fd_socket != Protocol::DISCONNECTED_USER_SOCKET) {
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - user->last_active);
            
            if (duration > timeout) {
                timeouted_fds.push_back(user->fd_socket);
            }
        }
    }
    return timeouted_fds;
}