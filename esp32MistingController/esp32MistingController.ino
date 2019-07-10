#include <WiFi.h>
#include <EEPROM.h>
#include <WiFiClient.h> 
#include <WiFiUdp.h>
#include <ArduinoJson.h>

HardwareSerial Serial(0);
HardwareSerial Serial1(1);
HardwareSerial Serial2(2);


#include "timerFw.h"
#include "eventFw.h"

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#include "udpMulticast.h"
#include "tcpControlServer.h"
#include "wifi.h"
#include "hardwareControl.h"
#include "bootConfiguration.h"
#include "deviceMonitor.h"

#include "jumpStateMachine.h"

#define PLATFORM_ARDUINO

#define WIFI_PERIODIC_CHECK_TIME    60    //time in seconds to check wifi status for connected/disconnected
#define SEND_DEV_STATUS_TIME        5     //time in seconds to Send Device Status Data to Mobile and Cloud
#define SEND_DEV_MONITOR_TIME       4     // Device Monitor
#define FLUSH_BUFFER_TIME           7    //flush tcp buffer

typedef enum Events_enum {
  EVT_INIT_UDP,
  EVT_INIT_TCP,
  EVT_PAR_UDP_MSG,
  EVT_SEND_TCP_DEV_PRES,
  EVT_SEND_TCP_DEV_CONFIG,
  EVT_GET_AP_LIST,
  EVT_AP_CONNECT,
  EVT_CONTROL,
  EVT_CONFIG_UPDATE,
  EVT_AP_DISCONNECT,
  EVT_WIFI_STATUS,
  EVT_SEND_DEVICE_STATUS,
  EVT_DEVICE_MONITOR_STATUS,
  EVT_FLUSH_BUFFER,
  // CLOUD
  EVT_DEVICE_REGISTRATION,
  EVT_DEV_CONFIG_CLOUD_UPDATE,
  // Alerts
  EVT_DEV_HANDLE_ALERT,
  EVT_MAX
} Events_e;

ApInfo_t **wifiNetworkList = NULL;        //ptr holding the list of scanned networks in vicinity
const char *apSsid = "ESP32_AP";           //ap mode/hotspot name
const char *apPassword = "nodemcu123";    //ap mode password
bool gLocalNetworkConnected = 0;          //to check for whether nodemcu is connected with home wifi network
byte mac[6];
String deviceName;
char deviceName1[30];
char configData[512];

//timers id
int32_t gWifiStatusTimerId;
int32_t gSendDeviceStatusTimerId;
int32_t gDeviceMonitorTimerId;
int32_t gFlushBufferTimerId;
extern int gDataRxMutex;
//char *responseMessage = "{\"cmd\":\"prsnt\",\"cl\":{\"dN\":\"deviceName\"}}";

char *presenceDeviceNameResponse() {
    DynamicJsonBuffer cmdRes;
    JsonObject& cmdResData = cmdRes.createObject();
    cmdResData["cmd"] = "prsnt";
    
    JsonObject& commandList = cmdResData.createNestedObject("cl");
    commandList["dN"] = deviceName;

    Serial.print("Device name to send : ");
    cmdResData.prettyPrintTo(Serial);
    char res[100];
    cmdResData.printTo(res);
    return res;
}

//char* parseJson(char key, Void *arg){
//  StaticJsonBuffer<200> keyCmd;  // Pre-allocated Memory Pool to store JSONobject tree
//  char *udpCommand = (char*)arg;
//  
//  JsonObject& keyCmdData = keyCmd.parseObject(udpCommand);
//  if(!keyCmdData.success()) {
//    Serial.println("parseObject() failed");
//    return false;
//  }
//  
//  //Parse the UDP JSON string and identify the command
//  
////  char cmd[20];
////  cmd = keyCmdData[key];
////  if (cmd) 
////  {
////    Serial.print("cmd : ");
////    Serial.println(cmd);
////    return cmd;
////  }
////  else {
////    return false;
////  }
//}

int resDevPresentTcp()
{
  DynamicJsonBuffer cmdRes;
  JsonObject& cmdResData = cmdRes.createObject();
  cmdResData["cmd"] = "prsnt";
  
  JsonObject& commandList = cmdResData.createNestedObject("cl");
  // Get Device Name from EEPROM
  char *device_Name = getConfiguration("dN");
  //Serial.print("device_Name");   Serial.println(device_Name);

  commandList["dN"] = device_Name;

  //Serial.print("Device name to send : ");
  //cmdResData.prettyPrintTo(Serial);
  char res[100];
  cmdResData.printTo(res);
  
  sendIdentityResponse(res);
  return 1;
}

int resDevConfig(){
  Serial.println("\n\n Sending Device Config :: resDevConfig() ");
  // Check if config available
  if(0 == isMotorConfigPresent()) {                         // If config not available, send no config present
    char *NoConfigJsonResponse = "{\"cmd\":\"Ncnfg\"}";
    sendIdentityResponse(NoConfigJsonResponse);
  }else{                                                    // Config Available
  // If config is available Send the config
  // Get Configuration from EEPROM
  DynamicJsonBuffer cmdRes;
  //StaticJsonBuffer<400> cmdRes;
  JsonObject& cmdResData = cmdRes.createObject();
  cmdResData["cmd"] = "cnfg";
  
  JsonObject& commandList = cmdResData.createNestedObject("cl");
  // Get Device Name from EEPROM
/*  MOTOR CONFIG NAME In EEPROM
* blSpd
* smSpd
* tOn
* tOff
* blBfnT
* blAffT
*/
  Serial.println("Retreiving data from EEPROM");
  char *blSpd  = getConfiguration("blSpd");
  char *smSpd  = getConfiguration("smSpd");
  char *tOn    = getConfiguration("tOn");
  char *tOff   = getConfiguration("tOff");
  char *blBfnT = getConfiguration("blBfnT");
  char *blAffT = getConfiguration("blAffT");

  
  
//  Serial.print("blSpd");   Serial.println(blSpd);
//  Serial.print("smSpd");   Serial.println(smSpd);
//  Serial.print("tOn");   Serial.println(tOn);
//  Serial.print("tOff");   Serial.println(tOff);
//  Serial.print("blBfnT");   Serial.println(blBfnT);
//  Serial.print("blAffT");   Serial.println(blAffT);
  
  commandList["blSpd"]  = blSpd;
  commandList["smSpd"]  = smSpd;
  commandList["tOn"]    = tOn;
  commandList["tOff"]   = tOff;
  commandList["blBfnT"] = blBfnT;
  commandList["blAffT"] = blAffT;
//  if(1 == isNetworkConfigPresent()){
//    commandList["ssid"] = getConfiguration("ssid");
//  }
  // check device last state
  char *deviceState = getConfiguration("dS");
  if(deviceState != NULL){
    Serial.printf("Last Device State : %s\n", deviceState);
    commandList["dS"] = deviceState;
  }else {
    Serial.println("Device State key \"dS\" Not present in the EEPROM. Considering it as device Off State.");
    commandList["dS"] = "0";
  }

  if(1 == isDeviceIdPresent()){
    commandList["dId"] = getConfiguration("dId");
  }
//  cmdResData.prettyPrintTo(Serial);
  char res[150];
  cmdResData.printTo(res);

  // Send Data to Mobile App
  sendIdentityResponse(res);

  // Send Data to Cloud
  sendIdentityResponse_Cloud(res);
  Serial.println("\n\n Sending Device Config END :: resDevConfig() ");
  return 1;
  }
}


String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 4; i < 6; ++i) {
    result += String(mac[i], 16);
    //if (i < 5)
    //  result += ':';
  }
  return result;
}


/************************************ EVENT HANDLERS *******************************************************************************/

/** < EVENT HANDLERS*/
static Void evtInitUdp(UInt32_t evtId, Void *arg, UInt32_t len)
{
  Serial.println("Init UDP event generated");
  initMulticastReceiver();
  initMulticastSender();
}

static Void evtInitTcpServer(UInt32_t evtId, Void *arg, UInt32_t len)
{
  Serial.println("Init TCP server event generated");
  reInitTcpServer();
}
static Void evtParseUdpMessage(UInt32_t evtId, Void *arg, UInt32_t len)
{ 
  //If arg is NULL, return from here
  //Serial.println("Parse UDP message event generated");

  StaticJsonBuffer<200> udpKeyCmd;  // Pre-allocated Memory Pool to store JSONobject tree
  char *udpCommand = (char*)arg;

  JsonObject& udpKeyCmdData = udpKeyCmd.parseObject(udpCommand);
  if(!udpKeyCmdData.success()) {
    Serial.println("parseObject() failed in evtParseUdpMessage");
  }
  
  //Parse the UDP JSON string and identify the command
  const char* cmd = udpKeyCmdData["cmd"];
  Serial.print("cmd : ");  Serial.println(cmd);
  // 1. Discovery Command
  if (!(strcmp(cmd, "dscvry"))){
    Serial.println("cmd key and dscvry value found");
    resDevPresentTcp();                         // Send Presence Message (TCP/IP)   
    resDevConfig();                             //Send Config Details
  }else{
    Serial.println("No key found");
  }
//  Serial.println("Free the Pointer :: evtParseUdpMessage()");
}

static Void evtSendTcpDeviceNamePresence(UInt32_t evtId, Void *arg, UInt32_t len)
{
  Serial.println("Send Device Presence message event generated");
  
}

static Void evtSendTcpDeviceConfig(UInt32_t evtId, Void *arg, UInt32_t len)
{
  Serial.println("Send Device Config message event generated");
  
}

static Void evtGetApList(UInt32_t evtId, Void *arg, UInt32_t len)
{
  Serial.println("Get AP List Event Generated");
  
  unsigned char networkCount = 0;
  
  DynamicJsonBuffer jsonBuffer(256);
  JsonObject& root = jsonBuffer.createObject();
  root["cmdr"] = "apLst";
  
  JsonObject& cmdResp = root.createNestedObject("cl");
  
  wifiNetworkList = (ApInfo_t **)malloc(sizeof(ApInfo_t **));
  
  if(initApScan() >= 0) {
    if((networkCount = getApList(wifiNetworkList)) >= 0) {
      cmdResp["ssCnt"] = networkCount;
      
      JsonArray &listArray = jsonBuffer.createArray();
      for(int i = 0; i < networkCount; i++) {
        listArray.add((char*)(*wifiNetworkList + i)->ssid);
      }
      cmdResp["ssLst"] = listArray;
      unsigned int arraySize = root.measureLength();
      Serial.print("arraySize of aplist: ");  Serial.println(arraySize);
      char jsonMsgArray[arraySize + 1];
      root.printTo((char*)jsonMsgArray, arraySize + 1);

      //Serial.print("sendTcpResponseMessages from  evtGetApList: ");
      //Serial.println((char*)jsonMsgArray);
      sendTcpResponseMessages(jsonMsgArray, arraySize);
    }
   }
   //root.printTo(Serial);
}

static Void evtApConnect(UInt32_t evtId, Void *argument, UInt32_t len)
{
  Serial.println("Ap Connect Event Generated");
  
  StaticJsonBuffer<200> tcpKeyCmd;  // Pre-allocated Memory Pool to store JSONobject tree
  JsonObject& tcpKeyCmdData = tcpKeyCmd.parseObject((const char*)argument);

  const char* ssid = tcpKeyCmdData["cl"]["ss"];
  const char* password = tcpKeyCmdData["cl"]["pwrd"];
  
  //send the command response after successfully parsed
  StaticJsonBuffer<100> tcpResp;
  JsonObject& root = tcpResp.createObject();
  root["cmdr"] = "apCnt";
  JsonObject& cmdResp = root.createNestedObject("cl");
  cmdResp["res"] = "ack";
  unsigned char arraySize = root.measureLength();
  char jsonMsgArray[arraySize + 1];
  root.printTo((char*)jsonMsgArray, arraySize + 1);
  sendTcpResponseMessages(jsonMsgArray, arraySize);
//  tcpResp.clear();  //clear json buffer
  delay(500);
  apModeDisconnect(); //disconnect with ap and try to connect with the router
  
  Serial.printf("Connecting with ssid: %s , password: %s\n", ssid, password);
  if(1 == connectWithAp(ssid, password)) {  //if connect fail, switch again back to ap mode
    //apModeSetup(apSsid, apPassword);
    apModeSetupOpen(apSsid);
    eventGenerate(EVT_INIT_UDP, NULL, 0);
    eventGenerate(EVT_INIT_TCP, NULL, 0);
  } else {  //if connected, start the multicast receiver 
     eventGenerate(EVT_INIT_UDP, NULL, 0);
     eventGenerate(EVT_INIT_TCP, NULL, 0);
     gLocalNetworkConnected = 1;  //raise flag ,connected with network successfully
     setConfiguration("ssid", (char*)ssid); //set ssid &
     setConfiguration("password", (char*)password); //password as network configurations
     setNetworkConfigPresence();
     eventGenerate(EVT_DEVICE_REGISTRATION, NULL, 0);  // Device Registration 
  }
}

static Void evtControl(UInt32_t evtId, Void *arg, UInt32_t len)
{
  Serial.println("AP control Event Generated");
  
}

//forgot configurations and disconnect with the local network
static Void evtApDisconnect(UInt32_t evtId, Void *arg, UInt32_t len)
{
  Serial.println("Ap Disconnect Event Generated");
   
  //send the command response
  StaticJsonBuffer<100> tcpResp;
  JsonObject& root = tcpResp.createObject();
  root["cmdr"] = "apDscnt";
  JsonObject& cmdResp = root.createNestedObject("cl");
  cmdResp["res"] = "ack";
  unsigned char arraySize = root.measureLength();
  char jsonMsgArray[arraySize + 1];
  root.printTo((char*)jsonMsgArray, arraySize + 1);
  sendTcpResponseMessages(jsonMsgArray, arraySize);
  //tcpResp.clear();  //clear json buffer
  delay(500);
  //delete eeprom data
  //delConfig("ssid");
  //delConfig("password");
  
  //clear network config boot param byte
  clearNetworkConfigPresence();

  //reset flag to tell the current status of the application
  gLocalNetworkConnected = 0;

  //disconnect from local network router
  wifiDisconnect();
  
  //setup ap mode
  //apModeSetup(apSsid, apPassword);
  apModeSetupOpen(getConfiguration("dN"));

  //generate event to start tcp/udp servers
  eventGenerate(EVT_INIT_UDP, NULL, 0);
  eventGenerate(EVT_INIT_TCP, NULL, 0);
}

static Void evtConfigUpdate(UInt32_t evtId, Void *arg, UInt32_t len)
{
  Serial.println("Config Update Event Generated");
  
}

static Void evtWifiStatus(UInt32_t evtId, Void *arg, UInt32_t len)
{
  Serial.println("1****. EVENT -~~~~~~~~~~~~~~~~~~~~~~~Wifi Status Event Generated~~~~~~~~~~~~~~~~~~~~~~****");
  int ret= -1;
  ret = detectWifiMode();
  if(!isConnectedWithRouter())
  {
    if(ret == 2) {  //in ap mode
      return;
    }
    wifiDisconnect();
    ret = detectWifiMode();
    
    if(ret == 2) {  //if wifi mode 2 is detected
      ret = 0;    
    }
    
    if(ret == 0) {  //not in any mode , send it to ap mode
      char *device_Name = getConfiguration("dN");
      Serial.printf("*************************\n*     device_Name     *\n*************************\n%s\n*************************\n", device_Name);
      apModeSetupOpen((const char*)device_Name);

      //generate event to start tcp/udp servers
      eventGenerate(EVT_INIT_UDP, NULL, 0);
      eventGenerate(EVT_INIT_TCP, NULL, 0);
    }else {
        Serial.println("Device Already in AP mode");
    }
  }
  Serial.println("1. EVENT Stop");
}
//char gDevStatJSON[] = "{\"cmd\":\"stat\",\"cl\":{\"blSpd\":\"0\",\"blTmp\":\"0\",\"smSpd\":\"0\",\"dTmp\":\"0\",\"dHum\":\"0\"}}";
//char gDevStatJSON[] = "{\"cmd\":\"stat\",\"cl\":{\"blSpd\":0,\"blTmp\":0,\"smSpd\":0,\"dTmp\":0,\"dHum\":0}}";

static Void evtSendDeviceStatus(UInt32_t evtId, Void *arg, UInt32_t len)
{
  Serial.println("2****. EVENT - Send Device Data Status Event Generated*****2");

  // Check for Misting ON or OFF

  //if (ON) Start
    // Retrieve Device Data from Structure
    extern struct deviceMonitorData *devMonData;
    extern unsigned int gDeviceAlertBitField;
//    int bldcRpm = devMonData->bldcRpm;
//    Serial.printf("2. EVENT :: BLDC Motor RPM : %d\n", bldcRpm);

    //Create JSON   // {"cmd":"stat","cl":{"blSpd":2000,"blTmp":26,"smSpd":200,"dTmp":"25","dHum":"50"}}
    
    DynamicJsonBuffer jsonBuffer;
    
    //StaticJsonBuffer<250> jsonBuffer;
    char gDevStatJSON[100] = "{\"cmd\":\"stat\",\"cl\":{\"blSpd\":0,\"blTmp\":0,\"smSpd\":0,\"dTmp\":0,\"dHum\":0, \"dWFR\":0}}";
    JsonObject& devStatJsonData = jsonBuffer.parseObject(gDevStatJSON);
    if(!devStatJsonData.success()) {
      Serial.println("parseObject() failed");
    }
    
    /*
     * blSpd
     * smSpd
     * blTmp
     * dTmp
     * dHum
     * dWFR
     * 
     */
    //devStatJsonData.prettyPrintTo(Serial);
    if(devMonData == NULL){
      Serial.println("5. no data available in the Device monitor structure. So Exiting..!!\n");
      Serial.println("2****. EVENT END - Send Device Data Status****2");
      return;
    }
    devStatJsonData["cl"]["smSpd"] = devMonData->smRpm;
    
    //Serial.printf("Got SMRPM\n");
    devStatJsonData["cl"]["blSpd"] = devMonData->bldcRpm;
    devStatJsonData["cl"]["blTmp"] = devMonData->bldcTemperature;
    //Serial.printf("Got BLDC data\n");
    devStatJsonData["cl"]["dTmp"] = devMonData->deviceTemperature;
    devStatJsonData["cl"]["dHum"] = devMonData->deviceHumidity;
    //Serial.printf("Got DEV data\n");
    devStatJsonData["cl"]["dWFR"] = devMonData->waterFlowRate;
    //Serial.printf("Got FLOW rate\n");
    devStatJsonData["cl"]["dS"] = checkDeviceLastState();
    
    if(1 == isDeviceIdPresent()){
      devStatJsonData["cl"]["dId"] = getConfiguration("dId");
    }
    
    JsonArray &listArray = jsonBuffer.createArray();
    Serial.printf("Alert value : %d\n", gDeviceAlertBitField);
    // Check for Alerts
    if(0 == gDeviceAlertBitField){
      Serial.println("No alert Found. Sending device status : 200");
      listArray.add(gNoAlertJsonArr);
    }else{
      Serial.printf("Alert Found. Alert Value : %d\n", gDeviceAlertBitField);
      
      // Check the alert  // 1 - Water Leak, 3,7,15,31,63  // if(NUM & (1<<N))
      if( gDeviceAlertBitField & (1<<ALERT_EVT_WATER_LEAK))   // Water Leak
      {
        Serial.println("Adding Water Leak in Array");
        //listArray.add("500");
        listArray.add(gWaterLeakAlertJsonArr);
        
      }
      if( gDeviceAlertBitField & (1<<ALERT_EVT_HIGH_CURRENT))   // High Current
      {
        Serial.println("Adding High Current in Array");
        listArray.add(gHighCurrentAlertJsonArr);
      }
          
      if( gDeviceAlertBitField & (1<<ALERT_EVT_HIGH_DEV_TEMP))   // High Device Temperature
      {
        Serial.println("Adding Device High Temperature in Array");
        listArray.add(gHighDevTempAlertJsonArr);
      }

      if( gDeviceAlertBitField & (1<<ALERT_EVT_HIGH_DEV_HUM))   // High Device Humidity
      {
        Serial.println("Adding High Device Humidity in Array");
        listArray.add(gHighDevHumAlertJsonArr);
      }
    }
    
    listArray.prettyPrintTo(Serial);
    devStatJsonData["cl"]["alrt"] = listArray;
    devStatJsonData.prettyPrintTo(Serial);

    char res[150];
    devStatJsonData.printTo(res);
    
    // UDP Multicast
    transmitMulticastMessage(res, strlen(res));
    // if (Connected to Wifi) start
       
      // Send data to Cloud
    // if (Connected to Wifi) End
  if(WiFi.status() == WL_CONNECTED)
  {
    // Send Data to Cloud
    sendIdentityResponse_Cloud(res);
  }
  //if (On) End
  Serial.println("2. EVENT END - Send Device Data Status");

}

static Void evtDeviceMonitorStatus(UInt32_t evtId, Void *arg, UInt32_t len)
{
  Serial.println("3. EVENT start- Device Monitor Event Generated");
  getDeviceMonitorData();
  Serial.println("3. EVENT END - Device Monitor Event");
}

static Void evtDeviceRegistrationToCloud(UInt32_t evtId, Void *arg, UInt32_t len)
{
  Serial.println("\n4. EVENT START - Device Registration Event Generated");
  // {"cmd":"devReg","cl":{"mac":"BH:hj:hj:jk","ssid":"WifiRouter","name":"MistController804d"}}
  // if Device is Connected to WIFI
  if(WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Connected to Wifi. Sending Device Registration");
    sendDeviceRegistration_Cloud();
  }else{
    Serial.println("Device Not Connected to Wifi. Not Sending Device Registration");
  }
  Serial.println("\n4. EVENT END - Device Registration Event");
}

static Void evtSendDeviceConfigUpdateToCloud(UInt32_t evtId, Void *arg, UInt32_t len)
{
  Serial.println("\n\n5. EVENT - Send Device Config update Data to Cloud");
  // Doing this thing with the Mobile part only
}

//Alerts


static Void evtHandleAlert(UInt32_t evtId, Void *arg, UInt32_t len)
{
  Serial.println("\n\n6. EVENT :: ALERT :: Some Alert in the device");

  // Stop the Device
  Serial.println("\n\n*** 5. STARTS ***");
  executeStateMachine(EVENT_END_MIST);
  Serial.println("*** 5. ENDS ***\n\n");
  
  // Set flag to Alert high
  // Already Set

  
  // set device State to 0 in EEPROM
  setConfiguration("dS", "0");

  // Set Led Indicator
  AlertledOn();
  
}

static Void evtFlushBuffer(UInt32_t evtId, Void *arg, UInt32_t len)
{
  Serial.println("\n\n6. EVENT :: FLUSH BUFFER");
  //reInitMessenger();
//  bool ret;
//  ret = isInApMode();
  Serial.printf("******************************DEVICE NETWORK STATS**********************************\r\n");
  Serial.println(detectWifiMode());
  if(detectWifiMode() == 2) {
    Serial.printf("Device is in Ap mode with ip 192.168.4.1\r\n");
  } else {
    if(isConnectedWithRouter()) {
      Serial.printf("Device is connected to router\r\n");
      Serial.println(WiFi.localIP());
    }
  }
  Serial.printf("EEPROM CONFIG:\n");
  char readConfigArray[512];
  readConfigurations((char*)readConfigArray);
  Serial.printf("************************************************************************************\r\n");
}

/************************************ EVENT HANDLERS END *******************************************************************************/

int sendDeviceRegistration_Cloud()
{
  Serial.println("\n\n Sending Device Registration Command to Cloud ");

  DynamicJsonBuffer devReg;
  JsonObject& devRegData = devReg.createObject();
  devRegData["cmd"] = "devRg";
  
  JsonObject& commandList = devRegData.createNestedObject("cl");

  // Device Name
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

  commandList["mac"]  = WiFi.macAddress();
  commandList["dN"]  = getConfiguration("dN");
  commandList["ss"]  = getConfiguration("ssid");
  
  devRegData.prettyPrintTo(Serial);
  char res[150];
  devRegData.printTo(res);

  // Send Data to Cloud
  sendIdentityResponse_Cloud(res);
  return 1;
}

/************************************ TIMER HANDLERS **********************************************************************************/

static Void onWifiStatusTimerExpiry(Int32_t timerId, Void *arg)  // if wifi status timer expired 
{
    Serial.println("1. TIMER - wifi status timer expired");
    eventGenerate(EVT_WIFI_STATUS, NULL, 0);  // evtWifiStatus()
}

static Void onSendDeviceStatusTimerExpiry(Int32_t timerId, Void *arg)  // if Send Device Status Timer Expired
{
    Serial.println("2. TIMER - Send Device Status Timer Expired");
    eventGenerate(EVT_SEND_DEVICE_STATUS, NULL, 0);  // evtSendDeviceStatus()
    
}

static Void onDeviceMonitorTimerExpiry(Int32_t timerId, Void *arg)  // if Device Monitor Timer Expired
{
    Serial.println("3. TIMER - Device Monitor Timer Expired");
    eventGenerate(EVT_DEVICE_MONITOR_STATUS, NULL, 0);  // evtDeviceMonitorStatus()
    
}

static Void onFlushBufferTimerExpiry(Int32_t timerId, Void *arg)  //if flush buffer expired
{
  eventGenerate(EVT_FLUSH_BUFFER, NULL, 0);
}

/**************************** TIMER HANDLERS END****************************************************************************************/

static void registerEventHandlers()
{
    
  /***********************************************************
  Misting Controller functionality
  ***********************************************************/
  /* Create Events
   * 1. inform presence - informPresenceTcp
   * 2. Configuration Update - configUpdateUdp
   * 3. UDP Multicast Listener - evtInitUdp, EVT_INIT_UDP
   * 4. TCP/IP Server - evtInitTcpServer, EVT_INIT_TCP
  */
  eventAddHandler(EVT_INIT_UDP, evtInitUdp); // Create UDP listener
  eventAddHandler(EVT_INIT_TCP, evtInitTcpServer);        // Create TCP server
  eventAddHandler(EVT_PAR_UDP_MSG, evtParseUdpMessage); // Parse the incoming UDP message
  eventAddHandler(EVT_SEND_TCP_DEV_PRES, evtSendTcpDeviceNamePresence); // Send Presence to Mobile Device
  eventAddHandler(EVT_SEND_TCP_DEV_CONFIG, evtSendTcpDeviceConfig); // Send device config to Mobile Device  UDP
  eventAddHandler(EVT_GET_AP_LIST, evtGetApList); // event to fetch the visible networks list
  eventAddHandler(EVT_AP_CONNECT, evtApConnect); // event to connect with a network
  eventAddHandler(EVT_AP_DISCONNECT, evtApDisconnect); // event to disconnect with a network
  eventAddHandler(EVT_CONTROL, evtControl); // event to handle control commands
  eventAddHandler(EVT_CONFIG_UPDATE, evtConfigUpdate); // event update the device configuration
  eventAddHandler(EVT_WIFI_STATUS, evtWifiStatus); // event generated to get wifi status
  eventAddHandler(EVT_SEND_DEVICE_STATUS, evtSendDeviceStatus); // event generated to Send Device Status to Mobile and Cloud
  eventAddHandler(EVT_DEVICE_MONITOR_STATUS, evtDeviceMonitorStatus); // event generated to Monitor Device
  eventAddHandler(EVT_FLUSH_BUFFER, evtFlushBuffer); // event generated to flush the tcp msg buffer

  // Cloud Events
  eventAddHandler(EVT_DEVICE_REGISTRATION, evtDeviceRegistrationToCloud); // event generated to Monitor Device
  eventAddHandler(EVT_DEV_CONFIG_CLOUD_UPDATE, evtSendDeviceConfigUpdateToCloud); // event generated to Monitor Device

  // Alerts handling
  eventAddHandler(EVT_DEV_HANDLE_ALERT, evtHandleAlert); // event generated to Handle Water leak
}

static void createApplicationTimers()
{
  // 1. Wifi Connection Timer
  gWifiStatusTimerId = timerCreate(TIMER_LOW_PRIO, WIFI_PERIODIC_CHECK_TIME * 1000, True, onWifiStatusTimerExpiry, NULL);
  if (gWifiStatusTimerId < 0) {
    Serial.println("Error creating wifi status timer\r\n");
  }   
  if (E_SUCCESS != timer_c_Start(gWifiStatusTimerId)) {
    Serial.println("Error starting wifi status timer\r\n");
  }

  // 2. Start Timer to Send Motor/Device Status Data to Mobile and Cloud
  gSendDeviceStatusTimerId = timerCreate(TIMER_LOW_PRIO, SEND_DEV_STATUS_TIME * 1000, True, onSendDeviceStatusTimerExpiry, NULL);
  if (gSendDeviceStatusTimerId < 0) {
    Serial.println("Error creating wifi status timer\r\n");
  }   
  if (E_SUCCESS != timer_c_Start(gSendDeviceStatusTimerId)) {
    Serial.println("Error starting wifi status timer\r\n");
  }

  // 3. Start Timer for Device Monitoring
  gDeviceMonitorTimerId = timerCreate(TIMER_LOW_PRIO, SEND_DEV_MONITOR_TIME * 1000, True, onDeviceMonitorTimerExpiry, NULL);
  if (gDeviceMonitorTimerId < 0) {
    Serial.println("Error creating Device Monitor timer\r\n");
  }   
  if (E_SUCCESS != timer_c_Start(gDeviceMonitorTimerId)) {
    Serial.println("Error starting Device Monitor timer\r\n");
  }

   // 2. Start Timer to flush tcp msg buffer
  gFlushBufferTimerId = timerCreate(TIMER_LOW_PRIO, FLUSH_BUFFER_TIME * 1000, True, onFlushBufferTimerExpiry, NULL);
  if (gFlushBufferTimerId < 0) {
    Serial.println("Error creating flush buffer timer\r\n");
  }   
  if (E_SUCCESS != timer_c_Start(gFlushBufferTimerId)) {
    Serial.println("Error starting flush buffer timer\r\n");
  }
}

static void initNetworkConnection()
{
  char *config_ssid = NULL;
  char *config_password = NULL;
  if(0 == isNetworkConfigPresent()) {                       //if network configuration is not saved go into ap mode
    //apModeSetup(apSsid, apPassword);
    
    //apModeSetupOpen(apSsid);
    
    char *device_Name = getConfiguration("dN");
    Serial.printf("No Config for Wifi found. Going in AP mode - %s\n", device_Name);
    //Serial.print("device_Name");   Serial.println(device_Name);
    apModeSetupOpen((const char*)device_Name);
    
    eventGenerate(EVT_INIT_UDP, NULL, 0);
    eventGenerate(EVT_INIT_TCP, NULL, 0);
    Serial.println("UDP Rx and TCP server started");
  } else {  //try connecting with the saved configuration
    config_ssid = getConfiguration("ssid");
    config_password = getConfiguration("password");
    
    Serial.printf("ssid : %s\n", config_ssid);
    Serial.printf("Password : %s\n", config_password);
                                          
    if(1 == connectWithAp(config_ssid, config_password)) {  //if connect fail, switch again back to ap mode
      delConfig("ssid");
      delConfig("password");
      clearNetworkConfigPresence();                         //clr nw config boot param
      //apModeSetup(apSsid, apPassword);
      apModeSetupOpen(apSsid);
      eventGenerate(EVT_INIT_UDP, NULL, 0);
      eventGenerate(EVT_INIT_TCP, NULL, 0);    
    } else {                                                //if connected
       eventGenerate(EVT_INIT_UDP, NULL, 0);
       eventGenerate(EVT_INIT_TCP, NULL, 0);
       gLocalNetworkConnected = 1;                          //raise flag ,connected with network successfully
       detectWifiMode();
       eventGenerate(EVT_DEVICE_REGISTRATION, NULL, 0);  // Device Registration with Cloud  // evtDeviceRegistrationToCloud
    }
  }
}

int saveDeviceName(){
  WiFi.mode(WIFI_MODE_STA);
  
  // Device Name
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

  deviceName += "MistController";
  uint8_t mac[6];
  WiFi.macAddress(mac);
  deviceName += macToStr(mac);
  Serial.print("Device Name : ");
  Serial.println(deviceName);

  return 1;
}

int checkDeviceLastState()
{
  // check device last state
  char *deviceState = getConfiguration("dS");
  if(deviceState != NULL){
  Serial.printf("Last Device State : %s\n", deviceState);
  if (0 == atoi(deviceState))
  {
    //Serial.println("Device Last State is Off");
    return 0;
  }
  else{
    //Serial.println("Device last State is On");
    return 1;
  }
  }else {
    Serial.println("Device State key \"dS\" Not present in the EEPROM. Considering it as device Off State.");
    return 0;
  }

  return -1;
}

void setDeviceNameEEPROM()
{
  // Device Name
  if(0 == getConfiguration("dN"))
  {
    saveDeviceName();
    // save the Name of Device in EEPROM
    int str_len = deviceName.length() + 1; 
    char devName[str_len];
    deviceName.toCharArray(devName, str_len);
    setConfiguration("dN", devName); //set DeviceName
    char *device_Name = getConfiguration("dN");
    //Serial.print("Fresh name given to Device \n\nNew Device_Name : ");   Serial.println(device_Name);
    Serial.printf("*************************\n*    New device Name   *\n*************************\n%s\n*************************\n", device_Name);

  }else{
  //initialize eeprom config module
    char *device_Name = getConfiguration("dN");
    //Serial.print("Name already Present in the EEPROM \n\nOld Device_Name : ");   Serial.println(device_Name);
    Serial.printf("*************************\n*    Old device Name    *\n*************************\n%s\n*************************\n", device_Name);
  }
}
void setup() {
  Serial.begin(115200);
  gDataRxMutex = 1;
  //initialize and create timers
  timerFwInit(100, 15); //100 ms tick timer
  createApplicationTimers();

  //initialize and register events
  eventFwInit(20, 10);
  registerEventHandlers();
  
  initHardware();
  Serial.println("\n\n*******************************system start********************************************\n\n");
  
  // flush all eeprom configurations + boot params
  //resetConfigurations();
  //flushBootConfig();
  
  initializeSetUp();   // to initialize the EEPROM with all config data

  setDeviceNameEEPROM();
  
  char readConfigArray[512];
  readConfigurations((char*)readConfigArray);
  // Create Servers

  detectWifiMode();
  Serial.println("Calling initNetworkConnection");
  initNetworkConnection();
  
  // Initiate State machine
  Serial.println("Initialize state Machine");
  initJumpStateMachine();

  if (0 == isMotorConfigPresent()){
    Serial.println("\n No Motor Configuration Present in EEPROM. Device Cannot Start");
  }else{
    Serial.println("\n Motor Configuration Present in EEPROM");
    if(1 == checkDeviceLastState())
    {
      Serial.println("Last State of Device - ON. Starting StateMachine");
      Serial.println("*** 1. Starts ***");
      executeStateMachine(EVENT_INIT_BLDC);
      Serial.println("*** 1. Ends ***");
    }else
    {
      Serial.println("Last State of Device - OFF. StateMachine in STATE_MIST_END state");
      executeStateMachine(EVENT_END_MIST);
    }
  }

  /********* Cloud Device Registration ****************/
  eventGenerate(EVT_DEVICE_REGISTRATION, NULL, 0);  // Device Registration // evtDeviceRegistrationToCloud
}

void loop() {
  
  timerFwProcess();
  eventFwProcess();

  if(gDataRxMutex == 1) {
    // handle UDP message
    receiveMulticastMessage();
  }
  
  // handle control messages from tcp client
  handleTcpControlMessages();

  // Handle Incoming Reply from Cloud
  handleTcpControlMessages_CLoud();

  // Receive data from bldc STEVAL
  bldcRxMotorStatusData();
}
