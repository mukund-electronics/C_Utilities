/*!
 * \file genericLL.h
 * \brief Generic SIngle LinkList
 *
 * Generic LinkList to create dynamic linklist and perform various generic operations.
 *
 * Revision History:
 * Date          User             Comments
 * 4-Apr-19    Mukund A        Create Original
 */
#ifndef __G_LINKLIST__
#define __G_LINKLIST__

typedef int (*sListFindKey_f)(void* data, void* key);

typedef int (*sListComparator_f)(void* left, void* right);

typedef void (*sListIterator_f)(void* data);

/*!
 *  \fn sListNew
 *  \brief Create new Single LinkList
 *  \param Element size for Dynamic memory allocation
 *  \return Returns object pointer
 */
void *sListNew(int elementSize);

/*!
 *  \fn sListReset
 *  \brief Delete all Nodes in LinkList and free up memory
 *  \param Handle of LinkList
 */
void sListReset(void *list);

/*!
 *  \fn sListDestroy
 *  \brief Delete all Nodes in LinkList and list Handler
 *  \param Handle of LinkList
 */
void sListDestroy(void *list);

/*!
 *  \fn sListItemAdd
 *  \brief Add new element by adding new node
 *  \param Handle of LinkList
 *  \param Element to be added
 */
void sListItemAdd(void *list, void *element);

/*!
 *  \fn sListItemDel
 *  \brief Delete Item from LinkList
 *  \param Handle of LinkList
 *  \param Element to be deleted
 *  \param Reference function
 */
void sListItemDel(void *list, void *key, sListFindKey_f keyFn);

/*!
 *  \fn sListItemGet
 *  \brief Search for element
 *  \param Handle of LinkList
 *  \param Element to be search
 *  \param Reference function
 *  \return Returns the required Node
 */
void *sListItemGet(void *list, void *key, sListFindKey_f keyFn);

/*!
 *  \fn sListSort
 *  \brief Sort the LinkList
 *  \param Handle of LinkList
 *  \param Comparator function for reference
 */
void sListSort(void *list, sListComparator_f cmpFn);

/*!
 *  \fn sListReverse
 *  \brief Reverse the LinkList
 *  \param Handle of LinkList
 */
void sListReverse(void *list);

/*!
 *  \fn sListGetCount
 *  \brief Get the Current number of Nodes in the LinkList
 *  \param Handle of LinkList
 *  \return Returns the count as integer value
 */
int sListGetCount(void *list);

/*!
 *  \fn sListGetHighMark
 *  \brief Get the Maximum number of Nodes
 *  \param Handle of LinkList
 *  \return Returns the highMark as Integer value
 */
int sListGetHighMark(void *list);

/*!
 *  \fn sListIterator
 *  \brief Traverse each node in LinkList and provide the address to user. User can use this address to print or modify data in each node.
 *  \param Handle of LinkList
 *  \param Comparison funtion pointer to call and pass the value
 */
void sListIterator(void *list, sListIterator_f dataFn);
#endif
