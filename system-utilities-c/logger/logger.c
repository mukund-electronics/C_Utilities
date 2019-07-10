#include <stdio.h>
#include <stdarg.h>
#include "logger.h"

static const int *gCfgPtr = NULL;
static const char **gLogLvlNames = NULL;
static const char **gModuleNames = NULL;
static int gMaxLevels = 0;
static int gMaxModules = 0;
static int gLoggerEnabled = 0;

int initLogger(int maxLevels, int maxModules, const int *modulecfg, const char **logLeveNames, const char **moduleNames)
{
    if (gCfgPtr) {
        printf("Logger already initialised\n");
        return -1;
    }
    // For Debugging
    printf("data received in the init_logger\n");
    
    printf("maxlevels : %d\nmaxModules : %d\n", maxLevels, maxModules);
    
    for(int i = 0; i < maxModules; i++){
        printf("module %d : %d\n", i, modulecfg[i]);
    }

    // Saving the array in the logger local array
    gCfgPtr = modulecfg;
    gMaxLevels = maxLevels;
    gMaxModules = maxModules;
    gLogLvlNames = logLeveNames;
    gModuleNames = moduleNames;

    gLoggerEnabled = 1;

    return 0;
}

int printLog(unsigned int moduleId, unsigned int logLevel, char *data, ...)
{
    char formatted_string[200];
    if (0 == gLoggerEnabled) {
        return 0;
    }

    if (moduleId < gMaxModules && logLevel < gMaxLevels) {
        // Condition check for Printing the logs according to Priority
        if(gCfgPtr[moduleId] >= logLevel && gCfgPtr[moduleId] >= 0) {

            va_list argptr;
            va_start(argptr,data);
            vsprintf(formatted_string, data, argptr);
            va_end(argptr);
            printf("%s:%s:%s\n", gModuleNames[moduleId], gLogLvlNames[logLevel], formatted_string);
        }
    } else {
        printf("moduleId or logLevel out of bounds");
        return -1;
    }
    return 0;
}

void enableLogger(void)
{
    gLoggerEnabled = 1;
}

void disableLogger(void)
{
    gLoggerEnabled = 0;
}

int isLoggerEnabled(void)
{
    return gLoggerEnabled;
}
