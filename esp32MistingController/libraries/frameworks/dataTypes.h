/*!
 *  \file dataTypes.h
 *  \brief Platform dependent data type definitions
 *
 *  Platform dependent datatypes are defined in this header to make the rest of
 *  code data type independent.
 *
 *  Revision History:
 *  Date		User		Comments
 *  21-Sep-18	Saurabh S	Create Original
 */
#ifndef __DATA_TYPES_H__
#define __DATA_TYPES_H__

#define True                1
#define False               0

#ifndef NULL
#define NULL                0
#endif

typedef unsigned char       Bool;
typedef unsigned char       UChar;
typedef unsigned char       UInt8_t;
typedef unsigned short      UInt16_t;
typedef unsigned int        UInt32_t;
typedef unsigned long       UInt64_t;
typedef char                Char;
typedef char                Int8_t;
typedef short               Int16_t;
typedef int                 Int32_t;
typedef long                Int64_t;
typedef void                Void;


#endif

