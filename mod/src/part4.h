




#ifndef PART4_H
#define PART4_H


#include "common.h"




#define SLOTERROR_T1_3RETRY_FAIL_RESYNCH_PASS  0xE1
#define SLOTERROR_T1_3RETRY_FAIL_RESYNCH_FAIL  0xE2
#define SLOTERROR_T1_CHECKSUM_ERROR            0xE3
#define SLOTERROR_T1_OTHER_ERROR               0xE4
#define SLOTERROR_T1_LEN_INF_CONFLICT          0xE5

//tcl
#define SLOTERROR_TCL_3RETRANSMIT_FAIL  0xC1
#define SLOTERROR_TCL_3RETRY_TIMEOUT    0xC2
#define SLOTERROR_TCL_BLOCK_INVALID     0xc3


// picc.fgTCL detail
#define BIT_PCDBLOCKNUMBER    BIT(0)
#define BIT_CIDPRESENT        BIT(1)
#define BIT_PCDCHAINING       BIT(2)
#define BIT_PICCCHAINING      BIT(3)
#define BIT_WTXREQUEST        BIT(4)
#define BIT_TYPEBATTRIB       BIT(5)
#define BIT_WTXREQBEFORE      BIT(6)
#define BIT_NADPRESENT        BIT(7)

#define APDURECLENTHREHOLD    512







UINT8 PiccStandardApduTCL(UINT8 *cmdBuf, UINT16 senLen, UINT8 *recBuf, UINT16 *recLen, UINT8 *level);
UINT8 TCL_Select(UINT8 blockPCB);
UINT8 DeselectRequest(void);
void PiccPPSCheckAndSend(void);
void TCLPrologueFieldLoad(void);
void PcdSetTimeout(UINT8 timeout);
void PiccHighSpeedConfig(UINT8 speedParam, UINT8 typeB);
UINT8 PiccSpeedCheck(void);
UINT8 PcdRequestATS(void);
UINT8 GetCID(UINT8 *uid);


#endif


