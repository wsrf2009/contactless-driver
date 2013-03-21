



#include <linux/string.h>


#include "common.h"
#include "mifare.h"
#include "typeA.h"
#include "picc.h"
#include "pn512app.h"
#include "pn512.h"
#include "delay.h"
#include "debug.h"


UINT8 PICC_MIFARE_KEY[2][6] = 
{ 
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
};

struct mifareInfo mifare;



/*****************************************************************/
//       Mifare select
/****************************************************************/
UINT8 MifareSelect(void)
{
    UINT8 ret;
    UINT8 i;
    UINT8 tempUIDLength;
    UINT8 tempUID[5];
    UINT8 level = 0;


//    PrtMsg(DBGL6, "%s: start\n", __FUNCTION__);   

    ret = PcdRequestA(PICC_WUPA, (UINT8*)picc.ATQA);
    if(ret == ERROR_NOTAG)
    {
        ret = PcdRequestA(PICC_WUPA, (UINT8*)picc.ATQA);
    }
    if(ret != ERROR_NOTAG)
    {
        tempUIDLength = picc.snLen;
        i = 0;
        while(tempUIDLength)
        {
            if(tempUIDLength == 4)
            {
                memcpy(tempUID, picc.sn + i, 4);
                tempUIDLength -= 4;
            }
            else
            {
                tempUID[0] = 0x88;
                memcpy(tempUID + 1, picc.sn + i,3);
                tempUIDLength -= 3;
                i += 3;
            }

            ret = PiccCascSelect(selectCmd[level], tempUID, (UINT8*)&(picc.SAK));
            level++;
        }
    }
    if(ret == ERROR_NO)
    {
        mifare.authNeed = 0x01;
        picc.states = PICC_SELECTED;
    }

//    PrtMsg(DBGL6, "%s: exit, ret = %02X\n", __FUNCTION__, ret);

    return(ret);
}


static UINT8 MifareAuthentAnalyze(UINT8 *MFAuthKey)
{
//    UINT8 i;
    UINT8 ret;


    PrtMsg(DBGL6, "%s: start\n", __FUNCTION__);

    FIFOFlush();        // Empty FIFO
    RegWrite(REG_FIFODATA, mifare.keyType);
    RegWrite(REG_FIFODATA, mifare.block);
    FIFOWrite(MFAuthKey, 6);

    if(picc.snLen == 7)
    {
        FIFOWrite(&picc.sn[3], 4);
    }
    else
    {
        FIFOWrite(picc.sn, 4);
    }

    SetTimer100us(3000);
    ret = PcdHandlerCmd(CMD_MFAUTHENT, BIT_PARITYERR);
    if((ret == ERROR_NO) || (ret == ERROR_NOTAG))
    {
        if(RegRead(REG_STATUS2) & 0x08)
        {
            ret = SLOT_NO_ERROR;
        }
        else
        {
            ret = SLOTERROR_ICC_MUTE;
        }
    }
    else
    {
        ret = SLOTERROR_ICC_MUTE;
    }

    PrtMsg(DBGL6, "%s: exit, ret = %02X\n", __FUNCTION__, ret);
    
    return(ret);
}


static UINT8 CheckBinaryRdWrtLen(UINT8 BlockNum, UINT8 TempLe)
{
    UINT8 ret = SLOT_NO_ERROR;
    
    if((picc.SAK & 0xDF) == SAK_MIFARE_4K)
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
    else if(((picc.SAK & 0xDF) == SAK_MIFARE_1K) || ((picc.SAK & 0xDF) == SAK_MIFARE_MINI))
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
    
    return(ret);
}


static UINT8 MifareAuthent(void)
{
    UINT8 ret;


    PrtMsg(DBGL6, "%s: start\n", __FUNCTION__);
    
    if(((mifare.keyValid & 0x01) && (mifare.keyNo == 0x00))
        || ((mifare.keyValid & 0x02) && (mifare.keyNo == 0x01)))
    {
        ret = MifareAuthentAnalyze((UINT8*)mifare.workKey);
        if((ret == SLOT_NO_ERROR))
        {
            mifare.authNeed = 0x00;
        }
        else
        {
            mifare.authNeed = 0x01;
        }
    }
    else
    {
        ret = SLOT_NO_ERROR;
        mifare.authNeed = 0x01;
    }

    PrtMsg(DBGL6, "%s: exit, ret = %02X\n", __FUNCTION__, ret);
    
    return(ret);
}


static UINT8 MifareBlockCheck(UINT8 srcBlock, UINT8 desBlock, BOOL multBlockMode)
{
    UINT8 i;
    UINT8 j;
    UINT8 ret = SLOT_NO_ERROR;
    
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
    if(mifare.block >= 0x80)         //For Mifare 4K large block
    {
        i=(UINT8)(srcBlock / 16);        // calculate the source sector number
        j=(UINT8)(desBlock / 16);        // calculate the target sector number
        if((i != (UINT8)(mifare.block / 16)) || (j != (UINT8)(mifare.block / 16)))
        {
            ret = 1;                    // skip
        }
    }
    else
    {
        i=(UINT8)(srcBlock / 4);        // calculate the source sector number
        j=(UINT8)(desBlock / 4);        // calculate the target sector number
        if((i!=(UINT8)(mifare.block / 4)) || (j!=(UINT8)(mifare.block / 4)))
        {
            ret = 1;                   // skip
        }
    }
    
    return(ret);
}


static UINT8 MifareBlockRead(UINT8 addr, UINT8 *blockData)
{
    UINT8 j;
    UINT8 nBytesReceived;
    UINT8 ret;

    FIFOFlush();            // Empty FIFO
    RegWrite(REG_FIFODATA, PICC_MF_READ);    // read command code
    RegWrite(REG_FIFODATA, addr);
    SetTimer100us(40);
    ret = PcdHandlerCmd(CMD_TRANSCEIVE, BIT_PARITYERR | BIT_CRCERR);

    nBytesReceived = RegRead(REG_FIFOLEVEL);
    //Read in the Data
    for(j=0; j<nBytesReceived; j++) 
    {
        blockData[j] = RegRead(REG_FIFODATA);
    }
    if(ret != ERROR_NO)
    {
        ret = SLOTERROR_ICC_MUTE;
    }
    else
    {
        if (nBytesReceived != 16) 
        {
            ret = SLOTERROR_ICC_MUTE;
        }
        else
        {
            ret = SLOT_NO_ERROR;
        }
    }
    
    return(ret);
}


static UINT8 MifareBlockWrite(UINT8 opcode, UINT8 addr, UINT8 *blockData)
{
    UINT8 i;
//    UINT8 j;
    UINT8 nBytesReceived;
    UINT8 ret;
    UINT8 tempBuf[5];


    PrtMsg(DBGL6, "%s: start\n", __FUNCTION__);

    FIFOFlush();            // Empty FIFO
    RegWrite(REG_FIFODATA, opcode);        // Write command code
    RegWrite(REG_FIFODATA, addr);
    if(opcode == PICC_MF_WRITE_4_BYTES)
    {
        FIFOWrite(blockData, 4);
    }
    SetTimer100us(600);
    ret = PcdHandlerCmd(CMD_TRANSCEIVE, BIT_PARITYERR);

    nBytesReceived = RegRead(REG_FIFOLEVEL);
    i = GetBitNumbersReceived();

    //Read in the Data
    FIFORead(tempBuf, nBytesReceived);

    if(ret != ERROR_NOTAG)
    {
        if(i != 4)
        {
            ret = SLOTERROR_ICC_MUTE;
        }
        else
        {
            if((tempBuf[0] & 0x0f) == 0x0A)
            {
                ret = SLOT_NO_ERROR;
                if(opcode == PICC_MF_WRITE_4_BYTES)
                {
                    return(ret);
                }
            }
            else
            {
                ret = SLOTERROR_ICC_MUTE;
            }
        }
    }
    else
    {
        ret = SLOTERROR_ICC_MUTE;
    }
    if(ret == SLOT_NO_ERROR)
    {
        FIFOWrite(blockData, 16);
        
        SetTimer100us(600);
        ret = PcdHandlerCmd(CMD_TRANSCEIVE, BIT_PARITYERR);

        nBytesReceived = RegRead(REG_FIFOLEVEL);
        i = GetBitNumbersReceived();

        // *** Read in the Data
        FIFORead(tempBuf, nBytesReceived);
  	
        if (ret & 0x80) 
        {   
            // timeout occured
            ret = SLOTERROR_ICC_MUTE;
        } 
        else 
        {
            if (i != 4)           // 4 bits are necessary
            {
               ret = SLOTERROR_ICC_MUTE;
            }
            else  
            {
                // 4 bit received
                if((tempBuf[0] & 0x0f) == 0x0A)
                {
                    ret = SLOT_NO_ERROR;
                }
                else
                {
                    ret = SLOTERROR_ICC_MUTE;
                }

            }
        } 
    }

    PrtMsg(DBGL6, "%s: exit, ret = %02X\n", __FUNCTION__, ret);

    return(ret);
}


static UINT8 MifareIncDec(UINT8 opcode, UINT8 addr, UINT8 *value)
{
    UINT8 i;
    UINT8 j;
    UINT8 nBytesReceived;
    UINT8 ret;
    UINT8 tempBuf[5];


    FIFOFlush();                          // Empty FIFO
    RegWrite(REG_FIFODATA, opcode);       // Write command code
    RegWrite(REG_FIFODATA, addr);

    if(opcode == PICC_MF_TRANSFER)
    {
        SetTimer100us(120);
    }
    else
    {
        SetTimer100us(15);
    }
    ret = PcdHandlerCmd(CMD_TRANSCEIVE, BIT_PARITYERR);
    nBytesReceived = RegRead(REG_FIFOLEVEL);

    i = GetBitNumbersReceived();
    //Read in the Data
    for (j = 0; j < nBytesReceived; j++) 
    {
        tempBuf[j] = RegRead(REG_FIFODATA);
    }  
    if(ret != ERROR_NOTAG)
    {
        if(i != 4)
        {
            ret = SLOTERROR_ICC_MUTE;
        }
        else
        {
            if((tempBuf[0] & 0x0f) == 0x0A)
            {
                ret = SLOT_NO_ERROR;
            }
            else
            {
                ret = SLOTERROR_ICC_MUTE;
            }
        }
    }
    else
    {
        ret = SLOTERROR_HW_ERROR;
    }

    if((ret == SLOT_NO_ERROR) && (opcode != PICC_MF_TRANSFER))
    {
        j = 4;
        while(j--)
        {
            RegWrite(REG_FIFODATA, value[j]);
        }
        
        SetTimer100us(15);          // long timeout
        ret = PcdHandlerCmd(CMD_TRANSCEIVE, BIT_PARITYERR);

        nBytesReceived = RegRead(REG_FIFOLEVEL);
        //i = GetBitNumbersReceived();
        //  Read in the Data
        for (j = 0; j < nBytesReceived; j++)
        {
            tempBuf[j] = RegRead(REG_FIFODATA);
        }
        if (ret == ERROR_NOTAG) 
        {   
           	ret = SLOT_NO_ERROR;
        } 
        else 
        {
            ret = SLOTERROR_ICC_MUTE;           
        } 
    }
    
    return(ret);
}



UINT8 MifarePcscCommand(UINT8 *senBuf, UINT16 senLen, UINT8 *recBuf, UINT16 *recLen)
{
    UINT8 i;
    UINT8 ret;
    UINT8 mifareBlock;
    UINT8 tempLe;
    BOOL  multBlockMode;
    UINT8 mifareOpcode;
    UINT8 *pResAddr;
    

    /******* Load Authentication Keys ************/
    // accroding to pcsc part3, Requirements for PC-Connected Interface Devices
    if((senLen == 11) && (senBuf[1] == 0x82) && (senBuf[4] == 0x06))
    {
        if((senBuf[2] == 0x00) && (senBuf[3] <= 0x01))
        {
            for(i = 0; i < 6; i++)
            {
                PICC_MIFARE_KEY[senBuf[3]][i] = senBuf[5 + i];
            }

            recBuf[0] = 0x90;
            recBuf[1] = 0x00;
            *recLen   = 0x02;
            ret       = SLOT_NO_ERROR;
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
        if(picc.states != PICC_SELECTED)
        {
            ret = MifareSelect();
        }
        if(((senBuf[8] == PICC_MF_KEY_A)||(senBuf[8] == PICC_MF_KEY_B)) && (senBuf[9] <= 0x01))
        {
            mifare.block   = senBuf[7];
            mifare.keyType = senBuf[8];
            mifare.keyNo   = senBuf[9];
            ret = MifareAuthentAnalyze(PICC_MIFARE_KEY[mifare.keyNo]);
            if(ret == SLOT_NO_ERROR)
            {
                if(mifare.keyNo == 0x00) 
                {
                    mifare.keyValid |= 0x01;
                }
                else
                {
                    mifare.keyValid |= 0x02;
                }
                for(i = 0; i < 6; i++)
                {
                    mifare.workKey[i] = PICC_MIFARE_KEY[mifare.keyNo][i];
                }

                recBuf[0] = 0x90;
                recBuf[1] = 0x00;
                *recLen   = 0x02;
            }
            else
            {
                if(mifare.keyNo == 0x00)
                {
                    mifare.keyValid &= 0xFE;
                }
                else
                {
                    mifare.keyValid &= 0xFD;
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
        if(picc.states != PICC_SELECTED)
        {
            ret = MifareSelect();
        }
        if(((senBuf[4] == PICC_MF_KEY_A) || (senBuf[4] == PICC_MF_KEY_B)) && (senBuf[5] <= 0x01))
        {
            mifare.block   = senBuf[3];
            mifare.keyType = senBuf[4];
            mifare.keyNo   = senBuf[5];
            ret = MifareAuthentAnalyze(PICC_MIFARE_KEY[mifare.keyNo]);
            if(ret == SLOT_NO_ERROR)
            {
                if(mifare.keyNo == 0x00) 
                {
                    mifare.keyValid |= 0x01;
                }
                else
                {
                    mifare.keyValid |= 0x02;
                }
                for(i = 0; i < 6; i++)
                {
                    mifare.workKey[i] = PICC_MIFARE_KEY[mifare.keyNo][i];
                }

                recBuf[0] = 0x90;
                recBuf[1] = 0x00;
                *recLen   = 0x02;
            }
            else
            {
                if(mifare.keyNo == 0x00)
                {
                    mifare.keyValid &= 0xFE;
                }
                else
                {
                    mifare.keyValid &= 0xFD;
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
    else if((senLen == 0x05) && (senBuf[1] == 0xB0) && (senBuf[2] == 0x00))
    {
        ret = CheckBinaryRdWrtLen(senBuf[3], senBuf[4]);
        if(ret == SLOT_NO_ERROR)
        {
            if((mifare.authNeed == 0x01) && (picc.SAK != SAK_MIFARE_ULTRALIGHT))
            {
                MifareAuthent();
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
                if(picc.SAK != SAK_MIFARE_ULTRALIGHT)
                {
                    ret     = MifareBlockCheck(mifareBlock, mifareBlock, multBlockMode);
                    tempLe -= 0x10;
                }
                else 
                {
                    ret = SLOT_NO_ERROR;
                }
                if(ret == SLOT_NO_ERROR)
                {
                    ret = MifareBlockRead(mifareBlock, pResAddr);
                    pResAddr += 0x10;
                }
                if(ret != SLOT_NO_ERROR)
                {
                    break;
                }
                mifareBlock++;
                if(picc.SAK == SAK_MIFARE_ULTRALIGHT)
                {
                    break;
                }
            }while(tempLe);
            if(ret == SLOT_NO_ERROR)
            {
                recBuf[(*recLen)++] = 0x90;
                recBuf[(*recLen)++] = 0x00;
            }
        }
    }
    // check Binary Write
    // FF D6 00 BLOCK_NO LE
    else if((senBuf[1] == 0xD6) && (senBuf[2] == 0x00))
    {
        ret = CheckBinaryRdWrtLen(senBuf[3], senBuf[4]);
        if(ret == SLOT_NO_ERROR)
        {
            if((mifare.authNeed == 0x01) && (picc.SAK != SAK_MIFARE_ULTRALIGHT))
            {
                MifareAuthent();
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
                if(picc.SAK != SAK_MIFARE_ULTRALIGHT)
                {
                    ret     = MifareBlockCheck(mifareBlock, mifareBlock, multBlockMode);
                    tempLe -= 0x10;
                    mifareOpcode = PICC_MF_WRITE_16_BYTES;
                }
                else 
                {
                    ret = SLOT_NO_ERROR;
                    tempLe -= 0x04;
                    mifareOpcode = PICC_MF_WRITE_4_BYTES;
                }
                if(ret == SLOT_NO_ERROR)
                {
                    ret = MifareBlockWrite(mifareOpcode, mifareBlock, pResAddr);
                    if(mifareOpcode == PICC_MF_WRITE_16_BYTES)
                    {
                        pResAddr += 0x10;
                    }
                    else
                    {
                        pResAddr += 0x04;
                    }
                }
                if(ret != SLOT_NO_ERROR)
                {
                    break;
                }
                mifareBlock++;
            }while(tempLe);
            
            if(ret == SLOT_NO_ERROR)
            {
                recBuf[0] = 0x90;
                recBuf[1] = 0x00;
                *recLen   = 0x02;
            }
            if(picc.SAK != SAK_MIFARE_ULTRALIGHT)
            {
                if((mifareBlock >= 0x80) && ((mifareBlock % 16) == 0x00))
                {
                    pcd.piccPoll = TRUE;
                }
                else if((mifareBlock % 4) == 0x00)
                {
                    pcd.piccPoll = TRUE;
                }
            }
        }
    }
    
    // check Value Block Read
    // FF B1 00 BLOCK_NO 04
    else if((senLen == 0x05) && (senBuf[1] == 0xB1) && (senBuf[2] == 0x00) && (senBuf[4] == 0x04))
    {
        if((mifare.authNeed == 0x01) && (picc.SAK != SAK_MIFARE_ULTRALIGHT))
        {
            MifareAuthent();
        }
        if(picc.SAK != SAK_MIFARE_ULTRALIGHT)
        {
            ret = MifareBlockCheck(senBuf[3], senBuf[3], FALSE);
        }
        else 
        {
            ret = SLOT_ERROR;
        }
        if( ret == SLOT_NO_ERROR)
        {
            ret = MifareBlockRead(senBuf[3], recBuf);
        }
        if(ret == SLOT_NO_ERROR)
        {
            // check the Value Block Format
            for(i = 0; i < 4; i++)
            {
                if((recBuf[i] != recBuf[8 + i])||((recBuf[i] ^ recBuf[4 + i]) != 0xff))
                {
                    ret = 1;
                }
            }

            if(ret == SLOT_NO_ERROR)
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
        if((mifare.authNeed == 0x01) && (picc.SAK != SAK_MIFARE_ULTRALIGHT))
        {
            MifareAuthent();
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
        if(picc.SAK != SAK_MIFARE_ULTRALIGHT)
        {
            ret = MifareBlockCheck(mifareBlock, mifareBlock, FALSE);
        }
        else 
        {
            ret = SLOT_ERROR;
        }
        if( ret == SLOT_NO_ERROR)
        {
            if(mifareOpcode == PICC_MF_WRITE_16_BYTES)
            {
                ret = MifareBlockWrite(mifareOpcode, mifareBlock, senBuf + 5);
            }
            else
            {
                ret = MifareIncDec(mifareOpcode, mifareBlock, senBuf + 6);
            }
        }
        if(ret == SLOT_NO_ERROR)
        {
            // Step 2. Transfer Operation
            if((mifareOpcode == PICC_MF_INCREMENT) || (mifareOpcode == PICC_MF_DECREMENT))
            {
                ret = MifareIncDec(PICC_MF_TRANSFER, mifareBlock, 0x00);
            }
            if(ret == SLOT_NO_ERROR)
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
        if((mifare.authNeed == 0x01) && (picc.SAK != SAK_MIFARE_ULTRALIGHT))
        {
            MifareAuthent();
        }
        mifareBlock = senBuf[6];
        // step 1. Restore Value Operation
        // Restore Value Operation

        if(picc.SAK != SAK_MIFARE_ULTRALIGHT)
        {
            ret = MifareBlockCheck(senBuf[3], senBuf[6], FALSE);
        }
        else 
        {
            ret = SLOT_ERROR;
        }
        if( ret == SLOT_NO_ERROR)	
        {
            ret = MifareIncDec(PICC_MF_RESTORE, senBuf[3], 0);
        }
        if(ret == SLOT_NO_ERROR)
        {
            // Step 2. Transfer Operation
            ret = MifareIncDec(PICC_MF_TRANSFER, mifareBlock, 0);
            if(ret == SLOT_NO_ERROR)
            {
                recBuf[0] = 0x90;
                recBuf[1] = 0x00;
                *recLen = 2;
            }
        }
    }
    else
    {
        ret = SLOTERROR_CMD_ABORTED;
    }

    if(ret == SLOT_NO_ERROR)
    {
        pcd.pollDelay = 1000;                 // 1000ms, start another poll
        pcd.piccPoll = FALSE;
    }
    else if(ret == SLOTERROR_CMD_ABORTED)
    {
        recBuf[0] = 0x6A;
        recBuf[1] = 0x81;
        *recLen = 0x02;
        ret = SLOT_NO_ERROR;
        pcd.piccPoll = TRUE;
    }
    else
    {
        recBuf[0] = 0x63;
        recBuf[1] = 0x00;
        *recLen = 0x02;
        ret = SLOT_NO_ERROR;	
        pcd.piccPoll = TRUE;
    }

    return(ret);
}


