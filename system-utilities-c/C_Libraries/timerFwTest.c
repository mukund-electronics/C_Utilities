/*!
 * \file main.c
 * \brief Source file to test library implementation
 *
 * Revision History:
 * Date		    User		Comments
 * 21-Sep-18	Saurabh S	Create Original
 */

#include "timerFw.h"

static Bool gFlag = 0;
Void rtCallback(Int32_t timerId, Void *arg)
{
    gFlag = 1;
}

Void callback(Int32_t timerId, Void *arg)
{
    printf("Tick: %d\n", timerExpiryCount(timerId));
}

Int32_t main(Void)
{
    Int32_t timerId[2];

    printf("Timer Fw Test\n");
    timerFwInit(100, 10);

    timerId[0] = timerCreate(TIMER_REALTIME, 5000, True, rtCallback, NULL);
    if (timerId[0] < 0) {
        printf("Error creating timer\n");
        return 1;
    }
    printf("RT Timer Created: %d\n", timerId[0]);
    if (E_SUCCESS != timerStart(timerId[0])) {
        printf("Error starting timer\n");
    }
    printf("RT Timer Started\n");


    timerId[1] = timerCreate(TIMER_LOW_PRIO, 500, True, callback, NULL);
    if (timerId[1] < 0) {
        printf("Error creating timer\n");
        return 1;
    }
    printf("LP Timer Created: %d\n", timerId[1]);
    if (E_SUCCESS != timerStart(timerId[1])) {
        printf("Error starting timer\n");
    }
    printf("LP Timer Started\n");
    while (True) {
        if (gFlag)
            timerFwProcess();
        usleep(1000);
    }
}
