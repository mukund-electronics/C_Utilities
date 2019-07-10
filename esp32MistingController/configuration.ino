#include "configuration.h"
#include "platformConfig.h"

int setConfiguration(char *key, char *value)
{
  if(setConfig(key, value)){
    return -1;
  }
  return 0;
}

char* getConfiguration(char *key)
{
  return getConfig(key);
}

int delConfiguration(char *key)
{
  if(delConfig(key)){
    return -1;
  }
  return 0;
}

int resetConfigurations()
{
  if(flushConfig()){
    return -1;
  }
  return 0;
}

int readConfigurations(char *configData)
{
  if(readConfig(configData)){
    return -1;
  }
  return 0;
}

int initializeSetUp()
{
  int ret = initSetUp();
  if(-1 == ret){
    return -1;
  }
  return 0;
}
