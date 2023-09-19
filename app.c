/**
 * @file app.c
 * @author Dario Chiantello
 * @brief Simple heater controller
 * @version 1.0
 * @date 2023-09-19
 * @note //compiler command: gcc app.c getSettings.c -lm -o app
 * 
 * @copyright GNU 2023
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include <unistd.h>

#include "getSettings.h"


typedef struct timeval Timeval;


/**
 * @brief application control struct
 * 
 */
typedef struct appStruct
{
    fd_set      dSet;
    Timeval     timeout;
    Timeval     period;
    double      timeKeeper;
    char        buffer[MAX_BUFFER_LEN];
    bool        updateTimer;
    bool        isCommand;
    bool        debug;
    char *      logFile;
    FILE *      logPointer;

}AppStruct;


/**
 * @brief heater control struct
 * 
 */
typedef struct controlStruct
{
    float       cyclePeriod;
    char        controlSequence[MAX_BUFFER_LEN];
    int         numberOfPeriods;
    int         periodCounter;

}ControlStruct;




/**
 * @brief application terminator
 * 
 * @param appStruct 
 * @param stat 
 */
void terminateApp(AppStruct * appStruct, int stat)
{
    if(appStruct->logPointer != NULL)
    {
        fclose(appStruct->logPointer); 
    }
   
   exit(stat);
}


/**
 * @brief Print to terminal and log file
 * 
 * @param allowTerminal 
 * @param appStruct 
 * @param fmt 
 * @param ... 
 */
void logPrint(bool allowTerminal, AppStruct * appStruct, const char *fmt,  ...)
{
    va_list ap;

    if(appStruct->debug || allowTerminal)
    {
        va_start(ap, fmt);
        vfprintf(stdout, fmt, ap);
        va_end(ap);
    }

    if(appStruct->debug && appStruct->logPointer != NULL)
    {
        va_start(ap, fmt);
        vfprintf(appStruct->logPointer, fmt, ap);
        va_end(ap);
    }
}


/**
 * @brief get double timestamp in seconds
 * 
 * @return timestamp
 */
double timeLog(void)
{
    Timeval startTime;
    gettimeofday(&startTime, NULL);
    return ((double)startTime.tv_sec + (double)startTime.tv_usec / 1000000);
}


/**
 * @brief receive commands and manage tx cycles. 
 *         
 * 
 * @param appStruct 
 * @param controlStruct 
 * @return -1 terminate program failure, 1 terminate program success, 0 continue
 */
int txrx(AppStruct * appStruct, ControlStruct * controlStruct)
{
    FD_ZERO(&(appStruct->dSet)); 
    FD_SET(0, &(appStruct->dSet));

    //keep the cycle timer if the received sequence is not valid
    if(appStruct->updateTimer)
    {
        appStruct->timeout.tv_sec  = appStruct->period.tv_sec;
        appStruct->timeout.tv_usec = appStruct->period.tv_usec;
    }


    //timeout or message received. Process message if there is any
    if (select(1, &(appStruct->dSet), NULL, NULL, &(appStruct->timeout)) < 0)
    {
        logPrint(false, appStruct, "Error 1\n");
        return -1;
    }
    else
    {
        //something received
        if(FD_ISSET(0, &(appStruct->dSet)))
        {
            unsigned int count   = 0;
            bool         isValid = true;

            memset(appStruct->buffer, 0, MAX_BUFFER_LEN);

            appStruct->isCommand = false;

            //read, check and flush stdin
            char temp = getc(stdin);

            while(temp != EOF && temp != '\n')
            {
                if(temp == 'q' && count == 0)
                {
                    logPrint(false, appStruct, "Quit application\n");
                    return 1;
                }
                if(temp == 'd' && count == 0)
                {
                    appStruct->debug     = !appStruct->debug;
                    appStruct->isCommand = true;
                }

                if(isValid && count < MAX_BUFFER_LEN-1)
                {
                    appStruct->buffer[count] = temp;
                    count++;  

                    if(temp != '0' && temp != '1')
                    {
                        isValid = false;
                    }   

                    if(count == MAX_BUFFER_LEN-1)
                    {
                        logPrint(false, appStruct, "Warning: buffer full\n");
                    }
                }

                temp = getc(stdin);
            }

            appStruct->buffer[count] = '\n';

            //valid message, set new sequence
            if(isValid)
            {
                if(appStruct->debug)
                {
                    logPrint(false, appStruct, "New control sequence:\n");
                    logPrint(false, appStruct, "%s", appStruct->buffer);
                }

                memset(controlStruct->controlSequence, 0, MAX_BUFFER_LEN);
                memcpy(controlStruct->controlSequence, appStruct->buffer, count);

                controlStruct->periodCounter   = 0;
                controlStruct->numberOfPeriods = count;

                appStruct->timeKeeper = timeLog();
            }
            //invalid message or command, continue cycle
            else
            {
                if(!appStruct->isCommand)
                    logPrint(false, appStruct, "Invalid control sequence received\n");

                appStruct->updateTimer = false;
                return 0;
            }
        }
    }

    //prepare next cycle
    appStruct->updateTimer = true;

    return 0;
}


/**
 * @brief manages the heater sequence commands
 * 
 * @param appStruct 
 * @param controlStruct 
 */
void controlOutput(AppStruct * appStruct, ControlStruct * controlStruct)
{
   if(appStruct->updateTimer)
   {
        if(controlStruct->periodCounter == controlStruct->numberOfPeriods)
        {
            logPrint(false, appStruct, "No input sequence.\n");

            controlStruct->periodCounter++;
        }

        if(controlStruct->periodCounter < controlStruct->numberOfPeriods)
        {
            if(controlStruct->controlSequence[controlStruct->periodCounter] == '1')
            {
                logPrint(true, appStruct, "[ON]");
            }
            else
            {
                logPrint(true, appStruct, "[OFF]");
            }

            if(!(appStruct->debug))
                printf("\n"); 

            controlStruct->periodCounter++;
        }

        //tick timing check
        logPrint(false, appStruct, ">%0.3f\n", timeLog() - appStruct->timeKeeper);  
   }
}


/**
 * @brief Set cycle period
 * 
 * @param timeval 
 * @param period 
 */
void setPeriod(Timeval * timeval, float period)
{
    timeval->tv_sec  = (long)floor(period);
    timeval->tv_usec = ((long)(round(period*10)))%10 * 100000;
}



/**
 * @brief main function
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char * const argv[])
{
    //init
    AppStruct       appStruct;
    ControlStruct   controlStruct;

    memset(&appStruct,      0, sizeof(AppStruct));
    memset(&controlStruct,  0, sizeof(ControlStruct));

    controlStruct.periodCounter = 1;
    appStruct.updateTimer       = true;


    //get settings
    SettingStruct settingStruct;

    settingStruct.debug        = &(appStruct.debug);
    settingStruct.cyclePeriod  = &(controlStruct.cyclePeriod);

    int getSettingsRes = getSettings(&settingStruct, argc, argv);
    
    if( getSettingsRes < 0)
        terminateApp(&appStruct, EXIT_FAILURE);
    else if (getSettingsRes > 0)
        terminateApp(&appStruct, EXIT_SUCCESS);

    appStruct.logFile = settingStruct.logFile;

    setPeriod(&(appStruct.period), controlStruct.cyclePeriod);



    //log
    appStruct.timeKeeper = timeLog();

    if(appStruct.debug)
        appStruct.logPointer = fopen(appStruct.logFile, "w+");

    logPrint(false, &appStruct, "Settings:\n");
    logPrint(false, &appStruct, "-Cycle period (sec): %.1f \n", controlStruct.cyclePeriod);
    logPrint(false, &appStruct, "-Log file: %s\n\n", appStruct.logFile);
        
    logPrint(false, &appStruct, "Type a control sequence (0101001...) and press enter.\n");  


    while(1)
    {  
        //process tx rx
        int txrxRes = txrx(&appStruct, &controlStruct);

        if( txrxRes < 0)
            terminateApp(&appStruct, EXIT_FAILURE);

        else if (txrxRes > 0)
            terminateApp(&appStruct, EXIT_SUCCESS);


        //control output
        controlOutput(&appStruct, &controlStruct);
    }

    terminateApp(&appStruct, EXIT_SUCCESS);
}




