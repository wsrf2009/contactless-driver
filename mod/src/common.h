

#ifndef COMMON_H
#define COMMON_H

#include <linux/bitops.h>



typedef u8    BOOL;



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


#define MAKEWORD(msb, lsb)      (((u16)msb <<8) | (u16)lsb)
#define MAKEUINT32(msb,midb,mida,lsb)      (((u32)msb << 24) | ((u32)midb << 16)\
                                         | ((u32)mida << 8) | (u32)lsb)

//#define BIT(n)    (1 << n)



//#define OK    0





#endif

