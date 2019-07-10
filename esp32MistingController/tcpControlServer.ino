#include "tcpControlServer.h"
#include "hardwareControl.h"
#include "eventFw.h"

#define CONTROL_MESSAGE_BUFF_LEN 256 
static unsigned int tcpServerPort = 6677;  //listening port for tcp server(for handling control messages)

WiFiClient tcpClient_control;  //to read messages from tcp client meant for hardware control 
WiFiServer tcpServer(tcpServerPort);  //tcp server listening for control messages

//static unsigned int tcpServerPort_Cloud = 1337;  //listening port for tcp server CLOUD
//WiFiServer tcpServer_Cloud(tcpServerPort_Cloud);  //tcp server listening for control messages
//WiFiClient tcpClient_control_Cloud;  //to read messages from tcp client meant for hardware control

char tcpClientControlMessageBuffer[CONTROL_MESSAGE_BUFF_LEN] = {0}; //buffer holding the control messages
char tcpClientControlMessageBuffer_Cloud[CONTROL_MESSAGE_BUFF_LEN] = {0}; //buffer holding the control messages
unsigned char messageWritePtr = 0;  //pointer indicating the data upto which control message buffer is filled
unsigned char messageWritePtr_Cloud = 0;  //pointer indicating the data upto which control message buffer is filled
bool messageReady = 0;  //flag to indicate if control message received fully
bool messageReady_Cloud = 0;  //flag to indicate if control message received fully
char previousDataByte = 0;
int gDataRxMutex = 1;
int gManualModeOnFlag = 0;

char *gCommandParamSsid = NULL; //ptr holding ssid parameter
char *gCommandParamPassword = NULL; //ptr holding password parameter//enum to indicate command get, which needs to be processed
typedef enum {
  CMDSTATUS_NA = 0,
  CMDSTATUS_GET_AP_LIST,            // App request for the list of Routers in vicinity of Device
  CMDSTATUS_AP_CONNECT,             // App provides SSID and Password to Device to make connection with WIfi Router
  CMDSTATUS_AP_DISCONNECT,          // App command to Disconnect from Wifi
  CMDSTATUS_CONTROL,
  CMDSTATUS_CONFIG_UPDATE,          // App update the Motor Config
  CMDSTATUS_REQ_WIFI_CONFIG,        // App requesting for SSID and Password
  CMDSTATUS_DEV_STATE,              // App sending command to make Device ON/OFF - start misting or stop misting
  CMDSTATUS_DEV_NAME_UPDATE,        // App change Device Name
  CMDSTATUS_DEV_CHECK_HV_MODULE,    // App will check the HV module
  CMDSTATUS_MAX  
} commandStatus_e;
commandStatus_e gCommandStatus = CMDSTATUS_NA;

typedef enum {
  CLOUD_CMDSTATUS_NA = 0,
  CLOUD_CMDSTATUS_DEVICE_REGISTER,
  CLOUD_CMDSTATUS_MAX  
} cloudCommandStatus_e;
cloudCommandStatus_e gCloudCommandStatus = CLOUD_CMDSTATUS_NA;

void initTcpServer()
{
  tcpServer.begin();  //start tcp server
}

void reInitTcpServer()
{
  tcpServer.close();
  tcpServer.begin();  //start tcp server
}

void sendTcpResponseMessages(char *msg, uint32_t len)
{
  Serial.print("sendTcpResponseMessages() : ");
  Serial.println((char*)msg);
  tcpClient_control.write((char*)msg, len);
  
}

static int storeMessage(char dataByte)
{
  //flush buffer
  Serial.printf("1. messageWritePtr address : %d", messageWritePtr);
  if(messageWritePtr > (CONTROL_MESSAGE_BUFF_LEN - 1)) {//if buffer overflows, flush it
    Serial.printf("2. messageWritePtr address : %d", messageWritePtr);
    messageWritePtr = 0;
    memset(tcpClientControlMessageBuffer, '\0', sizeof(tcpClientControlMessageBuffer));
  }

  if(dataByte == '\n' || dataByte == '\0' || dataByte == '\r') { //message string complete
    tcpClientControlMessageBuffer[messageWritePtr] = '\0';
    //Serial.println("\nMessage is Ready\n");
    Serial.printf("\nbuffer content: %s\n", tcpClientControlMessageBuffer);
    messageWritePtr = 0;
    messageReady = 1;
    return 0;
  }
  
  //store messages
  if(dataByte == '{' && messageWritePtr < 5)
  {
    messageWritePtr = 0;
  }
  tcpClientControlMessageBuffer[messageWritePtr++] = dataByte;
 
  //messageWritePtr++;
  if (dataByte == '\0') {
    Serial.println("\\0");
  } else if (dataByte == '\r') {
    Serial.println("\\r");
  } else if (dataByte == '\n') {
    Serial.println("\\n");
  } else {
    //Serial.print("storeMessage else condition : ");
    //Serial.print(dataByte);
  }
  
  return 1;
}
static int commandDecoder()
{
  Serial.printf("tcpControlServer.ino :: commandDecoder :: Buffer Content : %s\n\n", tcpClientControlMessageBuffer);
  DynamicJsonBuffer tcpKeyCmd;
  JsonObject& tcpKeyCmdData = tcpKeyCmd.parseObject((const char*)tcpClientControlMessageBuffer);
  if(!tcpKeyCmdData.success()) {
    Serial.println("parseObject() failed in Command Decoder");
  }

  //Parse the UDP JSON string and identify the command
  const char* cmd = tcpKeyCmdData["cmd"];
  Serial.print("cmd : ");  Serial.println(cmd);
  
  if(NULL != cmd) {
    if(0 == strcmp(cmd, "cnfgUpdt")) {
      gCommandStatus = CMDSTATUS_CONFIG_UPDATE;
       
    } else if(0 == strcmp(cmd, "apLst")) {
      gCommandStatus = CMDSTATUS_GET_AP_LIST; 
      
    } else if(0 == strcmp(cmd, "apCnt")) {
      gCommandStatus = CMDSTATUS_AP_CONNECT; 
      
    } else if(0 == strcmp(cmd, "apDscnt")) {
      gCommandStatus = CMDSTATUS_AP_DISCONNECT; 
      
    } else if(0 == strcmp(cmd, "control")) {
      gCommandStatus = CMDSTATUS_CONTROL; 
      
    } else if(0 == strcmp(cmd, "rqtWfCnfg")) {
      // Mobile App requesting the device SSID and Password with which it is connected
      
      gCommandStatus = CMDSTATUS_REQ_WIFI_CONFIG;
      
    } else if(0 == strcmp(cmd, "dSt")) {
      // App sending command to make Device ON/OFF - start misting or stop misting
      
      gCommandStatus = CMDSTATUS_DEV_STATE;
      
    }else if(0 == strcmp(cmd, "nmChng")) {
      // Mobile App change the Name of the Device
      
      gCommandStatus = CMDSTATUS_DEV_NAME_UPDATE;
      
    }else if(0 == strcmp(cmd, "cHv")) {       //  {"cmd":"cHv"}
      // Mobile App change the Name of the Device
      
      gCommandStatus = CMDSTATUS_DEV_CHECK_HV_MODULE;
      
    } else {
      gCommandStatus = CMDSTATUS_NA;
      Serial.println("Unknown command");
      
    }
  } else {
    gCommandStatus = CMDSTATUS_NA;
  } 
  return 0;    
}
static int storeMessage_Cloud(char dataByte)
{
  //flush buffer
  if(messageWritePtr_Cloud > CONTROL_MESSAGE_BUFF_LEN - 1) {//if buffer overflows, flush it
    messageWritePtr_Cloud = 0;
    memset(tcpClientControlMessageBuffer_Cloud, '\0', sizeof(tcpClientControlMessageBuffer_Cloud));
  }
  
  if(dataByte == '\n' || dataByte == '\0' || dataByte == '\r') { //message string complete
    tcpClientControlMessageBuffer_Cloud[messageWritePtr_Cloud] = '\0';
    //Serial.println("\nMessage is Ready\n");
    Serial.printf("\nbuffer content: %s\n", tcpClientControlMessageBuffer_Cloud);
    messageReady_Cloud = 1;
    return 0;
  }
  
  //store messages
  tcpClientControlMessageBuffer_Cloud[messageWritePtr_Cloud] = dataByte;
  messageWritePtr_Cloud++;
  if (dataByte == '\0') {
    Serial.println("\\0");
  } else if (dataByte == '\r') {
    Serial.println("\\r");
  } else if (dataByte == '\n') {
    Serial.println("\\n");
  } else {
    //Serial.print("storeMessage else condition : ");
    Serial.print(dataByte);
  }
  
  return 1;
}

static int commandDecoder_Cloud()
{
  Serial.printf("tcpControlServer.ino :: commandDecoder_Cloud :: Buffer Content : %s\n\n", tcpClientControlMessageBuffer_Cloud);
  DynamicJsonBuffer tcpKeyCmd;
  JsonObject& tcpKeyCmdData = tcpKeyCmd.parseObject((const char*)tcpClientControlMessageBuffer_Cloud);
  if(!tcpKeyCmdData.success()) {
    Serial.println("parseObject() failed in Command Decoder");
  }

  //Parse the UDP JSON string and identify the command
  const char* cmdr = tcpKeyCmdData["cmdr"];
  Serial.print("cmdr : ");  Serial.println(cmdr);
  
  if(NULL != cmdr) {
    if(0 == strcmp(cmdr, "devRg")) {
      gCloudCommandStatus = CLOUD_CMDSTATUS_DEVICE_REGISTER;
       
    } else {
      gCloudCommandStatus = CLOUD_CMDSTATUS_NA;
    } 
  return 0;    
  }
}

static void reInitMessenger_Cloud()
{
  messageReady_Cloud = 0;
  messageWritePtr_Cloud = 0;
  memset(tcpClientControlMessageBuffer_Cloud, '\0', sizeof(tcpClientControlMessageBuffer_Cloud));
  //Serial.println("message reinitialized for Cloud\n");
}

static int processCommand_Cloud()
{

  switch(gCloudCommandStatus)
  {
    case CLOUD_CMDSTATUS_DEVICE_REGISTER: 
                                      Serial.println("Device Registration Command Received");
                                      storeDeviceId();
                                      break;
                                            

    default:
                                      break;              
  }
  Serial.println("command processed");
  return 0;
}

int storeDeviceId(){
  DynamicJsonBuffer tcpKeyCmdrCl;
  Serial.print("tcpClientControlMessageBuffer_Cloud : ");     Serial.println(tcpClientControlMessageBuffer_Cloud);
  
  JsonObject& tcpKeyCmdrClData = tcpKeyCmdrCl.parseObject((const char*)tcpClientControlMessageBuffer_Cloud);
  if(!tcpKeyCmdrClData.success()) {
    Serial.println("parseObject() failed in storeDeviceId");
    return -1;
  }
  
  const char* cmd = tcpKeyCmdrClData["cmdr"];
  Serial.print("cmd    : ");  Serial.println(cmd);
  
  const char* dId = tcpKeyCmdrClData["cl"]["dId"];          // New Device Name from App
  Serial.printf("New device ID : %s\n", dId);

  if(0 == isDeviceIdPresent()){
    char *oldDid = getConfiguration("dId");
    if(oldDid != NULL){
      
      
    if(0 == strcmp(oldDid, dId))
    {
      Serial.println("Same Device ID already Registered");
      return 1;
    }
    }
  }

  Serial.println("New DID adding to EEPROM");
  // Save new Device Name in EEPROM
  setConfiguration("dId", (char *)dId); //set DeviceName
  setDeviceIdPresence();
  return 1;
}

static void reInitMessenger()
{
  //if(gDataRxMutex != 1) {
    messageReady = 0;
    messageWritePtr = 0;
    memset(tcpClientControlMessageBuffer, '\0', sizeof(tcpClientControlMessageBuffer));
    Serial.println("message reinitialized for Mobile APP\n");
  //}
}

static int processCommand()
{
  unsigned char networkCount = 0;
  
  switch(gCommandStatus)
  {
    case CMDSTATUS_GET_AP_LIST: 
                                      eventGenerate(EVT_GET_AP_LIST, NULL, 0); 
                                      break;
                                            
    case CMDSTATUS_AP_CONNECT:
                                      eventGenerate(EVT_AP_CONNECT, (Void*)tcpClientControlMessageBuffer, strlen(tcpClientControlMessageBuffer));
                                      eventGenerate(EVT_DEVICE_REGISTRATION, NULL, 0);  // Device Registration 
                                      break;
    case CMDSTATUS_CONTROL:
                                      eventGenerate(EVT_CONTROL, (Void*)tcpClientControlMessageBuffer, strlen(tcpClientControlMessageBuffer));       
                                      Serial.print("case : control");
                                      break;
                                      
    case CMDSTATUS_CONFIG_UPDATE:
                                      Serial.println("case: config update. App sends data to Device to change speed and other parameters.");
                                      handleConfigUpdateJson();
                                      eventGenerate(EVT_DEV_CONFIG_CLOUD_UPDATE, NULL, 0);  // callback 
                                      break;
    
    case CMDSTATUS_AP_DISCONNECT:
                                      eventGenerate(EVT_AP_DISCONNECT, NULL, 0);  // callback 
                                      Serial.println("case: ap disconnect");
                                      break; 
    case CMDSTATUS_REQ_WIFI_CONFIG:
                                      Serial.println("case: Request Wifi Config settings");
                                      responseWifiConfig();
                                      break;
    case CMDSTATUS_DEV_STATE:
                                      Serial.println("case: App sending command to make Device ON/OFF - start misting or stop misting");
                                      handleDeviceState();
                                      break;
    case CMDSTATUS_DEV_NAME_UPDATE:
                                      Serial.println("case: App change Device Name");
                                      changeDeviceName();
                                      eventGenerate(EVT_DEVICE_REGISTRATION, NULL, 0);  // Device Registration 
                                      break;
    case CMDSTATUS_DEV_CHECK_HV_MODULE:
                                      Serial.println("case: App will check the HV module");
                                      checkHVmoduleTcp();
                                      break;
    default:
                                      break;              
  }
  Serial.println("command processed");
  return 0;
}

int checkHVmoduleTcp()
{
  char *deviceState = getConfiguration("dS");
  if(deviceState != NULL){
    Serial.printf("Last Device State : %s", deviceState);
    if (0 == atoi(deviceState))
    {
      Serial.println("Device Last State is Off");
      checkHVmoduleHardware();
    }
    else  {
      // if (true)
      // Generate Event EVENT_START_MIST_CYCLE (Start Misting)
      Serial.println("Device last State is On");
      // Stop misting
    executeStateMachine(EVENT_END_MIST);
    checkHVmoduleHardware();
  
    // Start misting
    executeStateMachine(EVENT_INIT_BLDC);
    }
  } else {
    Serial.println("Device State key \"dS\" Not present in the EEPROM. Considering it as device Off State.");
  }
  
  return 0;
}
int changeDeviceName()
{
  //StaticJsonBuffer<200> tcpKeyCmdCl;  // Pre-allocated Memory Pool to store JSONobject tree
  DynamicJsonBuffer tcpKeyCmdCl;
  Serial.print("tcpClientControlMessageBuffer : ");     Serial.println(tcpClientControlMessageBuffer);
  
  JsonObject& tcpKeyCmdClData = tcpKeyCmdCl.parseObject((const char*)tcpClientControlMessageBuffer);
  if(!tcpKeyCmdClData.success()) {
    Serial.println("parseObject() failed in changeDeviceName");
    return -1;
  }
  
  const char* cmd = tcpKeyCmdClData["cmd"];
  Serial.print("cmd    : ");  Serial.println(cmd);
  
  const char* dvNm = tcpKeyCmdClData["cl"]["dvNm"];          // New Device Name from App
  Serial.printf("New deviceName : %s\n", dvNm);

  // Save new Device Name in EEPROM
  setConfiguration(eepromDeviceName, (char *)dvNm); //set DeviceName

  return 1;
}

int responseWifiConfig()
{
  // check whether device is in AP mode or connected to Wifi
  // use wifi.getmode()
  int wifiMode = 0;
  char wifiConfigApModeRes[] = "{\"cmdr\":\"rqtWfCnfg\",\"cl\":{\"cn\":\"0\"}}";
  wifiMode = detectWifiMode();

  if(wifiMode == 2)     // if in AP mode, {"cmdr":"rqtWfCnfg","cl":{"cn":"0"}}
  {
    Serial.println("Device in AP mode");
    sendTcpResponseMessages(wifiConfigApModeRes, strlen(wifiConfigApModeRes));
  }else if(wifiMode == 1)   // wifi connected
  {
    Serial.println("Device in Station mode");
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("Device connected to wifi");
      char *res_ssid = getConfiguration("ssid");
      char *res_password = getConfiguration("password");
      if(res_ssid != NULL) {
       Serial.printf("ssid : %s\n", res_ssid);
      }
      if(res_password != NULL) {
        Serial.printf("Password : %s\n", res_password);
      }
      // create Json Message Response
      StaticJsonBuffer<200> tcpResp;
      JsonObject& wifiConfigData = tcpResp.createObject();
      wifiConfigData["cmdr"] = "rqtWfCnfg";
      JsonObject& cmdResp = wifiConfigData.createNestedObject("cl");
      //char *str = "hello";
      cmdResp["cn"] = "1";
      cmdResp["ss"]  = res_ssid;
      cmdResp["pwd"] = res_password;
        
        
      unsigned char arraySize = wifiConfigData.measureLength();
      char jsonMsgArray[arraySize + 1];
      wifiConfigData.printTo(Serial);
      wifiConfigData.printTo((char*)jsonMsgArray, arraySize + 1);
      sendTcpResponseMessages(jsonMsgArray, arraySize);
      tcpResp.clear();  //clear json buffer
    }else{
      Serial.println("Device Not connected to wifi");
    }
    
  }
  // if device connected to some Router
  // Retreive the SSID and Password from the EEPROM
  //char *res_ssid = getConfiguration("ssid");
  //char *res_password = getConfiguration("password");
  // Generate JSON  {"cmdr":"rqtWfCnfg","cl":{"cn":"1","ss":"router1","pwd":"password123"}}
  
  // Send this Json to device via TCP/IP
}

int handleDeviceState()
{
  // Parse Json to start stop the device
  StaticJsonBuffer<200> tcpKeyCmdCl;  // Pre-allocated Memory Pool to store JSONobject tree
  
  Serial.print("tcpClientControlMessageBuffer : ");     Serial.println(tcpClientControlMessageBuffer);
  
  JsonObject& tcpKeyCmdClData = tcpKeyCmdCl.parseObject((const char*)tcpClientControlMessageBuffer);
  if(!tcpKeyCmdClData.success()) {
    Serial.println("parseObject() failed in handleConfigUpdateJson");
  }
  
  const char* cmd = tcpKeyCmdClData["cmd"];
  Serial.print("cmd    : ");  Serial.println(cmd);

  const char* deviceState = tcpKeyCmdClData["cl"]["st"];        // Device State

  StaticJsonBuffer<200> tcpResp;
  JsonObject& deviceStateRes = tcpResp.createObject();
  deviceStateRes["cmdr"] = "dSt";
  JsonObject& cmdResp = deviceStateRes.createNestedObject("cl");
  
  if (0 == atoi(deviceState))
  {
    // if stop state, Device Off
    //{"cmdr":"dSt","cl":{"st":"0","stC":"200"}}
    cmdResp["st"] = "0";
    cmdResp["stC"]  = "200";
    // if Start State i.e. Device ON

    // Generate Event EVENT_END_MIST to stop the device

    // Update Motor Config, to device OFF in EEPROM
    setConfiguration(eepromDeviceState, (char *)deviceState); //set Mist State to OFF
    Serial.println("\n\n*** 5. STARTS ***");
    executeStateMachine(EVENT_END_MIST);
    Serial.println("*** 5. ENDS ***\n\n");
    
  }else if (1 == atoi(deviceState))
  {
    // check whether any configuration present in EEPROM
    if(0 == isMotorConfigPresent() || 0 != gDeviceAlertBitField)  // Configuration Not Present, Send {"cmdr":"dSt","cl":{"st":"0","stC":"400"}}
    {
      cmdResp["st"] = "0";
      cmdResp["stC"]  = "400";
      
    }else if (1 == isMotorConfigPresent() && 0 == gDeviceAlertBitField){
    // configuration Present, Send {"cmdr":"dSt","cl":{"st":"1","stC":"200"}}
    
    // Generate Event EVENT_INIT_BLDC to stop the device
    
      cmdResp["st"] = "1";
      cmdResp["stC"]  = "200";

      // Update Motor Config, to device OFF in EEPROM
      setConfiguration(eepromDeviceState, (char *)deviceState); //set Mist State to OFF
      executeStateMachine(EVENT_INIT_BLDC);
    }
  }
  unsigned char arraySize = deviceStateRes.measureLength();
  char jsonMsgArray[arraySize + 1];
  deviceStateRes.printTo(Serial);
  deviceStateRes.printTo((char*)jsonMsgArray, arraySize + 1);
  sendTcpResponseMessages(jsonMsgArray, arraySize);
  tcpResp.clear();  //clear json buffer
}

//float roundChartoFloat(char* varChar) 
//{ 
//    // 37.66666 * 100 =3766.66 
//    // 3766.66 + .5 =37.6716    for rounding off value 
//    // then type cast to int so value is 3766 
//    // then divided by 100 so the value converted into 37.66
//    float var = atof(varChar);
//    float value = (int)(var * 100 + .5); 
//    return (float)value / 100; 
//}


int handleConfigUpdateJson()
{
  //StaticJsonBuffer<200> tcpKeyCmdCl;  // Pre-allocated Memory Pool to store JSONobject tree
  DynamicJsonBuffer tcpKeyCmdCl;
  Serial.print("tcpClientControlMessageBuffer : ");     Serial.println(tcpClientControlMessageBuffer);
  
  JsonObject& tcpKeyCmdClData = tcpKeyCmdCl.parseObject((const char*)tcpClientControlMessageBuffer);
  if(!tcpKeyCmdClData.success()) {
    Serial.println("parseObject() failed in handleConfigUpdateJson");
  }
  
  const char* cmd = tcpKeyCmdClData["cmd"];
  Serial.print("cmd    : ");  Serial.println(cmd);

  // Check whether the cl contains cancel manual Mode command or the new Configuration
  if(tcpKeyCmdClData.containsKey("cnMm"))
  {
    // Case 4 : Cancel Manual mode and roll back to Auto Mode
    const char* cnMm = tcpKeyCmdClData["cnMm"];        // Cancel manual Mode command
    Serial.print("cnMm   : ");  Serial.println(cnMm);
    gManualModeOnFlag = 0;
    // Stop Motors BLDC and SM
//    bldcOff();
//    smOff();
//    executeStateMachine(EVENT_END_MIST);
    char *deviceState = getConfiguration("dS");
      if(deviceState != NULL){
      Serial.printf("Last Device State : %s", deviceState);
      if (0 == atoi(deviceState))
      {
        Serial.println("Device Last State is Off");
          bldcOff();
    smOff();
    executeStateMachine(EVENT_END_MIST);
      }
      else{
        // if (true)
        // Generate Event EVENT_START_MIST_CYCLE  (Start Misting)
        Serial.println("Device last State is On");
        executeStateMachine(EVENT_END_MIST);
        Serial.println("*** 1. Start ***");
        executeStateMachine(EVENT_INIT_BLDC);
        Serial.println("*** 1. End ***");
      }
      }else {
        Serial.println("Device State key \"dS\" Not present in the EEPROM. Considering it as device Off State.");
      }
    
  }else{
      
    // Parse the UDP JSON string and identify the command
    // const char* cmd = tcpKeyCmdClData["cl"];
    const char* mlMd = tcpKeyCmdClData["cl"]["mlMd"];          // Manual Mode on/off (0 – off, 1 - on)
    const char* blSpd = tcpKeyCmdClData["cl"]["blSpd"];        // bldc rpm
    const char* smSpd = tcpKeyCmdClData["cl"]["smSpd"];        // stepper motor rpm
    const char* tOn = tcpKeyCmdClData["cl"]["tOn"];            // mist on time
    const char* tOff = tcpKeyCmdClData["cl"]["tOff"];          // mist off time
    const char* blBfnT = tcpKeyCmdClData["cl"]["blBfnT"];      // bldc before On time
    const char* blAffT = tcpKeyCmdClData["cl"]["blAffT"];      // bldc after off time
    const char* cfrm = tcpKeyCmdClData["cl"]["cfrm"];          // For Device to know whether to save config or not

    
    
    Serial.print("mlMd   : ");  Serial.println(mlMd);
    Serial.print("blSpd  : ");  Serial.println(blSpd);
    Serial.print("smSpd  : ");  Serial.println(smSpd);
    Serial.print("tOn    : ");  Serial.println(tOn);
    Serial.print("tOff   : ");  Serial.println(tOff);
    Serial.print("blBfnT : ");  Serial.println(blBfnT);
    Serial.print("blAffT : ");  Serial.println(blAffT);
    Serial.print("cfrm   : ");  Serial.println(cfrm);
  
    // Case 1 : Manual Mode OFF and Confirm to Save (Auto Mode)
     // Case 2 : Manual Mode ON and NOT Confirm
      // Case 3 : Manual Mode ON and Confirm to Save
       // Case 4 : Cancel Manual mode and roll back to Auto Mode


    // Generate Event EVENT_END_MIST to stop the Misting
//    Serial.println("*** 8. Start***");
//    executeStateMachine(EVENT_END_MIST);
//    Serial.println("*** 8. End***");
    
    if(atoi(mlMd) == 0 && atoi(cfrm) == 1)
    {
      gManualModeOnFlag = 0;
      Serial.println("Case 1 : Manual Mode OFF and Confirm to Save");
      Serial.println("*** 8. Start***");
    executeStateMachine(EVENT_END_MIST);
    Serial.println("*** 8. End***");
      /*
       * blSpd
       * smSpd
       * tOn
       * tOff
       * blBfnT
       * blAffT
       */
  
      // Update the New Values in the EEPROM
      setConfiguration("blSpd",  (char*) blSpd  );
      setConfiguration("smSpd",  (char*) smSpd  );
      setConfiguration("tOn",    (char*) tOn    );
      setConfiguration("tOff",   (char*) tOff   );
      setConfiguration("blBfnT", (char*) blBfnT );
      setConfiguration("blAffT", (char*) blAffT );
  
      setMotorConfigPresence();

      // check device last state
      char *deviceState = getConfiguration("dS");
      if(deviceState != NULL){
      Serial.printf("Last Device State : %s", deviceState);
      if (0 == atoi(deviceState))
      {
        Serial.println("Device Last State is Off");
      }
      else{
        // if (true)
        // Generate Event EVENT_START_MIST_CYCLE  (Start Misting)
        Serial.println("Device last State is On");
        Serial.println("*** 1. Start ***");
        executeStateMachine(EVENT_INIT_BLDC);
        Serial.println("*** 1. End ***");
      }
      }else {
        Serial.println("Device State key \"dS\" Not present in the EEPROM. Considering it as device Off State.");
      }
      
    }else 
    if(atoi(mlMd) == 1 && atoi(cfrm) == 0){
      Serial.println("Case 2 : Manual Mode ON and NOT Confirm");
      gManualModeOnFlag = 1;

      Serial.println("*** 8. Start***");
    executeStateMachine(EVENT_END_MIST);
    Serial.println("*** 8. End***");
      // Run BLDC
      bldcOn(atoi(blSpd));
      // Run Stepper Motor
      smOn(atoi(smSpd));
      
      
    }else 
    if(atoi(mlMd) == 1 && atoi(cfrm) == 1){
      Serial.println("Case 3 : Manual Mode ON and Confirm to Save");
      gManualModeOnFlag = 1;
//      // Stop BLDC
//      bldcOff();
//      // Stop SM
//      smOff();
      Serial.println("*** 8. Start***");
    executeStateMachine(EVENT_END_MIST);
    Serial.println("*** 8. End***");
      // Update the New Values in the EEPROM
      setConfiguration("blSpd",  (char*) blSpd  );
      setConfiguration("smSpd",  (char*) smSpd  );
      setConfiguration("tOn",    (char*) tOn    );
      setConfiguration("tOff",   (char*) tOff   );
      setConfiguration("blBfnT", (char*) blBfnT );
      setConfiguration("blAffT", (char*) blAffT );
  
      setMotorConfigPresence();
        // check device last state
      char *deviceState = getConfiguration("dS");
      if(deviceState != NULL){
      Serial.printf("Last Device State : %s", deviceState);
      if (0 == atoi(deviceState))
      {
        Serial.println("Device Last State is Off");
      }
      else{
        // if (true)
        // Generate Event EVENT_START_MIST_CYCLE  (Start Misting)
        Serial.println("Device last State is On");
        Serial.println("*** 1. Start ***");
        executeStateMachine(EVENT_INIT_BLDC);
        Serial.println("*** 1. End ***");
      }
      }else {
        Serial.println("Device State key \"dS\" Not present in the EEPROM. Considering it as device Off State.");
      }
    }
  }
  return 0;
}

int handleTcpControlMessages()
{
  if (!tcpClient_control.connected()) {  //try to connect to a new client
    tcpClient_control = tcpServer.available();
  } else {  //read control message from tcp client
    char dataByte = tcpClient_control.read(); 
    if(dataByte != 0xff) { //discard the garbage character read after connecting to client
      gDataRxMutex = 0; 
      storeMessage(dataByte);
      Serial.println(dataByte);
    }
    
    //if message is ready, decode and process it
    if(1 == messageReady) {
      Serial.println("\n\n\n ********Data Received on TCP******  \n\n\n");      
      commandDecoder(); //decode command 
      processCommand(); //respond to command
      reInitMessenger();  //reInit message buffer    
      gDataRxMutex = 1;     
    }
  }
}
