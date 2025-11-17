#ifndef __UTILITY__
#define __UTILITY__

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

/* 
-------------------------
----- Param Symbols -----
-------------------------
*/
// Init of these variables is in utility.cpp
extern int PORT;
extern int ROOMS_COUNT;
extern int CLIENTS_COUNT;

/*
-------------------------
------- Functions -------
-------------------------
*/

void handle_params(int argc, char *argv[]);


#endif