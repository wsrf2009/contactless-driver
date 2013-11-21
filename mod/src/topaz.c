


#include <linux/string.h>


#include "common.h"
#include "picc.h"
#include "topaz.h"
#include "pcd_config.h"
#include "iso14443_typeA.h"
#include "pn512.h"
#include "delay.h"

#include "debug.h"


#include "crc.c"



#define TOPAZ_RID				0x78
#define TOPAZ_RALL				0x00
#define TOPAZ_READ				0x01
#define TOPAZ_WRITE_E			0x53
#define TOPAZ_WRITE_NE			0x1A



static int topaz_parity_check(u8 *in_buf, u32 in_len, 
								u8 *out_buf, u32 *out_len)
{
	u8 j;
	u8 parity;
	u8 n;
	u8 rol;
	unsigned long long temp_data;
	u32 m = 0;
	u8 remain_len;
	unsigned long long temp;
	u32 i;
	int ret;


	for(i=0; i<in_len; i=i+9)
	{
		remain_len = in_len-i;
		
		out_buf[m] = in_buf[i];
		remain_len--;
		
		temp_data = 0;
		if(remain_len >= 8)
		{
			for(j=0; j<8; j++)
			{
				temp = in_buf[i+1+j];
				temp_data |= (temp << (j*8));
			}

			n = 8;
			
		}
		else
		{
			for(j=0; j<remain_len; j++)
			{
				temp = in_buf[i+1+j];
				temp_data |= (temp << (j*8));
			}

			n = remain_len;
			
		}

		while(--n)
		{
			parity = 0;
			rol = 0;
			do{
				if(out_buf[m] & (1 << rol))
					parity++;
			
				rol++;
				
			}while(rol < 8);

			if(temp_data & 0x01)
				parity++;

			if(!(parity & 0x01))
			{
				ret = -ERROR_PARITY;
				goto err;
			}

			temp_data >>= 1;
			out_buf[++m] = (u8)temp_data & 0xFF;
			temp_data >>= 8;
			
		};
		
	}

	ret = 0;

err:

	*out_len = m+1;
	return ret;

}

static int topaz_transceive_handler(struct picc_device *picc, u8 *cmdBuf, u32 cmdLen, u8 *resBuf, u32 *resLen, u16 timeout)
{
    u32 i;
    int ret = 0;
    u8 crcL;
    u8 crcH;
	struct pn512_request	*req = picc->request;



//	TRACE_TO("enter %s\n", __func__); 

    pn512_reg_set(ManualRCVReg, ParityDisable);        //Parity disable

	ComputeCrc(CRC_B, cmdBuf, cmdLen, cmdBuf + cmdLen, cmdBuf + cmdLen + 1);
	cmdLen += 2; 

	req->buf[0] = cmdBuf[0];
	req->bit_frame = 0x07;
	req->length = 1;
	req->command = CMD_TRANSMIT;
	req->direction = TRANSMIT;
	req->time_out = 0;
	req->tx_done = 0;
	picc_wait_for_req(req);

	for(i = 1; i < cmdLen-1; i++)
    {

		req->buf[0] = cmdBuf[i];
		req->bit_frame = 0x00;
		req->length = 1;
		req->command = CMD_TRANSMIT;
		req->direction = TRANSMIT;
		req->time_out = 0;
		req->tx_done = 0;
		picc_wait_for_req(req);
    }
	
	req->buf[0] = cmdBuf[cmdLen-1];
	req->bit_frame = 0x00;
	req->length = 1;
	req->command = CMD_TRANSCEIVE;
	req->direction = TRANSCEIVE;
	req->time_out = timeout;
	req->tx_done = 0;
	picc_wait_for_req(req);


	ret = topaz_parity_check(req->buf, req->actual, resBuf, &i);
	if(ret < 0)
		goto err;
	
     // Response   Processing   
	if(i >= 4)
    {
        // Read the data from card to buffer       
        ComputeCrc(CRC_B, resBuf, (i - 2), &crcL, &crcH);
        if((crcL == resBuf[i-2]) || (crcH == resBuf[i-1]))
        {
            *resLen = i-2;
            ret = 0;
        }
        else
        {
            *resLen  = 0;
            ret = -ERROR_CRC;
        }
    }
    else
    {
        ret = -ERROR_BYTECOUNT;
    } 

err:
	
//	TRACE_TO("exit %s ret = %d\n", __func__, ret); 
    
    return(ret);
}


/*****************************************************************/
//      Topaz RID Command
/*****************************************************************/
static int topaz_get_rid(struct picc_device *picc)       
{
    int ret = 0;
    u32 nbytes;
    u8 tempBuf[9];


//    TRACE_TO("enter %s\n", __func__);    
    
    pn512_reg_clear(TxModeReg, TxCRCEn);         // Disable TxCRC
    pn512_reg_clear(RxModeReg, RxCRCEn);         // Disable RxCRC
    pn512_reg_clear(Status2Reg, MFCrypto1On);    // Disable crypto 1 unit

    memcpy(tempBuf, "\x78\x00\x00\x00\x00\x00\x00", 7);
    ret = topaz_transceive_handler(picc, tempBuf, 7, tempBuf, &nbytes, 9);
    if(!ret)
    {
        picc->sn_len = 4;
        memcpy(picc->sn, tempBuf + 2, 4);

		INFO_TO("TOPAZ ID: %02X %02X %02X %02X\n",
				tempBuf[2],  tempBuf[3], tempBuf[4], tempBuf[5]);
    }

//	TRACE_TO("exit %s\n", __func__);
	
    return(ret);  
}



/*****************************************************************/
void topaz_polling_tags(struct picc_device *picc)
{

//	TRACE_TO("enter %s\n", __func__);

    // reset speed settings to 106Kbps
    pcd_config_iso14443_card(CONFIGTYPEA, TYPEA_106TX);
    pcd_config_iso14443_card(CONFIGNOTHING, TYPEA_106RX);
    
    // check for any card in the field
    if(typeA_request(picc, PICC_WUPA) == -ERROR_NOTAG)
    {
        picc->type = PICC_ABSENT;
		picc->name = "none";
    }
    else
    {
        if(!topaz_get_rid(picc))
        {
            picc->type = PICC_TOPAZ;
			picc->name = "topaz";
        }
    }

//	TRACE_TO("exit %s\n", __func__);
}


/*****************************************************************/
int topaz_xfr_handler(struct picc_device *picc, u8 *cmdBuf, u32 cmdLen, u8 *resBuf, u32 *resLen)
{
    int ret = 0;
    u16  timeOut;
    u32 tempLen;
    

    if(cmdBuf[0] == TOPAZ_RID)
    {
        memset(cmdBuf + 1, 0x00, 6);
        timeOut = 8;
        cmdLen = 7;
    }
    else if(cmdBuf[0] == TOPAZ_RALL)
    {
        cmdBuf[1] = 0x00;
        cmdBuf[2] = 0x00;
        timeOut = 8;
        memcpy(cmdBuf + 3, picc->sn, 4);
    }
    else if(cmdBuf[0] == TOPAZ_READ)
    {
        cmdBuf[2] = 0x00;
        timeOut = 8;
        memcpy(cmdBuf + 3, picc->sn, 4);
    }
    else if(cmdBuf[0] == TOPAZ_WRITE_E)
    {
        timeOut = 70;
        memcpy(cmdBuf + 3, picc->sn, 4);
    }
    else if(cmdBuf[0] == TOPAZ_WRITE_NE)
    {
        timeOut = 40;
        memcpy(cmdBuf + 3, picc->sn, 4);
    }
    else
    {
        timeOut = 8;
    }
    cmdLen = 7;
    ret = topaz_transceive_handler(picc, cmdBuf, cmdLen, resBuf, &tempLen, timeOut);
    if(!ret)
    {
        *resLen = tempLen;
        ret = 0;
    }
    else
    {
        resBuf[0] = 0x64;
        resBuf[1] = 0x01;
        *resLen = 2;
        ret = -PICC_ERRORCODE_MUTE;
    }
    
    return(ret);
}

