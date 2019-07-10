// Application example using the logger framework

#include <stdio.h>

#include "logger.h"

enum logLevel{          // LOG Levels defined according to the priority levels
    LVL_DEBUG = 0,
    LVL_INFO,
    LVL_WARNING,
    LVL_ERROR,
    LVL_CRITICAL,
    LVL_DISABLE
};

enum modules{           // Modules in the application program
    MOD_GPS = 0,
    MOD_GYRO,
    MOD_MAIN,
    MOD_LAST
};

const char* gLogLevelNames[LVL_DISABLE] = {
    [LVL_DEBUG] = "DBG",
    [LVL_INFO] = "INF",
    [LVL_WARNING] = "WAR",
    [LVL_ERROR] = "ERR",
    [LVL_CRITICAL] = "CRT"
};

const char* gModuleNames[MOD_LAST] = {
    [MOD_GPS] = "GPS",
    [MOD_GYRO] = "GYRO",
    [MOD_MAIN] = "MAIN"
};

const int gLogCfg[MOD_LAST] = {                         // Configuring the modules with the log levels
    [MOD_GPS] = LVL_INFO,
    [MOD_GYRO] = LVL_ERROR,
    [MOD_MAIN] = LVL_CRITICAL
};

int main(void){

    int i = 10;
    initLogger(LVL_DISABLE, MOD_LAST, gLogCfg, gLogLevelNames, gModuleNames);            // Initializing the Logger framework

    printLog(MOD_GPS, LVL_INFO, "test data : %d", i);           // print
    printLog(MOD_GPS, LVL_ERROR, "test data");          // 
    printLog(MOD_GPS, LVL_DEBUG, "test data");          // print
    
    printLog(MOD_MAIN, LVL_INFO, "test data");      //
    printLog(MOD_MAIN, LVL_DEBUG, "test data");     //
    
    printLog(MOD_GYRO, LVL_DEBUG, "test data");         // print
    printLog(MOD_GYRO, LVL_CRITICAL, "test data");      // 

    return 0;
}
