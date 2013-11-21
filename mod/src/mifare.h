




#ifndef MIFARE_H
#define MIFARE_H



#include "common.h"




#define PICC_MF_KEY_A           0x60    // authentication using key A
#define PICC_MF_KEY_B           0x61    // authentication using key B
#define PICC_MF_READ            0x30    // read block
#define PICC_MF_WRITE_16_BYTES  0xA0    // 16 bytes write block
#define PICC_MF_WRITE_4_BYTES   0xA2    // 4 bytes write block
#define PICC_MF_INCREMENT       0xC1    // increment value
#define PICC_MF_RESTORE         0xC2    // restore value
#define PICC_MF_DECREMENT       0xC0    // decrement value
#define PICC_MF_TRANSFER        0xB0    // transfer


#define SLOT_ERROR    0x01




int mifare_select(struct picc_device *picc);
int mifare_pcsc_command(struct picc_device *picc, u8 *senBuf, u32 senLen, u8 *recBuf, u32 *recLen);
void mifare_type_coding(struct picc_device *picc);


#endif


