#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

char* eepromDeviceName = "dN";
char* eepromDeviceState = "dS";
char* eepromSSID = "ss";
char* eepromPassword = "pwd";

/*  MOTOR CONFIG NAME In EEPROM
* blSpd
* smSpd
* tOn
* tOff
* blBfnT
* blAffT
*/


/* write configuration
   @param key - parameter name
   @param value - parameter value
   @returns -1 if not enough space
             0 if configuration set successfully(add if new key otherwise overwrite) 
*/
int setConfiguration(char *key, char *value);

/* get configuration
   @param key - parameter name
   @returns parameter value address for the matched key
            else return NULL 
*/
char* getConfiguration(char *key);

/* delete the configuration
   @param key - parameter name
   @returns -1 if no parameter present
             0 if configuration deleted successfully 
*/
int delConfiguration(char *key);

/* flush all configurations*/
int resetConfigurations();

/* initialize the memory after power on */
int initializeSetUp();

//FOR DEBUGGING
/* Read complete config string */
int readConfigurations();

#endif // _CONFIGURATION_H
