#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <csignal>
#include "../logger/logger.hpp"
#include "../config.hpp"
#include "../exceptions/exceptions.hpp"
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>

/**
 * Namespace containing global configuration variables
 */
namespace Utility {

    // ====================
    // Preprocess symbols
    inline constexpr int  MIN_ARG = 5;  /** Minimal number of expected arguments */
    inline constexpr int MAX_ARG = 9;   /** Maximum number of expected arguments */

    // ====================
    // Error codes
    inline constexpr int ERROR_INVALID_PARAM = 1;           /** Provided parameter value is invalid or out of range */
    inline constexpr int ERROR_UNSET_PARAMETERS = 2;        /** Required parameters were not provided */
    inline constexpr int ERROR_LOGGER_UNOPEN = 3;           /** Logging file could not be opened for writing */
    inline constexpr int ERROR_UNCREATED_SERVER_SOC = 4;    /** Failed to create the main server socket */
    inline constexpr int ERROR_BINDING = 5;                 /** Failed to bind the socket to the port */     
    inline constexpr int ERROR_LISTEN = 6;                  /** Failed to set the socket to listening mode */

    // =====================
    // Parameter symbols
    extern unsigned int PORT;                               /** Global port number used by the server */
    extern unsigned int ROOMS_COUNT;                        /** Maximum allowed number of game rooms  */
    extern unsigned int CLIENTS_COUNT;                      /** Maximum number of clients that can be connected at one time */
    extern std::string IP_ADDRESS;                          /** IP address of server */
    extern volatile sig_atomic_t server_running;            /** Signal indicating if the server should continue running (safe shutdown usage) */

    // =====================
    // Functions

    /**
     * Parses command-line arguments to set server configuration
     *  - Supports flags: -p (port), -c (clients), and -r (rooms)
     * @param argc Number of arguments
     * @param argv Array of argument strings
     * @throws MyExceptions::UtilityException if a parameter is missing, invalid, or out of range
     */
    void handle_params(int argc, char *argv[]);

    /**
     * Signal handler to catch termination signals (like SIGINT)
     */
    void ending_signal_handler(int);

    /**
     * Splits a string into a vector of tokens based on a spliter character
     * @param s The input string to split
     * @param spliter The character used as the spliter character
     * @return A vector of splitted strings
     */
    std::vector<std::string> split(const std::string& s, char spliter);

    /**
     * Checks if ip address value is set in valid format
     * @param ip The string representation of IP address
     * @return true if valid otherwise false
     */
    bool is_valid_ipv4(const std::string& ip);
}

#endif