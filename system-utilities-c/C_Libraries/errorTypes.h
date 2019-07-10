/*!
 *  \file errorTypes.h
 *  \brief System vide error types
 *
 *  Revision History:
 *  Date		User		Comments
 *  21-Sep-18	Saurabh S	Create Original
 */

#ifndef __ERROR_TYPES_H__
#define __ERROR_TYPES_H__

typedef enum ErrorType {
    E_MAX = -5000,
    /* General Errors */
    E_NOMEM,                    /*Dynamic Memory exhausted*/
    E_NULL,                     /*NULL Pointer execption*/
    E_BUG,                      /*Implementation Bug Detected*/
    E_INTERNAL,                 /*Internal Error from library*/

    /* Timer Related Errors*/
    E_TMR_READY = -4500,
    E_TMR_NOT_READY,
    E_TMR_NOT_EXIST,
    E_TMR_OUT_OF_BOUND,
    E_TMR_TOO_SMALL,
    E_TMR_RUNNING,
    E_TMR_PAUSED,
    E_TMR_NOT_PAUSED,
    E_TMR_NOT_RUNNING,
    E_TMR_MAX,

    /*Event Related Errors*/
    E_EVT_EXIST = -4000,        /*Event already registered*/
    E_EVT_OUT_OF_BOUNDS,        /*Event is greater than Max event*/
    E_EVT_NOT_EXIST,            /*Event is not registered*/
    E_EVT_QUEUE_FULL,           /*Event queue is full*/
    E_EVT_NOT_READY,            /*Event Fw not initialised*/
    E_EVT_READY,                /*Event Fw already initialised*/

    /*Queue Related Errors*/
    E_QUEUE_SIZE_RESTRICTION = -3500,   /*Size Restriction on max length*/
    E_QUEUE_INVALID,            /*Invalid queue Object*/
    E_QUEUE_FULL,               /*Queue is Full*/
    E_QUEUE_EMPTY,              /*Queue is Empty*/

    E_SUCCESS = 0
} ErrorType_e;

#endif
