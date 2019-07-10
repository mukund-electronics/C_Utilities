/*!
 * \file staticQueue.c
 * \brief Source file for Static queue implementation
 *
 * Refer to header file for detailed description.
 *
 * Revision History:
 * Date		    User		Comments
 * 19-Dec-18	Saurabh S	Create Original
 */

#include "staticQueue.h"

/*Limit of queue size*/
#define STATIC_QUEUE_MAX_LEN    50

#define STATIC_QUEUE_OBJ_VALID_MARK 0xAA55AA55

/**
 * Structure to represent a queue object
 */
typedef struct StaticQueue_s {
    UInt32_t validMark;        /*Validation Marker, to detect incorrect pointer*/
    UInt32_t queueLen;          /*Length of queue*/
    UInt32_t queueDataSize;     /*Size of data associated with queue*/
    UInt32_t queueWritePtr;     /*Index where new element to be added*/
    UInt32_t queueReadPtr;      /*Index where last element to be read from*/
    UInt32_t queueMsgCount;     /*Current message count*/
    UInt32_t queueHighMark;     /*Highest message count reached*/
    UInt32_t queueDropCount;    /*Number of messages dropped due to Full Queue*/
    Void   * queueData;         /*Dynamic allocated memory for queue*/
} StaticQueue_t;

Void *staticQueueCreate(UInt32_t queueLen, UInt32_t dataSize)
{
    StaticQueue_t *sQ = calloc(1, sizeof(StaticQueue_t));
    if (NULL == sQ) {
        return NULL;
    }
    /*Allocate memory for the data array*/
    Void *qData = calloc(queueLen, dataSize);
    if (NULL == qData) {
        free(sQ);
        return NULL;
    }
    /*Initialise structure*/
    sQ->validMark = STATIC_QUEUE_OBJ_VALID_MARK;
    sQ->queueLen = queueLen;
    sQ->queueDataSize = dataSize;
    sQ->queueWritePtr = 0;
    sQ->queueReadPtr = 0;
    sQ->queueMsgCount = 0;
    sQ->queueHighMark = 0;
    sQ->queueDropCount = 0;
    sQ->queueData = qData;

    return (Void *)sQ;
}

ErrorType_e staticQueueDestroy(Void *qObjPtr)
{
    StaticQueue_t *qObj = (StaticQueue_t *)qObjPtr;

    if (NULL == qObj)
        return E_NULL;

    if (qObj->validMark != STATIC_QUEUE_OBJ_VALID_MARK)
        return E_QUEUE_INVALID;

    qObj->validMark = 0; /*Invalidate Mark for safety*/
    free(qObj->queueData);
    free(qObj);

    return E_SUCCESS;
}

ErrorType_e staticQueuePut(Void *qObjPtr, Void *data)
{
    StaticQueue_t *qObj = (StaticQueue_t *)qObjPtr;

    if (NULL == qObj || NULL == data)
        return E_NULL;

    if (qObj->validMark != STATIC_QUEUE_OBJ_VALID_MARK)
        return E_QUEUE_INVALID;

    if (qObj->queueMsgCount == qObj->queueLen) {
        qObj->queueDropCount++;
        return E_QUEUE_FULL;
    }
    /*Copy data on to queue*/
    memcpy(qObj->queueData + (qObj->queueWritePtr * qObj->queueDataSize), data, qObj->queueDataSize);

    qObj->queueWritePtr++; //Increment Write Ptr
    /*Rotate Write Ptr if it has reached end*/
    if (qObj->queueWritePtr == qObj->queueLen) {
        qObj->queueWritePtr = 0;
    }
    qObj->queueMsgCount++; //Increment Msg Count
    /*Check queue sanity in case queue is full*/
    if (qObj->queueMsgCount == qObj->queueLen) {
        if (qObj->queueWritePtr != qObj->queueReadPtr) {
            return E_BUG;
        }
    }
    return E_SUCCESS;
}

ErrorType_e staticQueueGet(Void *qObjPtr, Void *data)
{
    StaticQueue_t *qObj = (StaticQueue_t *)qObjPtr;

    if (NULL == qObj || NULL == data)
        return E_NULL;

    if (qObj->validMark != STATIC_QUEUE_OBJ_VALID_MARK)
        return E_QUEUE_INVALID;

    if (qObj->queueMsgCount == 0) {
        return E_QUEUE_EMPTY;
    }
    /*Copy data from queue*/
    memcpy(data, qObj->queueData + (qObj->queueReadPtr * qObj->queueDataSize), qObj->queueDataSize);

    qObj->queueReadPtr++; //Increment Read Ptr
    /*Rotate Read Ptr if it has reached end*/
    if (qObj->queueReadPtr == qObj->queueLen) {
        qObj->queueReadPtr = 0;
    }
    qObj->queueMsgCount--; //Decrement Msg Count
    /*Check queue sanity in case queue is empty*/
    if (qObj->queueMsgCount == 0) {
        if (qObj->queueWritePtr != qObj->queueReadPtr) {
            return E_BUG;
        }
    }
    return E_SUCCESS;
}

UInt32_t staticQueueMaxLen(Void *qObjPtr)
{
    StaticQueue_t *qObj = (StaticQueue_t *)qObjPtr;

    if (NULL == qObj)
        return E_NULL;

    if (qObj->validMark != STATIC_QUEUE_OBJ_VALID_MARK)
        return E_QUEUE_INVALID;

    return qObj->queueLen;
}

UInt32_t staticQueueMsgCount(Void *qObjPtr)
{
    StaticQueue_t *qObj = (StaticQueue_t *)qObjPtr;

    if (NULL == qObj)
        return E_NULL;

    if (qObj->validMark != STATIC_QUEUE_OBJ_VALID_MARK)
        return E_QUEUE_INVALID;

    return qObj->queueMsgCount;
}

UInt32_t staticQueueHighMark(Void *qObjPtr)
{
    StaticQueue_t *qObj = (StaticQueue_t *)qObjPtr;

    if (NULL == qObj)
        return E_NULL;

    if (qObj->validMark != STATIC_QUEUE_OBJ_VALID_MARK)
        return E_QUEUE_INVALID;

    return qObj->queueHighMark;
}

UInt32_t staticQueueDropCount(Void *qObjPtr)
{
    StaticQueue_t *qObj = (StaticQueue_t *)qObjPtr;

    if (NULL == qObj)
        return E_NULL;

    if (qObj->validMark != STATIC_QUEUE_OBJ_VALID_MARK)
        return E_QUEUE_INVALID;

    return qObj->queueDropCount;
}
