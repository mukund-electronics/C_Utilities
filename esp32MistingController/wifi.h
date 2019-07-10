#ifndef _WIFI_H
#define _WIFI_H

//structure to hold ssids of scanned networks
typedef struct{
  char ssid[40];
}ApInfo_t;

/*@brief start the wifi in access point mode
 * @param hotspotSsid - ssid of the hotspot
 * @param hotspotPassword - authentication password for hotspot
 * returns 0 if started in ap mode successfully
 *         1 for failure
*/
int apModeSetup(const char *hotspotSsid, const char *hotspotPassword);
int apModeSetupOpen(const char *hotspotSsid); //open for every user

/*@brief disconnect from the access point mode
*/
void apModeDisconnect();

/*@brief disconnect from the wifi router
*/
void wifiDisconnect();

/*@brief connect with available access point
 * @param ssid - ssid of the hotspot
 * @param password - password for hotspot
 * returns 0 if connected successfully
 *         1 for failure
*/
int connectWithAp(const char *ssid, const char *password);

/*@brief scan for available networks present
  returns the no. of scanned networks
*/
int scanAvailableNetworks();

/*@brief initialize the scan for the nearby networks
 * returns  no. of scanned networks
 *         -1 for failure
*/
int initApScan();

/*@brief get the scanned networks list
 * @param - apScanList - container to fetch the list of scanned ssids
 * returns  no. of scanned networks
 *          
*/
int getApList(ApInfo_t **apScanList);

/*@brief to check whether device is connected with wifi router
 * returns true is connected else fasle
 */
bool isConnectedWithRouter();


/*@brief get the mac address of device
 *param - mac - container to fetch the 6 byte mac address of device
 */
void getWifiMacAddress(uint8_t* mac);

int detectWifiMode();

/*@brief to check if device in ap mode
 return true is in ap mode else false
 */
bool isInApMode();

#endif // _WIFI_H
