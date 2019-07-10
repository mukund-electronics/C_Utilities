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
