#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include "../utility/utility.h"

#include <stdexcept>
#include <string>

namespace MyExceptions {

    class BaseException: public std::runtime_error {
        private:
            int _err_num;

        protected:
            BaseException(int err_code = 0)
                : std::runtime_error(build_msg(err_code)),
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
                : BaseException(my_err_code){};
    };

    class UtilityException: public BaseException {
        protected:
            std::string build_msg(int err_code) const override;

        public:
            UtilityException(int my_err_code)
                : BaseException(my_err_code) {}
    };
};

#endif