

#ifndef CCID_H
#define CCID_H



// Slot error register when bmCommandStatus = 1 refer to Table 6.2-2 in CCID specification
#define RDR_TO_PC_SLOTERROR_COMMAND_NOT_SUPPORTED			0	// 0x00
#define RDR_TO_PC_SLOTERROR_CMD_ABORTED						1	// 0xFF, Host aborted the current activity
#define RDR_TO_PC_SLOTERROR_ICC_MUTE						2	// 0xFE, CCID timed out while talking to the ICC
#define RDR_TO_PC_SLOTERROR_XFR_PARITY_ERROR				3	// 0xFD, Parity error while talking to the ICC
#define RDR_TO_PC_SLOTERROR_XFR_OVERRUN						4	// 0xFC, Overrun error while talking to the ICC
#define RDR_TO_PC_SLOTERROR_HW_ERROR						5	// 0xFB, An all inclusive hardware error occurred
#define RDR_TO_PC_SLOTERROR_BAD_ATR_TS						8	// 0xF8
#define RDR_TO_PC_SLOTERROR_BAD_ATR_TCK						9	// 0xF7
#define RDR_TO_PC_SLOTERROR_ICC_PROTOCOL_NOT_SUPPORTED		10	// 0xF6
#define RDR_TO_PC_SLOTERROR_ICC_CLASS_NOT_SUPPORTED			11	// 0xF5
#define RDR_TO_PC_SLOTERROR_PROCEDURE_BYTE_CONFLICT			12	// 0xF4
#define RDR_TO_PC_SLOTERROR_DEACTIVATED_PROTOCOL			13	// 0xF3
#define RDR_TO_PC_SLOTERROR_BUSY_WITH_AUTO_SEQUENCE			14	// 0xF2, Automatic Sequence Ongoing
#define RDR_TO_PC_SLOTERROR_PIN_TIMEOUT						16	// 0xF0
#define RDR_TO_PC_SLOTERROR_PIN_CANCELLED					17	// 0xEF
#define RDR_TO_PC_SLOTERROR_CMD_SLOT_BUSY					32	// 0xE0, A second command was sent to a slot which was already processing a command
// these following slot error number is the supplement for above
#define RDR_TO_PC_SLOTERROR_NONE									0
#define RDR_TO_PC_SLOTERROR_BAD_LENGTGH								1
#define RDR_TO_PC_SLOTERROR_SLOT_NO_EXIST							5
#define RDR_TO_PC_SLOTERROR_POWERSELECT_NO_SUPPORTED				7
#define RDR_TO_PC_SLOTERROR_PROTOCOL_INVALID_OR_NOT_SUPPORTED		7
#define RDR_TO_PC_SLOTERROR_BAD_LEVEL_PARAMETER						8
#define RDR_TO_PC_SLOTERROR_FI_DI_PAIR_INVALID_OR_NOT_SUPPORTED		10
#define RDR_TO_PC_SLOTERROR_INVALID_TCCKTS_PARAMETER				11
#define RDR_TO_PC_SLOTERROR_GUARD_TIME_NOT_SUPPORTED				12
#define RDR_TO_PC_SLOTERROR_WI_BWI_CWI_INVALID_OR_NOT_SUPPORTED		13
#define RDR_TO_PC_SLOTERROR_CLOCK_STOP_REQUEST_FALID				14
#define RDR_TO_PC_SLOTERROR_IFSC_SIZE_INVALID_OR_NOT_SUPPORTED		15
#define RDR_TO_PC_SLOTERROR_NAD_VALUE_INVALID_OR_NOT_SUPPORTED		16

// Defines for bICCStatus in slot status (2 Bits)
#define RDR_TO_PC_ICCSTATUS_NONE					0x00
#define RDR_TO_PC_ICCSTATUS_ACTIVE					0x00	// Present and active
#define RDR_TO_PC_ICCSTATUS_INACTIVE				0x01	// Present and inactive
#define RDR_TO_PC_ICCSTATUS_NOICC					0x02	// Absent
#define RDR_TO_PC_ICCSTATUS_RFU						0x03



struct ccid_msg_data {
	unsigned char *sBuf;
	unsigned char *rBuf;
	unsigned int sLen;
	unsigned int rLen;
	unsigned int param;
	unsigned char cardStatus;
	unsigned char slot;
	signed int errCode;

};

struct ccid_operations {
	void (*card_power_on)(struct ccid_msg_data *);
	void (*card_power_off)(struct ccid_msg_data *);
	void (*get_slot_status)(struct ccid_msg_data *);
	void (*xfr_block)(struct ccid_msg_data *);
	void (*get_parameters)(struct ccid_msg_data *);
	void (*reset_parameters)(struct ccid_msg_data *);
	void (*set_parameters)(struct ccid_msg_data *);
	void (*escape)(struct ccid_msg_data *);
	void (*card_clock)(struct ccid_msg_data *);
	void (*t0_APDU)(struct ccid_msg_data *);
	void (*secure)(struct ccid_msg_data *);
	void (*mechanical)(struct ccid_msg_data *);
	void (*abort)(struct ccid_msg_data *);
	void (*set_data_rate_and_clock_frequency)(struct ccid_msg_data *);
};


#endif

