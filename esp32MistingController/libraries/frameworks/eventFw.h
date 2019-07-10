/*!
 * \file eventFw.h
 * \brief Event framework to send events between multiple contexts
 *
 * Event framework to send events or message between two contexts.
 * Following is the principle of operations:
 * - Main task / context intend to receive events from high priority tasks /
 *   context, in order to take actions or proessing which is not to be
 *   performed in high prio context, e.g. interrupt handler can generate an
 *   event, which can later be processed in the main context.
 * - Main task registers callbacks corresponding to events
 * - Main task polls the event queue in a while loop
 * - High Prio task generate event.
 * - Main tasks processes the event after polling the event queue via
 *   registered callback
 *
 * Revision History:
 * Date		    User		Comments
 * 19-Dec-18	Saurabh S	Create Original
 */
#ifndef __EVENT_FW_H__
#define __EVENT_FW_H__

#include "platform.h"

typedef Void(*EventCallback_t)(UInt32_t eventId, Void *arg, UInt32_t paramLen);

/*!
 *  \fn eventFwInit
 *  \brief Initialise the event framework
 *  \param maxEvents Maximum number of events to be supported
 *  \param maxQueue Maximum number of events to be queued
 *  \return error or success
 */
ErrorType_e eventFwInit(UInt32_t maxEvents, UInt32_t maxQueue);

/*!
 *  \fn eventFwProcess
 *  \brief To poll for events and process them via registered callbacks
 *  \return Number of events processed
 */
UInt32_t eventFwProcess();

/*!
 *  \fn eventAddHandler
 *  \brief Registers an event handler
 *  \param event Event Identifier
 *  \param cb Callback function to process event
 *  \return Returns ErrorType
 */
ErrorType_e eventAddHandler(UInt32_t event, EventCallback_t cb);

/*!
 *  \fn eventGenerate
 *  \brief Generates an event
 *  \param event Event Identifier
 *  \param param Pointer to any parameter to be passed or NULL
 *  \param paramLen Length of parameter or 0
 *  \return Returns ErrorType
 */
ErrorType_e eventGenerate(UInt32_t event, Void *param, UInt32_t paramLen);

/*!
 *  \fn eventQueueHighWaterMark
 *  \brief Get maximum number of events queued at any point in time
 *  \return Returns high watermark value
 */
UInt32_t eventQueueHighWaterMark();

/*!
 *  \fn eventDropCount
 *  \brief Get total number of events dropped due to queue size limitation
 *  \return Returns drop count */
UInt32_t eventDropCount();

/*!
 *  \fn eventProcessCount
 *  \brief Get total number of events processed till now
 *  \return Returns process count */
UInt32_t eventProcessCount();
#endif

