



#ifndef TYPEB_H
#define TYPEB_H



#include "common.h"



// b5 = 1, extended ATQB not supported by the PCD(ISO 14443-3:2011)
#define PICC_REQB      0x10    // b4 = 0, REQB, request normal
#define PICC_WUPB      0x18    // b4 = 1, WUPB, request wakeup


#define N_1_SLOT       0x00
#define N_2_SLOT       0x01
#define N_4_SLOT       0x02
#define N_8_SLOT       0x03
#define N_16_SLOT      0x04



int typeB_request(struct picc_device *picc, u8 reqCmd, u8 N);
int typeB_halt(struct picc_device *picc);
void typeB_polling_tags(struct picc_device *picc);



#endif

