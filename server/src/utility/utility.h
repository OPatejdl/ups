#ifndef UTILITY_H
#define UTILITY_H

#include <csignal>
#include "../logger/logger.h"
#include "../config.h"
#include "../exceptions/exceptions.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>

namespace Utility {
    /* 
    ------------------------
    -- Preprocess Symbols --
    ------------------------
    */
    inline constexpr int  MIN_ARG = 5;
    inline constexpr int MAX_ARG = 7;

    /* 
    ------------------------
    -- Error Codes --
    ------------------------
    */
    inline constexpr int ERROR_INVALID_PARAM = 1;
    inline constexpr int ERROR_UNSET_PARAMETERS = 2;
    inline constexpr int ERROR_LOGGER_UNOPEN = 3;
    inline constexpr int ERROR_UNCREATED_SERVER_SOC = 4;
    inline constexpr int ERROR_BINDING = 5;
    inline constexpr int ERROR_LISTEN = 6;

    /* 
    -------------------------
    ----- Param Symbols -----
    -------------------------
    */
    // Init of these variables is in utility.cpp
    extern unsigned int PORT;
    extern unsigned int ROOMS_COUNT;
    extern unsigned int CLIENTS_COUNT;
    extern volatile sig_atomic_t server_running;

    /*
    -------------------------
    ------- Functions -------
    -------------------------
    */
    void handle_params(int argc, char *argv[]);
    void ending_signal_handler(int);
    std::vector<std::string> split(const std::string& s, char spliter);
}

#endif