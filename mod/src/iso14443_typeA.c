

#include <linux/string.h>


#include "common.h"
#include "picc.h"
#include "iso14443_typeA.h"
#include "iso14444.h"
#include "pcd_config.h"
#include "delay.h"
#include "debug.h"
#include "mifare.h"



const u8 selectCmd[3] = {PICC_SELVL1, PICC_SELVL2, PICC_SELVL3};




void typeA_halt(struct picc_device *picc)
{
	struct pn512_request	*req = picc->request;
	

//	TRACE_TO("enter %s\n", __func__);
	
    if (picc->states != PICC_POWEROFF)
    {
        pn512_reg_set(TxModeReg, TxCRCEn);    // TXCRC enable
        pn512_reg_set(RxModeReg, RxCRCEn);    // RXCRC enable

		req->buf[0] = PICC_HALT;
		req->buf[1] = 0x00;
		req->length = 2;
		req->bit_frame = 0;
		req->command = CMD_TRANSCEIVE;
		req->direction = TRANSCEIVE;
		req->time_out = 10;

		picc_wait_for_req(req);

        picc->states = PICC_IDLE;
    }

//	TRACE_TO("enter %s\n", __func__);
	
}


static int typeA_cascade_anticollision (struct picc_device *picc,
											u8 selCode, u8 *uid)       
{
    int ret = 0;
    u8 nbytes;
    u8 nbits;
    u8 i;
    u8 byteOffset;
    u8 uidCRC;
    u8 bcnt = 0;  
	struct pn512_request	*req = picc->request;


//	TRACE_TO("enter %s\n", __func__);    

    pn512_reg_clear(TxModeReg, TxCRCEn);            // Disable TxCRC
    pn512_reg_clear(RxModeReg, RxCRCEn);            // Disable RxCRC
    pn512_reg_write(CollReg, 0x00);                        // ValuesAfterColl = 0
    
    while(!ret)
    {
        nbits = bcnt & 0x07;                         // remaining number of bits
        if(nbits) 
        {
            nbytes = (bcnt / 8) + 1;   
        } 
        else 
        {
            nbytes = bcnt / 8;
        }

		req->buf[0] = selCode;
		req->buf[1] = 0x20 + ((bcnt/8) << 4) + nbits;
		for(i = 0; i < nbytes; i++) 
			req->buf[2+i] = uid[i];						   // UID: 0~40 data bits

		req->length = nbytes+2;
		req->bit_frame = (nbits << 4) | nbits;
		req->command = CMD_TRANSCEIVE;
		req->direction = TRANSCEIVE;
		req->time_out = 500;

		picc_wait_for_req(req);
		
		ret = req->error_code;

		if(!ret || ret == -ERROR_COLL)        // no other occurred
        {   
            // Response   Processing   
            bcnt += req->bit_numbers - nbits;
            
            // no. of bits received
            if(bcnt > 40) 
            {
                ret = -ERROR_BITCOUNT;
            } 
            else 
            {
                byteOffset = 0;
                if(nbits != 0)            // last byte was not complete
                {        
					uid[nbytes-1] = req->buf[0];
                    byteOffset = 1;
                }
                for(i = 0; i < (4 - nbytes); i++) 
                {
					uid[nbytes + i] = req->buf[i+byteOffset];
                }
  
                if(!ret)       // no error and no collision
                { 
                    // bcc check
                    uidCRC = uid[0] ^ uid[1] ^ uid[2] ^ uid[3];
					if(uidCRC != req->buf[req->actual-1])
                    {
                        ret = -ERROR_SERNR;
                    }
                    break;
                } 
                else      // collision occurred
                {                  
                    ret = 0;
                }
            }
        }
    }
    if (ret) 
    {                                                  
        memcpy(uid, "\x00\x00\x00\x00", 4);
    }
    
    pn512_reg_write(BitFramingReg, 0x00);        // TxLastBits/RxAlign 0
    pn512_reg_write(CollReg, 0x80);              // ValuesAfterColl = 1

//	TRACE_TO("exit %s\n", __func__);
	
    return(ret);
}


/*****************************************************************/
//       Type A Select
/*****************************************************************/
int typeA_cascade_select(struct picc_device *picc, u8 selCode, u8 *uid)
{
    int ret = 0;
    u8 i;
    u8 j;
	struct pn512_request	*req = picc->request;


//	TRACE_TO("enter %s\n", __func__);
	
    pn512_reg_set(TxModeReg, TxCRCEn);     // TXCRC enable
    pn512_reg_set(RxModeReg, RxCRCEn);     // RXCRC enable

	req->buf[0] = selCode;
	req->buf[1] = 0x70;
	for(i = 0, j = 0; i < 4; i++)
	{
		req->buf[2+i] = uid[i]; 					   // UID: 0~40 data bits
		j ^= uid[i];
	}
	req->buf[6] = j;
	req->length = 7;
	req->bit_frame = 0x00;
	req->command = CMD_TRANSCEIVE;
	req->direction = TRANSCEIVE;
	req->time_out = 500;
	
	picc_wait_for_req(req);

	ret = req->error_code;
	
    picc->SAK = 0;
	if(!ret)         // No timeout occured
    {   
        // i= no. of bits received
        if(req->bit_numbers != 8)             // last byte is not complete
        {  
            ret = -ERROR_BITCOUNT;
        }
        else 
        {
			picc->SAK = req->buf[0];

			INFO_TO("sak: %02X\n", picc->SAK);
        }
    }

//	TRACE_TO("exit %s\n", __func__);

    return(ret);
}


int typeA_request(struct picc_device *picc, u8 reqCmd)
{
    int ret = 0;
    unsigned int i;
	struct pn512_request	*req = picc->request;


//	TRACE_TO("enter %s[%02X]\n", __func__, reqCmd);  

    pn512_reg_clear(ModeReg, DetectSync);             // disable DetectSync if activated before 
    pn512_reg_clear(TxModeReg, TxCRCEn);              // Disable TxCRC
    pn512_reg_clear(RxModeReg, RxCRCEn);              // Disable RxCRC
    pn512_reg_clear(Status2Reg, MFCrypto1On);         // Disable crypto 1 unit

	req->buf[0] = reqCmd;
	req->length = 1;
	req->bit_frame = 0x07;
	req->command = CMD_TRANSCEIVE;
	req->direction = TRANSCEIVE;
	req->time_out = 500;
//	TRACE_TO("enter completely?\n");
	picc_wait_for_req(req);

//	TRACE_TO("completely?\n");
	
	ret = req->error_code;

	if(ret) 
    {       
        // error occur
        picc->ATQA[0] = 0x00;
        picc->ATQA[1] = 0x00;
    }
    else 
    {
		i = req->bit_numbers;

        // i= no. of bits received
        if(req->bit_numbers != 16) 
        {
            ret = -ERROR_BITCOUNT;
            picc->ATQA[0] = 0x00;
            picc->ATQA[1] = 0x00;

        } 
        else 
        {
			picc->ATQA[0] = req->buf[0];
			picc->ATQA[1] = req->buf[1];
            ret = 0;
            picc->states = PICC_READY;

//			INFO_TO("ATQA: %02X %02X\n", picc->ATQA[0], picc->ATQA[1]);
        }
    }

    
    pn512_reg_write(BitFramingReg, 0x00);            // Reset TxLastBits to 0

//	TRACE_TO("exit %s\n", __func__); 
	
    return(ret); 
}




int typeA_select(struct picc_device *picc)
{
    u8 level=0;
    u8 cardUID[5];
	int ret;


//	TRACE_TO("enter %s\n", __func__);

    // reset speed settings to 106Kbps
    pcd_config_iso14443_card(CONFIGTYPEA, TYPEA_106TX);
    pcd_config_iso14443_card(CONFIGNOTHING, TYPEA_106RX);

    if(typeA_request(picc, PICC_WUPA) == -ERROR_NOTAG)
    {
        Delay1us(300);
        if(typeA_request(picc, PICC_WUPA) == -ERROR_NOTAG)
        {
            ret = -ERROR_NOTAG;
			goto done;
        }
    }

    do
    {
        Delay1us(100);
        if(typeA_cascade_anticollision(picc, selectCmd[level], cardUID)) 
        {
            typeA_halt(picc);
            break;
        }
        Delay1us(100);
        if(typeA_cascade_select(picc, selectCmd[level], cardUID)) 
        {
            break;
        }

        if(level == 0) 
        {
            // Cascade Level 1
            if(cardUID[0] == 0x88)    // uid0 = 0x88, CT present and next cascade level will be implement
            {
                memcpy(picc->sn, cardUID+1, 3);
                picc->sn_len = 3;
            } 
            else 
            {
                memcpy(picc->sn, cardUID, 4);
                picc->sn_len = 4;
                level |= 0x80;            // quit the  loop
            }
        } 
        else if(level == 1) 
        {
            // Cascade Level 2
            if(cardUID[0] == 0x88)    // uid3 = 0x88, CT present and next cascade level will be implement
            {
                memcpy(picc->sn + 3, cardUID+1,3);
                picc->sn_len = 6;
            } 
            else 
            {
                memcpy(picc->sn + 3, cardUID, 4);
                picc->sn_len = 7;
                level |= 0x80;            // quit the loop
            }
        } 
        else 
        {
            // Cascade Level 3
            memcpy(picc->sn+6, cardUID, 4);
            picc->sn_len = 10;
            level |= 0x80;               // quit the  loop
        }
        
        level++;                         // next level code 
    } while(level < 0x80); 
    
    if(level & 0x80)
    {
		INFO_TO("UID:");
        for(level = 0; level < picc->sn_len; level++)
        {
            INFO_TO(" %02X", picc->sn[level]);
        }
        INFO_TO("\n");
        ret = 0;
    }
    else
    {
        ret = -ERROR_NOTAG;
    }


done:

//	TRACE_TO("exit %s\n", __func__);

	return ret;
}



void typeA_polling_tags(struct picc_device *picc)
{
    int ret;
	u8	sak;


//	TRACE_TO("enter %s\n", __func__);

    // check for any card in the field
//    pn512_reg_read(REG_MODE);
    ret = typeA_select(picc);

    if(!ret)
    {
    	sak = picc->SAK & 0x24;
        // Check the SAK
        if(sak == 0x20)
        {
            // picc compliant with ISO/IEC 14443-4
            picc->CID = get_cid(picc->sn);
            if((BITISSET(picc->pcd->flags_polling, AUTO_RATS)))
            {
                // auto ATS
                Delay1us(300);
                if(!typeA_request_ats(picc))
                {
                    picc->type = PICC_TYPEA_TCL;        // typeaA PICC which compliant to ISO/IEC 14443-4 
                    picc->name = "standard typeA";
                    typeA_pps_check_and_send(picc);
                    picc->FSC = FSCConvertTbl[picc->FSCI] - 3;      // FSC excluding EDC and PCB, refer to Figure14 --- Block format
                    if(BITISSET(picc->flags_TCL, CID_PRESENT))
                    {
                        picc->FSC--;                              // FSC excluding CID, refer to Figure14 --- Block format
                    }
                }
                else
                {
                    if(typeA_deselect_request(picc))
                    {
                        typeA_halt(picc);
                    }
                
                    picc->type = PICC_ABSENT;
					picc->name = "none";
                }
            }
        }
        else
        {
//            picc->type = PICC_MIFARE;
			mifare_type_coding(picc);
        }

    }
    else
    {
        picc->type = PICC_ABSENT;
		picc->name = "none";
    }

//	TRACE_TO("exit %s\n", __func__);
}





