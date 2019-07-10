#ifndef __JUMPSTATEMACHINE_H__
#define __JUMPSTATEMACHINE_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  EVENT_NA = 0,            //  0  
  EVENT_INIT_BLDC,         //  1
  EVENT_INIT_SM,           //  2
  EVENT_INIT_HV,           //  3
  EVENT_END_HV,            //  4
  EVENT_END_SM,            //  5
  EVENT_END_BLDC,          //  6
  EVENT_START_MIST_CYCLE,  //  7
  EVENT_OFF_MIST,          //  8
  EVENT_END_MIST,          //  9
  EVENT_COM_MIST_CYCLE,    //  10
  EVENT_MIST_END_DELAY,    //  11
  EVENT_MAX                //  12
}jumpEvents_e;


int initJumpStateMachine();

int executeStateMachine(jumpEvents_e);


#ifdef __cplusplus
} // extern "C"
#endif

#endif // __JUMPSTATEMACHINE_H__
