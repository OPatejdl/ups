#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>

namespace MyLogger {

    // Log levels
    enum class TYPE {
        INFO,               /** Information msg */
        ERROR,              /** Critical error msg */
        WARNING             /** Unexpected behavior or invalid moves by user */
    };


    /**
     * Singleton Logger class 
     */
    class Logger {
        private:
            std::ofstream log_file_;                /** Logging file */

            Logger();                               /** Constructor of logger */
            ~Logger();                              /** Destructor of logger */

            // Disable copy constructor and assignment to prevent duplicates
            Logger(const Logger&) = delete;
            Logger& operator = (const Logger&) = delete;

            /**
             * Generates a formatted date-time string for log entries
             * @return String in format DD-MM-YYYY_h:min:s
             */
            std::string get_timestamp();

            /**
             * Converts the TYPE enum to its string representation
             * @param type The log msg level
             * @return level of msg in string format
             */
            std::string get_log_type(TYPE type);

        public:

            /**
             * Accesses the global Logger instance
             * @return Reference to the Logger singleton
             */
            static Logger &get_instance();

            /**
             * Records a message to the console and the log file
             * @param type The severity level of the message
             * @param msg The actual log message content
             */
            void log(TYPE type, const std::string &msg);
    };
}

// =======================
// Logging Func Macros 

#define LOG_INFO(msg) MyLogger::Logger::get_instance().log(MyLogger::TYPE::INFO, msg)
#define LOG_ERROR(msg) MyLogger::Logger::get_instance().log(MyLogger::TYPE::ERROR, msg)
#define LOG_WARNING(msg) MyLogger::Logger::get_instance().log(MyLogger::TYPE::WARNING, msg)

#endif