#include "exceptions.h"

namespace MyExceptions {

    std::string ServerException::build_msg(int err_code) const {
        std::string str_err_code = std::to_string(err_code);

        switch(err_code) {
            case Utility::ERROR_UNCREATED_SERVER_SOC:
                return "Server error: Failed when creating server socket [Code: " + str_err_code + "]";
            case Utility::ERROR_BINDING:
                return "Server error: Failed when binding server socket [Code: " + str_err_code + "]";
            case Utility::ERROR_LISTEN:
                return "Server error: Failed to listen server socket [Code: " + str_err_code + "]";
            default:
                return "Unknown error has occurred [Code: " + str_err_code + "]";
        }
    }

    int BaseException::get_err_code() const {
        return _err_num;
    }

    std::string UtilityException::build_msg(int err_code) const {
        std::string str_err_code = std::to_string(err_code);

        switch(err_code) {
            case Utility::ERROR_INVALID_PARAM {
                return "Parameters handling error: Invalid input from user!  [Code: " + str_err_code + "]";
            }
            default:
                return "Unknown error has occurred [Code: " + str_err_code + "]";
        }
    }
}