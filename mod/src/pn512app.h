/*
* Name: RF IC head file
* Date: 2012/12/05
* Author: Alex Wang
* Version: 1.0
*/




#ifndef PN512APP_H
#define PN512APP_H



#include "common.h"


#define CONFIGNOTHING  0
#define CONFIGTYPEA    1
#define CONFIGTYPEB    2


/********************************************/
// PN512 Error Codes
// Each function returns a status value, which corresponds to the
// PN512 error codes. 
#define ERROR_NO                 0x00
#define ERROR_NOTAG              0xFF
#define ERROR_PROTOCOL           0xFE
#define ERROR_PARITY             0xFD
#define ERROR_BUFOVFL            0xFC
#define ERROR_CRC                0xFB
#define ERROR_COLL               0xFA
#define ERROR_SERNR              0xF9
#define ERROR_BYTECOUNT          0xF8
#define ERROR_BITCOUNT           0xF7
#define ERROR_WRONGPARAM         0xF6
#define ERROR_ATSLEN             0xF5
#define ERROR_FSDLENTH           0xF4
#define ERROR_UNKNOW_COMMAND     0xF3
#define ERROR_INVALID_DATA       0xF2
#define ERROR_SPEED              0xF1
#define ERROR_CID                0xF0



#define BModeIndex        0
#define RxAThres106       1
#define RxAThres212       2
#define RxAThres424       3
#define RxAThres848       4
#define RxBThres106       5
#define RxBThres212       6
#define RxBThres424       7
#define RxBThres848       8
#define ARFAmpCfg106      9
#define ARFAmpCfg212      10
#define ARFAmpCfg424      11
#define ARFAmpCfg848      12
#define BRFAmpCfg106      13
#define BRFAmpCfg212      14
#define BRFAmpCfg424      15
#define BRFAmpCfg848      16
#define TypeACWGsP        17
#define TypeBCWGsP        18



extern UINT8 RCRegFactor[19];





UINT8 PcdRawExchange(UINT8 Cmd, UINT8 *senBuf, UINT8 senLen, UINT8 *recBuf, UINT8 *recLen);
UINT8 PcdHandlerCmd(UINT8 Cmd, UINT8 errorCheckFlag);
void PcdConfigIso14443Type(UINT8 flagConfig, UINT8 cardType);
UINT8 GetBitNumbersReceived(void);
void SetRegBit(UINT8 reg, UINT8 bit);
void ClearRegBit(UINT8 reg, UINT8 bit);
UINT8 FIFORead(UINT8 *buf, UINT8 len);
INT32 FIFOWrite(UINT8 *buf, UINT8 len);
void FIFOFlush(void);
UINT8 RegRead(UINT8 reg);
void RegWrite(UINT8 reg, UINT8 value);
void AntennaPower(BOOL on);




#endif





