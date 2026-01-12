#include "utility/utility.hpp"
#include "config.hpp"
#include "logger/logger.hpp"
#include "server/server.hpp"
#include "exceptions/exceptions.hpp"

#include <iostream>
#include <string>
#include <cstdlib>


// exe cmd: server.exe -p (PORT) -C (MAX_CLIENT) -r (MAX_ROOMS)
int main(int argc, char *argv[]) {
    int err_code;
    // Check parameters
    if (argc < Utility::MIN_ARG || argc > Utility::MAX_ARG) {
        LOG_ERROR("Invalid arguments count.\n"
                "\tNeed to run starting command using format: ./server.exe <-p <PORT>> -c <MAX_CLIENT> -r <MAX_ROOMS> [-a <IP>]");
        return Utility::ERROR_INVALID_PARAM;
    };

    // Server set up and run
    try {
        // Set Parameters
        Utility::handle_params(argc, argv);

        // Check if rooms and client set
        if (Utility::CLIENTS_COUNT == Config::CLIENT_INIT_COUNT || Utility::ROOMS_COUNT == Config::ROOMS_INIT_COUNT) {
            LOG_ERROR("Unset rooms or clients count\n"
                    "\tNeed to run starting command in format: ./server.exe <-p <PORT>> -c <MAX_CLIENT> -r <MAX_ROOMS> [-a <IP>]");
            return Utility::ERROR_UNSET_PARAMETERS;
        }

        // set handler for Ctrl+C
        signal(SIGINT, Utility::ending_signal_handler);

        LOG_INFO("Initializing Server");
        MyServer::Server server;

        LOG_INFO("Starting server's main loop");
        server.run_server();

    // Error when handling user's params
    } catch (const MyExceptions::UtilityException& e) {
        err_code = e.get_err_code();
        std::cerr << "FATAL SEVER ERROR: " << e.what() <<std::endl;
        LOG_ERROR("Server terminated: " + std::string(e.what()));
        LOG_ERROR("\tError Code: " + std::to_string(err_code));

        return err_code;

    // Error while server running
    }catch (const MyExceptions::ServerException& e) {
        err_code = e.get_err_code();
        std::cerr << "FATAL SEVER ERROR: " << e.what() <<std::endl;
        LOG_ERROR("Server terminated: " + std::string(e.what()));
        LOG_ERROR("\tError Code: " + std::to_string(err_code));

        return err_code;

    // Unexpected Error
    } catch (const std::exception& e) {
        std::cerr << "UNEXPECTED ERROR: " << e.what() << std::endl;
        LOG_ERROR("Server terminated due to unexpected error: " + std::string(e.what()));
        return EXIT_FAILURE;
    }

    LOG_INFO("All went fine! Serve shut down (-:");
    return EXIT_SUCCESS;
}