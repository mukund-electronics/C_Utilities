/*!
 * \file platformTimer.c
 * \brief Source file for platform specific timer implementation
 *
 * Timer framework to cater multiple timer requirements.
 * It is implemented using a single high precision real timer.
 *
 * Revision History:
 * Date		    User		Comments
 * 21-Sep-18	Saurabh S	Create Original
 */


#include "platformTimer.h"

/*Linux Specific IMplementation - Start*/
#if PLATFORM_TYPE == LINUX

typedef Void (*TmrExpiryCb)(Void);
static TmrExpiryCb gExpCb = NULL;
static struct itimerval tval;

static Void handler(Int32_t signo)
{
    if (gExpCb)
        gExpCb();
}

ErrorType_e platformTimerStart(UInt32_t ms, TmrExpiryCb cb)
{
    UInt32_t seconds = ms / 1000;
    UInt32_t microSeconds = (ms % 1000) * 1000;

    gExpCb = cb;
    
	timerclear(& tval.it_interval);	/* zero interval means no reset of timer */
	timerclear(& tval.it_value);

	tval.it_value.tv_sec = seconds;
	tval.it_value.tv_usec = microSeconds;
	tval.it_interval.tv_sec = seconds;
	tval.it_interval.tv_usec = microSeconds;

	(Void) signal(SIGALRM, handler);
	(Void) setitimer(ITIMER_REAL, &tval, NULL);

    return E_SUCCESS;
}

#endif
/*Linux Specific IMplementation - End*/

/*NRF specific implementation -Start*/
#if PLATFORM_TYPE == NRF

#define APP_TIMER_PRESCALER             0                                           /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS            6 		                                    /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE         4                                           /**< Size of timer operation queues. */

typedef Void (*TmrExpiryCb)(Void);
static TmrExpiryCb gExpCb = NULL;

APP_TIMER_DEF(timerId);		//create instance for app timer

const app_timer_mode_t timerMode = APP_TIMER_MODE_REPEATED;
uint32_t gTimeOutValue = 0; 

static Void handler(void *argument)
{
    if (gExpCb)
        gExpCb();
}

static void timersInit(void)
{
	// Initialize timer module, making it use the scheduler
	APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);

	ret_code_t err_code ;
	err_code = app_timer_create(&timerId, timerMode, handler);
	APP_ERROR_CHECK(err_code);

	err_code = app_timer_start(timerId, gTimeOutValue, NULL);
	APP_ERROR_CHECK(err_code);
}

ErrorType_e platformTimerStart(UInt32_t ms, TmrExpiryCb cb)
{		
    gExpCb = cb;
	if(ms < 5) {//minimum tick value is 5 for nrf5x soc 
		return E_MAX;
	}
	gTimeOutValue = APP_TIMER_TICKS(ms, APP_TIMER_PRESCALER);
	
	timersInit(); 
    return E_SUCCESS;
}
#endif
/*NRF specific implementation -End*/

/*Arduino Specific IMplementation - Start*/
#if PLATFORM_TYPE == ARDUINO

typedef Void (*TmrExpiryCb)();
static TmrExpiryCb gExpCb = NULL;

Ticker gTimeKeeper;		/** < Timer Object*/

static void handler()
{
    if (gExpCb)
        gExpCb();
}

ErrorType_e platformTimerStart(UInt32_t ms, TmrExpiryCb cb)
{
    gExpCb = cb;
		
   	gTimeKeeper.attach_ms(ms, handler); 
    
	return E_SUCCESS;
}

#endif
