#include "deviceMonitor.h"
#include "configuration.h"




struct deviceMonitorData *devMonData ;
//= (struct deviceMonitorData*) malloc(sizeof(struct deviceMonitorData));

int getDeviceMonitorData()
{
  devMonData = (struct deviceMonitorData*) malloc(sizeof(struct deviceMonitorData));
  Serial.println("getDeviceMonitorData()");
  getBldcStats();
  
//  Serial.printf("size of struture : %d\n", sizeof(devMonData));
  devMonData->smRpm = getStepperMotorRpm();
  devMonData->deviceTemperature = getDeviceTemperature();
  devMonData->deviceHumidity = getDeviceHumidity();
  devMonData->waterFlowRate = getwaterFlowRate();
  
  devMonData->bldcRpm = getBldcRpm();
  devMonData->bldcTemperature = getBldcTemperature();
  
  displayDeviceMonitorData();
  
  //Alerts
  devMonData->waterSensorStatus = getWaterSensorData();
  getCurrentData();
  
  if(gDeviceAlertBitField != 0)
  {
    //Serial.println("Alert Found. Calling respected Events");
    //Serial.printf("Alert value : %d\n", gDeviceAlertBitField);
    // Generate Event to stop the device working and send notification to mobile and server
    eventGenerate(EVT_DEV_HANDLE_ALERT, NULL, 0);  // evtSendDeviceStatus()
    eventGenerate(EVT_SEND_DEVICE_STATUS, NULL, 0);  // evtSendDeviceStatus()
  }
  

  return 0;
}


int displayDeviceMonitorData()
{
  Serial.println("displayDeviceMonitorData()\n");
  // Motor Data  
  Serial.printf("BLDC Motor RPM     : %d\n", devMonData->bldcRpm);
  Serial.printf("Stepper Motor RPM  : %d\n", devMonData->smRpm);
  Serial.printf("BLDC Temperature   : %d\n", devMonData->bldcTemperature);
  // Device Data
  Serial.printf("Device Temperature : %d\n", devMonData->deviceTemperature);
  Serial.printf("Device Humidity    : %d\n", devMonData->deviceHumidity);
  Serial.printf("Water Flow Rate    : %d\n", devMonData->waterFlowRate);  
  // Alert Data  //1. Water leak  //2. High BLDC Current  //3. High BLDC Temperature  //4. High Device Temperture  
  return 0;
}

/*  MOTOR CONFIG NAME In EEPROM
 *   
 * blSpd
 * smSpd
 * blTmp
 * dTmp
 * dHum
 * dWFR
 * 
 */
static int convertDhzToRpm(int dHz)
{
  int rpm = map(dHz, 0, 2500, 0, 15000);
  Serial.printf("DHZ: %d RPM: %d---------------------------------------------------\n", dHz, rpm);
  return rpm;
}

static void commandRespDecode(char* gRxBuff)
{
  char *commandResp = NULL;
  char *params = NULL;

  commandResp = strtok(gRxBuff, &gCmdSeparator);
  if (commandResp != NULL) {
    Serial.printf("cmd resp recv: %s\n", commandResp);
    params = strtok(NULL, &gCmdSeparator);
    if (params != NULL) {
     Serial.printf("params recv: %s\n", params); 
    }
  } else {
    return;
  }

  if(0 == strcmp(commandResp, "statM")) {
     gMotorStats.temperatureC = atoi((const char*)strtok(params, &gParamSeparator));
     Serial.printf("temp: %d\n", gMotorStats.temperatureC);
     int rpm = convertDhzToRpm(atoi((const char*)strtok(NULL, &gParamSeparator)));
     gMotorStats.speedRpm = rpm;
     Serial.printf("speed: %d\n", gMotorStats.speedRpm);
     strcpy(gMotorStats.motorState, (const char*)strtok(NULL, &gParamSeparator));
     Serial.printf("state: %s\n", gMotorStats.motorState);
  } else {
    Serial.printf("invalid response\n");
  }
}
int getBldcTemperature()
{
  // get data from STEVAL through UART communication
  Serial.printf("temp: %d\n", gMotorStats.temperatureC);
  if(gMotorStats.temperatureC > -20 && gMotorStats.temperatureC < 100)
  {
    return gMotorStats.temperatureC;
  }else
  {
    return 0;
  }
}

int getStepperMotorRpm()
{
// get the Data from EEPROM
  
  char *smSpeed = getConfiguration("smSpd");
  
  if(smSpeed != NULL){
    int spd = atoi(smSpeed);
    if(isStepperRunning()) {
      return spd;
    } else {
      return 0; 
    } 
  } else {
    Serial.println("smSpeed key \"smSpd\" Not present in the EEPROM.");
  }
  return 0;
}

int getBldcRpm()
{
  // get data from STEVAL through UART communication
  Serial.printf("speed: %d\n", gMotorStats.speedRpm);
  if(gMotorStats.speedRpm > 0 && gMotorStats.speedRpm <= 15000)
  {
    return gMotorStats.speedRpm;
  }else
  {
    return 0;
  }
  
}

int getDeviceTemperature()
{
  // Get Sensor Data from DHT11
  int temp = getTemp();
  //temp = 35;
  if(-1 == temp){
    return -1;
  }else if(temp >= 30)
  {
    // Set 0th bit to 1 as High Device Temperature detected
    //Serial.println("Alert Found :: High Device Temperature");
    gDeviceAlertBitField = modifyBit(gDeviceAlertBitField, ALERT_EVT_HIGH_DEV_TEMP, 1);
    //Serial.printf("Alert value : %d\n", gDeviceAlertBitField);
  }else
  {
    // Set 0th bit to 0 as water leakage NOT detected
    //Serial.println("Alert Found :: No High Device Temperature");
    gDeviceAlertBitField = modifyBit(gDeviceAlertBitField, ALERT_EVT_HIGH_DEV_TEMP, 0);
    AlertledOff();
    //Serial.printf("Alert value : %d\n", gDeviceAlertBitField);
  }
  return temp;  // hardwarecontrol.ino
  //return 27;
}

int getDeviceHumidity()
{
  // Get Sensor Data from DHT11
  int Hum =  getHumidity();
  //Hum = 90;
  if(Hum >= 80)
  {
    // Set 0th bit to 1 as High Device Humidity detected
    Serial.println("Alert Found :: High Device Humidity");
    gDeviceAlertBitField = modifyBit(gDeviceAlertBitField, ALERT_EVT_HIGH_DEV_HUM, 1);
    Serial.printf("Alert value : %d\n", gDeviceAlertBitField);
  }else
  {
    // Set 0th bit to 0 as High Device Humidity NOT detected
    Serial.println("Alert Found :: No High Device Humidity");
    gDeviceAlertBitField = modifyBit(gDeviceAlertBitField, ALERT_EVT_HIGH_DEV_HUM, 0);
    AlertledOff();
    Serial.printf("Alert value : %d\n", gDeviceAlertBitField);
  }
  
  return Hum;
}

int modifyBit(int n, int p, int b) 
{ 
    int mask = 1 << p; 
    return (n & ~mask) | ((b << p) & mask); 
}

/* 
 *  ALert Bit Field
 *  
 *  0th - Water Leakage
 *  1th - High Current
 *  2nd - High Device Temperature
 *  3rd
 *  4th
 *  5th
 *  6th
 *  7th
 *  
 */

int getWaterSensorData()
{
  // Get Sensor Data from Water Sensor
  // Water Leakage Data
  int waterLeak = waterSensorData();
  waterLeak = 0;
  if(1 == waterLeak)
  {
    // Set 0th bit to 1 as water leakage detected
    //Serial.println("Alert Found :: Water Leakage");

    gDeviceAlertBitField = modifyBit(gDeviceAlertBitField, ALERT_EVT_WATER_LEAK, 1);
    //Serial.printf("Alert value : %d\n", gDeviceAlertBitField);
    return 1;
  }else
  {
    // Set 0th bit to 0 as water leakage NOT detected
    //Serial.println("Alert Found :: No Water Leakage");

    gDeviceAlertBitField = modifyBit(gDeviceAlertBitField, ALERT_EVT_WATER_LEAK, 0);
    AlertledOff();
    //Serial.printf("Alert value : %d\n", gDeviceAlertBitField);
    return 0;
  }
  return 0;
}

int getCurrentData()
{
  // Get Sensor Data from Water Sensor
  // Water Leakage Data
  int HighCurrent = 0;
  //HighCurrent = 1;
  if(1 == HighCurrent)
  {
    // Set 0th bit to 1 as water leakage detected
    //Serial.println("Alert Found :: HighCurrent");
    gDeviceAlertBitField = modifyBit(gDeviceAlertBitField, ALERT_EVT_HIGH_CURRENT, 1);
    //Serial.printf("Alert value : %d\n", gDeviceAlertBitField);
    return 1;
  }else
  {
    // Set 0th bit to 0 as water leakage NOT detected
    modifyBit(gDeviceAlertBitField, 0, 0);
    return 0;
  }
  return 0;
}

int getwaterFlowRate()
{
  // get sm rpm
  int rpm = getStepperMotorRpm();
  int flowRate = (rpm*10)/60;
  return flowRate;
}
