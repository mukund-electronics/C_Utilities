#include <stdio.h>
#include <stdlib.h>
#include <Arduino.h>
#include "jumpStateMachine.h"

int32_t gStartSMTimerId;
int32_t gStartHVTimerId;
int32_t gStopHVTimerId;

int32_t gStopSMTimerId;
int32_t gStopBLDCTimerId;
int32_t gDeviceStopTimerId;
int32_t gMistStopDelayTimerId;
extern int gManualModeOnFlag;

typedef enum{
  STATE_NA=0,             //  0
  STATE_MIST_END,         //  1           // Stop Misting
  STATE_BLDC_INIT,        //  2
  STATE_SM_INIT,          //  3
  STATE_HV_INIT,          //  4
  STATE_HV_END,           //  5
  STATE_SM_END,           //  6
  STATE_BLDC_END,         //  7
  STATE_MIST_OFF_TIME,    //  8
  STATE_MISTING_END,      //  9
  STATE_MAX               //  10
}jumpStates_e;

const char* gStates[]={
  [STATE_NA]                    = "NA",                         //  0
  [STATE_MIST_END]              = "STATE_MIST_END",             //  1
  [STATE_BLDC_INIT]             = "STATE_BLDC_INIT",            //  2
  [STATE_SM_INIT]               = "STATE_SM_INIT",              //  3
  [STATE_HV_INIT]               = "STATE_HV_INIT",              //  4
  [STATE_HV_END]                = "STATE_HV_END",               //  5
  [STATE_SM_END]                = "STATE_SM_END",               //  6
  [STATE_BLDC_END]              = "STATE_BLDC_END",             //  7
  [STATE_MIST_OFF_TIME]         = "STATE_MIST_OFF_TIME",        //  8
  [STATE_MISTING_END]           = "STATE_MISTING_END",          //  9
  [STATE_MAX]                   = "STATE_MAX"                   //  10
};

char* gEvents[EVENT_MAX];

/* Operation to be performed during state transition */
typedef jumpEvents_e (*Operation)();

/* Structure for State Machine Configuration Data*/
typedef struct {
    jumpStates_e nextState;     // Next State
    Operation operation;        // Function to be executed before transitioning to next state
}jumpStateMachine_t;

/* Functions to switch the State of the LEDs*/
jumpEvents_e start_Bldc__Timer_Init(); // 1
jumpEvents_e start_SM__Timer_Init();   // 2
jumpEvents_e start_HV__Timer_Init();   // 3
jumpEvents_e stop_HV__Timer_Init();    // 4
jumpEvents_e stop_SM__Timer_Init();    // 5
jumpEvents_e stop_Bldc__Timer_Init();  // 6
jumpEvents_e startMistOffTimer();      // 7
jumpEvents_e endMist();                // 8

// Internal functions for State Machine
jumpEvents_e bldcStop();               // 9   .... Stop BLDC motor
jumpEvents_e smStop();                 // 10  .... Stop Stepper Motor
jumpEvents_e hvStop();                 // 11  .... Stop High Voltage Module

jumpEvents_e bldcStart();              // 12  .... Start BLDC motor
jumpEvents_e smStart();                // 13  .... Start Stepper Motor
jumpEvents_e hvStart();                // 14  .... Start High Voltage Module

jumpEvents_e startSmInitTimer();      // 15
jumpEvents_e startHvInitTimer();      // 16
jumpEvents_e startHvEndTimer();       // 17
jumpEvents_e startHvEndTimer();       // 18
jumpEvents_e startSmEndTimer();       // 19
jumpEvents_e startBldcEndTimer();     // 20

jumpEvents_e mistingEnd();           // 21
/* State Table for jump State Machine*/
jumpStateMachine_t gJumpSm[STATE_MAX][EVENT_MAX]; 

static jumpStates_e gCurrentState;              // To maintain the current state

/*********************************/
int floatToInt(char* var) 
{ 
  if(var != NULL) {
    float a = atof(var);
    int c;
    c = a*1000000;
    c = c/100000;
    return c;
  }
}
/********************************/

/**************** functions *********************************/

jumpEvents_e start_Bldc__Timer_Init() // 1
{
  // Start BLDC functioning
  Serial.println("start_Bldc__Timer_Init");
  bldcStart();
  // Start timer for next state
  return startSmInitTimer();
}

//blSpd 100
//smSpd 200 
//tOn 2 
//tOff 10
//blBfnT 2
//blAffT 4
//
//dId 2 dS 1

jumpEvents_e startSmInitTimer()
{
  Serial.println("startSmInitTimer");
  // BBMO - Bldc Before Mist On Timer
  // get data BLDC_BEFORE_MIST_ON time and after this time, our Stepper Motor will start after this time
  //int val = getConfiguration("blBfnT");
  //return EVENT_INIT_SM;

  char* startSMTimer = getConfiguration("blBfnT");
  int sSMT = floatToInt(startSMTimer);
  Serial.printf("Starting timer for : %d\n", sSMT*100);
  timerUpdateInterval(gStartSMTimerId, sSMT*100);
  if (E_SUCCESS != timer_c_Start(gStartSMTimerId)) 
  {
    Serial.println("Error starting Device Monitor timer\r\n");
  }
  return EVENT_NA;
}


jumpEvents_e start_SM__Timer_Init()   // 2
{
  Serial.println("start_SM__Timer_Init");
  // Start SM functioning
  smStart();

  // Start timer for next state
  return startHvInitTimer();
}

jumpEvents_e startHvInitTimer()
{
  Serial.println("startHvInitTimer");
  // this timer value is zero seconds right now
  //return EVENT_INIT_HV;
//  timerUpdateInterval(gStartHVTimerId, 20);
//  if (E_SUCCESS != timer_c_Start(gStartHVTimerId)) 
//  {
//    Serial.println("Error starting Device Monitor timer\r\n");
//  }
  
  return EVENT_NA;
}

jumpEvents_e start_HV__Timer_Init()   // 3
{
 Serial.println("start_HV__Timer_Init");
  // Start HV functioning
  hvStart();

  // Start timer for next state
  return startHvEndTimer();
}

jumpEvents_e startHvEndTimer()
{
 Serial.println("startHvEndTimer");
  // Get the MIST_ON_TIMER value, this long our misting ahs continue
  //return EVENT_END_HV;

  char* MistOnTimer = getConfiguration("tOn");
  int mONT = floatToInt(MistOnTimer);
  Serial.printf("Starting timer for : %d\n", mONT*100);
  timerUpdateInterval(gStopHVTimerId, mONT*100);
  if (E_SUCCESS != timer_c_Start(gStopHVTimerId)) {
    Serial.println("Error starting Device Monitor timer\r\n");
  }
  
  return EVENT_NA;
}

jumpEvents_e stop_HV__Timer_Init()    // 4
{
  Serial.println("stop_HV__Timer_Init");
  // Stop HV functioning
  hvStop();

  // Start timer for next state
  return startSmEndTimer();
}

jumpEvents_e startSmEndTimer()
{
  Serial.println("startSmEndTimer");
  //return EVENT_END_SM;
  return EVENT_NA;
}


jumpEvents_e stop_SM__Timer_Init()    // 5
{
  Serial.println("stop_SM__Timer_Init");
  // Stop SM functioning
  smStop();

  // Start timer for next state
  return startBldcEndTimer();
}

jumpEvents_e startBldcEndTimer()
{
  Serial.println("startBldcEndTimer");
  
  char* bldcAfterOffTimer = getConfiguration("blAffT");
  int blAOFT = floatToInt(bldcAfterOffTimer);
  Serial.printf("Starting timer for : %d\n", blAOFT*100);
  timerUpdateInterval(gStopBLDCTimerId, blAOFT*100);
  if (E_SUCCESS != timer_c_Start(gStopBLDCTimerId)) {
    Serial.println("Error starting Device Monitor timer\r\n");
  }
  //return EVENT_END_BLDC;
  return EVENT_NA;
}

jumpEvents_e stop_Bldc__Timer_Init()  // 6
{
 Serial.println("stop_Bldc__Timer_Init");
  // Stop bldc functioning
  bldcStop();

  // Start timer for next state
  // return startMistOffTimer();
  //return EVENT_COM_MIST_CYCLE;
  return EVENT_NA;
}

jumpEvents_e startMistOffTimer()      // 7
{
  Serial.println("startMistOffTimer");
  char* mistOffTimer = getConfiguration("tOff");
  int mOFT = floatToInt(mistOffTimer);
  Serial.printf("Starting timer for : %d\n", mOFT*100);
  timerUpdateInterval(gDeviceStopTimerId, mOFT*100);
  if (E_SUCCESS != timer_c_Start(gDeviceStopTimerId)) {
    Serial.println("Error starting Device Monitor timer\r\n");
  }
  //return EVENT_START_MIST_CYCLE;
  return EVENT_NA;
}

jumpEvents_e endMist()                // 8
{
  // stop bldc
  // stop SM
  // stop HV
  Serial.println("endMist() Called\n1. stop bldc\n2. stop SM\n3. stop HV\n");
  
  // From hardwareControl.h
  bldcOff();
  smOff();
  hvOff();

  timerUpdateInterval(gMistStopDelayTimerId, 1*1000);
  if (E_SUCCESS != timer_c_Start(gMistStopDelayTimerId)) {
    Serial.println("Error starting Device Monitor timer\r\n");
  }
  return EVENT_NA;
}

jumpEvents_e bldcStop()                // 9   .... Stop BLDC motor
{
  //Stop BLDC
  Serial.println("bldcStop() ");

  bldcOff();
  
  return EVENT_NA;
}

jumpEvents_e smStop()                // 10    .... Stop Stepper Motor
{
  //Stop Stepper Motor
  Serial.println("mStop() ");

  smOff();
  
  return EVENT_NA;
}

jumpEvents_e hvStop()                // 11    .... Stop High Voltage Module
{
  //Stop High Voltage Module
  Serial.println("hvStop() ");
  
  hvOff();
  
  return EVENT_NA;
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
jumpEvents_e bldcStart()                // 12  .... Start BLDC motor
{
  //Start BLDC
  Serial.println("bldcStart() ");
  
  // Get BLDC Speed from EEPROM
  char *blSpd = getConfiguration("blSpd");
  bldcOn(atoi(blSpd));
  
  return EVENT_NA;
}

jumpEvents_e smStart()                  // 13  .... Start Stepper Motor
{
  //Start Stepper Motor
  Serial.println("smStart() ");
  
  // Get Stepper Motor Speed from EEPROM
  char *smSpd = getConfiguration("smSpd");
  smOn(atoi(smSpd));
  return EVENT_NA;
}

jumpEvents_e hvStart()                  // 14  .... Start High Voltage Module
{
  //Start High Voltage Module
  Serial.println("hvStart() ");

  int hvPWM = 200;
  hvOn(hvPWM);
  
  return EVENT_NA;
}

jumpEvents_e mistingEnd()
{
  Serial.println("Misting Ends");
  return EVENT_NA;
}
/************** STATE MACHINE FLOW FUNCTIONS ****************************/


static Void onStartSMTimerExpiry(Int32_t timerId, Void *arg)  // if Device Monitor Timer Expired
{
  Serial.println("4. TIMER - Start SM Timer Expired");
  Serial.println("\n\n*** 2 & 3. Start***\n\n");
  executeStateMachine(EVENT_INIT_SM);
  executeStateMachine(EVENT_INIT_HV);
  Serial.println("\n\n*** 2 & 3. End***\n\n");
}

static Void onStartHVTimerExpiry(Int32_t timerId, Void *arg)  // if Device Monitor Timer Expired
{
  Serial.println("5. TIMER - Start HV Timer Expired"); 
}

static Void onStopHVTimerExpiry(Int32_t timerId, Void *arg)  // if Device Monitor Timer Expired
{
  Serial.println("6. TIMER - Stop HV Timer Expired");
  Serial.println("\n\n*** 4 & 5. Start***\n\n");
  executeStateMachine(EVENT_END_HV);
  executeStateMachine(EVENT_END_SM);
  Serial.println("\n\n*** 4 & 5. ENDS***\n\n");
}

static Void onStopSMTimerExpiry(Int32_t timerId, Void *arg)  // if Device Monitor Timer Expired
{
  Serial.println("7. TIMER - Stop SM Timer Expired"); 
}

static Void onStopBLDCTimerExpiry(Int32_t timerId, Void *arg)  // if Device Monitor Timer Expired
{
  Serial.println("8. TIMER - Stop BLDC Timer Expired");
  Serial.println("\n\n*** 6 & 7. Start***\n\n");
  executeStateMachine(EVENT_END_BLDC);
  executeStateMachine(EVENT_COM_MIST_CYCLE);
  Serial.println("\n\n*** 6 & 7. Ends***\n\n");
}

static Void onDeviceStopTimerExpiry(Int32_t timerId, Void *arg)  // if Device Monitor Timer Expired
{
  Serial.println("9. TIMER - MIST OFF Timer Expired.\nStart Timer after which the BLDC will start again (Mist Again)");
  Serial.println("\n\n*** 8. Start***\n\n");
  executeStateMachine(EVENT_START_MIST_CYCLE);
  Serial.println("\n\n*** 8. End***\n\n");
}

static Void onMistStopDelayTimerExpiry(Int32_t timerId, Void *arg)  // if 
{
  Serial.println("10. TIMER - MIST END Timer Expired.\nStart Timer to give delay to STOP command for the BLDC and Stepper Motor");
  Serial.println("\n\n*** 10. Start***\n\n");
  executeStateMachine(EVENT_MIST_END_DELAY);
  Serial.println("\n\n*** 10. End***\n\n");
  if(gManualModeOnFlag == 0) {
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

// Initialize the jump State Machine only if pixel count is greater than zero
int initJumpStateMachine()
{
//STATE_MISTING_END
  gJumpSm  [STATE_MISTING_END]     [EVENT_INIT_BLDC]        =   {STATE_BLDC_INIT,       start_Bldc__Timer_Init    };    // 1
  
  gJumpSm  [STATE_MIST_END]        [EVENT_MIST_END_DELAY]   =   {STATE_MISTING_END,     mistingEnd                };    // 2
  
  gJumpSm  [STATE_BLDC_INIT]       [EVENT_INIT_SM]          =   {STATE_SM_INIT,         start_SM__Timer_Init      };    // 3
  gJumpSm  [STATE_BLDC_INIT]       [EVENT_END_MIST]         =   {STATE_MIST_END,        endMist                   };    // 4
  
  gJumpSm  [STATE_SM_INIT]         [EVENT_INIT_HV]          =   {STATE_HV_INIT,         start_HV__Timer_Init      };    // 5
  gJumpSm  [STATE_SM_INIT]         [EVENT_END_MIST]         =   {STATE_MIST_END,        endMist                   };    // 6
  
  gJumpSm  [STATE_HV_INIT]         [EVENT_END_HV]           =   {STATE_HV_END,          stop_HV__Timer_Init       };    // 7
  gJumpSm  [STATE_HV_INIT]         [EVENT_END_MIST]         =   {STATE_MIST_END,        endMist                   };    // 8
  
  gJumpSm  [STATE_HV_END]          [EVENT_END_SM]           =   {STATE_SM_END,          stop_SM__Timer_Init       };    // 9
  gJumpSm  [STATE_HV_END]          [EVENT_END_MIST]         =   {STATE_MIST_END,        endMist                   };    // 10
  
  gJumpSm  [STATE_SM_END]          [EVENT_END_BLDC]         =   {STATE_BLDC_END,        stop_Bldc__Timer_Init     };    // 11
  gJumpSm  [STATE_SM_END]          [EVENT_END_MIST]         =   {STATE_MIST_END,        endMist                   };    // 12
  
  gJumpSm  [STATE_BLDC_END]        [EVENT_COM_MIST_CYCLE]   =   {STATE_MIST_OFF_TIME,   startMistOffTimer         };    // 13
  gJumpSm  [STATE_BLDC_END]        [EVENT_END_MIST]         =   {STATE_MIST_END,        endMist                   };    // 14
  
  gJumpSm  [STATE_MIST_OFF_TIME]   [EVENT_START_MIST_CYCLE] =   {STATE_BLDC_INIT,       start_Bldc__Timer_Init    };    // 15
  gJumpSm  [STATE_MIST_OFF_TIME]   [EVENT_END_MIST]         =   {STATE_MIST_END,        endMist                   };    // 16
  gJumpSm  [STATE_MIST_END]        [EVENT_END_MIST]         =   {STATE_MIST_END,        endMist                   };    // 2
  
//  EVENT_INIT_BLDC,        //  1
//  EVENT_INIT_SM,          //  2
//  EVENT_INIT_HV,          //  3
//  EVENT_END_HV,           //  4
//  EVENT_END_SM,           //  5
//  EVENT_END_BLDC,         //  6
//  EVENT_START_MIST_CYCLE, //  7
//  EVENT_END_MIST,         //  8
//  EVENT_COM_MIST_CYCLE,   //  9
  gEvents [EVENT_NA]                = "EVENT_NA";               //  0
  gEvents [EVENT_INIT_BLDC]         = "EVENT_INIT_BLDC";        //  1
  gEvents [EVENT_INIT_SM]           = "EVENT_INIT_SM";          //  2
  gEvents [EVENT_INIT_HV]           = "EVENT_INIT_HV";          //  3
  gEvents [EVENT_END_HV]            = "EVENT_END_HV";           //  4
  gEvents [EVENT_END_SM]            = "EVENT_END_SM";           //  5
  gEvents [EVENT_END_BLDC]          = "EVENT_END_BLDC";         //  6
  gEvents [EVENT_START_MIST_CYCLE]  = "EVENT_START_MIST_CYCLE"; //  7
  gEvents [EVENT_END_MIST]          = "EVENT_END_MIST";         //  8
  gEvents [EVENT_COM_MIST_CYCLE]    = "EVENT_COM_MIST_CYCLE";   //  9
  gEvents [EVENT_MIST_END_DELAY]    = "EVENT_MIST_END_DELAY";   //  10
  gEvents [EVENT_MAX]               = "Max";                    //  11

  //gCurrentState = STATE_MIST_END;
  gCurrentState = STATE_MISTING_END;

  // Timers creation

  
  // 4. Start SM timer
  gStartSMTimerId = timerCreate(TIMER_LOW_PRIO, SEND_DEV_MONITOR_TIME * 1000, False, onStartSMTimerExpiry, NULL);
  if (gStartSMTimerId < 0) {
    Serial.println("Error creating Device Monitor timer\r\n");
  }   


  // 5. Start HV timer
  gStartHVTimerId = timerCreate(TIMER_LOW_PRIO, SEND_DEV_MONITOR_TIME * 1000, False, onStartHVTimerExpiry, NULL);
  if (gStartHVTimerId < 0) {
    Serial.println("Error creating Device Monitor timer\r\n");
  }   


  
  // 6. Stop HV timer
  gStopHVTimerId = timerCreate(TIMER_LOW_PRIO, SEND_DEV_MONITOR_TIME * 1000, False, onStopHVTimerExpiry, NULL);
  if (gStopHVTimerId < 0) {
    Serial.println("Error creating Device Monitor timer\r\n");
  }   


  
  // 7. Stop SM timer
  gStopSMTimerId = timerCreate(TIMER_LOW_PRIO, SEND_DEV_MONITOR_TIME * 1000, False, onStopSMTimerExpiry, NULL);
  if (gStopSMTimerId < 0) {
    Serial.println("Error creating Device Monitor timer\r\n");
  }   


  
  // 8. Stop bldc timer
  gStopBLDCTimerId = timerCreate(TIMER_LOW_PRIO, SEND_DEV_MONITOR_TIME * 1000, False, onStopBLDCTimerExpiry, NULL);
  if (gStopBLDCTimerId < 0) {
    Serial.println("Error creating Device Monitor timer\r\n");
  }   


  
  // 9. Start Device OFF timer
  gDeviceStopTimerId = timerCreate(TIMER_LOW_PRIO, SEND_DEV_MONITOR_TIME * 1000, False, onDeviceStopTimerExpiry, NULL);
  if (gDeviceStopTimerId < 0) {
    Serial.println("Error creating Device Monitor timer\r\n");
  }   

  // 10. Mist OFF timer to give motors proper time to stop
  gMistStopDelayTimerId = timerCreate(TIMER_LOW_PRIO, SEND_DEV_MONITOR_TIME * 1000, False, onMistStopDelayTimerExpiry, NULL);
  if (gMistStopDelayTimerId < 0) {
    Serial.println("Error creating Device Monitor timer\r\n");
  } 
  

  return 1;
}

int gPrevState = 0;
int gPrevEvent = 0;


// Execute State Machine and return 1 if jump condition statisfied
int executeStateMachine(jumpEvents_e mistEvent)   // from 
{
  jumpEvents_e event = mistEvent;

  Serial.println("\n\nExecute State Machine.\n\n");

  Serial.print("event : ");
  Serial.println(gEvents[event]);

//  if(event == EVENT_END_MIST)   // Get out of State Machine
//  {
//    printSerial("EVENT_END_MIST. End State machine");
// //   Serial.println("End Of Frame (EOF). Ending StateMachine");
//    return -1;
//  }

  while(event != EVENT_NA)
  {
    Serial.println("1. Inside While loop in exec state machine");
    Serial.printf("2. gCurrentState : %d, event : %d\n\n", gCurrentState,event);

    if(gJumpSm[gCurrentState][event].nextState > STATE_NA && gJumpSm[gCurrentState][event].nextState < STATE_MAX)
    {
      //Serial.println("3. Inside if condition in exec state machine");
      int currState = gCurrentState;  // temp hold the current state
      int currEvent = event;          // temp hold the current event
      
      Serial.printf("4. currState : %s, currEvent : %s, NextState : %s\n\n", gStates[currState], gEvents[currEvent], gStates[gJumpSm[currState][currEvent].nextState]);
      gPrevState = currState;
      gPrevEvent = currEvent;
 
      gCurrentState = gJumpSm[gCurrentState][event].nextState;
      
      if(gJumpSm[currState][event].operation != NULL)
      {
        event = gJumpSm[currState][currEvent].operation();
        Serial.printf("returned Event : %s\n\n",gEvents[event]);
      }else{
        event = EVENT_NA;
      }
    }else {
      Serial.println("No STATE condition.");
      break;
    }
  }
  
  return gCurrentState;
}
