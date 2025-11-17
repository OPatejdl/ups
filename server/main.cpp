#include "utility/utility.h"
#include "config.h"
#include "logger/logger.h"

#include <iostream>
#include <string>
#include <cstdlib>

#include <sys/types.h>
#include <sys/socket.h>


// exe cmd: server.exe -p (PORT) -C (MAX_CLIENT) -r (MAX_ROOMS)
int main(int argc, char *argv[]) {
    int server_socket, return_value;
    int client_socket;
    char buffer[BUFFER_SIZE];
    
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

    LOG_INFO("Server setup: \n\tPort: " + std::to_string(PORT) + 
            "\n\tClients: " + std::to_string(CLIENTS_COUNT) +
            "\n\tRooms: " + std::to_string(ROOMS_COUNT));

    LOG_INFO("Server ends");
    return EXIT_SUCCESS;
}