/*
* Name: common head file
* Date: 2012/12/04
* Author: Alex Wang
* Version: 1.0
*/


#ifndef COMMON_H
#define COMMON_H

#include <linux/bitops.h>

typedef unsigned char    UINT8;
typedef signed char      INT8;
typedef unsigned short   UINT16;
typedef signed short     INT16;
typedef unsigned int     UINT32;
typedef signed int       INT32;
typedef signed long      INT64;
typedef unsigned long    UINT64;


typedef UINT8    BOOL;



#ifndef TRUE
#define TRUE   1
#endif

#ifndef FALSE
#define FALSE  0
#endif



#define SET_BIT(reg, mask)       (reg  |= mask)
#define CLEAR_BIT(reg, mask)     (reg  &= (~mask))
#define TOGGLE_BIT(reg, mask)    (reg  ^= mask)
#define BITISSET(reg, mask)      (reg  &  mask)
#define BITISCLEAR(reg, mask)    ((reg &  mask) == 0)


#define MAKEWORD(msb, lsb)      (((UINT16)msb <<8) | (UINT16)lsb)
#define MAKEUINT32(msb,midb,mida,lsb)      (((UINT32)msb << 24) | ((UINT32)midb << 16)\
                                         | ((UINT32)mida << 8) | (UINT32)lsb)

//#define BIT(n)    (1 << n)



#define OK    0





#endif

