/*-------------------------------------------*/
/* Integer type definitions for FatFs module */
/*-------------------------------------------*/

#ifndef _INTEGER
#define _INTEGER

#ifdef WIN32
	#include <windows.h>
#else

#include <stdint.h>

/* These types must be 16-bit, 32-bit or larger integer */
typedef int32_t			INT;
typedef uint32_t		UINT;

/* These types must be 8-bit integer */
typedef int8_t          CHAR;
typedef uint8_t         UCHAR;
typedef uint8_t         BYTE;

/* These types must be 16-bit integer */
typedef int16_t			SHORT;
typedef uint16_t	    USHORT;
typedef uint16_t        WORD;
typedef uint16_t        WCHAR;

/* These types must be 32-bit integer */
typedef long			LONG;
typedef unsigned long	ULONG;
typedef unsigned long	DWORD;

/* Boolean type */
//typedef enum { FALSE = 0, TRUE } BOOL;

#endif


#endif
