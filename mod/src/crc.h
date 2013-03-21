





#ifndef CRC_H
#define CRC_H


#include "common.h"




void ComputeCrc(UINT8 CRCType, UINT8 *Data, unsigned int Length, UINT8 *TransmitFirst, UINT8 *TransmitSecond);


#endif
