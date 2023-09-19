/**
 * @file getSettings.c
 * @author Dario Chiantello
 * @brief Setting acquisition
 * @version 1.0
 * @date 2023-09-19
 * @note //compiler command: gcc app.c getSettings.c -lm -o app
 * 
 * @copyright GNU 2023
 * 
 */


#include "getSettings.h"


char defaultLogName[] = DEFAULT_LOG_NAME;


/**
 * @brief input filename string check (to be implemented)
 * 
 * @param appStruct 
 * @param stat 
 * @return true if format ok
 */
bool checkFileName(char* fileName)
{
    return true;
}


/**
 * @brief get settings and options
 * 
 * @param settingStruct 
 * @param argc 
 * @param argv 
 * @return -1 terminate program failure, 1 terminate program success, 0 continue
 */
int getSettings(SettingStruct * settingStruct, int argc, char * const argv[])
{

    int numOfArgs = argc;
    int optRes;

    *(settingStruct->debug)       = DEFAULT_DEBUG;
    settingStruct->logFile        = defaultLogName;

    *(settingStruct->cyclePeriod) = (float)DEFAULT_PERIOD_S;


    while ((optRes = getopt (argc, argv, CMD_STRING)) != -1)
    {
        switch (optRes)
        {
            case 'h': //Help
            {        
                printf("\n");
                printf("-%s                 help\n", HELP_CMD);
                printf("-%s [1/0]           activate or deactivate debug\n", DEBUG_CMD);
                printf("-%s [sec]           set heater cycle period in seconds (max: %.1f sec)\n", CYCLE_PERIOD_CMD, (float)MAX_PERIOD_S);
                printf("-%s [filename]      set log filename\n", LOG_FILE_CMD);

                printf("\n");
                printf("Default cycle period: %.1f sec\n", (float)DEFAULT_PERIOD_S);
                printf("Default log filename: %s\n", DEFAULT_LOG_NAME);

                printf("\n");
                printf("While executing:\n");
                printf("%s                  quit\n", QUIT_CMD);
                printf("%s                  toggle debug. Logging starts only in debug mode\n", DEBUG_CMD);
                printf("01010              binary string of arbitrary length to set the heater sequence.\n") ;

                printf("\n");
                printf("NOTE: max length of the sequence input buffer = %d chars.\n", MAX_BUFFER_LEN -1);
                printf("      The sequence beyond the max length will be ignored\n");
                printf("\n");

                return 1;
            }
            break;

            case 'd': //Debug
            {
                //null case covered
                if(*optarg == '0')
                    *(settingStruct->debug) = false;
                else if(*optarg == '1')
                    *(settingStruct->debug) = true;
                else
                {
                    printf("Invalid -d value. Use -%s for help\n", HELP_CMD);
                    return -1;
                }
            }
            break;

            case 't': //Cycle period
            {
                float cyclePeriod = (float)atof(optarg);

                if(cyclePeriod > 0 && cyclePeriod < MAX_PERIOD_S)
                {
                    *(settingStruct->cyclePeriod) = cyclePeriod;
                }
                else
                {
                    printf("Invalid period value. Use -%s for help\n", HELP_CMD);
                    return -1;
                }
            }
            break;

            case 'l': //log file
            {
                //null case covered. Check for valid filenames (simple solution: only lowercase or capital letters and numbers)
                if(checkFileName(optarg))
                {
                    settingStruct->logFile = optarg;
                }
                else
                {
                    printf("Invalid log filename\n");
                    return -1;
                }
            }
            break;

            case '?':
            break;

            default:
            return 0;
        }
    }

    if (*(settingStruct->debug) == true && numOfArgs == 1)
    {
        printf("\nUse -h for help\n\n");
    }

    return 0;
}
