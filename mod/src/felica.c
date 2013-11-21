



#include <linux/string.h>

#include "common.h"
#include "picc.h"
#include "felica.h"
#include "debug.h"



static const u8 gaFelRFCon[RF_FEL_CON_LEN] = {0x11, 0x85, 0x69, 0x3f, 0x11, 0x55, 0x69, 0x3f};

static const u8 CfgTbl_Pas106i[] =
{
    ModeReg,        0x39,    // CRCPreset = 6363H
    TxModeReg,      0x80,    // ISO/IEC 14443A/MIFARE and 106 kbit, TxCRCEn On
    RxModeReg,      0x80,    // ISO/IEC 14443A/MIFARE and 106 kbit, RxCRCEn On 		
    BitFramingReg,  0x00,
    RFCfgReg,       0x59,
    GsNOnReg,       0xF4,
    CWGsPReg,       0x3F,
    ManualRCVReg,   0x00,
    TxAutoReg,      0x43,
    DemodReg,       0x4D,
    RxThresholdReg, 0x55,
    ModWidthReg,    0x26,
    RxSelReg,       0x84,
    0x00
};

static const u8 CfgTbl_Pas212i[] =
{
    ModeReg,       0xB8,    // CRCPreset = 0000H
    TxModeReg,     0x92,
    RxModeReg,     0x92,
    BitFramingReg, 0x00,
    GsNOnReg,      0xF4,
    GsNOffReg,     0x6F,
    ManualRCVReg,  0x10,
    TxAutoReg,     0x03,
    DemodReg,      0x41,
    TxBitPhaseReg, 0x8F,
    RxSelReg,      0x84,
    0x00
};

static const u8 CfgTbl_Pas424i[] =
{
    ModeReg,       0xB8,    // CRCPreset = 0000H
    TxModeReg,     0xA2,
    RxModeReg,     0xA2,
    BitFramingReg, 0x00,
    GsNOnReg,      0xFF,
    GsNOffReg,     0x6F,
    ManualRCVReg,  0x10,
    TxAutoReg,     0x03,
    DemodReg,      0x41,
    TxBitPhaseReg, 0x8F,
    0x00
};

static const u8 CfgTbl_Act106i[] =
{
    ModeReg,        0x39,    // CRCPreset = 6363H
    TxModeReg,      0x81,    // ISO/IEC 14443A/MIFARE and 106 kbit, TxCRCEn On
    RxModeReg,      0x81,    // ISO/IEC 14443A/MIFARE and 106 kbit, RxCRCEn On 		
    BitFramingReg,  0x00,
    RFCfgReg,       0x59,
    GsNOnReg,       0xF4,
    GsNOffReg,      0x6F,
    CWGsPReg,       0x3F,
    ManualRCVReg,   0x00,
    TxAutoReg,      0xCB,    //
    DemodReg,       0x61,
    RxThresholdReg, 0x55,
    ModWidthReg,    0x26,
    TxBitPhaseReg,  0x8F,
    0x00
};

static const u8 CfgTbl_Act212i[] =
{
    ModeReg,        0xB8,    // CRCPreset = 0000H
    TxModeReg,      0x91,
    RxModeReg,      0x91,
    BitFramingReg,  0x00,
    RFCfgReg,       0x69,
    GsNOnReg,       0xFF,
    GsNOffReg,      0x6F,
    CWGsPReg,       0x3F,
    ManualRCVReg,   0x10,
    TxAutoReg,      0x8B,
    DemodReg,       0x61,
    RxThresholdReg, 0x55,
    ModGsPReg,      0x11,
    TxBitPhaseReg,  0x8F,
    0x00
};

static const u8 CfgTbl_Act424i[] =
{
    ModeReg,        0xB8,    // CRCPreset = 0000H
    TxModeReg,      0xA1,
    RxModeReg,      0xA1,
    BitFramingReg,  0x00,
    RFCfgReg,       0x69,
    GsNOnReg,       0xFF,
    GsNOffReg,      0x6F,
    CWGsPReg,       0x3F,
    ManualRCVReg,   0x10,
    TxAutoReg,      0x8B,
    DemodReg,       0x61,
    RxThresholdReg, 0x55,
    ModGsPReg,      0x11,
    TxBitPhaseReg,  0x8F,
    0x00
};






static void nfc_init_config(u8 NFCType)
{
    u8 Addr;
    const u8 *pTable;
    u8 i;


    switch(NFCType)
    {
        case PASSDEPI_106:
        case PASSDEPT_106:
            pTable = CfgTbl_Pas106i;
            break;
        case PASSDEPI_212:
        case PASSDEPT_212:
            pn512_reg_write(ModGsPReg, gaFelRFCon[FEL2_CON_ModGsP]);
            pn512_reg_write(RxThresholdReg, gaFelRFCon[FEL2_CON_RxThres]);
            pn512_reg_write(RFCfgReg, gaFelRFCon[FEL2_CON_RFCfg]);
            pn512_reg_write(CWGsPReg, gaFelRFCon[FEL2_CON_CWGsP]);
            pTable = CfgTbl_Pas212i;
            break;
        case PASSDEPI_424:
        case PASSDEPT_424:
            pn512_reg_write(ModGsPReg, gaFelRFCon[FEL4_CON_ModGsP]);
            pn512_reg_write(RxThresholdReg, gaFelRFCon[FEL4_CON_RxThres]);
            pn512_reg_write(RFCfgReg, gaFelRFCon[FEL4_CON_RFCfg]);
            pn512_reg_write(CWGsPReg, gaFelRFCon[FEL4_CON_CWGsP]);
            pTable = CfgTbl_Pas424i;
            break;
        case ACTDEPI_106:
        case ACTDEPT_106:
            pTable = CfgTbl_Act106i;
            break;
        case ACTDEPI_212:
        case ACTDEPT_212:
            pTable = CfgTbl_Act212i;
            break;
        default:
            pTable = CfgTbl_Act424i;
            break;
    }
    
    i = 0;
    Addr = pTable[i++];
    while(Addr) 
    {
        pn512_reg_write(Addr,pTable[i++]);
        Addr = pTable[i++];
    }
}
#if 0
/*****************************************************************/
//       START   A   PCD   transceive COMMAND 
/*****************************************************************/
static INT32 FelicaBulkTransceive(UINT8 *senBuf, UINT32 senLen, UINT32 MaxFirstFIFOLen, UINT8 *recBuf, UINT32 *recLen, BOOL parityCheck)      
{
    INT32 ret = 0;
    UINT32 i = 0;
    UINT32 tempLen;
    UINT8 waitFor;
    UINT8 temp;


//    PrtMsg(DBGL5, "%s: start\n", __FUNCTION__);

    i = 0;
    if(senLen)
    {
        tempLen = (senLen < MaxFirstFIFOLen) ? senLen : MaxFirstFIFOLen;
        pn512_fifo_write(senBuf, tempLen);
        senLen -= tempLen;
        i += tempLen;
    }
    pn512_reg_write(REG_COMMIRQ, 0x21);               // Clear the Rx Irq bit first
    pn512_reg_write(REG_COMMAND, CMD_TRANSCEIVE);
    pn512_reg_write(REG_BITFRAMING, 0x80);            // Start transmission

    while(senLen)
    {
        if(pn512_reg_read(REG_FIFOLEVEL) < 0x30)      // length < 48 ?
        {
            tempLen = (senLen < 0x0E) ? senLen : 0x0E;    // 0x0E ???
            pn512_fifo_write(senBuf + i, tempLen);
            senLen -= tempLen;
            i += tempLen;
        }
    }

    temp = 0;
    while((temp & 0x07) < 0x05)   //the value of the receicing is 110, Receving state
    {
        temp = pn512_reg_read(REG_STATUS2);
//        PrtMsg(DBGL4, "%s: temp = %02X\n", __FUNCTION__, temp);
        if(temp == 0x01)
        {
            pn512_reg_set(REG_BITFRAMING, BIT_STARTSEND);
        }
    }

    tempLen = 0;
    i = 0;
    while(!(pn512_reg_read(REG_COMMIRQ) & 0x21))
    {
        tempLen = pn512_reg_read(REG_FIFOLEVEL);
        if(tempLen > 1)                            //  >10
        {
            pn512_fifo_read(recBuf + i, tempLen);
            i += tempLen;
            if(i > FELINFFIELDLEN)
            {
                return(-ERROR_FSDLENTH);
            }
        }
    }
    tempLen = pn512_reg_read(REG_FIFOLEVEL);
    pn512_fifo_read(recBuf + i, tempLen);
    i += tempLen;
    if(i > FELINFFIELDLEN)
    {
        return(-ERROR_FSDLENTH);
    }
    
    *recLen = i;
    if(pn512_reg_read(REG_COMMIRQ) & 0x01) 
    {
        ret = -ERROR_NOTAG;         // Time Out Error
    }
   	else
   	{
        waitFor = pn512_reg_read(REG_ERROR) & 0x17;
        if(waitFor)  
        { 
            if(parityCheck)
            {
                if(waitFor & BIT_PARITYERR)
                {  
                    // parity error
                   	ret = -ERROR_PARITY;
                }        
            }
            if(waitFor & BIT_PROTOCOLERR) 
            {  
                // protocol error
                ret = -ERROR_PROTOCOL;
            }
            if(waitFor & BIT_BUFFEROVFL) 
            { 
                // FIFO overflow
                pn512_reg_write(REG_FIFOLEVEL, 0x80);
                ret = -ERROR_BUFOVFL;
            }
            if(waitFor & BIT_CRCERR) 
            {
                 ret = -ERROR_CRC;
            }
        }
    }

//    PrtMsg(DBGL5, "%s: exit, ret = %02X\n", __FUNCTION__, ret);

    return(ret);
}
#endif


/******************************************************************/
//       Configure NFC Type
/******************************************************************/
static int felica_request_REQC(struct picc_device *picc, u8 timeSlot, u16 SystemCode)
{
    int ret;
    u8 tempBuf[20];
	struct pn512_request	*req = picc->request;


    pn512_reg_set(TxAutoReg, InitialRFOn);    // Enable RF initial on
    pn512_reg_set(TxModeReg, TxCRCEn);    // Enable TxCRC, 
    pn512_reg_set(RxModeReg, RxCRCEn);
    pn512_reg_set(RxModeReg, RxMultiple);    // Enable RxCRC, Rx Mutiple
    pn512_reg_clear(Status2Reg, MFCrypto1On);              // Disable crypto 1 unit

    pn512_reg_read(TxAutoReg);
    pn512_reg_read(TxModeReg);
    pn512_reg_read(RxModeReg);
    pn512_reg_read(Status2Reg);


	req->buf[0] = 0x06;
	req->buf[1] = CMD_REQC;
	req->buf[2] = SystemCode >> 8;
	req->buf[3] = SystemCode;
	req->buf[4] = 0x00;
	req->buf[5] = timeSlot;
	req->length = 6;
	req->bit_frame = 0x00;
	req->command = CMD_TRANSCEIVE;
	req->direction = TRANSCEIVE;
	req->time_out = 8000;
	
	picc_wait_for_req(req);
	memcpy(tempBuf, req->buf, req->actual);
	ret = req->error_code;

//	TRACE_TO("%s ret=%d\n", __func__, req->error_code);
	
    pn512_reg_clear(RxModeReg, RxMultiple);
    if((!ret) || (ret == -ERROR_NOTAG) || (ret == -ERROR_CRC))
    {
		if(req->actual < 0x12)
        {
            ret = -ERROR_BYTECOUNT;
			goto err;
        }
        if(((tempBuf[0] != 0x12) && (tempBuf[0] != 0x14)) || (tempBuf[1] != RES_REQC))
        {
            ret = -ERROR_WRONGPARAM;
			goto err;
        }
        
        memcpy(picc->sn, tempBuf + 2, 8);    // NFCID2, 8 bytes

        INFO_TO("NFCID: %02X %02X %02X %02X %02X %02X %02X %02X\n", 
        		tempBuf[2], tempBuf[3], tempBuf[4], tempBuf[5], tempBuf[6], tempBuf[7], tempBuf[8], tempBuf[9]);

		picc->sn_len = 8;
        memcpy(picc->PAD, tempBuf + 10, 8);
        if(tempBuf[0] == 0x14)
        {
            memcpy(picc->system_code, tempBuf + 18, 2);
        }

		ret = 0;
    }

err:
//	TRACE_TO("exit %s\n", __func__);
	
    return(ret);
}

void felica_polling_tags(struct picc_device *picc, u8 feliType)
{

//	TRACE_TO("enter %s\n", __func__);

    nfc_init_config(feliType);

    if (!felica_request_REQC(picc, CODE_TIMESLOTNUMBER_1, FELICA_APPLICATION_ALL))
    {
        if (feliType == PASSDEPI_212)
        {
            picc->type = PICC_FELICA212;
			picc->name = "felica 212";
            picc->pcd->current_speed |= 0x09;        // ~ 212 kbit/s)
        }
        else
        {
            picc->type = PICC_FELICA424;
			picc->name = "felica 414";
            picc->pcd->current_speed |= 0x12;        // ~ 424 kbit/s)
        }
    }
    else
    {
        picc->type = PICC_ABSENT;
		picc->name = "none";
    }

//	TRACE_TO("exit %s\n", __func__);
}

/*********************************************/
//    Set the PN512 timer clock for FeliCa use
/********************************************/
static void felica_timer_set(struct picc_device *picc, u8 *FelCommandBuf)
{
    u8 timeout;
    u8 TempN;
    u16  ReloadVal;
    

    pn512_reg_write(TModeReg, 0x88);         //TAuto=1,TAutoRestart=0,TPrescaler=2047=7FFh
    pn512_reg_write(TPrescalerReg, 0x00);    // Indicate 302us per timeslot

    if((FelCommandBuf[1]&0x01) || (FelCommandBuf[1]>0x17))
    {
        timeout = 0x30;
        TempN = FelCommandBuf[0];
    }
    else
    {
        if(FelCommandBuf[1] == CMD_REQRESPONSE)
        {
            timeout = picc->PAD[3];
            TempN = 0;
        }
        else if(FelCommandBuf[1] == CMD_REQSERVICE)
        {
            timeout = picc->PAD[2];
            TempN = FelCommandBuf[10];                 //Number of service
        }
        else if(FelCommandBuf[1] == CMD_READFROMSECURE)
        {
            timeout = picc->PAD[5];
            TempN = FelCommandBuf[10];                 // Number of service
        }
        else if(FelCommandBuf[1] == CMD_WRITETOSECURE)
        {
            timeout = picc->PAD[6];
            TempN = FelCommandBuf[10];                 // Number of service
        }
        else if(FelCommandBuf[1] == CMD_READ_NONEAUTH)
        {
            timeout = picc->PAD[5];
            TempN = FelCommandBuf[10+FelCommandBuf[10]*2];   //Number of Blocks
        }
        else if(FelCommandBuf[1] == CMD_WRITE_NONEAUTH)
        {
            timeout = picc->PAD[6];
            TempN = FelCommandBuf[10+FelCommandBuf[10]*2];   //Number of Blocks
        }
        else if(FelCommandBuf[1] == CMD_AUTH1)
        {
            timeout = picc->PAD[4];
            TempN = FelCommandBuf[10+FelCommandBuf[10]*2];  
            TempN += FelCommandBuf[10];
        }
        else if(FelCommandBuf[1] == CMD_AUTH2)
        {
            timeout = picc->PAD[4];
            TempN = 0;
        }
        else
        {
            timeout = 0xC0;
            TempN = 0;
        }
    }

    // response time = T * [(B + 1) * n + (A + 1)] * 4 ^ E
    ReloadVal  = (((timeout & 0x38) >> 3) + 1) * TempN;
    ReloadVal += (timeout & 0x07) + 1;
    if(timeout & 0xC0)
    {
        TempN = 4 << (timeout >> 5);
        TempN >>= 2;
    }
    else
    {
        TempN = 1;
    }
    ReloadVal *= TempN;
    pn512_reg_write(TReloadVal_Hi, (u8)(ReloadVal >> 8));
    pn512_reg_write(TReloadVal_Lo, (u8)ReloadVal);
    pn512_reg_write(CommIRqReg, 0x01);                           // Clear the TimerIrq bit
}


int felica_request_response(struct picc_device *picc)
{
    int ret = 0;
    u8 tempBuf[11];
	struct pn512_request	*req = picc->request;


//	TRACE_TO("enter %s\n", __func__);

	req->buf[0] = tempBuf[0] = picc->sn_len + 2;
	req->buf[1] = tempBuf[1] = CMD_REQRESPONSE;
	memcpy(req->buf+2, picc->sn, picc->sn_len);
	req->length = picc->sn_len+2;
	req->bit_frame = 0x00;
	req->command = CMD_TRANSCEIVE;
	req->direction = TRANSCEIVE;
	req->time_out = 0;
	felica_timer_set(picc, tempBuf);
	picc_wait_for_req(req);


    if(!req->error_code)
    {
		if(req->actual != 11)
        {
            ret = -ERROR_BYTECOUNT;
			goto err;
        }
        if((req->buf[0] != 11) || (req->buf[1] != RES_REQRESPONSE))
        {
            ret = -ERROR_WRONGPARAM;
			goto err;
        }
    }

	ret = req->error_code;

err:
	
//	TRACE_TO("exit %s, ret = %d\n", __func__, ret);
	
    return(ret);
}


#if 0
int FelXfrHandle(struct piccInfo *picc, UINT8 *cmdBuf, UINT32 cmdLen, UINT8 *resBuf, UINT32 *resLen)
{
//    INT32 ret = 0;
    UINT32 tempLen = 0;
	struct pn512_request	*req = &picc->request;


//    FIFOFlush();
    if(cmdLen > FELINFFIELDLEN)
    {
        resBuf[0] = 0x67;
        resBuf[1] = 0x00;
        *resLen = 2;
        return(0);
    }
    
//    FelTimerSet(cmdBuf);
//    ret = FelicaBulkTransceive(cmdBuf, cmdLen, MAX_FIFO_LENGTH, resBuf, &tempLen, FLAG_PARITYCHECK);

	memcpy(req->buf, cmdBuf, cmdLen);
	req->length = cmdLen;
	req->actual = 0;
	req->bit_frame = 0x00;
	req->command = CMD_TRANSCEIVE;
	req->direction = TRANSCEIVE;
	req->time_out = 0;
	FelTimerSet(picc, cmdBuf);
	picc_wait_for_req(req);

//    if(!ret)
	if(!req->error_code)
    {
        *resLen = tempLen;
//         ret = 0;
    }
    else
    {
        resBuf[0] = 0x64;
        resBuf[1] = 0x01;
        *resLen = 2;
//        ret = 0;
    }
    
    return 0;
}
#endif

int felica_xfr_handler(struct picc_device *picc, u8 *cmdBuf, u32 cmdLen, u8 *resBuf, u32 *resLen)
{
	struct pn512_request	*req = picc->request;

	
//	TRACE_TO("enter %s\n", __func__);

    if(cmdLen > 254)
    {
        resBuf[0] = 0x67;
        resBuf[1] = 0x00;
        *resLen = 2;
        return(0);
    }

	memcpy(req->buf, cmdBuf, cmdLen);
	req->length = cmdLen;
	req->bit_frame = 0x00;
	req->command = CMD_TRANSCEIVE;
	req->direction = TRANSCEIVE;
	req->time_out = 0;
	felica_timer_set(picc, cmdBuf);
	picc_wait_for_req(req);

	if(!req->error_code)
    {
    	memcpy(resBuf, req->buf, req->actual);
        *resLen = req->actual;
    }
    else
    {
        resBuf[0] = 0x64;
        resBuf[1] = 0x01;
        *resLen   = 2;
    }

//	TRACE_TO("exit %s, ret=%d, actual=%d\n", __func__, req->error_code, req->actual);
	
    return 0;
}


