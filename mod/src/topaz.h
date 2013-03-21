



#ifndef TOPAZ_H
#define TOPAZ_H


#include "common.h"






//===== Topaz Protocol macros======
#define TOPAZ_RID       0x78
#define TOPAZ_RALL      0x00
#define TOPAZ_READ      0x01
#define TOPAZ_WRITE_E   0x53
#define TOPAZ_WRITE_NE  0x1A



//===== CRC macros======
#define CRC_A    1
#define CRC_B    2


UINT8 TopazTransmissionHandle(UINT8 *cmdBuf, UINT16 cmdLen, UINT8 *resBuf, UINT16 *resLen);
UINT8 TopazXfrHandle(UINT8 *cmdBuf, UINT16 cmdLen, UINT8 *resBuf, UINT16 *resLen);
void PollTopazTags(void);




#endif
