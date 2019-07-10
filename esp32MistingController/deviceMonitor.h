#ifndef _DEVICEMONITOR_H
#define _DEVICEMONITOR_H

struct deviceMonitorData{
  int bldcRpm;
  int smRpm;
  int bldcTemperature;
  int deviceTemperature;
  int deviceHumidity;
  int waterFlowRate;
  
  // Alerts
  int waterSensorStatus;
  
} ;

typedef enum Alerts_enum {
  ALERT_EVT_WATER_LEAK = 0,
  ALERT_EVT_HIGH_CURRENT,
  ALERT_EVT_HIGH_DEV_TEMP,
  ALERT_EVT_HIGH_DEV_HUM,
  ALERT_EVT_MAX
} Alert_Events_e;

unsigned int gDeviceAlertBitField = 0;

char *gNoAlertJsonArr  = "200";
char *gWaterLeakAlertJsonArr = "500";
char *gHighCurrentAlertJsonArr  = "501";
char *gHighDevTempAlertJsonArr  = "502";
char *gHighDevHumAlertJsonArr  = "503";


#endif // _DEVICEMONITOR_H
