#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include <vector>
#include <memory>
#include "user.h"
#include "../utility/utility.h"

namespace UserManaging {

    class UserManager {
        public:
            static std::vector<std::shared_ptr<MyUser::User>> user_list;
            static bool add_new_user(int fd);
    };

}
#endif