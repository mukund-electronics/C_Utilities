#ifndef _BOOT_CONFIGURATION_H    
#define _BOOT_CONFIGURATION_H

/**Boot EEPROM Bytes
 * Address 0 - check for the application booting first time(if read A5(not the first boot))
 * Address 1 - check for the already saved network configuration, ssid and password which needs to try to connect after bootup(if B5(nw config present))
 * Address 2 - Motor configuration Present or not
 * Address 3 - Device Id present or not, device Registered or not
 */


/**@brief check for the first boot after loading the application
  *return 0 if application starts first time
*/
int isFirstBoot();

/**@brief check for the saved ssid and password to connect after bootup
 * return 0 if no wifi ssid, password configuration present 
 */
int isNetworkConfigPresent();

/**@brief check for the saved ssid and password to connect after bootup
 * return 0 if no wifi ssid, password configuration present 
 */
int isMotorConfigPresent();

/**@brief set the eeprom data to mark the presence of network config on power on
 */
void setNetworkConfigPresence();

/**@brief clear the eeprom data to mark the absence of network config
 */
void clearNetworkConfigPresence();

/**@brief clear all the boot config flags;
 */
void flushBootConfig();

void clearMotorConfigPresence();
void setNetworkConfigPresence();

int isDeviceIdPresent();
void clearDeviceIdPresence();
void setDeviceIdPresence();

#endif // _BOOT_CONFIGURATION_H    
