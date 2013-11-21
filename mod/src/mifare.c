



#include <linux/string.h>


#include "common.h"
#include "picc.h"
#include "mifare.h"
#include "iso14443_typeA.h"
#include "debug.h"


int mifare_select(struct picc_device *picc)
{
    int ret;
    u8 i;
    u8 tempUIDLength;
    u8 tempUID[5];
    u8 level = 0;


//	TRACE_TO("enter %s\n", __func__);   

    ret = typeA_request(picc, PICC_WUPA);
    if(ret == -ERROR_NOTAG)
    {
        ret = typeA_request(picc, PICC_WUPA);
    }
    if(ret != -ERROR_NOTAG)
    {
        tempUIDLength = picc->sn_len;
        i = 0;
        while(tempUIDLength)
        {
            if(tempUIDLength == 4)
            {
                memcpy(tempUID, picc->sn + i, 4);
                tempUIDLength -= 4;
            }
            else
            {
                tempUID[0] = 0x88;
                memcpy(tempUID + 1, picc->sn + i, 3);
                tempUIDLength -= 3;
                i += 3;
            }

            ret = typeA_cascade_select(picc, selectCmd[level], tempUID);
            level++;
        }
    }
    if(!ret)
    {
        picc->authen_need = 0x01;
        picc->states = PICC_SELECTED;
    }

//	TRACE_TO("exit %s, ret = %d\n", __func__, ret);

    return(ret);
}


static int mifare_authen_analyze(struct picc_device *picc, u8 *MFAuthKey)
{
    int ret = 0;
	struct pn512_request	*req = picc->request;

//	TRACE_TO("enter %s\n", __func__);

	req->buf[0] = picc->key_type;
	req->buf[1] = picc->block;
	memcpy(req->buf+2, MFAuthKey, 6);
    if(picc->sn_len == 7)
    	memcpy(req->buf+8, &picc->sn[3], 4);
    else
    	memcpy(req->buf+8, picc->sn, 4);

	req->length = 12;
	req->bit_frame = 0x00;
	req->command = CMD_MFAUTHENT;
	req->direction = TRANSMIT;
	req->time_out = 3000;
	req->timer_start_now = 1;
	picc_wait_for_req(req);

    if(!req->error_code || req->error_code == -ERROR_NOTAG)
    {
        if(pn512_reg_read(Status2Reg) & 0x08)
			ret = 0;
        else
			ret = -PICC_ERRORCODE_MUTE;
    }
    else
        ret = -PICC_ERRORCODE_MUTE;

//	TRACE_TO("exit %s, ret = %d\n", __func__, ret);

	return ret;
}


static int mifare_check_read_write_len(struct picc_device *picc, u8 BlockNum, u8 TempLe)
{
    int ret = 0;
    
    if((picc->SAK & 0xDF) == SAK_MIFARE_4K)
    {
        if(BlockNum >= 0x80)        // sector 32 ~ sector 39, has 16 blocks in each sector
        {
            if((TempLe > 0xF0) || (TempLe & 0x0F))
            {
                ret = 0x01;
            }
        }
        else                        // sector  0 ~ sector 31, has  4 blocks in each sector
        {
            if((TempLe > 0x30) || (TempLe & 0x0F))
            {
                ret = 0x01;
            }
        }
    }
    else if(((picc->SAK & 0xDF) == SAK_MIFARE_1K) || ((picc->SAK & 0xDF) == SAK_MIFARE_MINI))
    {
        if((TempLe > 0x30) || (TempLe & 0x0F))
        {
            ret = 0x01;
        }
    }
    else
    {
        if((TempLe < 4) || (TempLe > 0x10))
        {
            ret = 0x01;
        }
    }

//	TRACE_TO("exit %s, ret = %d\n", __func__, ret);
	
    return(ret);
}


static int mifare_authen(struct picc_device *picc)
{
    int ret = 0;


//	TRACE_TO("enter %s\n", __func__);

    if(((picc->key_valid & 0x01) && (picc->key_No == 0x00))
        || ((picc->key_valid & 0x02) && (picc->key_No == 0x01)))
    {
        ret = mifare_authen_analyze(picc, (u8*)picc->work_key);
        if(!ret)
        {
            picc->authen_need = 0x00;
        }
        else
        {
            picc->authen_need = 0x01;
        }
    }
    else
    {
        ret = 0;
        picc->authen_need = 0x01;
    }

//	TRACE_TO("exit %s, ret = %d\n", __func__, ret);
    
    return(ret);
}


static int mifare_block_check(struct picc_device *picc, u8 srcBlock, u8 desBlock, BOOL multBlockMode)
{
    u8 i;
    u8 j;
    int ret = 0;
    
    if(multBlockMode == TRUE)
    {
        //For safety reason, the Multiple Block Mode is used for accessing Data Blocks only
        if(srcBlock >= 0x80)
        {
            if((srcBlock & 0x0F) == 0x0F)
            {
                ret = 0x01;
            }
        }
        else
        {
            if((srcBlock & 0x03) == 0x03)
            {
                ret = 0x01;
            }
        }
    }
    if(picc->block >= 0x80)         //For Mifare 4K large block
    {
        i=(u8)(srcBlock / 16);        // calculate the source sector number
        j=(u8)(desBlock / 16);        // calculate the target sector number
        if((i != (u8)(picc->block / 16)) || (j != (u8)(picc->block / 16)))
        {
            ret = 1;                    // skip
        }
    }
    else
    {
        i=(u8)(srcBlock / 4);        // calculate the source sector number
        j=(u8)(desBlock / 4);        // calculate the target sector number
        if((i!=(u8)(picc->block / 4)) || (j!=(u8)(picc->block / 4)))
        {
            ret = 1;                   // skip
        }
    }

//	TRACE_TO("exit %s, ret = %d\n", __func__, ret);
		
    return(ret);
}


static int mifare_block_read(struct picc_device *picc, u8 addr, u8 *blockData)
{
	struct pn512_request	*req = picc->request;
	int ret;


//	TRACE_TO("enter %s\n", __func__);

	req->buf[0] = PICC_MF_READ;
	req->buf[1] = addr;

	req->length = 2;
	req->bit_frame = 0x00;
	req->command = CMD_TRANSCEIVE;
	req->direction = TRANSCEIVE;
	req->time_out = 100;
	req->timer_start_auto = 1;
	picc_wait_for_req(req);
	
	memcpy(blockData, req->buf, req->actual);

	if(req->error_code)
        ret = -PICC_ERRORCODE_XFR_PARITY_ERROR;
    else
    {
        if (req->actual != 16) 
            ret = -PICC_ERRORCODE_HW_ERROR;
        else
            ret = 0;
    }

//	TRACE_TO("exit %s, ret = %d\n", __func__, ret);

	return ret;

}


static int mifare_block_write(struct picc_device *picc, u8 opcode, u8 addr, u8 *blockData)
{
    int ret = 0;
    u8 tempBuf[5];
	struct pn512_request	*req = picc->request;


//	TRACE_TO("enter %s\n", __func__); 


	req->buf[0] = opcode;
	req->buf[1] = addr;
	req->length = 2;

    if(opcode == PICC_MF_WRITE_4_BYTES)
    {
    	memcpy(req->buf+2, blockData, 4);
		req->length = 6;
    }
	
	req->bit_frame = 0x00;
	req->command = CMD_TRANSCEIVE;
	req->direction = TRANSCEIVE;
	req->time_out = 600;
	req->timer_start_auto = 1;
	picc_wait_for_req(req);


	memcpy(tempBuf, req->buf, req->actual);
	
	ret = req->error_code;
    if(ret != -ERROR_NOTAG)
    {
        if(req->bit_numbers != 4)
        {
            ret = -PICC_ERRORCODE_MUTE;
        }
        else
        {
            if((tempBuf[0] & 0x0f) == 0x0A)
            {
                ret = 0;
                if(opcode == PICC_MF_WRITE_4_BYTES)
                {
                    goto err;
                }
            }
            else
            {
                ret = -PICC_ERRORCODE_MUTE;
            }
        }
    }
    else
    {
        ret = -PICC_ERRORCODE_MUTE;
    }
	
    if(!ret)
    {
		memcpy(req->buf, blockData, 16);
		req->length = 16;

		req->bit_frame = 0x00;
		req->command = CMD_TRANSCEIVE;
		req->direction = TRANSCEIVE;
		req->time_out = 600;
		
		picc_wait_for_req(req);
		

		memcpy(tempBuf, req->buf, req->actual);
		
		if(req->error_code == -ERROR_NOTAG)
        {   
            // timeout occured
            ret = -PICC_ERRORCODE_MUTE;
        } 
        else 
        {
			if(req->bit_numbers != 4)
            {
               ret = -PICC_ERRORCODE_MUTE;
            }
            else  
            {
                // 4 bit received
                if((tempBuf[0] & 0x0f) == 0x0A)
                {
                    ret = 0;
                }
                else
                {
                    ret = -PICC_ERRORCODE_MUTE;
                }

            }
        } 
    }
	
err:

//	TRACE_TO("exit %s, ret = %d\n", __func__, ret); 

	return(ret);
}


static int mifare_inc_dec(struct picc_device *picc, u8 opcode, u8 addr, u8 *value)
{
    int ret;
    u8 tempBuf[5];
	struct pn512_request	*req = picc->request;


//    TRACE_TO("enter %s\n", __func__); 

	req->buf[0] = opcode;
	req->buf[1] = addr;
	req->length = 2;
	req->bit_frame = 0x00;
	req->command = CMD_TRANSCEIVE;
	req->direction = TRANSCEIVE;
    if(opcode == PICC_MF_TRANSFER)
        req->time_out = 120;
    else
        req->time_out = 15;

	picc_wait_for_req(req);
	
	memcpy(tempBuf, req->buf, req->actual);

	ret = req->error_code;

    if(ret != -ERROR_NOTAG)
    {
		if(req->bit_numbers != 4)
        {
            ret = -PICC_ERRORCODE_MUTE;
        }
        else
        {
            if((tempBuf[0] & 0x0f) == 0x0A)
            {
                ret = 0;
            }
            else
            {
                ret = -PICC_ERRORCODE_MUTE;
            }
        }
    }
    else
    {
        ret = -PICC_ERRORCODE_HW_ERROR;
    }

    if(!ret && opcode != PICC_MF_TRANSFER)
    {
		req->buf[0] = value[3];
		req->buf[1] = value[2];
		req->buf[2] = value[1];
		req->buf[3] = value[0];
		req->length = 4;
		req->bit_frame = 0x00;
		req->command = CMD_TRANSCEIVE;
		req->direction = TRANSCEIVE;
		req->time_out = 15;
		
		picc_wait_for_req(req);
		
		memcpy(tempBuf, req->buf, req->actual);

		if(req->error_code == -ERROR_NOTAG)
        {   
           	ret = 0;
        } 
        else 
        {
            ret = -PICC_ERRORCODE_MUTE;           
        } 
    }
    
//	TRACE_TO("enter %s, ret = %d\n", __func__, ret);     

    return(ret);
}



int mifare_pcsc_command(struct picc_device *picc, u8 *senBuf, u32 senLen, u8 *recBuf, u32 *recLen)
{
    u32 i;
    int ret = 0;
    u8 mifareBlock;
    u8 tempLe;
    BOOL  multBlockMode;
    u8 mifareOpcode;
    u8 *pResAddr;
    

//	TRACE_TO("enter %s, cmd_len=%d\n", __func__, senLen);
    /******* Load Authentication Keys ************/
    // accroding to pcsc part3, Requirements for PC-Connected Interface Devices
    if((senLen == 11) && (senBuf[1] == 0x82) && (senBuf[4] == 0x06))
    {
        if((senBuf[2] == 0x00) && (senBuf[3] <= 0x01))
        {
            for(i = 0; i < 6; i++)
            {
                picc->pcd->mifare_key[senBuf[3]][i] = senBuf[5 + i];
            }

            recBuf[0] = 0x90;
            recBuf[1] = 0x00;
            *recLen   = 0x02;
            ret       = 0;
        }
        // incorrect parameters
        else
        {
            ret = SLOT_ERROR;
        }
    }
    
    /************ Authentication for MIFARE 1K/4K ************/
    // Check Authentication V2.0X
    // FF 86 00 00 05 ADB
    else if((senLen == 10) && (senBuf[1] == 0x86) && (senBuf[2] == 0x00) && (senBuf[3] == 0x00) && (senBuf[4] == 0x05)) 
    {
        if(picc->states != PICC_SELECTED)
        {
            ret = mifare_select(picc);
        }
        if(((senBuf[8] == PICC_MF_KEY_A)||(senBuf[8] == PICC_MF_KEY_B)) && (senBuf[9] <= 0x01))
        {
            picc->block   = senBuf[7];
            picc->key_type = senBuf[8];
            picc->key_No   = senBuf[9];
            ret = mifare_authen_analyze(picc, picc->pcd->mifare_key[picc->key_No]);
            if(!ret)
            {
                if(picc->key_No == 0x00) 
                {
                    picc->key_valid |= 0x01;
                }
                else
                {
                    picc->key_valid |= 0x02;
                }
                for(i = 0; i < 6; i++)
                {
                    picc->work_key[i] = picc->pcd->mifare_key[picc->key_No][i];
                }

                recBuf[0] = 0x90;
                recBuf[1] = 0x00;
                *recLen   = 0x02;
            }
            else
            {
                if(picc->key_No == 0x00)
                {
                    picc->key_valid &= 0xFE;
                }
                else
                {
                    picc->key_valid &= 0xFD;
                }
            }
        }
        else
        {
            ret = SLOT_ERROR;
        }
    }
    // Check Authentication V1.0X
    // FF 88 00 BLOCK_NO KEY_TYPE KEY_NO
    else if((senLen == 0x06) && (senBuf[1] == 0x88) && (senBuf[2] == 0x00))
    {
        if(picc->states != PICC_SELECTED)
        {
            ret = mifare_select(picc);
        }
        if(((senBuf[4] == PICC_MF_KEY_A) || (senBuf[4] == PICC_MF_KEY_B)) && (senBuf[5] <= 0x01))
        {
            picc->block   = senBuf[3];
            picc->key_type = senBuf[4];
            picc->key_No   = senBuf[5];
            ret = mifare_authen_analyze(picc, picc->pcd->mifare_key[picc->key_No]);
            if(!ret)
            {
                if(picc->key_No == 0x00) 
                {
                    picc->key_valid |= 0x01;
                }
                else
                {
                    picc->key_valid |= 0x02;
                }
                for(i = 0; i < 6; i++)
                {
                    picc->work_key[i] = picc->pcd->mifare_key[picc->key_No][i];
                }

                recBuf[0] = 0x90;
                recBuf[1] = 0x00;
                *recLen   = 0x02;
            }
            else
            {
                if(picc->key_No == 0x00)
                {
                    picc->key_valid &= 0xFE;
                }
                else
                {
                    picc->key_valid &= 0xFD;
                }
            }
        }
        else
        {
            ret = SLOT_ERROR;
        }
    }
    // check Binary Read
    // FF B0 00 BLOCK_NO LE
    else if((senLen == 5) && (senBuf[1] == 0xB0) && (senBuf[2] == 0x00))
    {
        ret = mifare_check_read_write_len(picc, senBuf[3], senBuf[4]);
        if(!ret)
        {
            if((picc->authen_need == 0x01) && (picc->SAK != SAK_MIFARE_ULTRALIGHT))
            {
                mifare_authen(picc);
            }
            mifareBlock = senBuf[3];
            tempLe      = senBuf[4];
            pResAddr    = recBuf;
            *recLen     = tempLe;
            if(tempLe > 0x10) 
            {
                multBlockMode = TRUE;
            }
            else 
            {
                multBlockMode = FALSE;
            }
            
            do
            {
                if(picc->SAK != SAK_MIFARE_ULTRALIGHT)
                {
                    ret     = mifare_block_check(picc, mifareBlock, mifareBlock, multBlockMode);
                    tempLe -= 0x10;
                }
                else 
                {
                    ret = 0;
                }
                if(!ret)
                {
                    ret = mifare_block_read(picc, mifareBlock, pResAddr);
                    pResAddr += 0x10;
                }
                if(ret)
                {
                    break;
                }
                mifareBlock++;
                if(picc->SAK == SAK_MIFARE_ULTRALIGHT)
                {
                    break;
                }
            }while(tempLe);
            if(!ret)
            {
                recBuf[(*recLen)++] = 0x90;
                recBuf[(*recLen)++] = 0x00;
            }
//			TRACE_TO("%s: ret=%d\n", __func__, ret);
        }
    }
    // check Binary Write
    // FF D6 00 BLOCK_NO LE
    else if((senBuf[1] == 0xD6) && (senBuf[2] == 0x00))
    {
        ret = mifare_check_read_write_len(picc, senBuf[3], senBuf[4]);
        if(!ret)
        {
            if((picc->authen_need == 0x01) && (picc->SAK != SAK_MIFARE_ULTRALIGHT))
            {
                mifare_authen(picc);
            }
            mifareBlock = senBuf[3];
            tempLe      = senBuf[4];
            pResAddr    = senBuf+5;
            *recLen     = tempLe;
            if(tempLe > 0x10) 
            {
                multBlockMode = TRUE;
            }
            else 
            {
                multBlockMode = FALSE;
            }
            
            do
            {
                if(picc->SAK != SAK_MIFARE_ULTRALIGHT)
                {
                    ret     = mifare_block_check(picc, mifareBlock, mifareBlock, multBlockMode);
                    tempLe -= 0x10;
                    mifareOpcode = PICC_MF_WRITE_16_BYTES;
                }
                else 
                {
                    ret = 0;
                    tempLe -= 0x04;
                    mifareOpcode = PICC_MF_WRITE_4_BYTES;
                }
                if(!ret)
                {
                    ret = mifare_block_write(picc, mifareOpcode, mifareBlock, pResAddr);
                    if(mifareOpcode == PICC_MF_WRITE_16_BYTES)
                    {
                        pResAddr += 0x10;
                    }
                    else
                    {
                        pResAddr += 0x04;
                    }
                }
                if(ret)
                {
                    break;
                }
                mifareBlock++;
            }while(tempLe);
            
            if(!ret)
            {
                recBuf[0] = 0x90;
                recBuf[1] = 0x00;
                *recLen   = 0x02;
            }
            if(picc->SAK != SAK_MIFARE_ULTRALIGHT)
            {
                if((mifareBlock >= 0x80) && ((mifareBlock % 16) == 0x00))
                {
                    picc->pcd->piccPoll = TRUE;
                }
                else if((mifareBlock % 4) == 0x00)
                {
                    picc->pcd->piccPoll = TRUE;
                }
            }
        }
    }
    
    // check Value Block Read
    // FF B1 00 BLOCK_NO 04
    else if((senLen == 5) && (senBuf[1] == 0xB1) && (senBuf[2] == 0x00) && (senBuf[4] == 0x04))
    {
        if((picc->authen_need == 0x01) && (picc->SAK != SAK_MIFARE_ULTRALIGHT))
        {
            mifare_authen(picc);
        }
        if(picc->SAK != SAK_MIFARE_ULTRALIGHT)
        {
            ret = mifare_block_check(picc, senBuf[3], senBuf[3], FALSE);
        }
        else 
        {
            ret = SLOT_ERROR;
        }
        if( !ret)
        {
            ret = mifare_block_read(picc, senBuf[3], recBuf);
        }
        if(!ret)
        {
            // check the Value Block Format
            for(i = 0; i < 4; i++)
            {
                if((recBuf[i] != recBuf[8 + i])||((recBuf[i] ^ recBuf[4 + i]) != 0xff))
                {
                    ret = 1;
                }
            }

            if(!ret)
            {
                for(i = 0; i < 4; i++) 
                {
                    recBuf[i] = recBuf[11 - i];
                }

                recBuf[4] = 0x90;
                recBuf[5] = 0x00;
                *recLen = 6;
            }
        }
    }
    
    // check Value Block Operation
    // FF D7 00 BLOCK_NO 05 VB_OP VB_VALUE
    else if((senLen == 10) && (senBuf[1] == 0xD7) && (senBuf[2] == 0x00) && (senBuf[4] == 0x05) && (senBuf[5] < 0x03))
    {
        if((picc->authen_need == 0x01) && (picc->SAK != SAK_MIFARE_ULTRALIGHT))
        {
            mifare_authen(picc);
        }
        mifareBlock = senBuf[3];
        // step 1. Store Value Operation
        // STORE Value Operation
        if(senBuf[5] == 0x00)
        {
            mifareOpcode = PICC_MF_WRITE_16_BYTES;
            senBuf[13] = senBuf[9];
            senBuf[14] = senBuf[8];
            senBuf[15] = senBuf[7];
            senBuf[16] = senBuf[6];

            senBuf[5] = senBuf[13];
            senBuf[6] = senBuf[14];
            senBuf[7] = senBuf[15];
            senBuf[8] = senBuf[16];

            senBuf[9]  = ~senBuf[5];
            senBuf[10] = ~senBuf[6];
            senBuf[11] = ~senBuf[7];
            senBuf[12] = ~senBuf[8];

            senBuf[17] = mifareBlock;
            senBuf[18] = ~mifareBlock;
            senBuf[19] = mifareBlock;
            senBuf[20] = ~mifareBlock;

        }
        // Increment Operation
        else if(senBuf[5]==0x01)
        {
            mifareOpcode = PICC_MF_INCREMENT;
        }
        // Decrement Operation
        else
        {
            mifareOpcode = PICC_MF_DECREMENT;

        }
        if(picc->SAK != SAK_MIFARE_ULTRALIGHT)
        {
            ret = mifare_block_check(picc, mifareBlock, mifareBlock, FALSE);
        }
        else 
        {
            ret = SLOT_ERROR;
        }
        if(!ret)
        {
            if(mifareOpcode == PICC_MF_WRITE_16_BYTES)
            {
                ret = mifare_block_write(picc, mifareOpcode, mifareBlock, senBuf+5);
            }
            else
            {
                ret = mifare_inc_dec(picc, mifareOpcode, mifareBlock, senBuf+6);
            }
        }
        if(!ret)
        {
            // Step 2. Transfer Operation
            if((mifareOpcode == PICC_MF_INCREMENT) || (mifareOpcode == PICC_MF_DECREMENT))
            {
                ret = mifare_inc_dec(picc, PICC_MF_TRANSFER, mifareBlock, senBuf+6);
            }
            if(!ret)
            {
                recBuf[0] = 0x90;
                recBuf[1] = 0x00;
                *recLen = 2;
            }
        }
    }
    // check Restore Value Block Operation
    // FF D7 00 srcBlock 02 03 desBlock
    else if((senLen == 7) && (senBuf[1] == 0xD7) && (senBuf[2] == 0x00) 
            && (senBuf[4] == 0x02) && (senBuf[5] == 0x03))

    {
        if((picc->authen_need == 0x01) && (picc->SAK != SAK_MIFARE_ULTRALIGHT))
        {
            mifare_authen(picc);
        }
        mifareBlock = senBuf[6];
        // step 1. Restore Value Operation
        // Restore Value Operation

        if(picc->SAK != SAK_MIFARE_ULTRALIGHT)
        {
            ret = mifare_block_check(picc, senBuf[3], senBuf[6], FALSE);
        }
        else 
        {
            ret = SLOT_ERROR;
        }
		
        if( !ret)	
        {
            ret = mifare_inc_dec(picc, PICC_MF_RESTORE, senBuf[3], senBuf+6);
        }
		
        if(!ret)
        {
            // Step 2. Transfer Operation
            ret = mifare_inc_dec(picc, PICC_MF_TRANSFER, mifareBlock, senBuf+6);
            if(!ret)
            {
                recBuf[0] = 0x90;
                recBuf[1] = 0x00;
                *recLen = 2;
            }
        }
    }
    else
    {
        ret = -PICC_ERRORCODE_CMD_ABORTED;
    }

    if(!ret)
    {
        picc->pcd->poll_interval = 1000;                 // 1000ms, start another poll
        picc->pcd->piccPoll = FALSE;
    }
    else if(ret == -PICC_ERRORCODE_CMD_ABORTED)
    {
        recBuf[0] = 0x6A;
        recBuf[1] = 0x81;
        *recLen = 0x02;
        ret = 0;
        picc->pcd->piccPoll = TRUE;
    }
    else
    {
        recBuf[0] = 0x63;
        recBuf[1] = 0x00;
        *recLen = 0x02;
        ret = 0;	
        picc->pcd->piccPoll = TRUE;
    }

//	TRACE_TO("exit %s\n", __func__);

    return(ret);
}

void mifare_type_coding(struct picc_device *picc)
{
	picc->type = PICC_MIFARE;
	switch(picc->SAK)
	{
		case 0x00:
			picc->name = "mifare ultralight (C) CL2";
			break;

		case 0x09:
			picc->name = "mifare mini(0.3k)";
			break;

		case 0x08:
			picc->name = "mifare classic 1K";
			break;

		case 0x18:
			picc->name = "mifare classic 4K";
			break;

		case 0x20:
			picc->name = "mifare desfire";
			break;

		default:
			picc->name = "unkonw tag";
			break;
	}
}


