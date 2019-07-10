/*!
 * \file platform.h
 * \brief Platform specific things
 *
 * Revision History:
 * Date		    User		Comments
 * 21-Sep-18	Saurabh S	Create Original
 */

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#define LINUX   1
#define NRF     2
#define ARDUINO 3

#define PLATFORM_TYPE ARDUINO

#if PLATFORM_TYPE == LINUX
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#endif

#if PLATFORM_TYPE == NRF
#include <stdlib.h>
#include <stdio.h>
#include "nrf.h"
#include "app_timer.h"
#include "nrf_drv_clock.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "app_error.h"
#endif

#if PLATFORM_TYPE == ARDUINO

#include <cstdlib>
#include <cstring>
#include "Ticker.h"
#endif

#include "dataTypes.h"
#include "errorTypes.h"

#endif
