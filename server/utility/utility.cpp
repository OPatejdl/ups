#include "utility.h"
#include "../logger/logger.h"
#include "../config.h"
#include <iostream>
#include <cstdlib>
#include <string>

// ---- Init variables ----
int PORT = PORT_INIT;
int ROOMS_COUNT = ROOMS_INIT_COUNT;
int CLIENTS_COUNT = CLIENT_INIT_COUNT;

// ---- Functions ----

/**
* Process parameters given by user to command line
*
* @param argc count of the given arguments including exe file
* @param argv array of given arguments
*/
void handle_params(int argc, char *argv[]) {
    int value;
    std::string flag, value_str, msg;

    for (int i = 1; i < argc; i += 2) {
        // check if param has value
        if (i + 1 >= argc) {
            msg = "Parameter " + flag + " value was not set\n";
            LOG_ERROR(msg);
            exit(ERROR_INVALID_PARAM);
        }

        flag = argv[i];
        value_str = argv[i+1];

        // Try to convert arguments value
        try {
            value = std::stoi(value_str);
        } catch (const std::invalid_argument& e) {
            msg = "Invalid numeric value of parameter: " + flag + " -> setted value: " + value_str + "\n";
            LOG_ERROR(msg);
            exit(ERROR_INVALID_PARAM);
        } catch (const std::out_of_range& e) {
            msg = " Value out of range for parameter: " + flag + " -> setted value: " + value_str + "\n";
            LOG_ERROR(msg);
            exit(ERROR_INVALID_PARAM);
        }

        // Value at least 1
        if (value < 1) {
            msg = "Value of each parameter needs to be 1 or higher";
            LOG_ERROR(msg);
            exit(ERROR_INVALID_PARAM);
        }

        // set flags values
        if (flag == "-p") {
            if (value >= MIN_PORT_VALUE && value <= MAX_PORT_VALUE) {
                PORT = value;
                LOG_INFO("Port value is set to " + value_str);
            } else {
                msg = "Value of -p needs to be between " + std::to_string(MIN_PORT_VALUE) + " and " + std::to_string(MAX_PORT_VALUE) + "\n"; 
                LOG_ERROR(msg);
                exit(ERROR_INVALID_PARAM);
            }
        } else if (flag == "-c") {
            if (value >= MIN_CLIENT_COUNT) {
                CLIENTS_COUNT = value;
                LOG_INFO("Amount of allowed clients is set to: " + value_str);
            } else {
                msg = "Value of -c needs to be at least " + std::to_string(MIN_CLIENT_COUNT) +"\n";
                LOG_ERROR(msg);
                exit(ERROR_INVALID_PARAM);
            }
        } else if (flag == "-r") {
            ROOMS_COUNT = value;
            LOG_INFO("Amount of allowed rooms is set to: " + std::to_string(ROOMS_COUNT) + "\n");
        } else {
            msg = "Unknown flag parameter: " + flag + "\n";
            LOG_ERROR(msg);
            exit(ERROR_INVALID_PARAM);
        }
    }
}