#include "wifi.h"
#include "timerFw.h"

typedef enum{
  SCAN_STATE_IDLE,
  SCAN_STATE_INPROGRESS,
  SCAN_STATE_COMPLETE,
}ScanState_e;

static ScanState_e gScanState = SCAN_STATE_IDLE;
static int gScanApCount = 0;
static ApInfo_t *gApScanList = NULL;

int apModeSetup(const char* hotspotSsid, const char* hotspotPassword)
{
   int ret = 1;
   bool result = WiFi.softAP(hotspotSsid, hotspotPassword);
   if(result == true) {
    ret = 0;
    IPAddress accessPointIpAddress = WiFi.softAPIP();
    Serial.println(accessPointIpAddress);
    return ret;
   } else {
    Serial.println("AP Failed");
    return ret;
   }
   return ret;
}

int apModeSetupOpen(const char* hotspotSsid)
{
   int ret = 1;
   bool result = WiFi.softAP(hotspotSsid);
   if(result == true) {
    ret = 0;
    IPAddress accessPointIpAddress = WiFi.softAPIP();
    Serial.println(accessPointIpAddress);
    return ret;
   } else {
    Serial.println("AP Failed");
    return ret;
   }
   return ret;
}

void apModeDisconnect()
{
  WiFi.softAPdisconnect(true);  
}

void wifiDisconnect()
{
  WiFi.disconnect(true);  
}

int connectWithAp(const char* ssid, const char* password)
{
   WiFi.begin(ssid, password);
   int timeCount = 0;
   Serial.print("Connecting");
   while (WiFi.status() != WL_CONNECTED)
   {
     if((WiFi.status() == WL_NO_SSID_AVAIL) || (WiFi.status() == WL_CONNECT_FAILED) || 30 == timeCount){//break if no ssid available, password is incorrect or not getting connected in 30sec
      return 1;
     }
     delay(1000);
     timeCount++;
     Serial.print(".");
   }
   Serial.println();
   Serial.print("Connected, IP address : ");
   Serial.println(WiFi.localIP());
   
   return 0;
}

int detectWifiMode()
{
  // 0 - not configured 
  // 1 - Wifi Mode
  // 2 - Ap Mode
  // 3 - ap+sta
 
 int valWifiMode = WiFi.getMode();
 Serial.print("ESP32 Mode : "); Serial.println(valWifiMode);
 return valWifiMode;
}

int scanAvailableNetworks()
{
    WiFi.scanDelete();  //delete last scan from memory
    return WiFi.scanNetworks();
}

int getApDetail(int apIndex, ApInfo_t *apDetail)
{
    strcpy(apDetail->ssid, WiFi.SSID(apIndex).c_str());
    return 0;
}

int initApScan()
{
  if((gScanState == SCAN_STATE_IDLE) || (gScanState == SCAN_STATE_COMPLETE)) {
    gScanState = SCAN_STATE_INPROGRESS;
    gScanApCount = scanAvailableNetworks();
    
    if (NULL == gApScanList) {
      //Malloc - first time scan
      gApScanList = (ApInfo_t*)malloc(sizeof(ApInfo_t)*gScanApCount);
    } else {
      //Realloc - rescan
      gApScanList = (ApInfo_t*)realloc(gApScanList, sizeof(ApInfo_t)*gScanApCount);
    }
    memset(gApScanList, 0, sizeof(ApInfo_t)*gScanApCount); 
    for (int i = 0; i < gScanApCount; i++) {
      if (0 != getApDetail(i, (gApScanList + i))) {
        //Error
        return -1;
      }
    }
    gScanState = SCAN_STATE_COMPLETE;
    return gScanApCount;
  }else{
    return -1;
  }
}

int getApList(ApInfo_t **apScanList)
{
  if (SCAN_STATE_COMPLETE == gScanState) {
    *apScanList = gApScanList;
    return gScanApCount;
  } else {
    //Can't provide now - either scan not initiated or in progress
    return -1;
  }
}

bool isConnectedWithRouter()
{
    if (WiFi.status() != WL_CONNECTED) {
      return false;
    } else {
      return true;
    }
}

void getWifiMacAddress(uint8_t* mac)
{
  WiFi.macAddress(mac);  
}

bool isInApMode()
{
  int ret = 0;
  ret == WiFi.getMode();
  
  if(ret == 2) {
    return true;  
  } else {
    return false;
  }
}

//void updateTime()
//{
//  
//}
