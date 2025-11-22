#ifndef __UTILITY__
#define __UTILITY__

#include <signal.h>
/* 
------------------------
-- Preprocess Symbols --
------------------------
*/
#define MIN_ARG 5
#define MAX_ARG 7

/* 
------------------------
-- Error Codes --
------------------------
*/
#define ERROR_INVALID_PARAM 1
#define ERROR_UNSET_PARAMETERS 2
#define ERROR_LOGGER_UNOPEN 3
#define ERROR_UNCREATED_SERVER_SOC 4
#define ERROR_BINDING 5
#define ERROR_LISTEN 6

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


#endif