/*!
 * \file timerFw.h
 * \brief Callback based Timer framework
 *
 * Timer framework to cater multiple timer requirements.
 * It is implemented using a single high precision real timer.
 *
 * Revision History:
 * Date		    User		Comments
 * 21-Sep-18	Saurabh S	Create Original
 * 26-Sep-18    Saurabh S   Moved TimerState_e from header to src
 */
#ifndef __TIMER_FW_H__
#define __TIMER_FW_H__

#include "platform.h"

#define TIMER_REPEAT        True
#define TIMER_SINGLESHOT    False

typedef enum TimerType {
    TIMER_REALTIME = 1,
    TIMER_HIGH_PRIO,
    TIMER_LOW_PRIO,
} TimerType_e;

typedef void(*TimerCallback_t)(Int32_t timerId, Void *arg);

/*!
 *  \fn timerFwInit
 *  \brief Initialise the timer framework
 *  \param minTimerMs Minimum value of timer in milli seconds
 *  \param maxTimers  Maximum number of timers to be supported
 *  \param error or success
 */
ErrorType_e timerFwInit(UInt32_t minTimerMs, UInt8_t maxTimers);

Void timerFwProcess();

/*!
 *  \fn timerCreate
 *  \brief Creates a new timer
 *  Three types of timer are supported - Realtime, High Priority and
 *  Low Priority. Timers can be repetetive as well as single shot.
 *  Please note to use a very small callback for Realtime timer, as
 *  they will be invoked in Interrupt context.
 *  \param type Type of timer - Realtime, High Prio, Low Prio
 *  \param intervalMs Timer interval in milli seconds
 *  \param repeat To make the timer singleshot or repetetive
 *  \param cb Callback function when timer expires
 *  \param arg Argument to be passed to callback function
 *  \return Returns id the new timer object or ErrorType
 */
Int32_t timerCreate(TimerType_e type, UInt32_t intervalMs, Bool repeat, TimerCallback_t cb, Void *arg);
ErrorType_e timerDestroy(Int32_t timerId);
ErrorType_e timer_c_Stop(Int32_t timerId);
ErrorType_e timer_c_Start(Int32_t timerId);
ErrorType_e timerPause(Int32_t timerId);
ErrorType_e timerResume(Int32_t timerId);
ErrorType_e timerUpdateInterval(Int32_t timerId, UInt32_t newIntervalMs);

Int32_t timerInterval(Int32_t timerId);
Int32_t timerRemainingMs(Int32_t timerId);
Int32_t timerExpiryCount(Int32_t timerId);
Int32_t timerType(Int32_t timerId);
Int32_t timerRunning(Int32_t timerId);
Int32_t timerSingleshot(Int32_t timerId);


#endif
