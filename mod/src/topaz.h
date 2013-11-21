



#ifndef TOPAZ_H
#define TOPAZ_H


#include "common.h"




void topaz_polling_tags(struct picc_device *picc);
int topaz_xfr_handler(struct picc_device *picc, u8 *cmdBuf, u32 cmdLen, u8 *resBuf, u32 *resLen);



#endif
