#include "hardwareControl.h"
#include "TMCStepper.h"
#include "DHT.h"

#define DHTPIN 14     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

#define ALERT_LED 2
#define WATER_SENSOR_PIN  12// ADC1_0


#define HV_MODULE_PWM_PIN 22
const int freq = 1;   //1hz pwm
const int ledChannel = 0;
const int resolution = 8;

// BLDC UART 2 Communication Pins
#define RXD1 26   //uart1 rx gpio26 | pin 
#define TXD1 27   //uart2 tx gpio27 | pin


// Define pins on for stepper
#define EN_PIN    21  // LOW: Driver enabled. HIGH: Driver disabled(EN is IO20 on schematics)
#define STEP_PIN  19  // Step on rising edge
#define RX_PIN    16  // HardwareSerial pins
#define TX_PIN    17  //
#define R_SENSE 0.11 // Match to your driver SilentStepStick series use 0.11 UltiMachine Einsy and Archim2 boards use 0.2 Panucatt BSD2660 uses 0.1
#define STEP_TIMER_TIME 256  //time in us

TMC2208Stepper stepperDriver = TMC2208Stepper(&Serial2, R_SENSE);
hw_timer_t * gStepPulseTimerId = NULL;  //timer id ptr for step pulse timer | stepper motor 

bool gStepperRunning = false;  //flag to check whether stepper motor is running
unsigned int testCounter = 0;

//timer callbacks
void IRAM_ATTR onTimer(){
  digitalWrite(STEP_PIN, !digitalRead(STEP_PIN));
}

int initHardware()
{
  Serial.begin(115200); //for debug logs
  Serial1.begin(9600, SERIAL_8N1, RXD1, TXD1);    //for bldc
  Serial2.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);  //for stepper
  pinMode(ALERT_LED, OUTPUT);

  // For testing the BLDC, sm and HV
  //pinMode(27, OUTPUT);  // bldc
  //pinMode(26, OUTPUT);  // SM
  //pinMode(25, OUTPUT);  // HV
  dht.begin();

  AlertledOff();
  
  // configure LED PWM functionalitites
  ledcSetup(ledChannel, freq, resolution);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(HV_MODULE_PWM_PIN, ledChannel);
  
  //pinMode(D4, OUTPUT);
  //digitalWrite(D4,HIGH); //led off by default

  //initialize the hardware for stepper motor
  //stepperDriver.beginSerial(9600);  //set stepper uart driver baud rate
  stepperDriver.push();  //Push at the start of setting up the driver resets the register to default
  pinMode(EN_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  stepperDriver.pdn_disable(true);     // Use PDN/UART pin for communication
  stepperDriver.I_scale_analog(false); // Use internal voltage reference
  stepperDriver.rms_current(500);      // Set driver current = 500mA, 0.5 multiplier for hold current and RSENSE = 0.11.
  stepperDriver.toff(2);               // Enable driver in software
  digitalWrite(EN_PIN, LOW);    // Enable driver in hardware

  //initialize the step pulse timer
  gStepPulseTimerId = timerBegin(1, 80, true); /* 1 tick take 1/(80MHZ/80) = 1us so we set divider 80 and count up */
  /* Attach onTimer function to our timer */
  timerAttachInterrupt(gStepPulseTimerId, &onTimer, true);  //autoreload
  /* Set alarm to call onTimer function every second 1 tick is 1us
  => 1 second is 1000000us */
  /* Repeat the alarm (third parameter) */
  timerAlarmWrite(gStepPulseTimerId, STEP_TIMER_TIME, true);  //256 uS
}

static void clearCmdBuffer()
{
  memset(gRxBuff, '\0', strlen(gRxBuff));
  gDataPtr = 0;
}
static void storeMessage_BLDC(char dataByte)
{
  if(dataByte == '#') {  //cmd response received
    gRxBuff[gDataPtr] = '\0';
    commandRespDecode(gRxBuff);
    clearCmdBuffer();
  } else {
    gRxBuff[gDataPtr++] = dataByte;
  }
}

static int convertRpmToDhz(int rpm)
{
  int dHz = map(rpm, 0, 15000, 0, 2500);
  Serial.printf("----------------------------RPM: %d DHZ: %d\n", rpm, dHz);
  return dHz;
}

int bldcOn(int rpm)
{
  //convert rpm to dHz using map function
  Serial.printf("\nbldcOn : %d\n\n", rpm);
  int dHz = convertRpmToDhz(rpm);
  //digitalWrite(27, HIGH);
  char cmd[64];
  sprintf(cmd, "strtM:%d#", dHz);   //start motor command : "strtM:rpm#"
  transmitUartData(cmd);
  return 1;
}

int bldcOff()
{
  Serial.printf("bldcOff");
  //digitalWrite(27, LOW);
    char cmd[64];
  sprintf(cmd, "stopM:#");   //stop motor command : "stopM:#"
  transmitUartData(cmd);
  return 1;
}
//change the speed of bldc motor
static void changeBldcSpeed(int16_t rpm)
{
  char cmd[64];
  int dHz = convertRpmToDhz(rpm);
  sprintf(cmd, "cSpdM:%d#", dHz);   //change motor speed command : "cSpdM:rpm#"
  transmitUartData(cmd);
}

//start the bldc motor with a fixed rpm given
//static void startBldcMotor(int16_t rpm)
//{
//  char cmd[64];
//  sprintf(cmd, "strtM:%d#", rpm);   //start motor command : "strtM:rpm#"
//  transmitUartData(cmd);
//}

//stop the bldc motor
//static void stopBldcMotor()
//{
//  char cmd[64];
//  sprintf(cmd, "stopM:#");   //stop motor command : "stopM:#"
//  transmitUartData(cmd);
//}



//get motor statistics command for bldc motor
//temperature, rpm, motor state
static void getBldcStats()
{
  char cmd[64];
  sprintf(cmd, "getStatM:#");   //get motor command : "getStatM:#"
  transmitUartData(cmd);  
}

void bldcRxMotorStatusData()
{
  if (Serial1.available()){      //If data is received through rx port 
    char ch = Serial1.read();
    storeMessage_BLDC(ch);
    //Serial.printf("%c", ch);
  }
}

static void transmitUartData(char* txData)
{
  char dataByte;
  if(txData != NULL) {
    Serial1.write('#');
    for(int i = 0; i < strlen(txData); i++) {
      dataByte = txData[i];                                    
      Serial1.write(dataByte);
    }
  }
}

int waterSensorData()
{
  int sensorValue = analogRead(WATER_SENSOR_PIN);
  if(sensorValue > 100){
    Serial.printf("Water Leak Detected :: sensorValue : %d\n", sensorValue);
    return 1;
  }else{
    Serial.println("no leak detected");
    return 0;
  }
}

void AlertledOn()
{
  //digitalWrite(D4,LOW); //led on
  digitalWrite(ALERT_LED,HIGH);
}

void AlertledOff()
{
  //digitalWrite(D4,HIGH); //led on
  digitalWrite(ALERT_LED,LOW);
}

bool isStepperRunning()
{
  if(gStepperRunning) {
    return true;  
  } else {
    return false;
  }
}

int smOn(int rpm)
{
  int newTimerInterval; //in microseconds
  
  Serial.printf("smOn : %d\n\n", rpm);
  //digitalWrite(26, HIGH);
  
  //allocate timer value based on the rpm received
  if(rpm < 20 || rpm > 80){
    newTimerInterval = 0;
    return 0;
  }
   
  newTimerInterval = map(rpm, 20, 80, 1000, 256);
  
  if(gStepperRunning == true) {
    //stop the step pulse timer
    timerAlarmDisable(gStepPulseTimerId);
    
    //update new interval
    timerAlarmWrite(gStepPulseTimerId, newTimerInterval, true);

    //restart timer 
    timerAlarmEnable(gStepPulseTimerId);

    return 1;
  }

  /* Update timer interval */
  timerAlarmWrite(gStepPulseTimerId, newTimerInterval, true);
  
   /* Start timer */
  timerAlarmEnable(gStepPulseTimerId);
  
  gStepperRunning = true;
  return 1;
}

int smOff()
{
  Serial.printf("smOff");
  //digitalWrite(26, LOW);

  //stop the step pulse timer
  timerAlarmDisable(gStepPulseTimerId);
    
  gStepperRunning = false;
  return 1;
}

int hvOn(int pwmHVValue)
{
  Serial.printf("hvOn : %d\n\n", pwmHVValue);
  ledcWrite(ledChannel, 127); //0-256 duty cycle, 127-> 50% duty cycle
  return 1;
}

int hvOff()
{
  Serial.printf("hvOff");
  ledcWrite(ledChannel, 0);
  return 1;
}

int getTemp(){
  int t = dht.readTemperature();
  if (isnan(t) || t < -20 || t > 100) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return -1;
  }else{
    Serial.print(F("Temperature: "));  Serial.println(t);
    return t;
  }
  return 0;
}

int getHumidity(){
  int h = dht.readHumidity();
  if (isnan(h) || h < 0 || h > 100) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return 0;
  }else {
    Serial.print(F("Humidity: "));
    Serial.println(h);
    return h;
  }
  return 0;
}

int checkHVmoduleHardware()
{
  Serial.println("checkHVmoduleHardware()");
  return 1;
}
