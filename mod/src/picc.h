

#ifndef PICC_H
#define PICC_H

#include <linux/completion.h>

#include "common.h"
#include "ccid.h"
#include "pn512.h"



// picc type
#define PICC_ABSENT        0xCC
#define PICC_TYPEA_TCL     0x20    // typeA which support ISO/IEC 14443-4
#define PICC_MIFARE        0x10
#define PICC_TYPEB_TCL     0x23    // typeB which support ISO/IEC 14443-4
#define PICC_ST_MEMERY     0x24
#define PICC_TOPAZ         0x04
#define PICC_FELICA212     0x11  
#define PICC_FELICA424     0x12



#define PICC_ERRORCODE_NONE					RDR_TO_PC_SLOTERROR_NONE
#define PICC_ERRORCODE_CMD_ABORTED			RDR_TO_PC_SLOTERROR_CMD_ABORTED
#define PICC_ERRORCODE_MUTE					RDR_TO_PC_SLOTERROR_ICC_MUTE
#define PICC_ERRORCODE_XFR_PARITY_ERROR		RDR_TO_PC_SLOTERROR_XFR_PARITY_ERROR
#define PICC_ERRORCODE_XFR_OVERRUN			RDR_TO_PC_SLOTERROR_XFR_OVERRUN
#define PICC_ERRORCODE_HW_ERROR				RDR_TO_PC_SLOTERROR_XFR_OVERRUN

#define PICC_ICCSTATUS_ACTIVE				RDR_TO_PC_ICCSTATUS_ACTIVE
#define PICC_ICCSTATUS_INACTIVE				RDR_TO_PC_ICCSTATUS_INACTIVE
#define PICC_ICCSTATUS_NOICC				RDR_TO_PC_ICCSTATUS_NOICC



struct picc_device;

struct pcd_device {


#define TYPEA			(1<<0)		// bit 0: typeA
#define	TYPEB			(1<<1)		// bit 1: typeB
#define	FELICA212		(1<<2)		// bit 2: felica212
#define	FELICA414		(1<<3)		// bit 3: felica414
#define	TOPAZ			(1<<4)		// bit 4: topaz
    u8			support_card_type;


    u8			FSDI;        // codes FSD, FSD defines the maximum size of a fram the PCD is able to receive, default 8, 256 bytes( refer to FSCConvertTbl[])
    u8			max_speed;    // Maximum communication speed supported by the IFD
    u8			current_speed;
    BOOL		piccPoll;


    // describes prologue filed in Block Format,  ISO/IEC 14443-4
    u8			PCB;
    u8			CID;
    u8			NAD;


#define	AUTO_RATS				(1<<0)	// bit 0: Auto RATS
#define	AUTO_POLLING			(1<<1)	// bit 1: Auto poll
#define POLLING_CARD_ENABLE		(1<<2)	// bit 2: poll card
    u8			flags_polling;


	u8			mifare_key[2][6];


    u16			poll_interval;

	struct picc_device		*picc;

}__attribute__((packed, aligned(1)));

enum picc_state
{
	PICC_POWEROFF,
	PICC_IDLE,
	PICC_READY,
	PICC_SELECTED,
	PICC_ACTIVATED,
	PICC_UPDATE,
};

struct picc_device {

#define PRESENT			(1<<0)    // bit 0: present
#define ACTIVATED		(1<<1)    // bit 1: activated
#define CONNECTED		(1<<2)    // bit 2: connected
#define FIRST_INSERT	(1<<3)    // bit 3: first insert
#define SLOT_CHANGE		(1<<4)    // bit 4: slot change
    volatile u8		status;

    enum picc_state		states;    // picc states 
    u8 				type;      // picc type
    char				*name;
    u8 				support_part4;    // if support ISO14443-4
    u8 				sn[10];    // used to stored UID(type A, MAX 10 bytes), PUPI(type B, 4 bytes), NFCID2(felica, 8 bytes)
    u8 				sn_len;


    // typeA parameters
    u8 				ATQA[2];
    u8 				SAK;


    // typeB parameters
    u8 				ATQB[13];
    u8 				ATQB_len;
    u8 				attrib_param[4];
    u8 				attrib_response[7];


    // felica parameters
    u8 				PAD[8];    // used to stored PMm(manufacture parameter, 8 bytes) of felica
    u8 				system_code[2];    


    // the struct defines these parameter of the PICC which supports ISO14443-4
    u8 				FWI;         // codes FWT, Frame waiting Time Integer, defines the maximum time for a PICC to start its response after the end of a PCD frame
    u8 				SFGI;        // codes amultiplier value used to define the SFGT, SFGT defines guard time needed by the PICC before it is ready to receive the next fram after it has sent the ATS
    u8 				FSCI;        // codes FSC, FSC defines the maximum size of a fram accepted by the PICC, default 2, 32 bytes( refer to FSCConvertTbl[])
    u16 			FSC;
    u8 				speed;       // stored TA(1), specify the divisor D for each direction
    u8 				ATS[20];     // an array used to stores the ATS send from PICC


#define PCD_BLOCK_NUMBER		(1<<0)    // bit 0: pcd block number
#define CID_PRESENT				(1<<1)    // bit 1: cid  present
#define PCD_CHAINING			(1<<2)    // bit 2: pcd chaining
#define PICC_CHAINING			(1<<3)    // bit 3: picc chaning
#define WTX_REQUEST				(1<<4)    // bit 4: wtx request
#define TYPEB_ATTRIB			(1<<5)    // bit 5: typeB attrib
#define WTX_REQ_BEFORE			(1<<6)    // bit 6: wtx request before
#define NAD_PRESENT				(1<<7)    // bit 7: NAD present
	u8 				flags_TCL;

	u8 				WTXM;
    
    // prologue filed in Block Format,  ISO/IEC 14443-4
	u8 				PCB;
	u8 				NAD;
	u8 				CID;


	// mifare
    volatile u8			work_key[6];
    volatile u8			key_valid;
    volatile u8			key_type;
    volatile u8			key_No;
    volatile u8			authen_need;
    volatile u8			block;


	// pcsc
#define TXCRC			(1<<0)    // bit 0: TxCRC bit
#define RXCRC			(1<<1)    // bit 1: RxCRC bit
#define TXPARITY		(1<<2)    // bit 2: TxParity bit
#define RXPARITY		(1<<3)    // bit 3: RxParity bit
#define PROLOGUE		(1<<4)    // bit 4: prologue bit
#define VENDORSPEC		(1<<5)    // bit 5: vendor specify bit
		volatile u8 		flags_tx_rx;

		volatile u8 		previous_cmd;
		volatile u8			transfer_status;
		volatile u8			flags_status;
		volatile u8			last_rx_valid_bits;
		volatile u8			last_tx_valid_bits;
		volatile u8			next_cmd;


	struct pn512_request	*request;
	struct pcd_device		*pcd;
}__attribute__((packed, aligned(1)));




extern const u8 IFDVersion[];

extern const u16 FSCConvertTbl[9];


void picc_wait_for_req(struct pn512_request *req);
u8 get_cid(u8 *uid);
void picc_reset(struct picc_device *picc);


#endif


