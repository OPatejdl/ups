#include "logger.h"
#include "../config.h"
#include "../utility/utility.h"
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>

// ---- Private ----
/**
* Function opens logging file and cleans it, if exists
*/
Logger::Logger() {

    std::filesystem::create_directories(LOGS_FOLDER);

    std::ofstream(LOGGER_PATH, std::ios::trunc).close();

    log_file_.open(LOGGER_PATH, std::ios::app);

    if (!log_file_.is_open()) {
        std::cerr << "ERROR: Unable to open logging file: " << LOGGER_PATH << std::endl;
        exit(ERROR_LOGGER_UNOPEN);
    }
}

Logger::~Logger() {
    if (log_file_.is_open()) {
        log_file_.close();
    }
}


/**
* Function gets current timestamp in the string format
* @return String representation of current time
*/
std::string Logger::get_timestamp() {
    time_t timeNow;
    std::ostringstream oss;

    auto now = std::chrono::system_clock::now();
    timeNow = std::chrono::system_clock::to_time_t(now);

    oss << std::put_time(std::localtime(&timeNow), "%d-%m-%Y_%H:%M:%S");
    return oss.str();
}

/**
* Function gets string format of type
* @return string representation of log's type
*/
std::string Logger::get_log_type(TYPE type) {
    switch (type) {
        case TYPE::INFO:
            return "INFO";
        case TYPE::ERROR:
            return "ERROR";
        default:
            return "";
    }
}

// ---- Public ----

/**
* Function creates get instance of logger
* @return pointer to logger instance
*/
Logger &Logger::get_instance() {
    static Logger instance;
    return instance;
}

void Logger::log(TYPE type, const std::string &msg) {
    std::string str_type, str_time, whole_msg;

    str_type = get_log_type(type);
    str_time = get_timestamp();

    whole_msg = str_time + " -> [" +  str_type + "]: " + msg + "\n";

    std::cout << whole_msg;
    log_file_ << whole_msg;
}

