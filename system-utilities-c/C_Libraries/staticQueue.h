/*!
 * \file staticQueue.h
 * \brief Implementation of queue based on static preallocated memory
 *
 * This impementation is to support generic queue implementation.
 *
 * Revision History:
 * Date		    User		Comments
 * 19-Dec-18	Saurabh S	Create Original
 */
#ifndef __STATIC_QUEUE_H__
#define __STATIC_QUEUE_H__

#include "platform.h"

/*!
 *  \fn staticQueueCreate
 *  \brief Creates a new static queue
 *  \param queueLen Length of queue
 *  \param dataSize Size of data to be stored in queue
 *  \return Pointer to queue object on success, NULL on error
 */
Void *staticQueueCreate(UInt32_t queueLen, UInt32_t dataSize);

/*!
 *  \fn staticQueueDestroy
 *  \brief Destroys a static queue, all stored data will be lost
 *  \param qObj queue object pointer
 *  \return Returns Error code
 */
ErrorType_e staticQueueDestroy(Void *qObj);

/*!
 *  \fn staticQueuePut
 *  \brief Add a new element to queue
 *  \param qObj queue object pointer
 *  \param data pointer to data to be added to queue
 *  \return Returns Error code
 */
ErrorType_e staticQueuePut(Void *qObj, Void *data);

/*!
 *  \fn staticQueueGet
 *  \brief Get an element from queue
 *  \param qObj queue object pointer
 *  \param data pointer where data will be copied
 *  \return Returns Error code
 */
ErrorType_e staticQueueGet(Void *qObj, Void *data);

/*!
 *  \fn staticQueueMaxLen
 *  \brief Get max length of queue
 *  \param qObj queue object pointer
 *  \return Returns max length
 */
UInt32_t staticQueueMaxLen(Void *qObj);

/*!
 *  \fn staticQueueMsgCount
 *  \brief Get number of messages in queue
 *  \param qObj queue object pointer
 *  \return Returns msg count
 */
UInt32_t staticQueueMsgCount(Void *qObj);

/*!
 *  \fn staticQueueWaterMark
 *  \brief Get max msg ever pending in the queue
 *  \param qObj queue object pointer
 *  \return Returns water mark vaue
 */
UInt32_t staticQueueHighMark(Void *qObj);

/*!
 *  \fn staticQueueDropCount
 *  \brief Get count of messages dropped
 *  \param qObj queue object pointer
 *  \return Returns drop count
 */
UInt32_t staticQueueDropCount(Void *qObj);

#endif

