/*!
 * \file genericLL.c
 * \brief Source file for creating Generic Single LinkList
 * 
 * Generic LinkList to create Singly Linklists and perform Various operations. 
 * 
 * Revision History
 * Date        User              Comments
 * 4-Apr-19   Mukund A        Create original
 */

#include <stdlib.h>
#include <string.h>

#include "genericLL.h"

typedef struct ListNode_s{
  void *data;
  struct ListNode_s *next;
} ListNode_t;

typedef struct ListHandle_s{
  ListNode_t *head;
  int listCount;
  int highMark;
  int listLimit;
  int elementSize;
  int addCount;
  int delCount;
} ListHandle_t;


ListNode_t* sortMerge(ListNode_t*, ListNode_t*, sListComparator_f);

void sNodeSplit(ListNode_t*, ListNode_t**, ListNode_t**);

void *sListNew(int elementSize)
{
    ListHandle_t *newList = malloc(sizeof(ListHandle_t));  // check whehter we got the malloc
    newList->elementSize = elementSize;  // unsigned int...return NULL
    newList->head      = NULL;
    newList->listCount = 0;
    newList->highMark  = 0;
    newList->listLimit = 0;
    newList->addCount  = 0;
    newList->delCount  = 0;
    return newList;
}

void sListItemAdd(void *list, void *element)
{
    // check the pointers not to be NULL - list and element
    if(list == NULL || element == NULL) {
        return;
    }
    ListNode_t *node = malloc(sizeof(ListNode_t));  // check malloc
    if(node == NULL) {
        return;
    }
    node->data = malloc( ((ListHandle_t *)list)->elementSize);  // check malloc
    if(node->data == NULL) {
        return;
    }
    memcpy(node->data, element, ((ListHandle_t *)list)->elementSize);
    //if(0 == ((ListHandle_t *)list)->listCount) {
    if(!((ListHandle_t *)list)->head) {
        ((ListHandle_t *)list)->head = node;
    } else {
        node->next = ((ListHandle_t *)list)->head;
        ((ListHandle_t *)list)->head = node;
    }
    ((ListHandle_t *)list)->listCount++;
    ((ListHandle_t *)list)->addCount++;
    if(((ListHandle_t *)list)->listCount > ((ListHandle_t *)list)->highMark)
        ((ListHandle_t *)list)->highMark++;
}

int sListGetCount(void *list)
{
    if(list != NULL)
        return ((ListHandle_t *)list)->listCount;
    else
        return -1;
}

int sListGetHighMark(void *list)
{
    if(list != NULL)
        return ((ListHandle_t *)list)->highMark;
    else
        return -1;
}

void sListReset(void *list)
{
    if(list == NULL)
        return;

    ListNode_t* currentNode;
    while(((ListHandle_t *)list)->head != NULL) {
        currentNode = ((ListHandle_t *)list)->head;
        ((ListHandle_t *)list)->head = currentNode->next;

        free(currentNode->data);
        free(currentNode);
    }
    ((ListHandle_t *)list)->addCount = 0;
    ((ListHandle_t *)list)->listCount = 0;
    ((ListHandle_t *)list)->delCount = 0;
}

void sListDestroy(void *list)
{
    if(list == NULL)
        return;
    sListReset(list);
    free((ListHandle_t *)list);
    list = NULL;
}

void sListIterator(void *list, sListIterator_f dataFn)
{
    if(list == NULL)
        return;
    ListNode_t *node = ((ListHandle_t *)list)->head;
    while(node != NULL) {
        dataFn(node->data);     // iterateInt(data);
        node = node->next;
    }
}

void sListItemDel(void *list, void *key, sListFindKey_f keyFn)
{
    if(list == NULL || key == NULL)
        return;

    ListNode_t *node = ((ListHandle_t *)list)->head;
    ListNode_t *prev = NULL;
    int result = 0;
    if(node != NULL && 1 == keyFn(node->data, key)) {
        ((ListHandle_t *)list)->head = node->next;
        free(node->data);
        free(node);
        // Decrease the ListCount
        ((ListHandle_t *)list)->delCount++;
        ((ListHandle_t *)list)->listCount--;
        return;
    }
    while(node != NULL) {
        result = keyFn(node->data, key);     // iterateInt(data);
        if(1 == result) {
            prev->next = node->next;

            free(node->data);
            free(node);

            // Decrease the list count
            ((ListHandle_t *)list)->delCount++;
            ((ListHandle_t *)list)->listCount--;
        }
        prev = node;
        node = node->next;
    }
}

void *sListItemGet(void *list, void *key, sListFindKey_f keyFn)
{
    if(list == NULL)
        return NULL;
    ListNode_t *node = ((ListHandle_t *)list)->head;
    int result = 0;
    while(node != NULL) {
        result = keyFn(node->data, key);     // iterateInt(data);
        if(1 == result) {
            return node->data;
        }
        node = node->next;
    }
    return NULL;
}

void sListSort(void *list, sListComparator_f cmpFn)
{
    if(list == NULL)
        return;
    ListNode_t *headRef =  ((ListHandle_t *)list)->head;
    ListNode_t *firstHalf;
    ListNode_t *secondHalf;
    
    // Base case -- length 0 or 1
    if((headRef == NULL) || (headRef->next == NULL)) {
        return;
    }
    // Split Head in two Halfs
    sNodeSplit(headRef, &firstHalf, &secondHalf);
    
    // Recursively sort the sublists
    sListSort(&firstHalf, cmpFn);
    sListSort(&secondHalf, cmpFn);

    // Merge the two sorted lists together
    ((ListHandle_t *)list)->head = sortMerge(firstHalf, secondHalf, cmpFn);
    
}

ListNode_t* sortMerge(ListNode_t* firstHalf, ListNode_t* secondHalf, sListComparator_f cmpFn)
{
    ListNode_t* result = NULL;
         
    // Base Cases
    if(firstHalf == NULL) {
        return secondHalf;
    } else if (secondHalf == NULL) {
        return firstHalf;
    }
    
    // pick either firstHalf or secondHalf and recur
    // use comparator function
    int resCompare = (cmpFn(firstHalf->data, secondHalf->data));
    if(-1 == resCompare) {
        result = firstHalf;
        result->next = sortMerge(firstHalf->next, secondHalf, cmpFn);
    } else if(1 == resCompare || 0 == resCompare) {
        result = secondHalf;
        result->next = sortMerge(firstHalf, secondHalf->next, cmpFn);
    }
    return result;
}

void sNodeSplit(ListNode_t* source, ListNode_t** frontRef, ListNode_t** backRef)
{
    ListNode_t* fast;
    ListNode_t* slow;

    slow = source;
    fast = source->next;

    // Advance 'fast' two nodes and advance slow one node
    while(fast != NULL) {
        fast = fast->next;
        if(fast != NULL) {
            slow = slow->next;
            fast = fast->next;
        }
    }

    // Slow is before the midpoint in the list, so split at that point
    *frontRef = source;
    *backRef = slow->next;
    slow->next = NULL;
}


void sListReverse(void *list)
{
    if(list == NULL)
        return;
    ListNode_t *curr= ((ListHandle_t *)list)->head;
    ListNode_t *prev = NULL;
    ListNode_t *next = NULL;

    while(curr != NULL) {
        // Store next
        next = curr->next;

        // Reverse current node Pointer
        curr->next = prev;

        //Move pointers one position ahead
        prev = curr;
        curr = next;
    }
    ((ListHandle_t *)list)->head = prev;
}
