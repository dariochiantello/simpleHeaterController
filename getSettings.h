/**
 * @file getSettings.h
 * @author Dario Chiantello
 * @brief Default settings and setting acquisition header file
 * @version 1.0
 * @date 2023-09-19
 * @note //compiler command: gcc app.c getSettings.c -lm -o app
 * 
 * @copyright GNU 2023
 * 
 */

#ifndef GET_SETTINGS
#define GET_SETTINGS


#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>



#define MAX_BUFFER_LEN          101

#define DEFAULT_LOG_NAME        "log.txt"
#define DEFAULT_DEBUG           true
#define DEFAULT_PERIOD_S        2.5f
#define MAX_PERIOD_S            3600


#define HELP_CMD                "h"
#define DEBUG_CMD               "d"
#define CYCLE_PERIOD_CMD        "t"
#define LOG_FILE_CMD            "l"
#define QUIT_CMD                "q"

#define CMD_STRING              "hd:t:l:"

typedef struct settingStruct
{
    bool  * debug;
    char  * logFile;
    float * cyclePeriod;

}SettingStruct;


int getSettings(SettingStruct * settingStruct, int argc, char * const argv[]);


#endif

