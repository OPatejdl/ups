#ifndef __SERVER_EXCEPTION__
#define __SERVER_EXCEPTION__

#include "../utility/utility.h"

#include <stdexcept>
#include <string>

class ServerException: public std::runtime_error {
    public:
        ServerException(int my_err_code = 0)
            : std::runtime_error(build_msg(my_err_code)),
            _err_num(my_err_code) {}
        
        int get_err_code() const;
    private:
        int _err_num;
        static std::string build_msg(int err_code);
};

#endif