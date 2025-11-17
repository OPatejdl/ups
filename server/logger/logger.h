#ifndef __LOGGER__
#define __LOGGER__

#include <string>
#include <fstream>

/* 
-----------------
-- Log's types --
-----------------
*/
enum class TYPE {
    INFO,
    ERROR
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

/* 
-------------------------
-- Logging Func Macros --
-------------------------
*/

#define LOG_INFO(msg) Logger::get_instance().log(TYPE::INFO, msg)
#define LOG_ERROR(msg) Logger::get_instance().log(TYPE::ERROR, msg)

#endif