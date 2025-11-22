#include "utility/utility.h"
#include "config.h"
#include "logger/logger.h"
#include "server/server.h"
#include "exceptions/serverException.h"

#include <iostream>
#include <string>
#include <cstdlib>


// exe cmd: server.exe -p (PORT) -C (MAX_CLIENT) -r (MAX_ROOMS)
int main(int argc, char *argv[]) {

    // Check parameters
    if (argc < MIN_ARG || argc > MAX_ARG) {
        LOG_ERROR("Invalid arguments count.\n"
                "\tNeed to run starting command using format: ./main <-p <PORT>> -c <MAX_CLIENT> -r <MAX_ROOMS>");
        return ERROR_INVALID_PARAM;
    };

    // Set Parameters
    handle_params(argc, argv);

    // Check if rooms and client set
    if (CLIENTS_COUNT == CLIENT_INIT_COUNT || ROOMS_COUNT == ROOMS_INIT_COUNT) {
        LOG_ERROR("Unset rooms or clients count\n"
                "\tNeed to run starting command in format: ./main -c <MAX_CLIENT> -r <MAX_ROOMS>");
        return ERROR_UNSET_PARAMETERS;
    }

    // set handler for Ctrl+C
    signal(SIGINT, ending_signal_handler);

    // Server set up and run
    try {
        LOG_INFO("Initializing Server");
        Server server;

        LOG_INFO("Starting server's main loop");
        server.run_server();

    } catch (const ServerException& e) {
        std::cerr << "FATAL SEVER ERROR: " << e.what() <<std::endl;
        LOG_ERROR("Server terminated: " + std::string(e.what()));
        LOG_ERROR("\tError Code: " + std::to_string(e.get_err_code()));

        return EXIT_FAILURE;

    } catch (const std::exception& e) {
        std::cerr << "UNEXPECTED ERROR: " << e.what() << std::endl;
        LOG_ERROR("Server terminated due to unexpected error: " + std::string(e.what()));
        return EXIT_FAILURE;
    }

    LOG_INFO("All went fine! Serve shut down (-:");
    return EXIT_SUCCESS;
}