/*!
 * \file eventFw.c
 * \brief Source file for Event framework
 *
 * Event framework to send events or message between two contexts.
 * For details refer to description in header file
 *
 * Revision History:
 * Date		    User		Comments
 * 19-Dec-18	Saurabh S	Create Original
 */

#include "eventFw.h"
#include "staticQueue.h"

/**
 * Structure to represent event information. May add stats to it later
 */
typedef struct EventInfo_s {
    EventCallback_t cb;
} EventInfo_t;

/**
 * Structure to represent event
 */
typedef struct Event_s {
    UInt32_t eventId;
    UInt32_t paramLen;
    Void *param;
} Event_t;

static Void *gEventQueue = NULL; /**< Event Queue */
static EventInfo_t *gEventInfoTable = NULL; /*Event Table*/
static UInt8_t gMaxEvents = 0; /**< Maximum number of events */

ErrorType_e eventFwInit(UInt32_t maxEvents, UInt32_t maxQueue)
{
    if (gEventQueue != NULL)
        return E_EVT_READY;

    /*Initialise Event Queue*/
    gEventQueue = staticQueueCreate(maxQueue, sizeof(Event_t));
    if (NULL == gEventQueue)
        return E_INTERNAL;

    /*Initialise Event Info array*/
    gEventInfoTable = (EventInfo_t*)calloc(maxEvents, sizeof(EventInfo_t));
    if (NULL == gEventInfoTable) {
        staticQueueDestroy(gEventQueue);
        gEventQueue = NULL;
        return E_NOMEM;
    }
    gMaxEvents = maxEvents;

    return E_SUCCESS;
}

ErrorType_e eventAddHandler(UInt32_t event, EventCallback_t cb)
{
    if (gEventQueue == NULL)
        return E_EVT_NOT_READY;
    if (event > gMaxEvents - 1)
        return E_EVT_OUT_OF_BOUNDS;
    if (cb == NULL)
        return E_NULL;
    if (gEventInfoTable[event].cb != NULL)
        return E_EVT_EXIST;

    gEventInfoTable[event].cb = cb;

    return E_SUCCESS;
}

ErrorType_e eventGenerate(UInt32_t event, Void *param, UInt32_t paramLen)
{
    Event_t evt;

    if (gEventQueue == NULL)
        return E_EVT_NOT_READY;
    if (event > gMaxEvents - 1)
        return E_EVT_OUT_OF_BOUNDS;
    if (gEventInfoTable[event].cb == NULL)
        return E_EVT_NOT_EXIST;

    /*Populate Event structure*/
    evt.eventId = event;
    evt.paramLen = paramLen;
    if (paramLen) {
        /*Allocate Memory for param*/
        evt.param = calloc(1, paramLen);
        if (NULL == evt.param)
            return E_NOMEM;
        memcpy(evt.param, param, paramLen);
    } else {
        evt.param = NULL;
    }
    /*Add event to queue*/
    if (E_SUCCESS != staticQueuePut(gEventQueue, (Void *)&evt)) {
        if (evt.param)
            free(evt.param);
        return E_INTERNAL;
    }
    return E_SUCCESS;
}

UInt32_t eventFwProcess()
{
    UInt32_t evtCount = 0;
    Event_t evt;
    /*Process all events till queue is empty*/
    while (E_SUCCESS == staticQueueGet(gEventQueue, &evt)) {
        evtCount++;
        if (evt.eventId > gMaxEvents - 1)
            return E_BUG;
        if (gEventInfoTable[evt.eventId].cb == NULL)
            return E_BUG;

        /*Invoke Event Callback and pass the parameter received*/
        gEventInfoTable[evt.eventId].cb(evt.eventId, evt.param, evt.paramLen);
        if (evt.paramLen) {
            if (evt.param)
                free(evt.param); /*Release Memory*/
            else
                return 0xFFFFFFFF; /*To Indicate Bug*/
        }
    }
    return evtCount;
}

UInt32_t eventQueueHighWaterMark()
{
    if (gEventQueue == NULL)
        return 0; /*Error Scenario*/
    return staticQueueHighMark(gEventQueue);
}

UInt32_t eventDropCount()
{
    if (gEventQueue == NULL)
        return 0; /*Error Scenario*/
    return staticQueueDropCount(gEventQueue);
}

UInt32_t eventProcessCount()
{
    if (gEventQueue == NULL)
        return 0; /*Error Scenario*/
    return staticQueueMsgCount(gEventQueue);
}



