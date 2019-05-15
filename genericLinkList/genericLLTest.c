/*!
 * \file genericLLTest.c
 * \brief Source file to test generic LinkList
 *
 * Revision History:
 * Date		    User		Comments
 * 4-Apr-19	   Mukund A	    Create Original
 */


#include <stdio.h>
#include <stdbool.h>
#include "genericLL.h"

void iterateInt(void *);

int findKey(void *, void *);

int comparatorFn(void *left, void *right);

void main()
{
    // create new linklist
    printf("\n\n\n Sorting List\n\n");
    void *ptrInt3 = sListNew(sizeof(int));
    // insert values
    int arr[] = {13,15,1,5,2,99,45,21,8,12,9};
    int arrSize = sizeof(arr)/sizeof(int);
    printf("array size to be inserted : %d\n", arrSize);
    for(int i = 0; i < arrSize; i++) {
        printf("Value inserting : %d\n", *(arr+i));
        sListItemAdd(ptrInt3, (arr+i));
    }
    // List size
    printf("Node Count in ptrInt3 : %d\n", sListGetCount(ptrInt3));
    // print unsorted list
    printf("\n\n Unsorted List : \n");
    sListIterator(ptrInt3, iterateInt);
    // sort the list
    sListSort(ptrInt3, comparatorFn);
    //print the sorted list
    printf("\n\n Sorted List : \n");
    sListIterator(ptrInt3, iterateInt);

    // get Count
    printf("HighMarkCount : %d\n", sListGetHighMark(ptrInt3));
    printf("Count : %d\n", sListGetCount(ptrInt3));
    // Pop a node
    printf("Delete a node\n");
    int a = 21;
    int b = 45;
    sListItemDel(ptrInt3, &a, findKey);
    sListItemDel(ptrInt3, &b, findKey);

    // get Count
    printf("HighMarkCount : %d\n", sListGetHighMark(ptrInt3));
    printf("Count : %d\n", sListGetCount(ptrInt3));

        sListItemAdd(ptrInt3, &a);
    // get Count
    printf("HighMarkCount : %d\n", sListGetHighMark(ptrInt3));
    printf("Count : %d\n", sListGetCount(ptrInt3));
        sListItemAdd(ptrInt3, &a);
    // get Count
    printf("HighMarkCount : %d\n", sListGetHighMark(ptrInt3));
    printf("Count : %d\n", sListGetCount(ptrInt3));
        sListItemAdd(ptrInt3, &a);
    // get Count
    printf("HighMarkCount : %d\n", sListGetHighMark(ptrInt3));
    printf("Count : %d\n", sListGetCount(ptrInt3));
}

void iterateInt(void *data)
{
    printf("Value : %d\n", *(int *)data);
    //    (*(int *)data)++;                 // User can modify the value
}

int findKey(void *data, void *key)
{
    if(*(int *)data == *(int *)key) {
        return 1;
    } else {
        return 0;
    }
}

int comparatorFn(void *left, void *right)
{
    if(*(int *)left == *(int *)right) {
        return 0;
    } else if (*(int *)left <= *(int *)right) {
        return -1;
    } else {
        return 1;
    }
}
