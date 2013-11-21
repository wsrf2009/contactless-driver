


#ifndef TYPEA_H
#define TYPEA_H


#include "common.h"






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


extern const u8 selectCmd[3];


void typeA_halt(struct picc_device *picc);
int typeA_cascade_select(struct picc_device *picc, u8 selCode, u8 *uid);
int typeA_request(struct picc_device *picc, u8 reqCmd);
int typeA_select(struct picc_device *picc);
void typeA_polling_tags(struct picc_device *picc);




#endif

