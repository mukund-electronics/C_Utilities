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

#define PLATFORM_TYPE LINUX
//#define PLATFORM_TYPE NRF

#if PLATFORM_TYPE == LINUX
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#endif

#include "dataTypes.h"
#include "errorTypes.h"

#endif
