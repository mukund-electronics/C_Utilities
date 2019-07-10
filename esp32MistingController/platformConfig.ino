#ifdef PLATFORM_ARDUINO

#include "platform.h"
#include "platformConfig.h"

typedef struct{  //struct to hold config parameters
  char *key;
  char *value;
}CfgElem_t;

int gEpCfgStart = 13; //starting address of eeprom
int gEpCfgLength = 500; //number of data bytes available to write in eeprom
char gEpKeyValueSeparator = ':'; //config data separator (key:value)
char gEpKeyValueTerminator = '\0'; //config data terminator (key:value\0)
char *gEpCfg = NULL;  //ptr to eeprom data
char *gEpCfgCurrKeyValuePtr = NULL; //ptr to hold starting address of key,value pairs in epCfg string
CfgElem_t *gCfg = NULL; //ptr to array of key,value pair holding structure
unsigned char gSetUpInit = 0; //to keep record the power up of the system
unsigned int gConfigCount = 0; //total no. of ep configurations
unsigned int gConfigBytes = 0; //total no. of ep config bytes

//initialize setup on powerup
int initSetUp()
{
  Serial.print("Initializing...\n");
  if(readEpCfg()){ //read epconfig
    Serial.print("initializing setup failed in reading epcfg\n");
    return -1;
  }
  int ret = loadCfg(); //load configurations to config str
  if(-1 == ret){
    Serial.print("initializing setup failed in loading cfg\n");
    return -1; 
  }
  gSetUpInit = 1;
  return 0;
}

//flush the configurations
int flushConfig()  
{ 
  Serial.print("Flushing the config...\n");
  int startAddress = gEpCfgStart;
  unsigned char dataByte = 0x00;
  setPersistentBytesWithDataByte(startAddress, gEpCfgLength, dataByte); //set the eeprom with a data byte from a specific address
  return 0;
}

//read configuration data from eeprom
int readEpCfg()
{
//  Serial.print("readEpCfg\n");
  if(NULL == gEpCfg) { //if eeprom data ptr is null allocate memory  
    gEpCfg = (char*)malloc(sizeof(char) * gEpCfgLength);
    if(NULL == gEpCfg) {
      Serial.print("malloc failed\n");
      return -1;
    }
    memset(gEpCfg, '\0', sizeof(char) * gEpCfgLength);
  }
  getPersistentBytes(gEpCfgStart, gEpCfg, gEpCfgLength); //read gEpCfgLength bytes from eeprom
  return 0;
}

//write configuration data to eeprom
int writeEpCfg()
{
//  Serial.print("writeEpCfg\n");
  if(gEpCfg) {
    setPersistentBytes(gEpCfgStart, gEpCfg, gEpCfgLength); //write gEpCfgLength bytes to eeprom
    return 0;
  } else {
    return -1;
  }
}

//reset the pointer to the starting address of configuration
int epCfgResetKeyValuePtr()
{
//  Serial.print("epCfgResetKeyValuePtr\n");
  gEpCfgCurrKeyValuePtr = gEpCfg;
  return 0;
}

//increment the current key,value pointer to the next pair and return the previous addresss
char *epCfgGetNextKeyValuePair()
{
  Serial.print("epCfgGetNextKeyValuePair\n");
  if(NULL == gEpCfgCurrKeyValuePtr){
    Serial.print("currKeyValuePtr is NULL\n");
    return NULL;
  }
  char *tempPtr = gEpCfgCurrKeyValuePtr;
  Serial.printf("gEpCfg:%p gEpCfgCurrKeyValuePtr:%p\n", gEpCfg, gEpCfgCurrKeyValuePtr);
  if('\0' == *tempPtr) {
    gConfigBytes = gEpCfgCurrKeyValuePtr - gEpCfg + 1; //update the no. bytes present in epCfg
    Serial.printf("gConfigBytes : %d\n", gConfigBytes);
    return NULL;
  }
  gEpCfgCurrKeyValuePtr = tempPtr + strlen(tempPtr) + 1; //to advance the ptr to next key, value pair
  return tempPtr;     
}

// load/fetch the eeprom configuration to the key,value pair config structure
int loadCfg()
{
  Serial.print("loadCfg....\n");
  int ret = initCfg(); //initialize the cfg structure (alloc mem for no. of configurations , reset the key, value ptr)
  if(1 == ret){
    return 1;
  } else if(-1 == ret) {
    return -1;
  }
  int cfgCount = 0;
  char *key = NULL;
  char *value = NULL;
  epCfgResetKeyValuePtr(); //reset key,value curr ptr
  for(char *keyValuePtr = epCfgGetNextKeyValuePair(); (keyValuePtr != NULL); keyValuePtr = epCfgGetNextKeyValuePair()) { //fetch key,value pair from config str
      //separate key value pair
      key = strtok(keyValuePtr, &gEpKeyValueSeparator);
      value = strtok(NULL, &gEpKeyValueSeparator);
//      Serial.printf("key:%s\n", key);
//      Serial.printf("value:%s\n", value);
      //add cfg to the key,value config structure
      if(0 != cfgAdd(cfgCount, key, value)) {
        return -1;
      }
      cfgCount++;
  }
  return 0;
}

//add the key,value pairs to config structure
int cfgAdd(int index, char *key, char *value)
{
//    Serial.print("cfgAdd\n");
    if(NULL == gCfg){ //if no mem allocated for key, value pair structure
      return -1;
    }
    if(gCfg[index].key) { //if already allocated free and allocate new
      free(gCfg[index].key);
    }
    gCfg[index].key = strdup(key);

    if(NULL == gCfg[index].key) {
      Serial.print("strdup failed\n");
      return -1; 
    }
    if(gCfg[index].value) {
      free(gCfg[index].value);
    }
    gCfg[index].value = strdup(value);

    if(NULL == gCfg[index].value) {
      Serial.print("strdup failed\n");
      return -1; 
    }
//    Serial.printf("key added:%s\n",  gCfg[index].key);
//    Serial.printf("val added:%s\n",  gCfg[index].value);
    return 0;
}

//update the cofig value at specified index of config str
int cfgUpdate(int index, char *value)
{
//  Serial.print("cfgUpdate\n");
  free(gCfg[index].value);
  gCfg[index].value = strdup(value);
  if(NULL == gCfg[index].value) {
    return -1;
  }
  return 0;
}

//allocate memory for the number of configurations present
int initCfg()
{
  Serial.print("initCfg...\n");
  int count = 0;
  count = epCfgCountKeyValuePair(); //get no. of configurations
  if(count) {
    gCfg = (CfgElem_t *)malloc(sizeof(CfgElem_t) * count); //allocate memory for count no. of key,value pairs
    if(NULL == gCfg) {
      Serial.print("malloc failed\n");
      return -1;
    }
    memset(gCfg, '\0', sizeof(CfgElem_t) * count);
  } else if(0 == count) {
    return 1; //return 1 for no configuration present
  }
  return 0;
}

//count the no. of configurations
int epCfgCountKeyValuePair()
{
//  Serial.print("epCfgCountKeyValuePair\n");
  int count = 0;
  epCfgResetKeyValuePtr();  //reset the key, value ptr
  for(char *keyValuePtr = epCfgGetNextKeyValuePair(); (keyValuePtr != NULL) && (*keyValuePtr != '\0'); keyValuePtr = epCfgGetNextKeyValuePair()) {
    count++;   
  }
  gConfigCount = count;
//  Serial.printf("(epCfgCountKeyValuePair) gConfigCount : %d\n", gConfigCount);
  return count;  
}

//get value corresponding to the matched key
int getKeyValue(int index, char *getValue)
{
  strcpy(getValue, gCfg[index].value);
  return 0;
}

//return index if key is matched
int findKeyMatchedIndex(char *key)
{
  int ret;
    for(int i = 0; i < gConfigCount; i++) {
      ret = strcmp(key, gCfg[i].key);
      if(0 == ret) { //key matched return index
        return i;
      }
    }
  return -1;
}

//check for the matched key,return its value if found
int isKeyPresent(char *key, char *getValue)
{
  int index = findKeyMatchedIndex(key);
  if(index >= 0) { //key matched , copy the corresponding value
    strcpy(getValue, gCfg[index].value);
    return 0;
  } else {
//    strcpy(getValue, NULL);
    *getValue = '\0';
    return -1;
  }
  return -1;
}

//return index if key is matched
char *getCfgKeyValue(char *key)
{
  int ret;
  for(int i = 0; i < gConfigCount; i++) {
    ret = strcmp(key, gCfg[i].key);
    if(0 == ret) { //key matched return index
      return gCfg[i].value;
    }
  }
  return NULL;
}

//to fetch the value corresponding to key otherwise return NULL
char *getConfig(char *key)
{
//  Serial.printf("configurations present : %d\n", gConfigCount);
//  Serial.print("Getting the configuration...\n");
  if(gSetUpInit) { //if setup is initialized successfully
    return getCfgKeyValue(key);
  }
  return NULL;
}

//return the no. of bytes to write for a key,value pair
int keyValuePairEpConfigBytes(char *key, char *value)
{
//  Serial.print("keyValuePairEpConfigBytes\n");
  return ((strlen(key) + strlen(value) + 2)); // format: "key:value\0"  
}

//check if bytes not exceeding for the new configuration to add
int checkBytesLimitIfKeyNotFound(char *key, char *value)
{
//  Serial.print("checkBytesLimitIfKeyNotFound\n");
  int configBytesCount = keyValuePairEpConfigBytes(key, value); //extra bytes to write for new config
  
  if((gConfigBytes + configBytesCount) > gEpCfgLength) { //if bytes exceed the eeprom mem limit
    return -1;
  }
  return 0;
}

//check if bytes not exceeding for replacing the configuration
int checkBytesLimitIfKeyFound(int extraBytesToAdd)
{
//  Serial.print("checkBytesLimitIfKeyFound\n");
  if((gConfigBytes + extraBytesToAdd) > gEpCfgLength) { //if bytes exceed the eeprom mem limit
    return -1;
  }
  return 0;
}

//allocate mem for config str to add new config
int allocMemForNewConfig()
{
//  Serial.print("allocMemForNewConfig\n");
  if(gCfg) { //if already allocated realloc
    gCfg = (CfgElem_t *)realloc(gCfg, sizeof(CfgElem_t) * (gConfigCount + 1));
    memset(gCfg + gConfigCount, '\0', sizeof(CfgElem_t));
    } else { //allocate new block
    gCfg = (CfgElem_t *)malloc(sizeof(CfgElem_t) * 1);
    memset(gCfg, '\0', sizeof(CfgElem_t));
  }
  if(NULL == gCfg){
    Serial.print("mem allocation failed\n");
    return -1;
  }
  return 0;
}

// create the new ep config from the existing config str
int createEpCfg()
{
//  Serial.print("createEpConfig\n");
  epCfgResetKeyValuePtr(); //reset the key value ptr to the starting address of epCfg
  for(int i = 0; i < gConfigCount; i++) {
    sprintf(gEpCfgCurrKeyValuePtr, "%s%c%s",gCfg[i].key, gEpKeyValueSeparator, gCfg[i].value); //prepare the format:: "key:value\0"
    gEpCfgCurrKeyValuePtr += strlen(gEpCfgCurrKeyValuePtr) + 1;
  }
  //add extra '\0' for configuration terminator
  *gEpCfgCurrKeyValuePtr = '\0';
  gConfigBytes = gEpCfgCurrKeyValuePtr - gEpCfg + 1; //update no. of epCfg bytes
  epCfgResetKeyValuePtr(); // reset again
  return 0;
}

//add configuration to cfg str
int addConfig(char *key, char *value)
{
  Serial.print("Adding the configuration...\n");
  int ret = allocMemForNewConfig();
  if(-1 == ret) { //if mem allocation failed
    return -1;    
  }
  
  int writeIndex = gConfigCount;
  //add key value pair to config str
  ret = cfgAdd(writeIndex, key, value);
  if(-1 == ret) { //if add configuration failed
    return -1;    
  }
  gConfigCount++; //update config count
  return 0;
}

//check the already present config, verify space and then update config str
int updateConfig(int index, char *key, char *value)
{
   Serial.print("Updating the configuration...\n");
  //check is value is same, then no need to set the config
  int ret = strcmp(gCfg[index].value, value);
  if(0 == ret) { //if value string is same
    Serial.print("same config, already set\n");
    return 0;
  }
  
  int valueAlreadySetStringLen = strlen(gCfg[index].value);
  int valueToSetStringLen = strlen(value);
  
  if(valueAlreadySetStringLen < valueToSetStringLen) { //if new string is big, check the memory limit
    ret = checkBytesLimitIfKeyFound(valueToSetStringLen - valueAlreadySetStringLen);
    if(-1 == ret) { //if mem is going to exceed
      Serial.print("not enough space\n");
      return 0;
    }
  }
  
  //replace the old config with new one in config str
  ret = cfgUpdate(index, value);
  if(-1 == ret) { //if replace configuration failed
    return -1;    
  }
  return 0;  
}

// write the configuration to eeprom
int setConfig(char *key, char *value)
{
  int ret = 0;
  if(gSetUpInit) { //setup initialized sucessfully
    //check if key is present and get its index
    int index = findKeyMatchedIndex(key);
    if(-1 == index) { //if key not found
      
      //check if bytes to add not exceeding memory limit
      ret = checkBytesLimitIfKeyNotFound(key, value);
      if(-1 == ret) {
        Serial.print("not enough space\n");
        return -1; 
      } else {
        // add new configuration
        ret = addConfig(key, value);
        if(-1 == ret) { //if add config failed
          return -1;    
        }
//        Serial.printf("config count becomes:%d\n", gConfigCount);
      }
      
    } else { //if key is found
      ret = updateConfig(index, key, value);
      if(-1 == ret) { //if verify and set config failed
        return -1;    
      }
    }
    
    //append the epCfg (eeprom data string)
    ret = createEpCfg();
    if(-1 == ret) { //if create configuration failed
      return -1;    
    }
  
    //write the config to eeprom
    ret = writeEpCfg();
    if(-1 == ret) { //if write epCfg failed
      return -1;    
    }
    
  } else {
    Serial.print("setup not initialized\n");
    return -1;  
  }
  return 0;
}

//deallocate config str memory by one element
int deallocateConfigElement(int index)
{
  free(gCfg[index].key);  // free key value pair 
  free(gCfg[index].value); //
//  memset(gCfg + index, '\0', sizeof(CfgElem_t));
  gCfg = (CfgElem_t *)realloc(gCfg, sizeof(CfgElem_t) * (gConfigCount - 1)); //free the config str memory by 1 element
  if(NULL == gCfg) {
    Serial.print("realloc mem allocation failed\n");
    return -1;
  }
  return 0;
}

//shift the config str after deleting the configuration
int cfgShift(int index)
{
  int configToShift = gConfigCount - index - 1;
  int ret = 0;
  if(0 == configToShift) { //if last config to be deleted
    ret = deallocateConfigElement(index); //dealloc mem for last config elem
    if(-1 == ret) {
      return -1;
    }
    return 0;
  }

  int currIndex = index;
  int nextIndex = index + 1;
  for(int i = 0; i < configToShift; i++) {
    ret = cfgAdd(currIndex, gCfg[nextIndex].key, gCfg[nextIndex].value);
    if(-1 == ret) { //cfg Add failed
      return -1;
    }
    currIndex++;
    nextIndex = currIndex + 1;
  }
  deallocateConfigElement(gConfigCount -1); //dealloc mem for last element
  return 0;
}

//initialize the epCfg by \0
int flushEpCfg()
{
  memset(gEpCfg, '\0', sizeof(char)*gEpCfgLength);
  return 0;
}

//delete the config at specified index
int deleteAndUpdateConfig(int index)
{
  Serial.print("deleting the configuration...\n");
  int ret;
  ret = cfgShift(index);
  if(-1 == ret) { //if updation of config str failed
    return -1;    
  }
  
  gConfigCount--; //update config count

  ret = flushEpCfg(); 
  
  //append the epCfg (eeprom data string)
  ret = createEpCfg();
  if(-1 == ret) { //if create configuration failed
    return -1;    
  }
  
  //write the config to eeprom
  ret = writeEpCfg();
  if(-1 == ret) { //if write epCfg failed
    return -1;    
  }
  return 0;  
}

//delete the eeprom configuration
int delConfig(char *key)
{
  //check if key is present
  int index = findKeyMatchedIndex(key);
  if(-1 == index) {
    Serial.print("key not matched to delete\n");
  } else { //key found
    int ret = deleteAndUpdateConfig(index);
    if(-1 == ret) { //not deleted
      return -1;
    }
  }
  return 0; 
}

int readConfig(char *configData)
{
  Serial.print("readConfig\n");
  for(int i = 0; i < gEpCfgLength; i++) {
    *(configData + i) = *(gEpCfg + i);
    //Serial.printf("%d: %c | %x", i + 13, *(gEpCfg + i), *(gEpCfg + i));
    Serial.printf("%c", *(gEpCfg + i));
  }
  Serial.print("\n");
  return 0;
}
#endif //PLATFORM_ARDUINO
