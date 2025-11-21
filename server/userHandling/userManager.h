#ifndef __USER_MANAGER__
#define __USER_MANAGER__

#include <vector>
#include <memory>
#include "user.h"
#include "../utility/utility.h"

class UserManager {
    public:
        static std::vector<std::shared_ptr<User>> user_list;
        static bool addUser(int fd);
};

#endif