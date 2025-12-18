#include "userManager.h"

namespace UserManaging {
    std::vector<std::shared_ptr<MyUser::User>> UserManager::user_list;

    /**
        Adds user in case there is still space for one more user

        @returns true if adding was successful otherwise false
    */
    bool UserManager::add_new_user(int fd) {
        if (user_list.size() < Utility::CLIENTS_COUNT ) {
            std::shared_ptr<MyUser::User> new_user = std::make_shared<MyUser::User>(fd); 
            user_list.push_back(new_user);

            return true;
        }

        return false;
    }
}