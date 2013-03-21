


//****************************************************
// Name: ISO 14443 Type A head file
// Date: 2012/12/05
// Author: Alex Wang
// Version: 1.0
//****************************************************


#ifndef TYPEA_H
#define TYPEA_H


#include "common.h"







#define TYPEA_106TX    0x00
#define TYPEA_106RX    0x01
#define TYPEA_212TX    0x02
#define TYPEA_212RX    0x03
#define TYPEA_424TX    0x04
#define TYPEA_424RX    0x05
#define TYPEA_848TX    0x06
#define TYPEA_848RX    0x07

#define PICC_REQA       0x26    // request idle
#define PICC_WUPA       0x52    // request all

#define PICC_SELVL1     0x93    // select1
#define PICC_SELVL2     0x95    // select2
#define PICC_SELVL3     0x97    // select3

#define PICC_HALT       0x50    // halt


//SAK
#define SAK_TYPEA_TCL          0x20  
#define SAK_MIFARE_1K          0x08 
#define SAK_MIFARE_4K          0x18 
#define SAK_MIFARE_ULTRALIGHT  0x00
#define SAK_MIFARE_MINI        0x09
#define SAK_SLE66R35           0x88 


extern const UINT8 selectCmd[3];


void PollTypeATags(void);
UINT8 TypeASelect(void);
UINT8 PcdRequestA(UINT8 reqCmd, UINT8 *pATQ);
UINT8 PiccCascSelect(UINT8 selCode, UINT8 *uid, UINT8 *sak);
void PiccHaltA(void);



#endif

