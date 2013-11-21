
#include <linux/string.h>
#include <linux/err.h>


#include "common.h"
#include "picc.h"
#include "pn512.h"
#include "delay.h"
#include "iso14443_typeA.h"
#include "iso14443_typeB.h"
#include "felica.h"
#include "topaz.h"
#include "pcsc.h"
#include "mifare.h"
#include "iso14444.h"




static const u8 vendorName[] = {'A', 'C', 'S'};
static const u8 productName[] = {'A', 'C', 'R', '8', '9', '0'};
static const u8 driverVersion[] = {'1', '.', '0', '0'};
static const u8 firmwareVersion[] = {'A', 'C', 'R', '8', '9', '0', ' ', '1', '0', '0'};
const u8 IFDVersion[] = {0x01, 0x00, 0x00};

const u16 FSCConvertTbl[9] = {16, 24, 32, 40, 48, 64, 96, 128, 256};







void picc_wait_for_req(struct pn512_request *req)
{	
	req->actual = 0;
	req->error_code = 0;
	req->rx_last_bits = 0;
	req->tx_done = 0;
	pn512_process_request(req);
}


#ifdef USECID

typedef struct
{
    u8 NO;
    u8 *UID;
    BOOL  used;
}CIDInfo;

CIDInfo CIDNO[15];

static void cid_init(void)
{
    u8 i;

    for(i = 0; i < 15; i++)
    {
        CIDNO[i].NO   = i;
        CIDNO[i].UID  = NULL;
        CIDNO[i].used = FALSE;
    }
}

static void free_cid(u8 cid)
{
    CIDNO[cid].UID  = NULL:
    CIDNO[cid].used = FALSE;
}

#endif

u8 get_cid(u8 *uid)
{
#ifdef USECID

    UINT i = 0;


    do
    {
        if(CIDNO[i].used == TRUE)
        {
            i++;
        }
        else
        {
            break;
        }
    }while(i < 15);

    CIDNO[i].used = TRUE;
    CIDNO[i].UID  = uid;

    return(CIDNO[i].NO);
#else
    return(0);
#endif
}


/******************************************************************/
//       PICC reset
/******************************************************************/
void picc_reset(struct picc_device *picc)
{
    turn_off_antenna();    // Turn off the Antenna
    Delay1ms(9);
    turn_on_antenna();    // Turn on the Antenna
    picc->states = PICC_IDLE;
}


static void picc_polling_tags(struct picc_device *picc)
{
    bool card_still_preset = FALSE;
    

    TRACE_TO("enter %s\n", __func__);

    if(BITISCLEAR(picc->status, PRESENT))        //Case 1: No card present before 
    {
        picc->type = PICC_ABSENT;
		picc->name = "none";
        turn_on_antenna();

        if(BITISSET(picc->pcd->support_card_type, TYPEA))
        {
            Delay1ms(10);
            typeA_polling_tags(picc);
        }
        if((picc->type == PICC_ABSENT) && (BITISSET(picc->pcd->support_card_type, TYPEB)))
        {
            Delay1ms(6);
            typeB_polling_tags(picc);
        }
        if((picc->type == PICC_ABSENT) && (BITISSET(picc->pcd->support_card_type, FELICA212)))
        {
            Delay1ms(5);
            felica_polling_tags(picc, PASSDEPI_212);
        }
        if((picc->type == PICC_ABSENT) && (BITISSET(picc->pcd->support_card_type, FELICA414)))
        {
            Delay1ms(5);
            felica_polling_tags(picc, PASSDEPI_424);
        }
        if((picc->type == PICC_ABSENT) && (BITISSET(picc->pcd->support_card_type, TOPAZ)))
        {
            Delay1ms(5);
            topaz_polling_tags(picc);
        }

        
        if(picc->type == PICC_ABSENT)
        {
            //No Tag found
            turn_off_antenna();
            CLEAR_BIT(picc->status, PRESENT);
            CLEAR_BIT(picc->status, FIRST_INSERT);
            CLEAR_BIT(picc->status, ACTIVATED);
            picc->key_valid = 0x00;
        }
        else
        {
            //Tag found
            SET_BIT(picc->status, PRESENT);        // Card Inserted
            SET_BIT(picc->status, FIRST_INSERT);
            SET_BIT(picc->status, ACTIVATED);
            SET_BIT(picc->status, SLOT_CHANGE);
            picc->pcd->poll_interval = 1000;

			INFO_TO("found picc %s\n", picc->name);
        }
    }
    else            // card present before
    {
        if(picc->type == PICC_MIFARE)
        {
            picc_reset(picc);
            Delay1ms(6);
            if(mifare_select(picc))
            {
                if(!mifare_select(picc))
                {
                    card_still_preset = true;
                }
                else
                {
                    card_still_preset = false;
                }
            }
            else
            {
                card_still_preset = true;
            }
        }
        else if((picc->type == PICC_TYPEA_TCL) || (picc->type == PICC_TYPEB_TCL))
        {
            // ISO/IEC 14443-4 PICC
            if(picc->states == PICC_ACTIVATED)
            {
                if(typeA_select_(picc, 0xB2))        // R-block: ACK
                {
                    if(!typeA_select_(picc, 0xB2))
                    {
                        card_still_preset = true;
                    }
                    else
                    {
                        card_still_preset = false;
                    }
                }
                else
                {
                    card_still_preset = true;
                }
            }
            else
            {
                if(picc->type == PICC_TYPEA_TCL)
                {
                    if(picc->states == PICC_POWEROFF)
                    {
                        turn_on_antenna();
                        Delay1ms(10);
                    }

                    if(typeA_request(picc, PICC_WUPA) == -ERROR_NOTAG)
                    {
                        if(typeA_request(picc, PICC_WUPA) == -ERROR_NOTAG)
                        {
                            card_still_preset = 0x00;
                        }
                        else
                        {
                            card_still_preset = 0x01;
                        }
                    }
                    else
                    {
                        card_still_preset = 0x01;
                    }

                }
                else
                {
                    if(picc->states == PICC_POWEROFF)
                    {
                        turn_on_antenna();
                        Delay1us(100);
                    }

                    if(typeB_request(picc, PICC_WUPB,0) == -ERROR_NOTAG)
                    {
                        if(typeB_request(picc, PICC_WUPB,0) == -ERROR_NOTAG)
                        {
                            card_still_preset = false;
                        }
                        else
                        {
                            card_still_preset = true;
                        }
                    }
                    else
                    {
                        card_still_preset = true;
                    }
                }
            }
        }  
        else if((picc->type == PICC_FELICA212) || (picc->type == PICC_FELICA424))   // add--s
        {
            if(picc->states == PICC_POWEROFF)
            {
                turn_on_antenna();
                Delay1ms(6);
            }
            if(!felica_request_response(picc))
            {
                card_still_preset = true;
            }
            else
            {
                card_still_preset = false;
            }
        }
        else if(picc->type == PICC_TOPAZ)
        {
            if(picc->states == PICC_POWEROFF)
            {
                turn_on_antenna();
                Delay1ms(6);
            }
            if(typeA_request(picc, PICC_WUPA) == -ERROR_NOTAG)
            {
                if(typeA_request(picc, PICC_WUPA) == -ERROR_NOTAG)
                {
                    card_still_preset = false;
                }
                else
                {
                    card_still_preset = true;
                }
            }
            else
            {
                card_still_preset = true;
            }
        }

        // Success a tag is still there
        if(card_still_preset)
        {
            SET_BIT(picc->status, PRESENT);
        }
        else
        {  
        	turn_off_antenna();
            CLEAR_BIT(picc->status, PRESENT);
            CLEAR_BIT(picc->status, FIRST_INSERT);
            CLEAR_BIT(picc->status, ACTIVATED);
            SET_BIT(picc->status, SLOT_CHANGE);

            picc->type = PICC_ABSENT;
			picc->name = "none";
            picc->pcd->current_speed = 0x80;

			INFO_TO("PICC removed\n");
        }
    }

//    TRACE_TO("exit %s, piccType = %02X\n", __func__, picc->type);
    
}


static int picc_power_on(struct picc_device *picc, u8 *atrBuf, u32 *atrLen)
{
    int ret = 0;



    if(BITISCLEAR(picc->pcd->flags_polling, AUTO_POLLING))
    {
        picc_polling_tags(picc);
    }

 
    if(picc->states == PICC_POWEROFF)
    {
        turn_on_antenna();
        if((picc->type == PICC_MIFARE) || (picc->type == PICC_TYPEA_TCL)) 
        {
            picc->key_valid = 0x00;
            Delay1ms(10);
            typeA_polling_tags(picc);
        }
        else if(picc->type == PICC_TYPEB_TCL)
        {
            Delay1ms(6);
            typeB_polling_tags(picc);
        }
        else if(picc->type == PICC_FELICA212)
        {
            Delay1ms(5);
            felica_polling_tags(picc, PASSDEPI_212);   
        }
        else if(picc->type == PICC_FELICA424)
        {
            Delay1ms(5);
            felica_polling_tags(picc, PASSDEPI_424);
        }
        else if(picc->type == PICC_TOPAZ)
        {
            Delay1ms(5);
            topaz_polling_tags(picc);
        }  
        else
        { 
            picc->type = PICC_ABSENT;
			picc->name = "none";
        }
    }

    if(picc->type == PICC_ABSENT)
    {
        *atrLen = 0;
        CLEAR_BIT(picc->status, ACTIVATED);
        ret = -PICC_ERRORCODE_MUTE;
        turn_off_antenna();
    }
    else  
    {
        SET_BIT(picc->status, ACTIVATED);       // Card Activate
        pcsc_building_atr(picc, atrBuf, atrLen);
        ret = 0;
    }

    
    return(ret);
}


static void picc_power_off(struct picc_device *picc)
{

    if(BITISCLEAR(picc->status, FIRST_INSERT))
    {  
        if((picc->type == PICC_TYPEA_TCL) || (picc->type == PICC_TYPEB_TCL))
        {
            if(typeA_deselect_request(picc))
            {
                if(picc->type == PICC_TYPEA_TCL)
                {
                    typeA_halt(picc);
                }
                else
                {
                    typeB_halt(picc);
                }
            }
        }
        else if(picc->type == PICC_MIFARE)
        {
            typeA_halt(picc);
        }
        CLEAR_BIT(picc->status, ACTIVATED);

        if(BITISCLEAR(picc->pcd->flags_polling, AUTO_POLLING))
        {
            // if PCD do not auto poll, turn off the antenna
            turn_off_antenna();
        }

    }

}


static u8 bsi_cmd_dispatch(u8 *pcmd, u32 cmdLen, u8 *pres, u32 *presLen)
{
    u32 recLen;


    if(pcmd[2] == 0x01)
    {
        switch(pcmd[3])
        {
            case 0x01:
                recLen = sizeof(vendorName);
                memcpy(pres, vendorName, recLen);
                break;
                
            case 0x02:
                pres[0] = 0x2F;
                pres[1] = 0x07;
                recLen = 2;
                break;
                
            case 0x03:
                recLen = sizeof(productName);
                memcpy(pres, productName, recLen);
                break;
                
            case 0x04:
                pres[0] = 0x01;
                pres[1] = 0x09;
                recLen = 2;
                break;
                
            case 0x06:
                recLen = sizeof(firmwareVersion);
                memcpy(pres, firmwareVersion, recLen);
                break;
                
            case 0x07:
                recLen = sizeof(driverVersion);
                memcpy(pres, driverVersion, recLen);
                break;
                
            case 0x08:
                pres[0] = 0x00;
                pres[1] = 0x02;
                recLen = 2;
                break;
                
            case 0x09:
                pres[0] = 0x4F;    // 847 Kbps
                pres[1] = 0x03;
                recLen = 2;
                break;
                
            default:
                recLen = 0;
                break;
        }	
        if(recLen == 0)
        {
            pres[0] = 0x6A;
            pres[1] = 0x89;
            *presLen = 2;
        }
        else
        {
            pres[recLen]   = 0x90;
            pres[recLen+1] = 0x00;
            *presLen = 2 + recLen;
        }
    }
    else 
    {
        pres[0] = 0x6A;
        pres[1] = 0x90;
        *presLen = 2;
    }
    
    return(0);
}


static int picc_command_exchange(struct picc_device *picc, u8 *cmdBuf, u32 cmdLen, u8 *resBuf, u32 *resLen, u8 *level)
{
    int ret = 0;
    u32 tempLe;
    u8 i;
	struct pn512_request	*req = picc->request;


//	TRACE_TO("enter %s\n", __func__); 

    //Psuedo-APDU Get UID/ATS
    if((cmdBuf[0] == 0xFF) && (cmdLen >= 0x05) && (*level == 0x00))            //Get UID/ATS
    {
        if((cmdBuf[1] == 0x00) && (cmdBuf[2] == 0x00) && (cmdBuf[3] == 0x00) && (cmdLen > 5))
        {
            if((cmdLen != (cmdBuf[4] + 5)) && (cmdLen != (cmdBuf[4] + 6)))
            {
                resBuf[0] = 0x67;
                resBuf[1] = 0x00;
                *resLen = 2;
                ret = 0;
				goto err;
            }
            if((picc->type == PICC_FELICA212) || (picc->type == PICC_FELICA424))
            {
                ret = felica_xfr_handler(picc, cmdBuf + 5, cmdLen - 5, resBuf, resLen);
                if(*resLen > 2)
                {
                    resBuf[(*resLen)++] = 0x90;
                    resBuf[(*resLen)++] = 0x00;
                }

            }
            else if(picc->type == PICC_TOPAZ)
            {
                ret = topaz_xfr_handler(picc, cmdBuf+5, cmdLen-5, resBuf, resLen);
                if(!ret)
                {
                    resBuf[(*resLen)++] = 0x90;
                    resBuf[(*resLen)++] = 0x00;
                }
                ret = 0;
            }
            else
            {
                pn512_reg_clear(Status2Reg, MFCrypto1On);    // disable crypto 1 unit    
                picc->flags_status = 0x00;

				memcpy(req->buf, &cmdBuf[5], cmdBuf[4]);
				req->length = cmdBuf[4];
				req->bit_frame = 0x00;
				req->command = CMD_TRANSCEIVE;
				req->direction = TRANSCEIVE;
				req->time_out = 100;
				
				picc_wait_for_req(req);
				memcpy(resBuf, req->buf, req->actual);
				ret = req->error_code;
				if (!ret)
                {
                    *resLen = req->actual;
                    resBuf[(*resLen)++] = 0x90;
                    resBuf[(*resLen)++] = 0x00;
                }
                else
                {
                    resBuf[0] = 0x63;
                    resBuf[1] = 0x00;
                    *resLen   = 0x02; 
                }
                ret = 0;
            }
        }
        else if((cmdBuf[1] == 0xCA) && (cmdBuf[3] == 0x00) && (cmdLen == 0x05))
        {
            //Get UID, accroding to pcsc part 3
            tempLe = cmdBuf[4];
            if(cmdBuf[2] == 0x00)
            {
                if(tempLe <= picc->sn_len)
                {

                    for(i = 0; i < picc->sn_len; i++)
                    {
                        resBuf[i] = picc->sn[i];
                    }
                    if((tempLe == 0x00) || (tempLe == picc->sn_len))
                    {
                        resBuf[i++] = 0x90;
                        resBuf[i]   = 0x00;
                        *resLen     = picc->sn_len + 2;
                    }
                    else
                    {
                        resBuf[tempLe]     = 0x6C;
                        resBuf[tempLe + 1] = picc->sn_len;
                        *resLen            = tempLe + 2;
                    }
                }
                else
                {
                    resBuf[0] = 0x62;
                    resBuf[1] = 0x82;
                    *resLen   = 0x02;
                }
            }
            else if (cmdBuf[2] == 0x01)
            {
                //Get ATS, accroding to pcsc part 3
                resBuf[0] = 0x6A;
                resBuf[1] = 0x81;
                *resLen   = 0x02;
                tempLe    = cmdBuf[4];
                if (picc->type == PICC_TYPEA_TCL)
                {
                    for(i = 0; i < picc->ATS[0]; i++)
                    {
                        resBuf[i] = picc->ATS[i];
                    }
                    if(tempLe && (tempLe != picc->ATS[0]))
                    {
                        resBuf[tempLe]     = 0x6C;
                        resBuf[tempLe + 1] = picc->ATS[0];
                        *resLen            = tempLe + 2;
                    }
                    else
                    {
                        resBuf[i++] = 0x90;
                        resBuf[i]   = 0x00;
                        *resLen     = picc->ATS[0] + 2;
                    }
                }
            }
            else
            {
                resBuf[0] = 0x63;
                resBuf[1] = 0x00;
                *resLen   = 0x02; 
            }
            ret = 0;
        }
        else if(cmdBuf[1] == 0xC2)
        {
            if(cmdLen > 5)
            {
                if((cmdLen != (cmdBuf[4] + 5)) && (cmdLen != (cmdBuf[4] + 6)))
                {
                    resBuf[0] = 0x67;
                    resBuf[1] = 0x00;
                    *resLen   = 2;
                    ret = (0);
					goto err;
                }
            }
            ret = pcsc_cmd_dispatch(picc, cmdBuf[3], cmdBuf+5, cmdBuf[4], resBuf, resLen);
        }
        else if((cmdBuf[1] == 0x9A) && (cmdLen >= 0x05))
        {

            ret = bsi_cmd_dispatch(cmdBuf, cmdLen, resBuf, resLen);
        }
        else 
        {
            if(picc->type == PICC_MIFARE)
            {
                ret = mifare_pcsc_command(picc, cmdBuf, cmdLen, resBuf, resLen);
            }
            else
            {
                resBuf[0] = 0x63;
                resBuf[1] = 0x00;
                *resLen   = 0x02;
            }
        }
    }
    else            // Standard APDUs
    {
        if(((picc->type == PICC_TYPEA_TCL)||(picc->type == PICC_TYPEB_TCL)) && (picc->states == PICC_ACTIVATED))
        {
            ret = typeA_standard_apdu_handler(picc, cmdBuf, cmdLen, resBuf, resLen, level);
            if(ret)
            {
                typeA_deselect_request(picc);
                CLEAR_BIT(picc->status, ACTIVATED);
            }
        }
        else if((picc->type == PICC_FELICA212) || (picc->type == PICC_FELICA424))
        {
            ret = felica_xfr_handler(picc, cmdBuf, cmdLen, resBuf, resLen);
            if(*resLen < 2)
            {
                resBuf[(*resLen)++] = 0x90;
                resBuf[(*resLen)++] = 0x00;
            }

        }
        else if(picc->type == PICC_TOPAZ)
        {
            ret = topaz_xfr_handler(picc, cmdBuf + 5,cmdLen - 5, resBuf, resLen);
            if(*resLen < 2)
            {
                resBuf[(*resLen)++] = 0x90;
                resBuf[(*resLen)++] = 0x00;
            }
            ret = 0;
        }
        else
        {
            ret = -PICC_ERRORCODE_CMD_ABORTED;
        }
    }
err:
//	TRACE_TO("exit %s\n", __func__);
	
    return(ret);
}


static int picc_param_init(struct pcd_common *common, u32 fsd)
{
	if(fsd < 16)	return -EINVAL;			// FSD is not less than 16 Bytes
	else if(fsd < 24)	common->pcd.FSDI = 0;		// FSD = 16 Bytes
	else if(fsd < 32)	common->pcd.FSDI = 1;		// FSD = 24 Bytes
	else if(fsd < 40)	common->pcd.FSDI = 2;		// FSD = 32 Bytes
	else if(fsd < 48)	common->pcd.FSDI = 3;		// FSD = 40 Bytes
	else if(fsd < 64)	common->pcd.FSDI = 4;		// FSD = 48 Bytes
	else if(fsd < 96)	common->pcd.FSDI = 5;		// FSD = 64 Bytes
	else if(fsd < 128)	common->pcd.FSDI = 6;		// FSD = 96 Bytes
	else if(fsd < 256)	common->pcd.FSDI = 7;		// FSD = 128 Bytes
	else		common->pcd.FSDI = 8;			// FSD is not more than 256 Bytes
	
	
    common->pcd.support_card_type = TOPAZ|FELICA414|FELICA212|TYPEB|TYPEA;    // poll all card type
	common->pcd.flags_polling     = POLLING_CARD_ENABLE|AUTO_POLLING|AUTO_RATS;    // auto RATS, auto poll, poll card
    common->pcd.max_speed   = 0x1B;
    common->pcd.max_speed   = 0x1B;
    common->pcd.current_speed   = 0x80;
    common->pcd.poll_interval = 500;     // poll card interval time: default 500ms

    common->picc.states   = 0x00;
    common->picc.flags_TCL    = 0x00;
    common->picc.FSCI     = 0x02;    // 32 bytes
    common->picc.FWI      = 0x04;    // 4.8ms
    common->picc.SFGI     = 0x00;    // default value is 0
    common->picc.speed    = 0x80;
	common->picc.key_valid = 0x00;
	
	memset(common->pcd.mifare_key, 0xFF, sizeof(common->pcd.mifare_key));

    common->picc.flags_status = 0x00;
	
	common->pcd.picc = &common->picc;
	common->picc.pcd = &common->pcd;

	return 0;
}

static int picc_init(struct pcd_common *common)
{
	int ret = 0;

	
	picc_param_init(common, 256);

	ret = pn512_init(&common->picc.request);

	return ret;
}

static void picc_uninit(void)
{
	pn512_uninit();
}

