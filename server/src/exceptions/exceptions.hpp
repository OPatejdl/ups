#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include "../utility/utility.hpp"

#include <stdexcept>
#include <string>

namespace MyExceptions {

    class BaseException: public std::runtime_error {
        private:
            int _err_num;

        protected:
            BaseException(const std::string& msg, int err_code = 0)
                : std::runtime_error(msg),
                _err_num(err_code) {}

            virtual std::string build_msg(int err_code) const = 0;

        public:
            int get_err_code() const;
    };

    class ServerException: public BaseException {
        protected:
            std::string build_msg(int err_code) const override;

        public:
            ServerException(int my_err_code)
                : BaseException(build_msg(my_err_code), my_err_code){};
    };

    class UtilityException: public BaseException {
        protected:
            std::string build_msg(int err_code) const override;

        public:
            UtilityException(int my_err_code)
                : BaseException(build_msg(my_err_code), my_err_code) {}
    };
};

#endif