/*!
 * \file timerFw.c
 * \brief Source file for Callback based Timer framework
 *
 * Timer framework to cater multiple timer requirements.
 * It is implemented using a single high precision real timer.
 *
 * Revision History:
 * Date		    User		Comments
 * 21-Sep-18	Saurabh S	Create Original
 * 26-Sep-18    Saurabh S   Moved TimerState_e from header to src
 */

#include "timerFw.h"
#include "platformTimer.h"

typedef enum TimerState {
	TIMER_INIT = 1,
	TIMER_RUNNING,
	TIMER_PAUSED,
	TIMER_EXPIRED,
} TimerState_e;

/**
 * Structure to represent timer object
 */
typedef struct Timer {
    Bool            repeat;
    UInt8_t         timerId;
    TimerCallback_t cb;
    Void            *arg;
    UInt32_t        ticks;
    UInt32_t        ticksRemaining;
    UInt32_t        expiryCount;
    TimerState_e    state;
    TimerType_e     type;
} Timer_t;

static Timer_t **gTimerCb = NULL; /**< Dynamic array of timer control blocks */
static UInt8_t gMaxTimers = 0; /**< Maximum number of timers allowed */
static UInt32_t gTickMs = 0; /**< Tick ms for one internal timer expiry */
static UInt8_t gTimerCount = 0; /**< Number of active timers */

static Bool gTimerFwInit = False; /**< True when framework is initialised */

/*Internal timer call back function*/
static void pfTmrCb(void)
{
    /*Process Timers*/
    for (int i = 0; i < gMaxTimers; i++) {
        if (gTimerCb[i] != NULL) { //Valid Timer block
            /*Avoid processing of EXPIRED, PAUSED or INIT Timers, Only process RUNNING timers*/
            if (gTimerCb[i]->state == TIMER_RUNNING) {
                /*This check avoid expired single shot timers*/
                if (gTimerCb[i]->ticksRemaining > 0) { //Active Timer
                    gTimerCb[i]->ticksRemaining--; //Reduce tick;
                    /*Check if ticks are zero, means timer expired*/
                    if (gTimerCb[i]->ticksRemaining == 0) { //Timer Expired

                        gTimerCb[i]->expiryCount++; //Increment Expiry Count
                        /*If it is a Realtime Timer, Invoke callback here from interrupt context*/
                        if (gTimerCb[i]->type == TIMER_REALTIME) {
                            if (gTimerCb[i]->cb != NULL) {
                                // Invoke Callback and pass argument
                                gTimerCb[i]->cb(i, gTimerCb[i]->arg);
                            }
                            gTimerCb[i]->expiryCount = 0; //As we have processed the expiry
                        }

                        /*If timer is repetetive, then reload ticks, state will be running*/
                        if (gTimerCb[i]->repeat == True) {
                            gTimerCb[i]->ticksRemaining = gTimerCb[i]->ticks;
                        } else { //Else Singleshot, update state as expired
                            gTimerCb[i]->state = TIMER_EXPIRED;
                        }
                    }
                } else { // If control reaches here then its a Bug
                }
            }
        }
    }

    /*Invoke Realtime timer callbacks from interrupt context*/
    //printf("Timer Tick\n");
}

static Void processNonRtTimers(TimerType_e type)
{
    for (int i = 0; i < gMaxTimers; i++) {
        if (gTimerCb[i] != NULL) { //Valid Timer block
            /*Only process Expired timers which are pending proessing*/
            if (gTimerCb[i]->expiryCount > 0 && gTimerCb[i]->type == type) {
                if (gTimerCb[i]->cb != NULL) {
                    // Invoke Callback and pass argument
                    gTimerCb[i]->cb(i, gTimerCb[i]->arg);
                }
                gTimerCb[i]->expiryCount = 0; // To mark timer expiry as processed
            }
        }
    }
}

static ErrorType_e validateTimer(Int32_t timerId)
{
    /*Validations*/
    if (timerId < 0 || timerId > gMaxTimers - 1) { //Id within Bounds
        return E_TMR_OUT_OF_BOUND;
    } else if (gTimerFwInit == False) { //Timer Framework initialised
        return E_TMR_NOT_READY;
    } else if (gTimerCb[timerId] == NULL) { //Timer block initialised
        return E_TMR_NOT_EXIST;
    }
    return E_SUCCESS;
}

ErrorType_e timerFwInit(UInt32_t minTimerMs, UInt8_t maxTimers)
{
    if (True == gTimerFwInit) {
        return E_TMR_READY; 
    }
    /*Setup Internal Structures and Initialise*/
    gMaxTimers = maxTimers;
    gTickMs = minTimerMs;

    /*Allocate memory to hold max timer block pointers*/
    gTimerCb = (Timer_t**)calloc(maxTimers, sizeof(Timer_t *));

    /*Setup Plaform Timer*/
    platformTimerStart(minTimerMs, pfTmrCb);

    gTimerFwInit = True; /*Timer Initialised*/
    return E_SUCCESS;
}

/*Invoke High Priority and Low Priority Timer Callbacks from main context*/
Void timerFwProcess()
{
    /*Process High Priority Timers first*/
    processNonRtTimers(TIMER_HIGH_PRIO);

    /*We may add some time check to defer processing of Low Prio timers if required*/

    /*Process Low Priority Timer*/
    processNonRtTimers(TIMER_LOW_PRIO);
}

Int32_t timerCreate(TimerType_e type, UInt32_t intervalMs, Bool repeat, TimerCallback_t cb, Void *arg)
{
    if (gTimerCount >= gMaxTimers)
        return E_TMR_MAX;
    else if (intervalMs < gTickMs)
        return E_TMR_TOO_SMALL;

    /*Search timer block array for empty space and setup the timer*/
    //// Deploy some sort of synchronization here, as gTimerCb is shared with Interrupt
    for (int i = 0; i < gMaxTimers; i++) {
        if (NULL == gTimerCb[i]) {
            gTimerCb[i] = (Timer_t*)calloc(1, sizeof(Timer_t));
            /*Initialise Timer Control Block*/
            gTimerCb[i]->repeat = repeat;
            gTimerCb[i]->timerId = i;
            gTimerCb[i]->cb = cb;
            gTimerCb[i]->arg = arg;
            gTimerCb[i]->ticks = intervalMs / gTickMs; /*Number of internal expiries*/
            gTimerCb[i]->ticksRemaining = 0; //Initiate Ticks remaining from timerStart()
            gTimerCb[i]->expiryCount = 0;
            gTimerCb[i]->state = TIMER_INIT;
            gTimerCb[i]->type = type;
			
            gTimerCount++; /*Increment timer count*/
            return i; /*Return timer id*/
        }
    }
    return 0;
}

ErrorType_e timer_c_Start(Int32_t timerId)
{
    ErrorType_e err = validateTimer(timerId);

    /*Validations*/
    if (E_SUCCESS != err) {
        return err;
    } else if (gTimerCb[timerId]->state == TIMER_RUNNING) {
        return E_TMR_RUNNING;
    } else if (gTimerCb[timerId]->state == TIMER_PAUSED) {
        return E_TMR_PAUSED;
    }

    /*Start Timer*/
    gTimerCb[timerId]->ticksRemaining = gTimerCb[timerId]->ticks; //Initiate Ticks remaining from timerStart()
    gTimerCb[timerId]->state = TIMER_RUNNING;
    gTimerCb[timerId]->expiryCount = 0;

    return E_SUCCESS;
}

ErrorType_e timerDestroy(Int32_t timerId)
{
    ErrorType_e err = validateTimer(timerId);
    Void *tmpPtr; //To free timer control block

    /*Validations*/
    if (E_SUCCESS != err) {
        return err;
    } else if (gTimerCb[timerId]->state == TIMER_RUNNING) {
        return E_TMR_RUNNING;
    } else if (gTimerCb[timerId]->state == TIMER_PAUSED) {
        return E_TMR_PAUSED;
    }

    /*Destroy Timer*/
    /// Some sort of syncronisation should be deployed here
    tmpPtr = gTimerCb[timerId];
    gTimerCb[timerId] = NULL;
    free(tmpPtr);
    gTimerCount--;

    return E_SUCCESS;
}

ErrorType_e timer_c_Stop(Int32_t timerId)
{
    ErrorType_e err = validateTimer(timerId);

    /*Validations*/
    if (E_SUCCESS != err)
        return err;

    /*Stop Timer*/
    gTimerCb[timerId]->ticksRemaining = 0;
    gTimerCb[timerId]->expiryCount = 0; //To avoid invocation of callback
    gTimerCb[timerId]->state = TIMER_INIT;

    return E_SUCCESS;
}

ErrorType_e timerPause(Int32_t timerId)
{
    ErrorType_e err = validateTimer(timerId);

    /*Validations*/
    if (E_SUCCESS != err) {
        return err;
    } else if (gTimerCb[timerId]->state != TIMER_RUNNING) {
        return E_TMR_NOT_RUNNING;
    }

    /*Pause Timer*/
    gTimerCb[timerId]->state = TIMER_PAUSED;
    return E_SUCCESS;
}

ErrorType_e timerResume(Int32_t timerId)
{
    ErrorType_e err = validateTimer(timerId);

    /*Validations*/
    if (E_SUCCESS != err) {
        return err;
    } else if (gTimerCb[timerId]->state != TIMER_PAUSED) {
        return E_TMR_NOT_PAUSED;
    }

    /*Resume Timer*/
    gTimerCb[timerId]->state = TIMER_RUNNING;
    return E_SUCCESS;
}

ErrorType_e timerUpdateInterval(Int32_t timerId, UInt32_t newIntervalMs)
{
    ErrorType_e err = validateTimer(timerId);

    /*Validations*/
    if (E_SUCCESS != err) {
        return err;
    } else if (newIntervalMs < gTickMs) {
        return E_TMR_TOO_SMALL;
    }

    gTimerCb[timerId]->ticks = newIntervalMs / gTickMs; /*Number of internal expiries*/
    return E_SUCCESS;
}


Int32_t timerInterval(Int32_t timerId)
{
    ErrorType_e err = validateTimer(timerId);

    /*Validations*/
    if (E_SUCCESS != err) {
        return err; //Return Error code to indicate error
    }

    return gTimerCb[timerId]->ticks * gTickMs;
}

Int32_t timerRemainingMs(Int32_t timerId)
{
    ErrorType_e err = validateTimer(timerId);

    /*Validations*/
    if (E_SUCCESS != err) {
        return err; //Return Error code to indicate error
    } else if (gTimerCb[timerId]->state != TIMER_RUNNING) {
        return E_TMR_NOT_RUNNING;
    }

    return gTimerCb[timerId]->ticksRemaining * gTickMs;
}

Int32_t timerExpiryCount(Int32_t timerId)
{
    ErrorType_e err = validateTimer(timerId);

    /*Validations*/
    if (E_SUCCESS != err) {
        return err; //Return Error code to indicate error
    }

    return gTimerCb[timerId]->expiryCount;
}

Int32_t timerType(Int32_t timerId)
{
    ErrorType_e err = validateTimer(timerId);

    /*Validations*/
    if (E_SUCCESS != err) {
        return err; //Return Error code to indicate error
    }
    return gTimerCb[timerId]->type;
}

Int32_t timerRunning(Int32_t timerId)
{
    ErrorType_e err = validateTimer(timerId);

    /*Validations*/
    if (E_SUCCESS != err) {
        return err; //Return Error code to indicate error
    }
    return (gTimerCb[timerId]->state == TIMER_RUNNING);
}

Int32_t timerSingleshot(Int32_t timerId)
{
    ErrorType_e err = validateTimer(timerId);

    /*Validations*/
    if (E_SUCCESS != err) {
        return err; //Return Error code to indicate error
    }
    return (gTimerCb[timerId]->type == TIMER_SINGLESHOT);
}

