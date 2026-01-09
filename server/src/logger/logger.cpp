#include "logger.hpp"
#include "../config.hpp"
#include "../utility/utility.hpp"
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>

namespace MyLogger {

    // ---- Private ----
    Logger::Logger() {

        std::filesystem::create_directories(Config::LOGS_FOLDER);

        std::ofstream(Config::LOGGER_PATH, std::ios::trunc).close();

        log_file_.open(Config::LOGGER_PATH, std::ios::app);

        if (!log_file_.is_open()) {
            std::cerr << "ERROR: Unable to open logging file: " << Config::LOGGER_PATH << std::endl;
            exit(Utility::ERROR_LOGGER_UNOPEN);
        }
    }

    Logger::~Logger() {
        if (log_file_.is_open()) {
            log_file_.close();
        }
    }


    std::string Logger::get_timestamp() {
        time_t timeNow;
        std::ostringstream oss;

        auto now = std::chrono::system_clock::now();
        timeNow = std::chrono::system_clock::to_time_t(now);

        oss << std::put_time(std::localtime(&timeNow), "%d-%m-%Y_%H:%M:%S");
        return oss.str();
    }

    std::string Logger::get_log_type(TYPE type) {
        switch (type) {
            case TYPE::INFO:
                return "INFO";
            case TYPE::ERROR:
                return "ERROR";
            case TYPE::WARNING:
                return "WARNING";
            default:
                return "";
        }
    }

    // ---- Public ----

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
        log_file_ << whole_msg << std::flush;
    }
}