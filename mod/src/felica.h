





#ifndef FELICA_H
#define FELICA_H


#include "common.h"




#define PASSDEPI_106    0x20
#define PASSDEPT_106    0x21
#define PASSDEPI_212    0x22
#define PASSDEPT_212    0x23
#define PASSDEPI_424    0x24
#define PASSDEPT_424    0x25
#define ACTDEPI_106     0x26
#define ACTDEPT_106     0x27
#define ACTDEPI_212     0x28
#define ACTDEPT_212     0x29
#define ACTDEPI_424     0x2A
#define ACTDEPT_424     0x2B

//-----------------------FeliCa Commands------------
#define CMD_REQC                 0x00
#define RES_REQC                 0x01
#define CMD_REQSERVICE           0x02
#define RES_REQSERVICE           0x03
#define CMD_REQRESPONSE          0x04
#define RES_REQRESPONSE          0x05
#define CMD_READ_NONEAUTH        0x06
#define RES_READ_NONEAUTH        0x07
#define CMD_WRITE_NONEAUTH       0x08
#define RES_WRITE_NONEAUTH       0x09
#define CMD_AUTH1                0x10
#define RES_AUTH1                0x11
#define CMD_AUTH2                0x12
#define RES_AUTH2                0x13
#define CMD_READFROMSECURE       0x14
#define RES_READFROMSECURE       0x15
#define CMD_WRITETOSECURE        0x16
#define RES_WRITETOSECURE        0x17

#define CODE_TIMESLOTNUMBER_1      0x00    // 0x00, time slot number is 1
#define CODE_TIMESLOTNUMBER_2      0x01    // 0x01, time slot number is 2
#define CODE_TIMESLOTNUMBER_4      0x03    // 0x03, time slot number is 4
#define CODE_TIMESLOTNUMBER_8      0x07    // 0x07, time slot number is 8
#define CODE_TIMESLOTNUMBER_16     0x0F    // 0x0F, time slot number is 16

#define FELICA_APPLICATION_ALL     0xFFFF        // all PICC shall respond when the system code is 'FFFF'

#define FLAG_NOPARITYCHECK  FALSE
#define FLAG_PARITYCHECK    TRUE

#define FELINFFIELDLEN      254    // the legth of Information field of felica, LEN + Data




#define     RF_FEL_CON_LEN      8
#define     FEL2_CON_ModGsP     0
#define     FEL2_CON_RxThres    1
#define     FEL2_CON_RFCfg      2
#define     FEL2_CON_CWGsP      3
#define     FEL4_CON_ModGsP     4
#define     FEL4_CON_RxThres    5
#define     FEL4_CON_RFCfg      6
#define     FEL4_CON_CWGsP      7

extern const UINT8 gaFelRFCon[RF_FEL_CON_LEN];



UINT8 FelTransmisionHandle(UINT8 *cmdBuf, UINT16 cmdLen, UINT8 *resBuf, UINT16 *resLen);
UINT8 FelXfrHandle(UINT8 *cmdBuf, UINT16 cmdLen, UINT8 *resBuf, UINT16 *resLen);
UINT8 FelReqResponse(void);
void PollFeliCaTags(UINT8 feliType);


#endif


