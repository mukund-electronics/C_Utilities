/*!
 * \file eventFwTest.c
 * \brief Source file to test Event Framework
 *
 * Revision History:
 * Date		    User		Comments
 * 19-Dec-18	Saurabh S	Create Original
 */

#include "eventFw.h"

typedef enum Events_enum {
    EVT_1 = 0,
    EVT_2,
    EVT_3,
    EVT_MAX
} Events_e;

Void evt1Hdlr(UInt32_t eventId, Void *arg, UInt32_t paramLen)
{
    UInt32_t param = *(UInt32_t *)arg;
    printf("Event Rx: EvtId: %d, ParamVal: %d\n", eventId, param);
}

Int32_t main(Void)
{
    UInt32_t param = 20;
    UInt32_t result = 0;

    if (E_SUCCESS != eventFwInit(EVT_MAX, 10)) {
        printf("Error initialising Event Framework\n");
        return 1;
    }
    printf("Event Framework initialised\n");

    if (E_SUCCESS != eventAddHandler(EVT_1, evt1Hdlr)) {
        printf("Error registering event handler\n");
        return 1;
    }
    printf("Event handler registered\n");
    if (E_SUCCESS != eventGenerate(EVT_1, (Void *)&param, sizeof(UInt32_t))) {
        printf("Error generating event\n");
        return 1;
    }
    printf("Event generated sucessfully\n");
    result = eventFwProcess();
    printf("%d Events processed\n", result);
    return 0;
}


