#ifndef _HARDWARE_CONTROL_H
#define _HARDWARE_CONTROL_H


typedef struct {              //structure receiving the stats of motor parameters
  int16_t temperatureC;
  int16_t speedRpm;
  char motorState[12];
}motorStats_t;
motorStats_t gMotorStats;

char gRxBuff[256] = {0}; 

//command format = "key:value#"
char gCmdSeparator = ':';
char gParamSeparator = ',';
char gDataPtr = 0;


int initHardware(); //init leds, baud rate
void AlertledOn(); //turn on the board led D4
void AlertledOff();  //turn off the board led D4

int bldcOn(int);
int bldcOff();

int smOn(int);
int smOff();

int hvOn(int);
int hvOff();

int getHumidity();
int getTemp();

int checkHVmoduleHardware();
int waterSensorData();

bool isStepperRunning();

#endif // _TCP_CONTROL_SERVER_H
