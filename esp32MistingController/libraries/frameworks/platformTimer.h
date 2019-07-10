/*!
 * \file platformTimer.h
 * \brief Platform specific timer implementation
 *
 * Revision History:
 * Date		    User		Comments
 * 21-Sep-18	Saurabh S	Create Original
 */
#ifndef __PLATFORM_TIMER_H__
#define __PLATFORM_TIMER_H__

#include "platform.h"

typedef Void (*TmrExpiryCb)();
ErrorType_e platformTimerStart(UInt32_t ms, TmrExpiryCb cb);

#endif
