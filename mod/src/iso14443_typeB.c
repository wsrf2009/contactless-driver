






#include <linux/string.h>


#include "common.h"
#include "picc.h"
#include "iso14443_typeB.h"
#include "iso14444.h"
#include "pcd_config.h"
#include "delay.h"
#include "debug.h"



int typeB_request(struct picc_device *picc, u8 reqCmd, u8 N)
{
    int ret = 0;
 	struct pn512_request	*req = picc->request;


//	TRACE_TO("enter %s\n", __func__);	

    pn512_reg_set(TxModeReg, TxCRCEn);            //TXCRC enable
    pn512_reg_set(RxModeReg, RxCRCEn);            //RXCRC enable, RxMulitple
    pn512_reg_clear(Status2Reg, MFCrypto1On);     // Disable crypto 1 unit
    
	req->buf[0] = 0x05;
	req->buf[1] = 0x00;
	req->buf[2] = reqCmd | (N & 0x07);
	req->length = 3;
	req->bit_frame = 0x00;
	req->command = CMD_TRANSCEIVE;
	req->direction = TRANSCEIVE;
	req->time_out = 50;
	
	picc_wait_for_req(req);


    ret = req->error_code;
    if(!ret) 
    {
		if((req->actual == 12) || (req->actual == 13))
        {
			memcpy(&picc->ATQB, req->buf, req->actual);

			picc->ATQB_len = req->actual;
            if(picc->ATQB[0] != 0x50)
            {
                ret = -ERROR_WRONGPARAM;
            }
            else
            {
                picc->states = PICC_READY;

            }
        }
        else
        {
            ret = -ERROR_BYTECOUNT;
        }
    } 

//	TRACE_TO("exit %s\n", __func__);
	
    return(ret);
}  

static int typeB_slot_marker(struct picc_device *picc, u8 N)
{
    int ret = 0;
 	struct pn512_request	*req = picc->request;

	
//	TRACE_TO("enter %s\n", __func__);
	
    pn512_reg_set(TxModeReg, TxCRCEn);    //TXCRC enable
    pn512_reg_set(RxModeReg, RxCRCEn);    //RXCRC enable
    
    if(N == 0 || N > 15) 
    {
        ret = -ERROR_WRONGPARAM;
    }
    else 
    {

		req->buf[0] = (N << 4) | 0x05;
		req->length = 1;
		req->bit_frame = 0x00;
		req->command = CMD_TRANSCEIVE;
		req->direction = TRANSCEIVE;
		req->time_out = 10;
		
		picc_wait_for_req(req);
		
		ret = req->error_code;
        if(!ret) 
        {
            if((req->actual == 12) || (req->actual == 13))
            {
				memcpy(&picc->ATQB, req->buf, req->actual);
				picc->ATQB_len = req->actual;
                if(picc->ATQB[0] != 0x50)
                {
                    ret = -ERROR_WRONGPARAM;
                }
                else
                {
                    picc->states = PICC_READY;

					INFO_TO("pupi ==> 0x%02X 0x%02X 0x%02X 0x%02X\n", 
							picc->ATQB[1], picc->ATQB[2], picc->ATQB[3], picc->ATQB[4]);
                }
            }
            else
            {
                ret = -ERROR_BYTECOUNT;
            }
        } 
    }

//	TRACE_TO("exit %s\n", __func__);  
    return(ret);
}  

static void typeB_ATQB_analysis(struct picc_device *picc, u8 *ATQB)
{

    CLEAR_BIT(picc->flags_TCL, PCD_BLOCK_NUMBER);
    
    memcpy(picc->sn, ATQB+1, 4);            // copy PUPI
    picc->sn_len = 4;
    
    picc->speed = ATQB[9];
    
    picc->FSCI = ATQB[10] >> 4;
    if(picc->FSCI > 8)
    {
        picc->FSCI = 8;
    }
    
    if(ATQB[11] & 0x01)
    {
        // CID supported by the PICC
        SET_BIT(picc->flags_TCL, CID_PRESENT);
        picc->CID = get_cid(picc->sn);
    }
    else
    {
        CLEAR_BIT(picc->flags_TCL, CID_PRESENT);
        picc->CID = 0;
    }
    
    picc->FWI = ATQB[11] >> 4;
    if(picc->FWI > 14)
    {
        picc->FWI = 4;          // compliant to ISO14443-3:2011
    }

    if(ATQB[10] & 0x01)
    {
        picc->support_part4 = 0x01;        // PICC compliant with ISO/IEC 14443-4
    }
    else
    {
        picc->support_part4 = 0x00;        // PICC not compliant with ISO/IEC 14443-4
    }

    // b3:b2 in Protocol_Type defines the minimum of TR2. compliant to ISO14443-4: 2011
    if((ATQB[10] & 0x06) == 0x00)
    {
        picc->SFGI = 0;    // 10etu + 32 / fs
    }
    else if((ATQB[10] & 0x06) == 0x02)
    {
        picc->SFGI = 1;    // 10etu + 128 / fs
    }
    else if((ATQB[10] & 0x06) == 0x04)
    {
        picc->SFGI = 1;   // 10etu + 256 / fs
    }
    else
    {
        picc->SFGI = 2;    // 10etu + 512 / fs    
    }

    if(picc->ATQB_len == 13)
    {
        // extended ATQB supported by PICC, it defines a specific guard time replacing TR2
        // SGT is needed by the PICC before it is ready to receive the next frame after it has sent the Answer to ATTRIB command
        picc->SFGI = ATQB[12] >> 4;
        if(picc->SFGI > 14)
        {
            picc->SFGI = 0;
        }
    }
}

static int typeB_attrib(struct picc_device *picc)
{
    int ret = 0;
	struct pn512_request	*req = picc->request;

    
//	TRACE_TO("enter %s\n", __func__);
	
    pn512_reg_set(TxModeReg, TxCRCEn);            //TXCRC enable
    pn512_reg_set(RxModeReg, RxCRCEn);            //RXCRC enable, RxMulitple
    
	req->buf[0] = 0x1D;
	memcpy(req->buf+1, picc->sn, 4);
	memcpy(req->buf+5, picc->attrib_param, 4);
	req->length = 9;
	req->bit_frame = 0x00;
	req->command = CMD_TRANSCEIVE;
	req->direction = TRANSCEIVE;
	req->time_out = 0;
	
	SET_BIT(picc->flags_TCL, TYPEB_ATTRIB);
	
	typeA_set_timeout(picc, picc->FWI);
	picc_wait_for_req(req);


    CLEAR_BIT(picc->flags_TCL, TYPEB_ATTRIB);
	
	ret = req->error_code;

	memcpy(picc->attrib_response, req->buf, req->actual);

    if((picc->attrib_response[0] & 0x0F) != picc->CID)
    {
        ret = -ERROR_CID;
    }

    if((ret == -ERROR_CRC) && (req->actual == 1)) 
    {
        // CRC Error
        ret = -ERROR_WRONGPARAM;
    }
	
//	TRACE_TO("exit %s\n", __func__); 
	
    return(ret);
}


static int typeB_standard_attrib(struct picc_device *picc)
{
    int ret = 0;
    u8 speedParam;
    

    typeB_ATQB_analysis(picc, picc->ATQB);
    
    speedParam = typeA_speed_check(picc);
    
    picc->attrib_param[0] = 0x00;                                // param 1:  TR0 = 64 /fs, TR1 = 80 / fs, SOF required, EOF required
    picc->attrib_param[1] = (speedParam << 4) | picc->pcd->FSDI;    // param 2: 
    picc->attrib_param[2] = picc->support_part4;                         // param 3:
    picc->attrib_param[3] = picc->pcd->CID & 0x0f;                  // param 4:

    ret = typeB_attrib(picc);
    if(!ret)
    {
        picc->states = PICC_ACTIVATED;
        typeA_high_speed_config(picc, speedParam, TYPEB_106TX);
    }
	
    return(ret);
} 


int typeB_halt(struct picc_device *picc)
{
    int ret = 0;
	struct pn512_request	*req = picc->request;


//    TRACE_TO("enter %s\n", __func__);

    if (picc->states != PICC_POWEROFF)
    {

        //-----------Initialization---------
        pn512_reg_set(TxModeReg, TxCRCEn);        // TXCRC enable 
        pn512_reg_set(RxModeReg, RxCRCEn);        // RXCRC enable

		req->buf[0] = 0x50;
		memcpy(&req->buf[1], picc->sn, 4);
		req->length = 5;
		req->bit_frame = 0x00;
		req->command = CMD_TRANSCEIVE;
		req->direction = TRANSCEIVE;
		req->time_out = 0;
		
		SET_BIT(picc->flags_TCL, TYPEB_ATTRIB);
		
		typeA_set_timeout(picc, picc->FWI);
		picc_wait_for_req(req);
		ret = req->error_code;
		if (req->actual != 1 || req->buf[0] != 0)
        {
            ret = -ERROR_NOTAG;
        }
        else
        {
            picc->states= PICC_IDLE;
        }
    }

//	TRACE_TO("exit %s\n", __func__);
	
    return(ret);
} 




void typeB_polling_tags(struct picc_device *picc)
{
    int ret;
    u8 i;


//	TRACE_TO("enter %s\n", __func__);   

    pcd_config_iso14443_card(CONFIGTYPEB, TYPEB_106TX);
    pcd_config_iso14443_card(CONFIGNOTHING, TYPEB_106RX);
    
    ret = typeB_request(picc, PICC_WUPB, N_1_SLOT);    // ReqB with 1 slot
    if (ret == -ERROR_NOTAG)
    {
        // if no card detected, quit the loop
        Delay1us(100);
        ret = typeB_request(picc, PICC_WUPB, N_1_SLOT);     // ReqB with 1 slot
        if (ret == -ERROR_NOTAG)
        {
            picc->type = PICC_ABSENT;
			picc->name = "none";
            return;
        }
    }

    if (ret)
    {
        Delay1us(200);
        if((ret = typeB_request(picc, PICC_WUPB, N_4_SLOT))) // ReqB with 4 slot
        {
            for (i = 1; i < 4; i++)
            {
                if (!(ret = typeB_slot_marker(picc, i)))
                {
                    break;
                }
            }
        }
    }
    if (!ret) 
    {
        Delay1us(600);
        ret = typeB_standard_attrib(picc);
        if(!ret)
        {
            picc->type = PICC_TYPEB_TCL;
			picc->name = "standard typeB";
            picc->FSC = FSCConvertTbl[picc->FSCI] - 3;    // FSC excluding EDC and PCB
            if(BITISSET(picc->flags_TCL, CID_PRESENT))
            {
                picc->FSC--;        // FSC excluding CID
            }
        }
        else
        {
            if(typeA_deselect_request(picc))
            {
                typeB_halt(picc);
            }
            picc->type = PICC_ABSENT;
			picc->name = "none";
        }
    }
    else
    {
        picc->type = PICC_ABSENT;
		picc->name = "none";
    }

//	TRACE_TO("exit %s\n", __func__);
}


