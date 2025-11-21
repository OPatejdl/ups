#include "userManager.h"

std::vector<std::shared_ptr<User>> UserManager::user_list;

/**
    Adds user in case there is still space for one more user

    @returns true if adding was successful otherwise false
*/
bool UserManager::addUser(int fd) {
    if (user_list.size() < CLIENTS_COUNT ) {
        std::shared_ptr<User> new_user = std::make_shared<User>(fd); 
        user_list.push_back(new_user);

        return true;
    } else {
        return false;
    }
}