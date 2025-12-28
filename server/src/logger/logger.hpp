#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>

namespace MyLogger {
    /* 
    -----------------
    -- Log's types --
    -----------------
    */
    enum class TYPE {
        INFO,
        ERROR,
        WARNING
    };

    /* 
    ----------------
    -- Logger Def --
    ----------------
    */
    class Logger {
        private:
            std::ofstream log_file_;                /* Logging file */

            Logger();                               /* Constructor of logger */
            ~Logger();                              /* Destructor of logger */

            // Singleton -> no copy or pointer
            Logger(const Logger&) = delete;
            Logger& operator = (const Logger&) = delete;

            std::string get_timestamp();            /* Function formats timestamp*/
            std::string get_log_type(TYPE type);    /* Function gets string format of type */

        public:
            static Logger &get_instance();                  /* Function gets logger's instance */
            void log(TYPE type, const std::string &msg);    /* Function creates new message in logger */
    };
}
/* 
-------------------------
-- Logging Func Macros --
-------------------------
*/

#define LOG_INFO(msg) MyLogger::Logger::get_instance().log(MyLogger::TYPE::INFO, msg)
#define LOG_ERROR(msg) MyLogger::Logger::get_instance().log(MyLogger::TYPE::ERROR, msg)
#define LOG_WARNING(msg) MyLogger::Logger::get_instance().log(MyLogger::TYPE::WARNING, msg)

#endif