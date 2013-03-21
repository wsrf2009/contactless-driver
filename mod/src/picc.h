/*
* Name: PICC head file
* Date: 2012/12/04
* Author: Alex Wang
* Version: 1.0
*/

#ifndef PICC_H
#define PICC_H


#include "common.h"





//pcd filter bit detail
#define BIT_TYPEAPOLL     BIT(0)
#define BIT_TYPEBPOLL     BIT(1)
#define BIT_FEL212POLL    BIT(2)
#define BIT_FEL424POLL    BIT(3)
#define BIT_TOPAZPOLL     BIT(4)
// pcd fgPoll bit detail
#define BIT_AUTORATS      BIT(0)
#define BIT_AUTOPOLL      BIT(1)
#define BIT_POLLCARD      BIT(2)
// picc status bit detail
#define BIT_PRESENT       BIT(0)
#define BIT_ACTIVATED     BIT(1)
#define BIT_CONNECTED     BIT(2)
#define BIT_FIRSTINSERT   BIT(3)
#define BIT_SLOTCHANGE    BIT(4)
// picc fgTxRx bit detail
#define BIT_TXCRC         BIT(0)
#define BIT_RXCRC         BIT(1)
#define BIT_TXPARITY      BIT(2)
#define BIT_RXPARITY      BIT(3)
#define BIT_PROLOGUE      BIT(4)
#define BIT_VENDORSPEC    BIT(5)

// picc type
#define PICC_ABSENT        0xCC
#define PICC_TYPEA_TCL     0x20    // typeA which support ISO/IEC 14443-4
#define PICC_MIFARE        0x10
#define PICC_TYPEB_TCL     0x23    // typeB which support ISO/IEC 14443-4
#define PICC_ST_MEMERY     0x24
#define PICC_TOPAZ         0x04
#define PICC_FELICA212     0x11  
#define PICC_FELICA424     0x12


// picc states
#define PICC_POWEROFF       0
#define PICC_IDLE           1
#define PICC_READY          2
#define PICC_SELECTED       3
#define PICC_ACTIVATED      4
#define PICC_UPDATE         5

#define SLOT_NO_ERROR               0x00
#define SLOTERROR_CMD_ABORTED       0xFF
#define SLOTERROR_ICC_MUTE          0xFE
#define SLOTERROR_XFR_PARITY_ERROR  0xFD
#define SLOTERROR_XFR_OVERRUN       0xFC
#define SLOTERROR_HW_ERROR          0xFB




struct pcdInfo
{
    // bit 0: typeA
    // bit 1: typeB
    // bit 2: felica212
    // bit 3: felica414
    // bit 4: topaz
    UINT8 filterType;
    UINT8 FSDI;        // codes FSD, FSD defines the maximum size of a fram the PCD is able to receive, default 8, 256 bytes( refer to FSCConvertTbl[])
    UINT8 maxSpeed;    // Maximum communication speed supported by the IFD
    UINT8 curSpeed;
    BOOL  piccPoll;

    // describes prologue filed in Block Format,  ISO/IEC 14443-4
    UINT8 PCB;
    UINT8 CID;
    UINT8 NAD;

    // bit 0: Auto RATS
    // bit 1: Auto poll
    // bit 2: poll card
    UINT8  fgPoll;
    UINT16 pollDelay;
};



struct piccInfo
{
    // bit 0: present
    // bit 1: activated
    // bit 2: connected
    // bit 3: first insert
    // bit 4: slot change
    volatile UINT8 status;
    UINT8 states;    // picc states 
    UINT8 type;      // picc type
    UINT8 sPart4;    // if support ISO14443-4
    UINT8 sn[10];    // used to stored UID(type A, MAX 10 bytes), PUPI(type B, 4 bytes), NFCID2(felica, 8 bytes)
    UINT8 snLen;


    // typeA parameters
    UINT8 ATQA[2];
    UINT8 SAK;


    // typeB parameters
    UINT8 ATQB[13];
    UINT8 ATQBLen;
    UINT8 attrPara[4];
    UINT8 resATTRIB[7];


    // felica parameters
    UINT8 PAD[8];    // used to stored PMm(manufacture parameter, 8 bytes) of felica
    UINT8 sysCode[2];    


    // the struct defines these parameter of the PICC which supports ISO14443-4
    UINT8 FWI;         // codes FWT, Frame waiting Time Integer, defines the maximum time for a PICC to start its response after the end of a PCD frame
    UINT8 SFGI;        // codes amultiplier value used to define the SFGT, SFGT defines guard time needed by the PICC before it is ready to receive the next fram after it has sent the ATS
    UINT8 FSCI;        // codes FSC, FSC defines the maximum size of a fram accepted by the PICC, default 2, 32 bytes( refer to FSCConvertTbl[])
    UINT16 FSC;
    UINT8 speed;       // stored TA(1), specify the divisor D for each direction
    UINT8 ATS[20];     // an array used to stores the ATS send from PICC
    // bit 0: pcd block number
    // bit 1: cid  present
    // bit 2: pcd chaining
    // bit 3: picc chaning
    // bit 4: wtx request
    // bit 5: typeB attrib
    // bit 6: wtx request before
    // bit 7: NAD present
    UINT8 fgTCL;
    UINT8 WTXM;
    
    // prologue filed in Block Format,  ISO/IEC 14443-4
    UINT8 PCB;
    UINT8 NAD;
    UINT8 CID;
};




extern struct pcdInfo pcd;
extern struct piccInfo picc;




extern const UINT8 vendorName[];
extern const UINT8 productName[];
extern const UINT8 driverVersion[];
extern const UINT8 firmwareVersion[];
extern const UINT8 IFDVersion[];

extern const UINT16 FSCConvertTbl[9];



void PiccReset(void);
void PiccPoll(void);
UINT8 PiccPowerON(UINT8 *atrBuf, UINT16 *atrLen);
void PiccPowerOff(void);
UINT8 PiccXfrDataExchange(UINT8 *cmdBuf, UINT16 cmdLen, UINT8 *resBuf, UINT16 *resLen, UINT8 *level);
void PiccInit(void);

#endif


