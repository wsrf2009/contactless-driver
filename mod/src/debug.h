
#ifndef DEBUG_H
#define DEBUG_H

#include <linux/kernel.h>

#define Debug

#define LEVEL1 1
#define LEVEL2 2
#define LEVEL3 3
#define LEVEL4 4



#ifdef Debug
#define pprintk(p, arg...)               \
({                                      \
    if(p > 0)    printk(arg);           \
                                        \
}) 
#else
#define pprintk(p, arg...)
#endif

#define TRACE_TO(arg...)		pprintk(LEVEL3,	arg)
#define WARN_TO(arg...)			pprintk(LEVEL2, arg)
#define	ERROR_TO(arg...)		pprintk(LEVEL1, arg)
#define INFO_TO(arg...)			pprintk(LEVEL4, arg)

#endif
