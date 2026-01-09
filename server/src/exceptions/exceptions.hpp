#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include "../utility/utility.hpp"

#include <stdexcept>
#include <string>

namespace MyExceptions {

    /**
     * Abstract class for all custom exceptions
     */
    class BaseException: public std::runtime_error {
        private:
            int _err_num;       /** Numeric error code associated with the exception */

        protected:
            /**
             * Protected constructor
             * @param msg The full error message
             * @param err_code The numeric error code
             */
            BaseException(const std::string& msg, int err_code = 0)
                : std::runtime_error(msg),
                _err_num(err_code) {}

            /**
             * Pure virtual function to construct a descriptive error message from a code
             *  - Must be implemented by specialized exception classes
             * @param err_code Numeric error identifier
             * @return A formatted string describing the error
             */
            virtual std::string build_msg(int err_code) const = 0;

        public:
            /**
             * Getter for the numeric error code
             * @return The error number stored in the exception
             */
            int get_err_code() const;
    };

    /**
     * Exception class for server-side networking errors
     *  - Handles issues related to socket creation, binding, and listening
     */
    class ServerException: public BaseException {
        protected:
            /**
             * Constructs specific messages for server errors
             * @param err_code The specific server error code
             * @return Formatted error message
             */
            std::string build_msg(int err_code) const override;

        public:
            /**
             * Constructor for ServerException
             * @param my_err_code Code representing the specific server failure
             */
            ServerException(int my_err_code)
                : BaseException(build_msg(my_err_code), my_err_code){};
    };

    /**
     * Exception class for utility and parameter handling errors
     *  - Used primarily for validating user input and configuration parameters
     */
    class UtilityException: public BaseException {
        protected:
            /**
             * Constructs messages for utility errors
             * @param err_code The specific utility error code
             * @return Formatted error message
             */
            std::string build_msg(int err_code) const override;

        public:
            /**
             * Constructor for UtilityException
             * @param my_err_code Code representing the specific utility failure
             */
            UtilityException(int my_err_code)
                : BaseException(build_msg(my_err_code), my_err_code) {}
    };
};

#endif