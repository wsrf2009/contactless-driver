



#ifndef TYPEB_H
#define TYPEB_H



#include "common.h"




#define TYPEB_106TX    0x10
#define TYPEB_106RX    0x11
#define TYPEB_212TX    0x12
#define TYPEB_212RX    0x13
#define TYPEB_424TX    0x14
#define TYPEB_424RX    0x15
#define TYPEB_848TX    0x16
#define TYPEB_848RX    0x17

// b5 = 1, extended ATQB not supported by the PCD(ISO 14443-3:2011)
#define PICC_REQB      0x10    // b4 = 0, REQB, request normal
#define PICC_WUPB      0x18    // b4 = 1, WUPB, request wakeup


#define N_1_SLOT       0x00
#define N_2_SLOT       0x01
#define N_4_SLOT       0x02
#define N_8_SLOT       0x03
#define N_16_SLOT      0x04



void PollTypeBTags(void);
UINT8 PiccHaltB(UINT8 *pupi);
UINT8 PiccRequestB(UINT8 reqCmd, UINT8 N);



#endif

