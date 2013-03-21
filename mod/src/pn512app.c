/*
* Name: RF IC source file
* Date: 2012/12/05
* Author: Alex Wang
* Version: 1.0
*/





#include "common.h"
#include "pn512app.h"
#include "pn512.h"
#include "delay.h"
#include "typeA.h"
#include "typeB.h"
#include "pcsc.h"
#include "picc.h"
#include "debug.h"



UINT8 RCRegFactor[19] = 
{
    0x12, 0x85, 0x85, 0x85, 0x85, 0x85, 0x85, 0x85, 0x85,
    0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x3F, 0x3F
};




//*******************************************
// ISO 14443 TypeA config data
//*******************************************
UINT8 CfgTbl_AGen[] =
{
    REG_MODE,       0x39,        //CRCPreset = 6363H
    REG_BITFRAMING, 0x00,
    REG_GSNON,      0xFF,
    REG_GSNOFF,     0x6F,
    REG_MANUALRCV,  0x00,
    REG_TXAUTO,     0x43,
    REG_DEMOD,      0x4D,
    0x00
};

UINT8 CfgTbl_A106Tx[] =
{
    REG_TXMODE,0x80,        //ISO/IEC 14443A/MIFARE and 106 kbit, TxCRCEn On
    REG_MODWIDTH,0x26,
    0x00
};

UINT8 CfgTbl_A106Rx[] =
{
    REG_RXMODE,0x80,        //ISO/IEC 14443A/MIFARE and 106 kbit, RxCRCEn On
    0x00
};

UINT8 CfgTbl_A212Tx[] =
{
    REG_TXMODE,0x90,
    REG_MODWIDTH,0x13,
    0x00
};

UINT8 CfgTbl_A212Rx[] =
{
    REG_RXMODE,0x90,
    0x00
};

UINT8 CfgTbl_A424Tx[] = 
{
    REG_TXMODE,0xA0,
    REG_MODWIDTH,0x0A,
    0x00
};

UINT8 CfgTbl_A424Rx[] =
{
    REG_RXMODE,0xA0,
    0x00
};

UINT8 CfgTbl_A848Tx[] =
{
    REG_TXMODE,0xB0,
    REG_MODWIDTH,0x05,
    0x00
};

UINT8 CfgTbl_A848Rx[] = 
{
    REG_RXMODE,0xB0,
    0x00
};


//*******************************************
// ISO 14443 TypeB config data
//*******************************************
UINT8 CfgTbl_BGen[] =
{
    REG_MODE,       0x3B,
    REG_BITFRAMING, 0x00,
    REG_GSNON,      0xFF,
    REG_TYPEB,      0x10,
    REG_DEMOD,      0x4D,
    REG_MANUALRCV,  0x10,
    0x00
};

UINT8 CfgTbl_B106Tx[] =
{
    REG_TXMODE,0x83,
    REG_TXAUTO,0x03,
    0x00
};

UINT8 CfgTbl_B106Rx[] = 
{
    REG_RXMODE,0x83,
    REG_RXSEL,0x84,
    0x00
};

UINT8 CfgTbl_B212Tx[] =
{
    REG_TXMODE,0x93,
    REG_TXAUTO,0x03,
    0x00
};


UINT8 CfgTbl_B212Rx[] = 
{
    REG_RXMODE,0x93,
    REG_RXSEL,0x84,
    0x00
};

UINT8 CfgTbl_B424Tx[] =
{
    REG_TXMODE,0xA3,
    REG_TXAUTO,0x03,
    0x00
};

UINT8 CfgTbl_B424Rx[] = 
{
    REG_RXMODE,0xA3,
    REG_RXSEL,0x82,
    0x00
};

UINT8 CfgTbl_B848Tx[] =
{
    REG_TXMODE,0xB3,
    REG_TXAUTO,0x03,
    0x00
};

UINT8 CfgTbl_B848Rx[] = 
{
    REG_RXMODE,0xB3,
    REG_RXSEL,0x82,
    0x00
};






void AntennaPower(BOOL on)
{

    PrtMsg(DBGL4, "%s: start, on = %02X\n", __FUNCTION__, on);
    if(on) 
    {
        PN512_RegWrite(REG_TXCONTROL, 0x83);
    }
    else
    {
        PN512_RegWrite(REG_TXCONTROL, 0x80);
    }
}

void RegWrite(UINT8 reg, UINT8 value)
{
    PN512_RegWrite(reg, value);
}

UINT8 RegRead(UINT8 reg)
{
    return(PN512_RegRead(reg));
}

void FIFOFlush(void)
{
    PN512_RegWrite(REG_FIFOLEVEL, 0x80);
}

INT32 FIFOWrite(UINT8 *buf, UINT8 len)
{
    return(PN512_FIFOWrite(buf, len));
}

UINT8 FIFORead(UINT8 *buf, UINT8 len)
{
    return(PN512_FIFORead(buf, len));
}

void ClearRegBit(UINT8 reg, UINT8 bitMask)
{
    UINT8 tempValue;


    tempValue = RegRead(reg);
    if(BITISSET(tempValue, bitMask))
    {
        CLEAR_BIT(tempValue, bitMask);
        RegWrite(reg, tempValue);
    }
}

void SetRegBit(UINT8 reg, UINT8 bitMask)
{
    UINT8 tempValue;
    

    tempValue = RegRead(reg);
    if(BITISCLEAR(tempValue, bitMask))
    {
        SET_BIT(tempValue, bitMask);
        RegWrite(reg, tempValue);
    }
}

UINT8 GetBitNumbersReceived(void)
{
    UINT8 bitNumbers = 0;
    UINT8 tempBytes = 0;


    PrtMsg(DBGL1, "%s: start\n", __FUNCTION__);

    bitNumbers = RegRead(REG_CONTROL) & 0x07;
    tempBytes = RegRead(REG_FIFOLEVEL);
    if((tempBytes == 0) || (tempBytes > 64))
    {
        bitNumbers = 0;
    }

    if( bitNumbers)
    {
        bitNumbers += (tempBytes - 1) * 8;
    }
    else
    {
        bitNumbers = tempBytes * 8;
    }

    PrtMsg(DBGL1, "%s: exit, bitNumbers = %02X\n", __FUNCTION__, bitNumbers);

    return(bitNumbers);
}



//******************************************************************
//       Configure ISO14443 Type
//******************************************************************
void PcdConfigIso14443Type(UINT8 flagConfig, UINT8 cardType)
{
    UINT8 regAddr;
    UINT8 i;
    UINT8 *pTable;
    

    PrtMsg(DBGL1, "%s: Start\n", __FUNCTION__);

    if(flagConfig)
    {
        if(flagConfig == CONFIGTYPEA)
        {
            pTable = CfgTbl_AGen;
            RegWrite(REG_CWGSP, RCRegFactor[TypeACWGsP]);
        }
        else
        {
            pTable = CfgTbl_BGen;
            RegWrite(REG_CWGSP, RCRegFactor[TypeBCWGsP]);
            RegWrite(REG_MODGSP, RCRegFactor[BModeIndex]);
        }
        i = 0;
        regAddr = pTable[i++];
        while(regAddr) 
        {
            RegWrite(regAddr,pTable[i++]);
            regAddr = pTable[i++];
        }
    }
    switch(cardType)
    {
        case TYPEA_106TX:
            pTable = CfgTbl_A106Tx;
            break;
        case TYPEA_212TX:
            pTable = CfgTbl_A212Tx;
            break;
        case TYPEA_424TX:
            pTable = CfgTbl_A424Tx;
            break;
        case TYPEA_848TX:
            pTable = CfgTbl_A848Tx;
            break;
        case TYPEB_106TX:
            pTable = CfgTbl_B106Tx;
            break;
        case TYPEB_212TX:
            pTable = CfgTbl_B212Tx;
            break;
        case TYPEB_424TX:
            pTable = CfgTbl_B424Tx;
            break;
        case TYPEB_848TX:
            pTable = CfgTbl_B848Tx;
            break;
        case TYPEA_106RX:
            RegWrite(REG_RFCFG, RCRegFactor[ARFAmpCfg106]);
            RegWrite(REG_RXTHRESHOLD, RCRegFactor[RxAThres106]);
            pTable = CfgTbl_A106Rx;
            break;
        case TYPEA_212RX:
            RegWrite(REG_RFCFG, RCRegFactor[ARFAmpCfg212]);
            RegWrite(REG_RXTHRESHOLD, RCRegFactor[RxAThres212]);
            pTable = CfgTbl_A212Rx;
            break;
        case TYPEA_424RX:
            RegWrite(REG_RFCFG, RCRegFactor[ARFAmpCfg424]);
            RegWrite(REG_RXTHRESHOLD, RCRegFactor[RxAThres424]);
            pTable = CfgTbl_A424Rx;
            break;
        case TYPEA_848RX:
            RegWrite(REG_RFCFG, RCRegFactor[ARFAmpCfg848]);
            RegWrite(REG_RXTHRESHOLD, RCRegFactor[RxAThres848]);
            pTable = CfgTbl_A848Rx;
            break;
        case TYPEB_106RX:
            RegWrite(REG_RFCFG, RCRegFactor[BRFAmpCfg106]);
            RegWrite(REG_RXTHRESHOLD, RCRegFactor[RxBThres106]);
            pTable = CfgTbl_B106Rx;
            break;
        case TYPEB_212RX:
            RegWrite(REG_RFCFG, RCRegFactor[BRFAmpCfg212]);
            RegWrite(REG_RXTHRESHOLD, RCRegFactor[RxBThres212]);
            pTable = CfgTbl_B212Rx;
            break;
        case TYPEB_424RX:
            RegWrite(REG_RFCFG, RCRegFactor[BRFAmpCfg424]);
            RegWrite(REG_RXTHRESHOLD, RCRegFactor[RxBThres424]);
            pTable = CfgTbl_B424Rx;
            break;
        default:
            RegWrite(REG_RFCFG, RCRegFactor[BRFAmpCfg848]);
            RegWrite(REG_RXTHRESHOLD, RCRegFactor[RxBThres848]);
            pTable = CfgTbl_B848Rx;
            break;
    }
    
    i = 0;
    regAddr = pTable[i++];
    while(regAddr) 
    {
        RegWrite(regAddr,pTable[i++]);
        regAddr = pTable[i++];
    }
}

/*****************************************************************/
//       START   A   PCD   COMMAND 
/*****************************************************************/
/** This command assumes all data has been written to the FIFO **/
UINT8 PcdHandlerCmd(UINT8 Cmd, UINT8 errorCheckFlag)
{
    UINT8 ret = ERROR_NO;
    UINT8 WaitFor = 0;
    UINT8 temp = 0x00;


//    PrtMsg(DBGL6, "%s: start, Cmd = %X\n", __FUNCTION__, Cmd);    

    switch(Cmd) 
    {         
        case CMD_TRANSMIT:           
            WaitFor = 0x40;
            break;
            
        case CMD_IDLE:
        case CMD_CALCCRC:
        case CMD_CONFIG:
        case CMD_MFAUTHENT:               // IdleIRq
            WaitFor = 0x10; 
            break;
            
        case CMD_RECEIVE:                
        case CMD_TRANSCEIVE:             // RxIrq
            WaitFor = 0x20;
            break;
            
        case CMD_TRANSCEIVE_TO:
            Cmd = CMD_TRANSCEIVE;
            break;
            
        default:
            ret = ERROR_UNKNOW_COMMAND;
            break;
    }        

    if(ret == ERROR_NO) 
    {
        RegWrite(REG_COMMIRQ, WaitFor);         // Clear the corresponding bits first
        RegWrite(REG_COMMAND, Cmd);

        if(Cmd == CMD_TRANSCEIVE)
        {
            SetRegBit(REG_BITFRAMING, BIT_STARTSEND);     //Start transmission
        }
        WaitFor |= 0x01;

        while(!(temp & WaitFor))
        {
            temp = RegRead(REG_TCOUNTERVAL_HI);
            temp = RegRead(REG_TCOUNTERVAL_LO);
            temp = RegRead(REG_COMMIRQ);
        }

        SetRegBit(REG_CONTROL, BIT_TSTOPNOW);            // Stop Timer Now
        if(temp & 0x01) 
        {
            ret = ERROR_NOTAG;                  // Time Out Error
        }       
        WaitFor = RegRead(REG_ERROR) & (errorCheckFlag | 0x11);
        if(ret == ERROR_NO) 
        {
            // no timeout error occured
            if(WaitFor)  
            { 
                // error occured
                if(BITISSET(WaitFor, BIT_COLLERR))  
                { 
                    ret = ERROR_COLL;                              // collision detected
                } 
                else 
                {
                    if(BITISSET(WaitFor, BIT_PARITYERR))
                    {  
                        ret = ERROR_PARITY;                        // parity error
                    }
                }
                if(BITISSET(WaitFor, BIT_PROTOCOLERR))
                {  
                    ret = ERROR_PROTOCOL;                    // framing error
                }
                if(BITISSET(WaitFor, BIT_BUFFEROVFL) )
                {  
                    FIFOFlush();                          // FIFO overflow
                    ret = ERROR_BUFOVFL;
                }

                if(BITISSET(WaitFor, BIT_CRCERR))
                {
                    ret = ERROR_CRC;
                }
            }
        }
        
        RegWrite(REG_COMMAND, CMD_IDLE);  // Reset Command Register
    }

//    PrtMsg(DBGL6, "%s: exit, ret = %02X\n", __FUNCTION__, ret);
  
    return(ret);
}



/*****************************************************************/
//       Start   PCD   Transparent  Command
/*****************************************************************/
UINT8 PcdRawExchange(UINT8 Cmd, UINT8 *senBuf, UINT8 senLen, UINT8 *recBuf, UINT8 *recLen)      
{
    UINT8 i = 0;
    UINT8 tempLen;
    

    if(senLen)
    {
        tempLen = (senLen < MAX_FIFO_LENGTH) ? senLen : MAX_FIFO_LENGTH;
        FIFOWrite(senBuf, tempLen);
        senLen -= tempLen;
        i += tempLen;
    }
    
    // Timer Configuration
    RegWrite(REG_COMMIRQ, 0x61);      // Clear the corresponding bits first      
    RegWrite(REG_COMMAND, Cmd);
    if(Cmd == CMD_TRANSCEIVE)
    {
        SetRegBit(REG_BITFRAMING, BIT_STARTSEND);     //Start transmission
    }
    while(senLen)
    {
        if((RegRead(REG_FIFOLEVEL)) < 0x30)
        {
            tempLen = (senLen<0x0E) ? senLen : 0x0E;
            FIFOWrite(senBuf+i,tempLen);
            senLen -= tempLen;
            i += tempLen;
        }
    }
    while(RegRead(REG_FIFOLEVEL));
    if(Cmd == CMD_TRANSMIT)
    {
        return(ERROR_NO);
    }

    if(pcsc.fgStatus)
    {
        if(BITISSET(pcsc.fgTxRx, BIT_RXPARITY))
        {
            SetRegBit(REG_MANUALRCV, BIT_PARITYDISABLE);        //Parity disable 
        }
        else
        {
            ClearRegBit(REG_MANUALRCV, BIT_PARITYDISABLE);      //Parity enable
        }
    }

    while((RegRead(REG_STATUS2) & 0x07) < 0x05);
    tempLen = 0;
    i = 0;
    while(!(RegRead(REG_COMMIRQ) & 0x21))
    {
        tempLen = RegRead(REG_FIFOLEVEL);
        if(tempLen > 10)
        {
            FIFORead(recBuf + i, tempLen);
            i += tempLen;
            if(i > FSDLENTH)
            {
                return(ERROR_FSDLENTH);
            }
        }
    }
    tempLen = RegRead(REG_FIFOLEVEL);
    FIFORead(recBuf + i, tempLen);
    i += tempLen;
    if(i > FSDLENTH)
    {
        return(ERROR_FSDLENTH);
    }

   	if(RegRead(REG_COMMIRQ) & 0x01) 
   	{
        return(ERROR_NOTAG);             // Time Out Error
   	}

    return(ERROR_NO);
}

