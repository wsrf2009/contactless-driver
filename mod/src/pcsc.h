









#ifndef PCSC_H
#define PCSC_H



#include "common.h"




#define MANAGE_SESSION       0x00
#define TRANSPARENT_EXCHANGE 0X01
#define SWITCH_PROTOCOL      0x02

// Data object Name
#define OJ_Idle                0x00
// manage session command
#define CMD_DO_VERSIONDATA        0x80
#define CMD_DO_STARTTRANS         0x81
#define CMD_DO_ENDTRANS           0x82
#define CMD_DO_TURNOFF_RF         0x83
#define CMD_DO_TURNON_RF          0x84
#define CMD_DO_TIMER              0x5F
#define CMD_DO_PARAMETERS         0xFF
//transparent exchange command APDU
#define CMD_DO_TRANSCEIVE_FLAG      0x90
#define CMD_DO_TRANS_BIT_FRAM       0x91
#define CMD_DO_REC_BIT_FRAM         0x92
#define CMD_DO_TRANSMIT           0x93
#define CMD_DO_RECEIVE            0x94
#define CMD_DO_TRANSCEIVE_DATA    0x95
// switch protocol command APDU
#define CMD_DO_SWITCH_PROTOCOL     0x8F
// response
#define RES_GENERICERR         0xC0
#define RES_VERSIONDATA        0x80
#define RES_PARAMETERS         0xFF
#define RES_NUMOFVALIDBITS     0x92
#define RES_RESPONSESTATUS     0x96
#define RES_ICCRESPONSE        0x97


// Data object Errors
#define ERROR_NO_GENERAL           0x00
#define ERROR_WARNING              0xFF
#define ERROR_NOINF                0xFE
#define ERROR_EXSTOP               0xFD
#define ERROR_OBJECT_NOTSUPPORTED  0xFC
#define ERROR_OBJECT_LENGTH        0xFB
#define ERROR_OBJECT_VALUE         0xFA
#define ERROR_NOIFDRES             0xF9
#define ERROR_NOICCRES             0xF8
#define ERROR_NOPRESIEDIAGNOSIS    0xF7

/****** switch protocol data object *******/
// the standard type
#define SWCH_ISO14443_TYPEA        0x00
#define SWCH_ISO14443_TYPEB        0x01
#define SWCH_ISO15693              0x02
#define SWCH_FELICA                0x03
#define SWCH_ICODE_EPC_UID         0x04
#define SWCH_ICODE_1               0x05
#define SWCH_HF_EPC_G2_ISO18000_3  0x06
#define SWCH_INNOVATRON            0x07
// the layer to switch
#define SWCH_NO_LAYER_SEPARATION   0x00
#define SWCH_TO_LAYER_2            0x02
#define SWCH_TO_LAYER_3            0x03
#define SWCH_TO_LAYER_4            0x04
#define SWCH_TO_LAYER_2X           0x20
#define SWCH_TO_LAYER_3X           0x30
#define SWCH_TO_HIGHLEVEL_PROTOCOL 0x40



struct pcscInfo
{
    // bit 0: TxCRC bit
    // bit 1: RxCRC bit
    // bit 2: TxParity bit
    // bit 3: RxParity bit
    // bit 4: prologue bit
    // bit 5: vendor specify bit
    volatile UINT8 fgTxRx;
    volatile UINT8 preCmd;
    volatile UINT8 trsStatus;
    volatile UINT8 fgStatus;
    volatile UINT8 lastRxValBits;
    volatile UINT8 lastTxValBits;
    volatile UINT8 nextCmd;
};

extern struct pcscInfo pcsc;


UINT8 PcscIfdCmdDispatch(UINT8 cmdtype, UINT8 *cmdBuf, UINT8 cmdlen, UINT8 *resBuf, UINT16 *resLen);
void PcscAtrBuild(UINT8 *atrBuf, UINT16 *atrLen);




#endif

