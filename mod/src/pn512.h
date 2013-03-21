/*
* Name: PN512 drive head file
* Date: 2012/12/04
* Author: Alex Wang
* Version: 1.0
*/




#ifndef PN512_H
#define PN512_H



#include "common.h"





// Important registers of the PN512 reader module

/*********** PAGE 0  Command and Status**************/
#define REG_PAGE            0x00    // Page Select Register
#define REG_COMMAND         0x01
#define REG_COMMIEN         0x02
#define REG_DIVIEN          0x03

#define BIT_SET1            BIT(7)
#define BIT_TXIRQ           BIT(6)
#define BIT_RXIRQ           BIT(5)
#define BIT_IDLEIRQ         BIT(4)
#define BIT_HIALERTIRQ      BIT(3)
#define BIT_LOALERTIRQ      BIT(2)
#define BIT_ERRIRQ          BIT(1)
#define BIT_TIMERIRQ        BIT(0)
#define REG_COMMIRQ         0x04    // Contains Interrupt Request bits.

#define REG_DIVIRQ          0x05

#define BIT_WRERR           BIT(7)
#define BIT_TEMPERR         BIT(6)
#define BIT_RFERR           BIT(5)
#define BIT_BUFFEROVFL      BIT(4)
#define BIT_COLLERR         BIT(3)
#define BIT_CRCERR          BIT(2)
#define BIT_PARITYERR       BIT(1)
#define BIT_PROTOCOLERR     BIT(0)
#define REG_ERROR           0x06    // error bit register showing the err status of the last command executed

#define REG_STATUS1         0x07 

#define BIT_MFCRYPTO1ON     BIT(3)
#define REG_STATUS2         0x08   

#define REG_FIFODATA        0x09
#define REG_FIFOLEVEL       0x0A
#define REG_WATERLEVEL      0x0B

#define BIT_TSTOPNOW        BIT(7)
#define BIT_TSTARTNOW       BIT(6)
#define BIT_WRNFCIDTOFIFO   BIT(5)
#define BIT_INITIATOR       BIT(4)
#define REG_CONTROL         0x0C    // Miscellaneous control bits.

#define BIT_STARTSEND       BIT(7)
#define REG_BITFRAMING      0x0D    // Adjustments for bit oriented frames

#define REG_COLL            0x0E

/*********** PAGE 1  Communication **************/
#define BIT_MSBFIRST        BIT(7)
#define BIT_DETECTSYNC      BIT(6)
#define REG_MODE            0x11

#define BIT_TXCRCEN         BIT(7)
#define REG_TXMODE          0x12

#define BIT_RXCRCEN         BIT(7)
#define BIT_RXMULTIPLE      BIT(2)
#define REG_RXMODE          0x13

#define REG_TXCONTROL       0x14

#define BIT_INIRFON         BIT(2)
#define REG_TXAUTO          0x15

#define REG_TXSEL           0x16
#define REG_RXSEL           0x17
#define REG_RXTHRESHOLD     0x18
#define REG_DEMOD           0x19
#define REG_FELNFC1         0x1A
#define REG_FELNFC2         0x1B
#define REG_MIFNFC          0x1C

#define BIT_PARITYDISABLE   BIT(4)
#define REG_MANUALRCV       0x1D

#define REG_TYPEB           0x1E
#define REG_SERIALSPEED     0x1F

/*********** PAGE 2  Configuration **************/
#define REG_CRCRESULTMSB    0x21
#define REG_CRCRESULTLSB    0x22
#define REG_GSNOFF          0x23
#define REG_MODWIDTH        0x24
#define REG_TXBITPHASE      0x25
#define REG_RFCFG           0x26
#define REG_GSNON           0x27
#define REG_CWGSP           0x28
#define REG_MODGSP          0x29

#define BIT_TAUTO           BIT(7)    // the timer starts automatically at the end of the transmission
#define BIT_TAUTORESTART    BIT(4)    // the timer automatically restart its count-down from TReloadValue
#define REG_TMODE           0x2A    // Defines settings for the timer

#define REG_TPRESCALER      0x2B

#define REG_TRELOADVAL_HI   0x2C
#define REG_TRELOADVAL_LO   0x2D
#define REG_TCOUNTERVAL_HI  0x2E
#define REG_TCOUNTERVAL_LO  0x2F

/*********** PAGE 3  TestRegister **************/
#define REG_TESTSEL1        0x31
#define REG_TESTSEL2        0x32
#define REG_TESTPINEN       0x33
#define REG_TESTPINVALUE    0x34
#define REG_TESTBUS         0x35
#define REG_AUTOTEST        0x36
#define REG_VERSION         0x37
#define REG_ANALOGTEST      0x38
#define REG_TESTDAC1        0x39
#define REG_TESTDAC2        0x3A
#define REG_TESTDAC         0x3B

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


#define MAX_FIFO_LENGTH     60
#define FSDLENTH            254





INT32 PN512_RegWrite(UINT8 reg, UINT8 value);
UINT8 PN512_RegRead(UINT8 reg);
INT32 PN512_FIFOWrite(UINT8 *data, UINT8 len);
INT32 PN512_FIFORead(UINT8 *data, UINT8 len);
void SetPN512Timer(UINT16 timeOut);
INT32 Pn512Init(void);
INT32 Pn512Uninit(void);


#endif

