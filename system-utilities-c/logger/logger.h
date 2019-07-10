#ifndef __LOGGER_H__
#define __LOGGER_H__

int initLogger(int maxLevels, int maxModules, const int *modulecfg, const char **logLeveNames, const char **moduleNames);
int printLog(unsigned int moduleId, unsigned int logLevel, char *data, ...);
void enableLogger(void);
void disableLogger(void);
int isLoggerEnabled(void);
#endif
