#ifndef __CONFIG__
#define __CONFIG__

/* 
----------------
-- Constrains --
----------------
*/
#define MIN_PORT_VALUE 1024
#define MAX_PORT_VALUE 65535
#define MIN_CLIENT_COUNT 2

/* 
-----------------
-- Init values --
-----------------
*/
#define PORT_INIT 10000
#define CLIENT_INIT_COUNT -1
#define ROOMS_INIT_COUNT -1
#define MAX_BUFFER_SIZE 1024
#define BACKLOG_SIZE 16
#define ADDITIONAL_STREAM 1

/* 
------------------
-- Logger Setup --
------------------
*/
#define LOGS_FOLDER "logs"
#define LOGGER_PATH "logs/server.log"

#endif