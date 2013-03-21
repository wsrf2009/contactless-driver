/*
* Name: Debug Message
* Date: 2012/10/08
* Author: Alex Wang
* Version: 1.0
*/

#ifndef DEBUG_H
#define DEBUG_H

#include <linux/kernel.h>

#define Debug

#define DBGL1 1
#define DBGL2 2
#define DBGL3 3
#define DBGL4 4
#define DBGL5 5
#define DBGL6 6


#ifdef Debug
#define PrtMsg(p, arg...)               \
({                                      \
    if(p > 0)    printk(arg);           \
                                        \
}) 
#else
#define PrtMsg(p, arg...)
#endif





#endif
