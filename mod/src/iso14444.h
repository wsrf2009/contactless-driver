




#ifndef PART4_H
#define PART4_H


#include "common.h"




#define SLOTERROR_T1_3RETRY_FAIL_RESYNCH_PASS  31
#define SLOTERROR_T1_3RETRY_FAIL_RESYNCH_FAIL  30
#define SLOTERROR_T1_CHECKSUM_ERROR            29
#define SLOTERROR_T1_OTHER_ERROR               28
#define SLOTERROR_T1_LEN_INF_CONFLICT          27

#define SLOTERROR_TCL_3RETRANSMIT_FAIL  63
#define SLOTERROR_TCL_3RETRY_TIMEOUT    62
#define SLOTERROR_TCL_BLOCK_INVALID     61



#define APDURECLENTHREHOLD    512




extern const u16 fsdi_to_fsd[];


int typeA_request_ats(struct picc_device *picc);
void typeA_set_timeout(struct picc_device *picc, u8 timeout);
u8 typeA_speed_check(struct picc_device *picc);
void typeA_high_speed_config(struct picc_device *picc, u8 speedParam, u8 typeB);
void typeA_prologue_feild_load(struct picc_device *picc);
void typeA_pps_check_and_send(struct picc_device *picc);
int typeA_select_(struct picc_device *picc, u8 blockPCB);
int typeA_deselect_request(struct picc_device *picc);
int typeA_standard_apdu_handler(struct picc_device *picc, u8 *cmdBuf, u32 senLen, u8 *recBuf, u32 *recLen, u8 *level);


#endif


