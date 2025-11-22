#include "serverException.h"

std::string ServerException::build_msg(int err_code) {
    std::string str_err_code = std::to_string(err_code);

    switch(err_code) {
        case ERROR_UNCREATED_SERVER_SOC:
            return "Server error: Failed when creating server socket [Code: " + str_err_code + "]";
        case ERROR_BINDING:
            return "Server error: Failed when binding server socket [Code: " + str_err_code + "]";
        case ERROR_LISTEN:
            return "Server error: Failed to listen server socket [Code: " + str_err_code + "]";
        default:
            return "Unknown error has occurred" + str_err_code;
    }
}

int ServerException::get_err_code() const {
    return _err_num;
}