#ifndef __USER_MANAGER__
#define __USER_MANAGER__

#include <vector>
#include <memory>
#include "user.h"

class UserManager {
    public:
        static std::vector<std::shared_ptr<User>> user_list;
        static void addUser(int fd);
};

#endif