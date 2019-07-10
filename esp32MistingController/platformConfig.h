#ifndef __PLATFORMCONFIG_H__
#define __PLATFORMCONFIG_H__

char* getConfig(char *key);
int setConfig(char *key, char *value);
int delConfig(char *key);
int flushConfig();
int readConfig(char *data);
int initSetUp();

#endif //__PLATFORMCONFIG_H__
