/*!
 * \file staticQueueTest.c
 * \brief Source file to test static queue library implementation
 *
 * Revision History:
 * Date		    User		Comments
 * 19-Dec-18	Saurabh S	Create Original
 */

#include "staticQueue.h"

Void addElements(Void *queue, UInt32_t count)
{
    for (UInt32_t i = 0; i < count; i++) {
        if (E_SUCCESS != staticQueuePut(queue, (void *)&i)) {
            printf("Error adding element\n");
            return;
        }
    }
    printf("%d elements added\n", count);
}

Void getElements(Void *queue, UInt32_t count)
{
    UInt32_t value;
    ErrorType_e err;
    for (UInt32_t i = 0; i < count; i++) {
        err = staticQueueGet(queue, (void *)&value);
        if (E_SUCCESS != err) {
            printf("Error getting element: %d\n", err);
            return;
        } else {
            printf("%d: %d\n", i+1, value);
        }
    }
}

Int32_t main(Void)
{
    UInt32_t data = 0;
    Void *queue = staticQueueCreate(50, sizeof(UInt32_t));

    if (NULL == queue) {
        printf("Error creating queue\n");
        return 1;
    }
    printf("queue created\n");

    addElements(queue, 50);
    getElements(queue, 25);
    addElements(queue, 20);
    getElements(queue, 25);
    addElements(queue, 20);
    getElements(queue, 25);
    addElements(queue, 20);
    getElements(queue, 35);
    if (E_SUCCESS != staticQueueDestroy(queue)) {
        printf("Error Destroting Queue\n");
        return 1;
    }
    printf("Queue Destroyed\n");
    return 0;
}

