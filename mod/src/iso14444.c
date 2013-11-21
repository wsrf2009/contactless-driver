



#include <linux/kernel.h>
#include <linux/string.h>

#include "common.h"
#include "picc.h"
#include "iso14444.h"

#include "delay.h"
#include "iso14443_typeA.h"
#include "iso14443_typeB.h"
#include "pcd_config.h"
#include "debug.h"

const u16 fsdi_to_fsd[] = {16, 24, 32, 40, 48, 64, 96, 128, 256};


static const u8 fwi_table[]=
{
// (330us) FWI=0
    0x00,0x11,
    0x00,0x80,
// (661us) FWI=1
    0x00,0x11,
    0x01,0x00,
// (1322us) FWI=2
    0x00,0x11,
    0x02,0x00,
// (2643us) FWI=3
    0x00,0x11,
    0x04,0x00,
// (5286us) FWI=4
    0x00,0x11,
    0x08,0x00, 
// (10.572ms) FWI=5
    0x00,0x44,
    0x04,0x16, 
// (21.14ms) FWI=6
    0x00,0x44,
    0x08,0x2C, 
// (42.29ms) FWI=7
    0x00,0x44,
    0x10,0x59, 
// (84.58ms) FWI=8
    0x00,0x44,
    0x20,0xB3, 
// (169.2ms) FWI=9
    0x02,0xA5,
    0x06,0x9C, 
// (338.3ms) FWI=10
    0x02,0xA5,
    0x0C,0x17, 
// (676.6ms;) FWI=11
    0x02,0xA5,
    0x1A,0x6E, 
// (1353.3ms) FWI=12
    0x02,0xA5,
    0x34,0xDD,
// (2706.5ms) FWI=13
    0x02,0xA5,
    0x69,0xB9,
// (5413.0ms) FWI=14
    0x02,0xA5,
    0xD3,0x72,
};


const u16 fsdi_to_fsd_convertion[] = {16, 24, 32, 40, 48, 64, 96, 128, 256};


u8 tempIBlockBuf[253];




/****************************************************************/
//       Type A RATS
/****************************************************************/
int typeA_request_ats(struct picc_device *picc)
{
    int ret = 0;
    u8 i;
    u8 retry = 0;
	struct pn512_request	*req = picc->request;

//	TRACE_TO("enter %s\n", __func__);
    while(retry < 3)
    {
        pn512_reg_set(TxModeReg, TxCRCEn);        //TXCRC enable
        pn512_reg_set(RxModeReg, RxCRCEn);        //RXCRC enable
        pn512_reg_clear(Status2Reg, MFCrypto1On); // disable crypto 1 unit                             add.....
        
		req->buf[0] = 0xE0;
		req->buf[1] = picc->pcd->FSDI << 4 | picc->CID;
		req->length = 2;
		req->bit_frame = 0x00;
		req->command = CMD_TRANSCEIVE;
		req->direction = TRANSCEIVE;
		req->time_out = 60;
		
		picc_wait_for_req(req);

        picc->ATS[0] = req->buf[0];
        retry++;
		ret = req->error_code;
        if((ret) && (retry >= 2))
        {
            break;
        }
        
        if(!ret)
        {
            // store ATS
			if(req->actual != picc->ATS[0])
            {
                ret = -ERROR_ATSLEN;
				goto err;
            }
           
            picc->states = PICC_ACTIVATED;
            
            CLEAR_BIT(picc->flags_TCL, PCD_BLOCK_NUMBER);    // reset block number
            SET_BIT(picc->flags_TCL, CID_PRESENT);          // CID supported by PICC by default
            
            if(req->actual < 2) 
            {
                goto err;
            }
            else
            {
				memcpy(&picc->ATS[1], &req->buf[1], req->actual-1);
                
                picc->FSCI = picc->ATS[1] & 0x0F;    // T0: format byte, codes Y(1) and FSCI
                if(picc->FSCI > 8)
                {
                    picc->FSCI = 8;
                }
                
                i = 1;
                if(picc->ATS[1] & 0x10)
                {
                    // TA(1) present
                    i++;
                    picc->speed = picc->ATS[i];
                    if(picc->speed & 0x08)
                    {
                        // b4 = 1 should be interpreted by the PCD as (b8 to b1) = (00000000)b (only ~106 kbit/s in both directions).
                        picc->speed = 0x00;    // complaint to ISO14443-4: 2008
                    }
                }
                if(picc->ATS[1] & 0x20)
                {
                    // TB(1) present
                    i++;
                    picc->FWI = (picc->ATS[i] & 0xF0) >> 4;   // Set FWT
                    if(picc->FWI > 14) 
                    {
                        // Until the RFU value 15 is assigned by ISO/IEC, a PCD receiving FWI = 15 should interpret it as FWI = 4.
                        picc->FWI = 4;    // complaint to ISO14443-4: 2008
                    }
                    picc->SFGI = picc->ATS[i] & 0x0F;
                    if(picc->SFGI > 14) 
                    {
                        // Until the RFU value 15 is assigned by ISO/IEC, a PCD receiving SFGI = 15 should interpret it as SFGI = 0.
                        picc->SFGI = 0;    // complaint to ISO14443-4: 2008
                    }
                }
                if(picc->ATS[1] & 0x40)
                {   
                    // TC(1) present
                    i++;
                    if((picc->ATS[i] & 0x02) == 0x00)
                    {
                        // CID do not supported by the PICC
                        CLEAR_BIT(picc->flags_TCL, CID_PRESENT);
                    }
                }
                break;
            }
        }
    }
err:
//	TRACE_TO("exit %s\n", __func__);
    return ret;
}


static void typeA_sfg_delay(u8 delay)
{

    switch(delay)
    {
        case 1:            //661us
            Delay1us(200);
            break;
        case 2:            //1322us
            Delay1us(600);
            break;
        case 3:            //2643us
            Delay1us(1900);
            break;
        case 4:            //5286us
            Delay1us(4500);
            break;
        case 5:            //10.572ms
            Delay1us(9800);
            break;
        case 6:            //21.14ms
            Delay1ms(20);
            Delay1us(400);
            break;
        case 7:            //42.29ms
            Delay1ms(40);
            Delay1us(1500);
            break;
        case 8:            //84.58ms
            Delay1ms(80);
            Delay1us(3800);
            break;
        case 9:            //169.2ms
            Delay1ms(160);
            Delay1us(8500);
            break;
        case 10:           // 338.3ms
            Delay1ms(330);
            Delay1us(7600);
            break;
        case 11:           //676.6ms
            Delay1ms(670);
            Delay1us(5900);
            break;
        case 12:           //1353.3ms
            Delay1ms(1350);
            Delay1us(2600);
            break;
        case 13:           //2706.5ms
            Delay1ms(2500);
            Delay1ms(200);
            Delay1us(5800);
            break;
        case 14:           //5413.0ms
            Delay1s(5);
            Delay1ms(410);
            Delay1us(2300);
            break;
        default:          //330us
            break;

    }
}


void typeA_set_timeout(struct picc_device *picc, u8 timeout)
{
    u16 preScaler;
    u16 reloadValue;
    u8  tempWTXM = picc->WTXM;


    preScaler   = MAKEWORD(fwi_table[timeout * 4],     fwi_table[timeout * 4 + 1]);
    reloadValue = MAKEWORD(fwi_table[timeout * 4 + 2], fwi_table[timeout * 4 + 3]);

    if(picc->type == PICC_TYPEB_TCL)
    {
        reloadValue += 8;
        if(BITISSET(picc->flags_TCL, TYPEB_ATTRIB))
        {
            reloadValue += 8;
        }
        if(!timeout)
        {
            reloadValue += 20;
        }
    }
    if(BITISSET(picc->flags_TCL, WTX_REQUEST))
    {
        if(tempWTXM > 59)
        {
            tempWTXM = 59;
        }
        if(timeout < 9)
        {
            preScaler *= tempWTXM;
            if(preScaler > 0x0FFF)
            {
                preScaler = 0x0FFF;
            }
        }
        else
        {
            reloadValue *= tempWTXM;
        }
        CLEAR_BIT(picc->flags_TCL, WTX_REQUEST);
    }
    pn512_reg_write(TModeReg, (u8)(preScaler>>8) | 0x80);
    pn512_reg_write(TPrescalerReg, (u8)preScaler);
    pn512_reg_write(TReloadVal_Hi, (u8)(reloadValue>>8));
    pn512_reg_write(TReloadVal_Lo, (u8)reloadValue);
    pn512_reg_write(CommIRqReg,0x01);          	// Clear the TimerIrq bit
}


u8 typeA_speed_check(struct picc_device *picc)
{
    u8 priByte;
    u8 curByte;


    picc->pcd->current_speed = 0x80;
    priByte = 0x00;
    
    if(picc->speed & 0x01)
    {
        // DR = 2 supported, PCD to PICC, 1 etu = 64 / fc, bit rate supported is fc / 64 (~ 212 kbit/s)
        priByte = 0x01;
    }
    if(picc->speed & 0x02)
    {
        // DR = 4 supported, PCD to PICC, 1 etu = 32 / fc, bit rate supported is fc / 32 (~ 424 kbit/s)
        priByte = 0x02;
    }
    if(picc->speed & 0x04)
    {
        // DR = 8 supported, PCD to PICC, 1 etu = 16 / fc, bit rate supported is fc / 16 (~ 848 kbit/s)
        priByte = 0x03;
    }
    
    curByte = 0x00;
    
    if(picc->speed & 0x10)
    {
        // DS = 2 supported,  PICC to PCD, 1 etu = 64 / fc, bit rate supported is fc / 64 (~ 212 kbit/s)
        curByte = 0x01;
    }
    if(picc->speed & 0x20)
    {
        // DS = 4 supported, PICC to PCD, 1 etu = 32 / fc, bit rate supported is fc / 32 (~ 424 kbit/s)
        curByte = 0x02;
    }
    if(picc->speed & 0x40)
    {
        // DS = 8 supported, PICC to PCD, 1 etu = 16 / fc, bit rate supported is fc / 16 (~ 848 kbit/s)
        curByte = 0x03;
    }


    if(picc->speed & 0x80)    
    {
        // Only the same D for both directions supported , if bit is set to 1
        if(curByte > priByte)
        {
            curByte = priByte;
        }
        else
        {
            priByte = curByte;
        }
    }
    
    if(picc->speed & 0x08)
    {
        // if b4 = 1, b8 to b1 should be interpreted  as (00000000)b, accroding to ISO14443-4: 2008(typeA) and ISO14443-3: 2011(typeB)
        curByte = priByte = 0x00;
    }
    
    curByte = (curByte << 2) | priByte;
    
    return curByte;
}

void typeA_high_speed_config(struct picc_device *picc, u8 speedParam, u8 typeB)
{
    picc->pcd->current_speed = picc->speed & 0x80;
    if((speedParam & 0x03) == 0x03)
    {
        // DR = 8 supported, PCD to PICC, 1 etu = 16 / fc, bit rate supported is fc / 16 (~ 848 kbit/s)
        picc->pcd->current_speed |= 0x03;
        pcd_config_iso14443_card(CONFIGNOTHING, TYPEA_848TX | typeB);
    }
    else if((speedParam & 0x03) == 0x02)
    {
        // DR = 4 supported, PCD to PICC, 1 etu = 32 / fc, bit rate supported is fc / 32 (~ 424 kbit/s)
        picc->pcd->current_speed |= 0x02;
        pcd_config_iso14443_card(CONFIGNOTHING, TYPEA_424TX | typeB);
    }
    else if((speedParam & 0x03) == 0x01)
    {
        // DR = 2 supported, PCD to PICC, 1 etu = 64 / fc, bit rate supported is fc / 64 (~ 212 kbit/s)
        picc->pcd->current_speed |= 0x01;
        pcd_config_iso14443_card(CONFIGNOTHING, TYPEA_212TX | typeB);
    }
    if((speedParam & 0x0c) == 0x0c)
    {
        // DS = 8 supported,  PICC to PCD, 1 etu = 16 / fc, bit rate supported is fc / 16 (~ 848 kbit/s) 
        picc->pcd->current_speed |= 0x18;
        pcd_config_iso14443_card(CONFIGNOTHING, TYPEA_848RX | typeB);
    }
    else if((speedParam & 0x0c) == 0x08)
    {
        // DS = 4 supported, PICC to PCD, 1 etu = 32 / fc, bit rate supported is fc / 32 (~ 424 kbit/s)
        picc->pcd->current_speed |= 0x10;
        pcd_config_iso14443_card(CONFIGNOTHING, TYPEA_424RX | typeB);

    }
    else if((speedParam & 0x0c) == 0x04)
    {
        // DS = 2 supported,  PICC to PCD, 1 etu = 64 / fc, bit rate supported is fc / 64 (~ 212 kbit/s)
        picc->pcd->current_speed |= 0x08;
        pcd_config_iso14443_card(CONFIGNOTHING, TYPEA_212RX | typeB);
    }
}


void typeA_prologue_feild_load(struct picc_device *picc)
{
    if((picc->pcd->PCB & 0xC0) != 0xC0)
    {
        // I-block or R-block
        if(BITISSET(picc->flags_TCL, PCD_BLOCK_NUMBER))
        {
            picc->pcd->PCB |= 0x01;
        }
        else
        {
            picc->pcd->PCB &= 0xFE;
        }
    }
    if(BITISSET(picc->flags_TCL, CID_PRESENT))
    {
        picc->pcd->PCB |= 0x08;
    }
    else
    {
        picc->pcd->PCB &= 0xF7;
    }
    
}



/****************************************************************/
//       Type A PPS
/****************************************************************/
void typeA_pps_check_and_send(struct picc_device *picc)
{
    u8 speedParam;
    u8 RecBuf[5];
	struct pn512_request	*req = picc->request;
    
//	TRACE_TO("enter %s\n", __func__);
    speedParam = typeA_speed_check(picc);
    if(speedParam)
    {
        //************* Cmd Sequence **********************************//
		req->buf[0] = 0xD0 | picc->CID;
		req->buf[1] = 0x11;
		req->buf[2] = speedParam;
		typeA_sfg_delay(picc->SFGI);
		Delay1ms(5);
		req->length = 3;
		req->bit_frame = 0x00;
		req->command = CMD_TRANSCEIVE;
		req->direction = TRANSCEIVE;
		req->time_out = 0;
		typeA_set_timeout(picc, picc->FWI);
		picc_wait_for_req(req);

	
		if(!req->error_code)
		{
			memcpy(RecBuf, req->buf, req->actual);
            if((RecBuf[0] & 0xF0) == 0xD0)
            {
                // return PPSS,  PPS successful
                typeA_high_speed_config(picc, speedParam, TYPEA_106TX);
            }
        }
    }

//	TRACE_TO("exit %s\n", __func__);
}


int typeA_select_(struct picc_device *picc, u8 blockPCB)
{
    u8 tempFWI;
	struct pn512_request	*req = picc->request;


//	TRACE_TO("enter %s\n", __func__);   

    picc->pcd->PCB = blockPCB;
    typeA_prologue_feild_load(picc);
    tempFWI = picc->FWI;
    tempFWI = (tempFWI > 0x06) ? 0x06 : tempFWI;

	req->buf[0] = picc->pcd->PCB;
	if(BITISSET(picc->flags_TCL, CID_PRESENT))
	{
		req->buf[1] = picc->CID;
		req->length = 2;
	}
	else
		req->length = 1;

	req->bit_frame = 0x00;
	req->command = CMD_TRANSCEIVE;
	req->direction = TRANSCEIVE;
	req->time_out = 0;
	typeA_set_timeout(picc, tempFWI);
	picc_wait_for_req(req);
	
	printk("%s: ret = %d\n", __func__, req->error_code);

//	TRACE_TO("exit %s\n", __func__); 

    return(req->error_code);
}


int typeA_deselect_request(struct picc_device *picc)
{
    int ret = 0;
	struct pn512_request	*req = picc->request;

//	TRACE_TO("enter %s\n", __func__);
    
    if(picc->states == PICC_ACTIVATED)
    {
        picc->pcd->PCB = 0xC2;    // S-block, DESELECT
        typeA_prologue_feild_load(picc);

		req->buf[0] = picc->pcd->PCB;
		if(BITISSET(picc->flags_TCL, CID_PRESENT))
		{
			req->buf[1] = picc->CID;
			req->length = 2;
		}
		else
			req->length = 1;
		
		req->bit_frame = 0x00;
		req->command = CMD_TRANSCEIVE;
		req->direction = TRANSCEIVE;
		req->time_out = 53;

		picc_wait_for_req(req);
		

        if(picc->states != PICC_POWEROFF)
        {
            picc->states = PICC_IDLE;
        }
        if(picc->type == PICC_TYPEA_TCL)
        {
            pcd_config_iso14443_card(CONFIGNOTHING, TYPEA_106TX);
            pcd_config_iso14443_card(CONFIGNOTHING, TYPEA_106RX);
        }
        else if(picc->type == PICC_TYPEB_TCL)
        {
            pcd_config_iso14443_card(CONFIGNOTHING,TYPEB_106TX);
            pcd_config_iso14443_card(CONFIGNOTHING,TYPEB_106RX);
        }
    }

//	TRACE_TO("exit %s\n", __func__);
    return(ret);
}

static int typeA_rblock_command(struct picc_device *picc, u8 blockPCB, u8 *recBuf, u32 *recLen)
{
    int ret = 0;
	struct pn512_request	*req = picc->request;

//	TRACE_TO("enter %s\n", __func__);
    picc->pcd->PCB = blockPCB;
    typeA_prologue_feild_load(picc);

	req->buf[0] = picc->pcd->PCB;
	if(BITISSET(picc->flags_TCL, CID_PRESENT))
	{
		req->buf[1] = picc->CID;
		req->length = 2;
	}
	else
		req->length = 1;
	
	req->bit_frame = 0x00;
	req->command = CMD_TRANSCEIVE;
	req->direction = TRANSCEIVE;
	req->time_out = 0;
	typeA_set_timeout(picc, picc->FWI);
	picc_wait_for_req(req);
	memcpy(recBuf, req->buf, req->actual);
	*recLen = req->actual;
//	TRACE_TO("exit %s\n", __func__);
    return(ret);
}




static int typeA_data_send_error_check(struct picc_device *picc, u8 *senBuf, u32 senLen, u8 *recBuf, u32 *recLen)
{
    u8 timeoutRetry = 0;
    u8 frameRetry = 0;
    u8 ackRetry = 0;
    u32 i;
    int ret = 0;
    u8 resend;
    u8 resendCount;
	struct pn512_request	*req = picc->request;


//	TRACE_TO("enter %s\n", __func__);
    resendCount = 0;
    do
    {
        resend = 0;
        picc->pcd->PCB = picc->PCB;
        typeA_sfg_delay(picc->SFGI);
        typeA_prologue_feild_load(picc);

		req->buf[0] = picc->pcd->PCB;
		if(BITISSET(picc->flags_TCL, CID_PRESENT))
		{
			req->buf[1] = picc->CID;
			memcpy(req->buf+2, senBuf, senLen);
			req->length = 2+senLen;
		}
		else
		{
			memcpy(req->buf+1, senBuf, senLen);
			req->length = 1+senLen;
		}
		req->bit_frame = 0x00;
		req->command = CMD_TRANSCEIVE;
		req->direction = TRANSCEIVE;
		req->time_out = 0;
		typeA_set_timeout(picc, picc->FWI);
		picc_wait_for_req(req);
		memcpy(recBuf, req->buf, req->actual);
		*recLen = req->actual;
		ret = req->error_code;
        while(1)
        {
            //Error handling
            if(ret == -ERROR_NOTAG)            //Time Out 
            {
                CLEAR_BIT(picc->flags_TCL, WTX_REQ_BEFORE);
                if(BITISSET(picc->flags_TCL, PICC_CHAINING))
                {
                    if(ackRetry == 2)
                    {
                        ret = -PICC_ERRORCODE_MUTE;
                        break;
                    }

                    ret = typeA_rblock_command(picc, 0xA2, recBuf, recLen);    // R-Block: ACK
                    ackRetry++;
                }
                else
                {
                    if(timeoutRetry == 2)
                    {
                        ret = -PICC_ERRORCODE_MUTE;
                        break;
                    }
                    ret = typeA_rblock_command(picc, 0xB2, recBuf, recLen);   // R-Block: NAK
                    timeoutRetry++;
                }
            }
            else if(!ret)
            {
                CLEAR_BIT(picc->flags_TCL, WTX_REQ_BEFORE);
                if((recBuf[0] & 0xC0) == 0xC0)
                {
                    //S-Block received
                    if((recBuf[0] & 0x30) == 0x30)
                    {
                        //S(WTX) received
                        if(recBuf[0] & 0x08)
                        {
                            // CID following
                            if(BITISCLEAR(picc->flags_TCL, CID_PRESENT))
                            {
                                ret = (-SLOTERROR_TCL_BLOCK_INVALID);
								goto err;
                            }
                        }
                        if(recBuf[0] & 0x04)
                        {
                            ret = (-SLOTERROR_TCL_BLOCK_INVALID);
							goto err;
                        }
                        
                        if(BITISSET(picc->flags_TCL, CID_PRESENT))
                        {
                            picc->WTXM = recBuf[2];

                        }
                        else
                        {
                            picc->WTXM = recBuf[1];
                        }
                        if((picc->WTXM == 0) || (picc->WTXM > 59)) 
                        {
                            // WTXM must be code in the range from 1 to 59, or it will be treat as a protocol error. ISO/IEC 14443-4:2008
                            ret = (-SLOTERROR_TCL_BLOCK_INVALID);
							goto err;
                        }
                        picc->pcd->PCB = 0xF2;
                        typeA_sfg_delay(picc->SFGI);
                        typeA_prologue_feild_load(picc);

						req->buf[0] = picc->pcd->PCB;
						if(BITISSET(picc->flags_TCL, CID_PRESENT))
						{
							req->buf[1] = picc->CID;
							req->buf[2] = picc->WTXM;
							req->length = 3;
						}
						else
						{
							req->buf[1] = picc->WTXM;
							req->length = 2;
						}

						req->bit_frame = 0x00;
						req->command = CMD_TRANSCEIVE;
						req->direction = TRANSCEIVE;
						req->time_out = 0;
						typeA_set_timeout(picc, picc->FWI);
						picc_wait_for_req(req);
						memcpy(recBuf, req->buf, req->actual);
						*recLen = req->actual;						
						ret = req->error_code;
                        SET_BIT(picc->flags_TCL, WTX_REQUEST);    // the time FWTtemp starts after the PCD has sent the S(WTX) response
                        SET_BIT(picc->flags_TCL, WTX_REQ_BEFORE);
                    }
                    else
                    {
                        ret = (-SLOTERROR_TCL_BLOCK_INVALID);
						goto err;
                    }
                }
                else if((recBuf[0] & 0xC0) == 0x80)
                {
                    //R-Block received
                    if(recBuf[0] & 0x14)
                    {
                        ret = (-SLOTERROR_TCL_BLOCK_INVALID);  //PICC Never send a R(NAK) and Never used NAD
                        goto err;
                    }
                    if((recBuf[0] & 0x20) == 0x00)
                    {
                        ret = (-SLOTERROR_TCL_BLOCK_INVALID);  // R(ACK) bit 6 must set to 1
                        goto err;
                    }
                    if(recBuf[0] & 0x08)
                    {
                        if(BITISCLEAR(picc->flags_TCL, CID_PRESENT))
                        {
                            ret = (-SLOTERROR_TCL_BLOCK_INVALID);
							goto err;
                        }
                    }

                    if((picc->PCB ^ recBuf[0]) & 0x01)
                    {
                        // the block number of R-block do not equal the block number of the last I-block
                        timeoutRetry = 0;
                        
                        if(resendCount == 2)
                        {
                            ret = -SLOTERROR_TCL_3RETRANSMIT_FAIL;
							goto err;
                        }
                        picc->PCB &= 0x1F;
                        for(i = 0; i < senLen; i++)
                        {
                            senBuf[i] = tempIBlockBuf[i]; 
                        }
                        
                        //re-transmit last I-block
                        resend = 01;
                        resendCount++;
                    }
                    else
                    {
                        if(BITISSET(picc->flags_TCL, PCD_CHAINING))
                        {
                            // PCD Chaining continue 
                            TOGGLE_BIT(picc->flags_TCL, PCD_BLOCK_NUMBER);
                        }
                        else
                        {
                            ret = (-SLOTERROR_TCL_BLOCK_INVALID);
							goto err;

                        }
                    }
                    break;
                }
                else if((recBuf[0] & 0xC0) == 0x00)
                {
                    //I-Block received
                    if(recBuf[0] & 0x08)
                    {
                        if(BITISCLEAR(picc->flags_TCL, CID_PRESENT))
                        {
                            ret = (-SLOTERROR_TCL_BLOCK_INVALID);
							goto err;
                        }
                    }

                    if((recBuf[0] & 0x02) == 0x00)
                    {
                        ret = (-SLOTERROR_TCL_BLOCK_INVALID);
						goto err;
                    }

                    if(recBuf[0] & 0x04)                     // Not used NAD
                    {
                        ret = (-SLOTERROR_TCL_BLOCK_INVALID);
						goto err;
                    }

                    if(BITISSET(picc->flags_TCL, PCD_CHAINING))
                    {
                        ret = (-SLOTERROR_TCL_BLOCK_INVALID);
						goto err;
                    }
                    else
                    {
                        if((picc->PCB ^ recBuf[0]) & 0x01) //Block number error
                        {
                            ret = (-SLOTERROR_TCL_BLOCK_INVALID);
							goto err;
                        }
                        
                        TOGGLE_BIT(picc->flags_TCL, PCD_BLOCK_NUMBER);
                        
                        if(recBuf[0] & 0x10)
                        {
                            SET_BIT(picc->flags_TCL, PICC_CHAINING);
                        }
                        else
                        {
                            CLEAR_BIT(picc->flags_TCL, PICC_CHAINING);
                        }
                        break;
                    }
                }
                else
                {
                    ret = (-SLOTERROR_TCL_BLOCK_INVALID);
					goto err;
                }
            }
            else if(ret == -ERROR_PROTOCOL)
            {
                if(BITISSET(picc->flags_TCL, WTX_REQ_BEFORE))
                {
                    SET_BIT(picc->flags_TCL, WTX_REQUEST);
                    CLEAR_BIT(picc->flags_TCL, WTX_REQ_BEFORE);
                }
                if(BITISSET(picc->flags_TCL, PICC_CHAINING))
                {
                    if(ackRetry == 2)
                    {
                        ret = -SLOTERROR_T1_3RETRY_FAIL_RESYNCH_FAIL;
                        break;
                    }
                    typeA_sfg_delay(picc->SFGI);
                    ret = typeA_rblock_command(picc, 0xA2,recBuf,recLen);
                    ackRetry++;
                }
                else
                {
                    if(frameRetry == 2)
                    {
                        ret = -SLOTERROR_T1_3RETRY_FAIL_RESYNCH_FAIL;
                        break;
                    }
                    typeA_sfg_delay(picc->SFGI);
                    ret = typeA_rblock_command(picc, 0xB2,recBuf,recLen);
                    frameRetry++;
                }
            }
            else
            {
                CLEAR_BIT(picc->flags_TCL, WTX_REQ_BEFORE);
                ret = -PICC_ERRORCODE_MUTE;
				goto err;
            }
        }
    }while(resend);
err:
//	TRACE_TO("exit %s\n", __func__);    
    return(ret);
}




int typeA_standard_apdu_handler(struct picc_device *picc, u8 *cmdBuf, u32 senLen, u8 *recBuf, u32 *recLen, u8 *level)
{
    int  ret = 0;
    u32 i;
    u32  tempSenLen;
    u32  offset;
    u32  tempRecLen;
    u8  *pSenAddr;
    u8  *pRecAddr;
    u32  ChainLastLen = 0;
    u32  LastCCIDRemainLen = 0x00;



    pSenAddr = cmdBuf;
    pRecAddr = recBuf;
    tempRecLen = 0;
    if(*level == 0x10)
    {
        senLen = 0;
        *recLen = 0;
        for(i = 0; i < ChainLastLen; i++)
        {
            recBuf[i] = tempIBlockBuf[i];
        }
        tempRecLen   = ChainLastLen;
        *recLen      = ChainLastLen;
        ChainLastLen = 0;
    }
    else if(((*level == 0x03) || (*level == 0x02)) && (LastCCIDRemainLen != 0))
    {
        i = senLen;
        while(i--)
        {
            pSenAddr[i + LastCCIDRemainLen] = pSenAddr[i];
        }
        for(i=0; i < LastCCIDRemainLen; i++)
        {
            pSenAddr[i] = tempIBlockBuf[i];
        }
        senLen += LastCCIDRemainLen;
        LastCCIDRemainLen = 0;
    }

    while(senLen)
    {
        CLEAR_BIT(picc->flags_TCL, PICC_CHAINING);
        if((senLen <= picc->FSC) && ((*level == 0x02) || (*level == 0x00)))
        {
            CLEAR_BIT(picc->flags_TCL, PCD_CHAINING);
            picc->PCB = 0x02;            // I-block, no chaining
            if(BITISSET(picc->flags_TCL, PCD_BLOCK_NUMBER))
            {
                picc->PCB |= 0x01;
            }
            if(BITISSET(picc->flags_TCL, CID_PRESENT))
            {
                picc->PCB |= 0x08;
            }
            tempSenLen = senLen;
            ret = typeA_data_send_error_check(picc, pSenAddr, tempSenLen, pRecAddr, &tempRecLen);

            senLen = 0;

            if(!ret)
            {
                if(BITISSET(picc->flags_TCL, CID_PRESENT))
                {
                    offset = 2;
                }
                else
                {
                    offset = 1;
                }
                tempRecLen -= offset;
                for(i = 0; i < tempRecLen; i++)
                {
                    pRecAddr[i] = pRecAddr[i + offset];
                }
                if(tempRecLen == 1)
                {
                    pRecAddr[1] = 0x90;
                    pRecAddr[2] = 0x00;
                    *recLen     = 3;
                }
                else
                {
                    *recLen = tempRecLen;
                }
            }
            else
            {
                CLEAR_BIT(picc->flags_TCL, PICC_CHAINING);
            }
        }
        else
        {
            // PCD chaining
            SET_BIT(picc->flags_TCL, PCD_CHAINING);
            picc->PCB = 0x12;
            if(BITISSET(picc->flags_TCL, PCD_BLOCK_NUMBER))
            {
                picc->PCB |= 0x01;
            }
            if(BITISSET(picc->flags_TCL, CID_PRESENT))
            {
                picc->PCB |= 0x08;
            }
            ret = typeA_data_send_error_check(picc, pSenAddr, picc->FSC, pRecAddr, &tempRecLen);
            CLEAR_BIT(picc->flags_TCL, PCD_CHAINING);
            if(!ret)
            {

                senLen   -= picc->FSC;
                pSenAddr += picc->FSC;
                if(((*level == 0x01) || (*level == 0x03)) && (senLen < picc->FSC))
                {

                    LastCCIDRemainLen = senLen;
                    for(i = 0; i < LastCCIDRemainLen; i++)
                    {
                        tempIBlockBuf[i] = pSenAddr[i];
                    }
                    *recLen = 0;
                    *level  = 0x10;
                    senLen  = 0;
                    CLEAR_BIT(picc->flags_TCL, PICC_CHAINING);
                    return(ret);
                }
            }
            else
            {
                senLen = 0;
                CLEAR_BIT(picc->flags_TCL, PICC_CHAINING);
            }
        }
    }
    // PICC Chaining
    while(BITISSET(picc->flags_TCL, PICC_CHAINING))
    {
        picc->PCB = 0xA2;
        if(BITISSET(picc->flags_TCL, PCD_BLOCK_NUMBER))
        {
            picc->PCB |= 0x01;
        }
        if(BITISSET(picc->flags_TCL, CID_PRESENT))
        {
            picc->PCB |= 0x08;
        }
        pRecAddr += tempRecLen;
        ret = typeA_data_send_error_check(picc, pSenAddr, 0, pRecAddr, &tempRecLen);

        if(!ret)
        {
            if(BITISSET(picc->flags_TCL, CID_PRESENT))
            {
                offset = 2;
            }
            else
            {
                offset = 1;
            }
            tempRecLen -= offset;
            for(i = 0; i < tempRecLen; i++)
            {
                pRecAddr[i] = pRecAddr[i + offset];
            }
            *recLen += tempRecLen;

            if(*recLen > APDURECLENTHREHOLD)
            {
                if(*level == 0x10)                    //Continue Chaining
                {
                    *level = 0x03;
                }
                else
                {
                    *level = 0x01;                    //Chaining Beginning
                }
                ChainLastLen = (u8)(*recLen - APDURECLENTHREHOLD);
                for(i = 0; i < ChainLastLen; i++)
                {
                    tempIBlockBuf[i] = recBuf[i + APDURECLENTHREHOLD];
                }
                *recLen       = APDURECLENTHREHOLD;
                picc->pcd->piccPoll  = FALSE;
                 picc->pcd->poll_interval = 1000;              // 1000ms, start another poll
                return(0);
            }
        }
        else 
        {
            CLEAR_BIT(picc->flags_TCL, PICC_CHAINING);
        }
    }
    if(*level == 0x10)
    {
        *level = 0x02;
    }
    else
    {
        *level = 0x00;
    }
    if(!ret)
    {

         picc->pcd->piccPoll  = FALSE;
         picc->pcd->poll_interval = 1000;    // 1000ms, start another poll
    }
    else
    {
         picc->pcd->piccPoll = TRUE;
        recBuf[0]        = 0x63;
        recBuf[1]        = 0x00;
        *recLen          = 2;
        CLEAR_BIT(picc->flags_TCL, PCD_CHAINING);
        CLEAR_BIT(picc->flags_TCL, PICC_CHAINING);
    }
    
    return(ret);
}


