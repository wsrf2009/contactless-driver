


#ifndef PN512_H
#define PN512_H

#include <linux/completion.h>
#include <linux/workqueue.h>

#include "common.h"




// Important registers of the PN512 reader module

/*********** PAGE 0  Command and Status**************/
#define PageReg				0x00    // Page Select Register
#define CommandReg			0x01

#define IRqInv				(1<<7)
#define TxIEn				(1<<6)
#define RxIEn				(1<<5)
#define IdleIEn				(1<<4)
#define HiAlertIEn			(1<<3)
#define LoAlertIEn			(1<<2)
#define ErrIEn				(1<<1)
#define TimerIEn			(1<<0)
#define CommIEnReg			0x02

#define DivIEnReg			0x03

#define Set1				(1<<7)
#define TxIRq				(1<<6)
#define RxIRq				(1<<5)
#define IdleIRq				(1<<4)
#define HiAlertIRq			(1<<3)
#define LoAlertIRq			(1<<2)
#define ErrIRq				(1<<1)
#define TimerIRq			(1<<0)
#define CommIRqReg			0x04    // Contains Interrupt Request bits.

#define DivIRqReg			0x05

#define WrErr				(1<<7)
#define TempErr				(1<<6)
#define RFErr				(1<<5)
#define BufferOvfl			(1<<4)
#define CollErr				(1<<3)
#define CRCErr				(1<<2)
#define ParityErr			(1<<1)
#define ProtocolErr			(1<<0)
#define ErrorReg			0x06    // error bit register showing the err status of the last command executed

#define IRq					(1<<4)
#define Status1Reg			0x07 

#define MFCrypto1On			(1<<3)
#define	ModemStateMask		0x07
#define Status2Reg			0x08   

#define FIFODataReg			0x09

#define FlushBuffer			(1<<7)
#define FIFOLevelReg		0x0A

#define WaterLevelReg		0x0B

#define TStopNow			(1<<7)
#define TStartNow			(1<<6)
#define WrNFCIDtoFIFO		(1<<5)
#define Initiator			(1<<4)
#define RxLastBitsMask		0x07
#define ControlReg			0x0C    // Miscellaneous control bits.

#define StartSend			(1<<7)
#define BitFramingReg		0x0D    // Adjustments for bit oriented frames

#define CollReg				0x0E

/*********** PAGE 1  Communication **************/
#define MSBFirst			(1<<7)
#define DetectSync			(1<<6)
#define ModeReg				0x11

#define TxCRCEn				(1<<7)
#define TxModeReg			0x12

#define RxCRCEn				(1<<7)
#define RxMultiple			(1<<2)
#define RxModeReg			0x13

#define TxControlReg		0x14

#define InitialRFOn			(1<<2)
#define TxAutoReg			0x15

#define TxSelReg			0x16
#define RxSelReg			0x17
#define RxThresholdReg		0x18
#define DemodReg			0x19
#define FelNFC1Reg			0x1A
#define FelNFC2Reg			0x1B
#define MifNFCReg			0x1C

#define ParityDisable		(1<<4)
#define ManualRCVReg		0x1D

#define TypeBReg			0x1E
#define SerialSpeedReg		0x1F

/*********** PAGE 2  Configuration **************/
#define CRCResultMSB		0x21
#define CRCResultLSB		0x22
#define GsNOffReg			0x23
#define ModWidthReg			0x24
#define TxBitPhaseReg		0x25
#define RFCfgReg			0x26
#define GsNOnReg			0x27
#define CWGsPReg			0x28
#define ModGsPReg			0x29

#define TAuto				(1<<7)    // the timer starts automatically at the end of the transmission
#define TAutoRestart		(1<<4)    // the timer automatically restart its count-down from TReloadValue
#define TModeReg			0x2A    // Defines settings for the timer

#define TPrescalerReg		0x2B

#define TReloadVal_Hi		0x2C
#define TReloadVal_Lo		0x2D
#define TCounterVal_Hi		0x2E
#define TCounterVal_Lo		0x2F

/*********** PAGE 3  TestRegister **************/
#define TestSel1Reg			0x31
#define TestSel2Reg			0x32
#define TestPinEnReg		0x33
#define TestPinValueReg		0x34
#define TestBusReg			0x35
#define AutoTestReg			0x36
#define VersionReg			0x37
#define AnalogTestReg 		0x38
#define TestDAC1Reg			0x39
#define TestDAC2Reg			0x3A
#define TestADCReg			0x3B


//************ PN512  C O M M A N D S *************
#define CMD_IDLE               0x00    // No action: cancel current command 
#define CMD_CONFIG             0x01
#define CMD_GENERATE_RANDOMID  0x02
#define CMD_CALCCRC            0x03
#define CMD_TRANSMIT           0x04
#define CMD_NOCMDCHANGE        0x07
#define CMD_RECEIVE            0x08
#define CMD_TRANSCEIVE         0x0C
#define CMD_TRANSCEIVE_TO      0x8C
#define CMD_AUTOCOLL           0x0D
#define CMD_MFAUTHENT          0x0E
#define CMD_SOFTRESET          0x0F



#define FSDLENTH            254


#define CONFIGNOTHING  0
#define CONFIGTYPEA    1
#define CONFIGTYPEB    2

#define ERROR_NOTAG              1
#define ERROR_PROTOCOL           2
#define ERROR_PARITY             3
#define ERROR_BUFOVFL            4
#define ERROR_CRC                5
#define ERROR_COLL               6
#define ERROR_SERNR              7
#define ERROR_BYTECOUNT          8
#define ERROR_BITCOUNT           9
#define ERROR_WRONGPARAM         10
#define ERROR_ATSLEN             11
#define ERROR_FSDLENTH           12
#define ERROR_UNKNOW_COMMAND     13
#define ERROR_INVALID_DATA       14
#define ERROR_SPEED              15
#define ERROR_CID                16



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




enum request_direction
{
	TRANSMIT = 1,
	RECEIVE,
	TRANSCEIVE,
};

struct pn512_request
{
	u8			buf[1024];
	u32			length;

	u32			actual;
	u32			bit_numbers;
	u8			command;
	u8			bit_frame;
	enum request_direction		direction;

	
	u8			timer_start_now:1;
	u8			timer_start_auto:1;

	// the max time for waiting acknowlage from pn512, unit is 100us
	u32			time_out;

	u8			rx_last_bits;

	u8			tx_done:1;
	int				error_code;

	void			*done_data;
	void			(*done)(struct pn512_request *);
};

struct pn512_common
{
	struct spi_device	*spi_device;
	struct spi_driver	*spi_driver;

	struct pn512_request	request;

	u8			water_level;

	struct work_struct	wq;

	spinlock_t	pn512_lock;

	struct completion	pn512_complete;

	int				reset_pin;
	int				intr_pin;

	u8			intr_enable_mask;

	




};


extern struct pn512_common *pn512;


int pn512_reg_write(u8 reg, u8 value);
u8 pn512_reg_read(u8 reg);
void pn512_reg_clear(u8 reg, u8 bitMask);
void pn512_reg_set(u8 reg, u8 bitMask);
int pn512_fifo_write(u8 *data, u32 len);
int pn512_fifo_read(u8 *data, u32 len);
void set_pn512_timer(u16 timeOut);
void turn_on_antenna(void);
void turn_off_antenna(void);
void pcd_config_iso14443_card(u8 flagConfig, u8 cardType);
void pn512_process_done(struct pn512_request *req);
void pn512_process_request(struct pn512_request *req);
int pn512_init(struct pn512_request **req);
int pn512_uninit(void);


#endif

