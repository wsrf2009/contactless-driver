





#include <linux/string.h>



#include "common.h"
#include "picc.h"
#include "pcsc.h"
#include "pn512.h"
#include "pn512app.h"
#include "part4.h"
#include "delay.h"
#include "typeB.h"
#include "typeA.h"




struct pcscInfo pcsc;


void PcscAtrBuild(UINT8 *atrBuf, UINT16 *atrLen)
{
    UINT8 i;
    UINT8 j;
    UINT8 Loop;
    const UINT8 PcscStoragePiccFullAtr[20] = 
    {
        0x3B, 0x8F, 0x80, 0x01, 0x80, 0x4F, 0x0C, 0xA0, 0x00, 0x00, 
        0x03, 0x06, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x6A
    };
    

    if((picc.type == PICC_MIFARE) || (picc.type == PICC_FELICA212) || (picc.type == PICC_FELICA424) || (picc.type == PICC_TOPAZ))
    {
        // contactless storage cards
        for(i = 0; i < sizeof(PcscStoragePiccFullAtr); i++)
        {
            atrBuf[i] = PcscStoragePiccFullAtr[i];            // ATR of Mifare 1K
        }
        *atrLen = sizeof(PcscStoragePiccFullAtr);
        
        if(picc.type == PICC_FELICA212)
        {
            atrBuf[12] = 0x11;    // SS-Byte For Standard: Felica
            atrBuf[13] = 0x00;
            atrBuf[14] = 0x3B;
        }
        else if(picc.type == PICC_FELICA424)
        {
            atrBuf[12] = 0x11;    // SS-Byte For Standard: Felica
            atrBuf[13] = 0xF0;
            atrBuf[14] = 0x12;
        }
        else if(picc.type == PICC_TOPAZ)
        {
            atrBuf[13] = 0x00;
            atrBuf[14] = 0x30;
        }
        else
        {
            if(picc.SAK == 0x08)
            {
                //Mifare 1K
                atrBuf[13] = 0x00;
                atrBuf[14] = 0x01;
            }
            else if(picc.SAK == 0x18)
            {
                //Mifare 4K
                atrBuf[13] = 0x00;
                atrBuf[14] = 0x02;
            }
            else if(picc.SAK == 0x00)
            {
                //Mifare Ultralight
                atrBuf[13] = 0x00;
                atrBuf[14] = 0x03;
            }
            else if(picc.SAK == 0x09)
            {
                // Mifare MINI
                atrBuf[13] = 0x00;
                atrBuf[14] = 0x26;
            }
            else
            {
                //Unknown
                atrBuf[13] = 0xFF;
                atrBuf[14] = picc.SAK;
            }
        }
        
        Loop = 0x00;
        for(i = 1; i < *atrLen-1; i++)
        {
            Loop ^= atrBuf[i];
        }
        atrBuf[*atrLen - 1] = Loop;
    }
    else                         
    {
        // contactless smart cards
        if(picc.type == PICC_TYPEA_TCL)
        {
            atrBuf[0] = 0x3B;
            i = 2;
            j = picc.ATS[0] - 2;        // Excluding TL and T0
            if(picc.ATS[1] & 0x10)      //TA1
            {
                i++;
                j--;
            }
            if(picc.ATS[1] & 0x20)      //TB1
            {
                i++;
                j--;
            }
            if(picc.ATS[1] & 0x40)      //TC1
            {
                i++;
                j--;
            }
            
            j = (j < 15) ? j : 15;
            *atrLen = j + 4;
            atrBuf[1] = j | 0x80;
            atrBuf[2] = 0x80;          //TD1
            atrBuf[3] = 0x01;          //TD2
            
            j = 0;
            while(i < picc.ATS[0])
            {
                atrBuf[4 + j] =  picc.ATS[i];
                i++;
                j++;
            }
            
            Loop = 0x00;
            for(i = 1; i < *atrLen; i++)
            {
                Loop ^= atrBuf[i];
            }
            atrBuf[*atrLen] = Loop;
            (*atrLen)++;
        }
        else if(picc.type == PICC_TYPEB_TCL)
        {
            atrBuf[0] = 0x3B;
            atrBuf[1] = 0x88;
            atrBuf[2] = 0x80;
            atrBuf[3] = 0x01;
            for(i = 0; i < 7; i++)
            {
                // application data(4 bytes), protocol info(3 bytes) of ATQB
                atrBuf[4+i] = picc.ATQB[5 + i];
            }
            atrBuf[11] = picc.resATTRIB[0] & 0xF0;
            
            Loop = 0x00;
            for(i = 1; i < 12; i++)
            {
                Loop ^= atrBuf[i];
            }
            atrBuf[12] = Loop;
            *atrLen = 13;
        }
        else
        {
            atrBuf[0] = 0x3B;
            atrBuf[1] = 0x00;
            *atrLen = 2;
        }
    }
}


static UINT8 ParamDataObject(UINT8 *paramCmd, UINT8 *paramRes, UINT16 *recLen, UINT8 WR)
{
    UINT8 ret = ERROR_NO_GENERAL;
    

    *recLen = 0;
    switch(paramCmd[0])
    {
        case 0x01:            // Frame size for IFD Integer (FSDI)
            if(WR)
            {
                pcd.FSDI = paramCmd[2]; 
            }
            else
            {
                paramRes[0] = 0x01;
                paramRes[1] = 0x01;
                paramRes[2] = pcd.FSDI;
                *recLen = 3;
            }
            break;
            
        case 0x02:            // Frame size for ICC Integer (FSCI)
            if(WR)
            {
                picc.FSCI = paramCmd[2]; 
            }
            else
            {
                paramRes[0] = 0x02;
                paramRes[1] = 0x01;
                paramRes[2] = picc.FSCI;
                *recLen = 3;
            }
            break;
            
        case 0x03:            // Frame waiting Time Integer (FWTI)
            if(WR)
            {
                picc.FWI = paramCmd[2]; 
            }
            else
            {
                paramRes[0] = 0x03;
                paramRes[1] = 0x01;
                paramRes[2] = picc.FWI;
                *recLen = 3;
            }
            break;
            
        case 0x04:            // Maximum communication speed supported by the IFD
            if(WR)
            {
                pcd.maxSpeed = paramCmd[2];
            }
            else
            {
                paramRes[0] = 0x04;
                paramRes[1] = 0x01;
                paramRes[2] = pcd.maxSpeed;
                *recLen = 3;
            }
            break;
            
        case 0x05:            // Communication speed of the current ICC
            if(WR)
            {
                pcd.curSpeed = paramCmd[2];
            }
            else
            {
                paramRes[0] = 0x05;
                paramRes[1] = 0x01;
                paramRes[2] = pcd.curSpeed;
                *recLen = 3;
            }
            break;
            
        case 0x06:            // Modulation index currently
            if(WR)
            {
                RCRegFactor[BModeIndex] = paramCmd[2];
            }
            else
            {
                paramRes[0] = 0x06;
                paramRes[1] = 0x01;
                paramRes[2] = RCRegFactor[BModeIndex];
                *recLen = 3;
            }
            break;
            
        case 0x07:            // PCB for ISO/IEC 14443
            if(WR)
            {
                picc.PCB = paramCmd[2];
            }
            else
            {
                paramRes[0] = 0x07;
                paramRes[1] = 0x01;
                paramRes[2] = picc.PCB;
                *recLen = 3;
            }
            break;
            
        case 0x08:            // CID for ISO/IEC 14443
            if(WR)
            {
                picc.CID       = paramCmd[2];
            }
            else
            {
                paramRes[0] = 0x08;
                paramRes[1] = 0x01;
                paramRes[2] = picc.CID;
                *recLen = 3;
            }
            break;
            
        case 0x09:            // NAD for ISO/IEC 14443
            ret = ERROR_WARNING;
            break;
            
        case 0x0A:            // Param 1 to Param 4 for ISO/IEC 14443 type B
            if(WR)
            {
                memcpy(picc.attrPara, paramCmd + 2, 4);
            }
            else
            {
                paramRes[0] = 0x0A;
                paramRes[1] = 0x01;
                memcpy(paramRes + 2, picc.attrPara, 4);
                *recLen = 6;
            }
            break;
            
        case 0x0B:            // Data coding (IFD to ICC) for ISO/IEC 15693
            ret = ERROR_WARNING;
            break;
            
        default:
            ret = ERROR_EXSTOP;
            break;
            
    }

    return ret;
}


static UINT8 PcscTrsprtExchange(UINT8 cmd, UINT8 *senBuf, UINT8 senLen, UINT8 *recBuf, UINT16 *RecLen)      
{
    UINT8 tempData;
    UINT8 ret;
    UINT8 tempLen;
    

    ClearRegBit(REG_STATUS2, BIT_MFCRYPTO1ON);    // disable crypto 1 unit    
    RegWrite(REG_BITFRAMING, pcsc.lastTxValBits & 0x07);   // set TxLastBits to 7 
    FIFOFlush();
    if(pcsc.fgStatus)
    {
        if(BITISCLEAR(pcsc.fgTxRx, BIT_TXCRC))
        {
            SetRegBit(REG_TXMODE, BIT_TXCRCEN);        //TXCRC enable
        }
        else
        {
            ClearRegBit(REG_TXMODE, BIT_TXCRCEN);      //TXCRC disable
        }
        if(BITISCLEAR(pcsc.fgTxRx, BIT_RXCRC))
        {
            SetRegBit(REG_RXMODE, BIT_RXCRCEN);        //RXCRC enable
        }
        else
        {
            ClearRegBit(REG_RXMODE, BIT_RXCRCEN);      //RXCRC disable
        }
        if(BITISCLEAR(pcsc.fgTxRx, BIT_TXPARITY))
        {
            SetRegBit(REG_MANUALRCV, BIT_PARITYDISABLE);     //Parity disable
        }
        else
        {
            ClearRegBit(REG_MANUALRCV, BIT_PARITYDISABLE);   //Parity enable
        }
        if(BITISCLEAR(pcsc.fgTxRx, BIT_PROLOGUE))
        {
            TCLPrologueFieldLoad();
        }
    }

    ret = PcdRawExchange(cmd, senBuf, senLen, recBuf + 9, &tempLen);

    if(ret == ERROR_NO)
    {
     

        ret = ERROR_NO_GENERAL;
        if(cmd == CMD_TRANSMIT)
        {
            return(ret);
        }
        recBuf[0] = CMD_DO_REC_BIT_FRAM;
        recBuf[1] = 0x01;
        recBuf[2] = RegRead(REG_CONTROL) & 0x07;
        recBuf[3] = RES_RESPONSESTATUS;
        recBuf[4] = 0x02;
        recBuf[5] = 0x00;


        tempData = RegRead(REG_ERROR) & 0x1F;
        if(tempData)  
        { 
            // error occured
            if(tempData & BIT_COLLERR)  
            {  
                // collision detected
                recBuf[5] |= 0x02;
            } 
            else if(tempData & BIT_PARITYERR) 
            {  
                // parity error
                recBuf[5] |= 0x04;
            }
                
            if(tempData & BIT_PROTOCOLERR) 
            {  
                // framing error
                recBuf[5] |= 0x08;
            }
            if(tempData & BIT_BUFFEROVFL) 
            { 
                // FIFO overflow
                FIFOFlush();
                ret = ERROR_NOICCRES;
                return(ret);
            }
            if(tempData & BIT_CRCERR) 
            {
                recBuf[5] |= 0x01;
            }
        }

        recBuf[6] = 0x00;
        recBuf[7] = RES_ICCRESPONSE;
        recBuf[8] = tempLen;
        *RecLen = tempLen + 9;
    }
    else
    {
        ret = ERROR_NOICCRES;
    }
    
    return(ret);
}


static void RcLongTimerSet(UINT32 timeVal)
{
    UINT32 tempTime;
    

    tempTime =   timeVal / 100;
    if (tempTime < 65536)
    {
        if(tempTime == 0)
        {
            tempTime = 1;
        }
        SetTimer100us((UINT16)tempTime);
    }
    else
    {
        tempTime /= 6;
        if (tempTime > 65535)
        {
            tempTime = 65535;
        }
        RegWrite(REG_TMODE, 0x8F);       //  TAuto=1,TAutoRestart=0,TPrescaler=677=2a5h
        RegWrite(REG_TPRESCALER, 0xE3); 	//  Indicate 100us per timeslot
        RegWrite(REG_TRELOADVAL_HI, (UINT8)(tempTime>>8));   // 
        RegWrite(REG_TRELOADVAL_LO, (UINT8)tempTime);        //
        RegWrite(REG_COMMIRQ, 0x01);                         // Clear the TimerIrq bit
}
}



UINT8 PcscIfdCmdDispatch(UINT8 cmdtype, UINT8 *cmdBuf, UINT8 cmdlen, UINT8 *resBuf, UINT16 *resLen)
{
    UINT8  ret = ERROR_NO_GENERAL;
    UINT8  i = 0;
    UINT16 j = 5;
    UINT8  ObjectNo = 0;
//    UINT8  tempLen;
    UINT32 timeVal = 0;
    UINT16 tempLen;
    UINT8  n;
    
    
    pcsc.preCmd = OJ_Idle;
    while(cmdlen)
    {
        //------------- Manage Session Command-----------------------//
        ObjectNo++;
        if(cmdBuf[i] == CMD_DO_VERSIONDATA)
        {
            //Version Data Object
            if((cmdBuf[i + 1] == 0x03) && (cmdlen >= 5))
            {
                resBuf[j++] = CMD_DO_VERSIONDATA;
                resBuf[j++] = 0x03;
                resBuf[j++] = IFDVersion[0];
                resBuf[j++] = IFDVersion[1];
                resBuf[j++] = IFDVersion[2];
                i += 5;
                cmdlen -= 5;
            }
            else
            {
                ret = ERROR_OBJECT_LENGTH;
                break;
            }
            pcsc.preCmd = CMD_DO_VERSIONDATA;
        }
        else if(cmdBuf[i] == CMD_DO_STARTTRANS)
        {
            if((cmdBuf[i + 1] == 0x00) && (cmdlen >= 2))
            {
                pcsc.trsStatus     = 0x01;
                pcsc.fgStatus      = 0x00;
                pcsc.lastRxValBits = 0x00;
                pcsc.lastTxValBits = 0x00;
                i      += 2;
                cmdlen -= 2;
            }
            else
            {
                ret = ERROR_OBJECT_LENGTH;
                break;
            }
            pcsc.preCmd = CMD_DO_STARTTRANS;
        }
        else if(cmdBuf[i] == CMD_DO_ENDTRANS)
        {
            if((cmdBuf[i + 1] == 0x00) && (cmdlen >= 2))
            {
                pcsc.trsStatus = 0x00;
                pcsc.fgStatus  = 0x00;
                i      += 2;
                cmdlen -= 2;
            }
            else
            {
                ret = ERROR_OBJECT_LENGTH;
                break;
            }
            pcsc.preCmd = CMD_DO_ENDTRANS;
        }
        else if(cmdBuf[i] == CMD_DO_TURNOFF_RF)
        {
            if((cmdBuf[i + 1] == 0x00) && (cmdlen >= 2))
            {
                if(pcsc.trsStatus == 0x01)
                {
                    AntennaPower(0);
                }
                else
                {
                    ret = ERROR_EXSTOP;
                    break;
                }
                
                i      += 2;
                cmdlen -= 2;
            }
            else
            {
                ret = ERROR_OBJECT_LENGTH;
                break;
            }
            pcsc.preCmd = CMD_DO_TURNOFF_RF;
        }
        else if((cmdBuf[i] == CMD_DO_TIMER) && (cmdBuf[i + 1] == 0x46))
        {
            if((cmdBuf[i + 2] == 0x04) && (cmdlen >= 7))
            {
                if(cmdlen <= 0x08)
                {
                    pcsc.nextCmd = OJ_Idle;
                }
                else if((cmdBuf[i + 7] == CMD_DO_TIMER) || (cmdBuf[i + 7] == CMD_DO_PARAMETERS))
                {
                    pcsc.nextCmd = cmdBuf[i + 7];

                }
                else if(cmdlen >= (cmdBuf[i + 8] + 9))
                {
                    pcsc.nextCmd = cmdBuf[i + 7];

                }
                else
                {
                    ObjectNo++;
                    ret = ERROR_OBJECT_LENGTH;
                    break;
                }

                if(pcsc.nextCmd == CMD_DO_TRANSCEIVE_DATA)
                {
                    timeVal = MAKEUINT32(cmdBuf[i + 6], cmdBuf[i + 5], cmdBuf[i + 4], cmdBuf[i + 3]);
                }
                else if(pcsc.nextCmd != OJ_Idle)
                {
                    Delay256P3us(cmdBuf[i + 6]);
                    Delay256P2us(cmdBuf[i + 5]);
                    Delay256us(cmdBuf[i + 4]);
                    Delay1us(cmdBuf[i + 3]);
                }
                
                i      += 7;
                cmdlen -= 7;
            }
            else
            {
                ret = ERROR_OBJECT_LENGTH;
                break;
            }
            
            pcsc.preCmd = CMD_DO_TIMER;
        }
        else if(cmdBuf[i] == CMD_DO_TURNON_RF)
        {
            if((cmdBuf[i + 1] == 0x00) && (cmdlen >= 2))
            {
                if(pcsc.trsStatus == 0x01)
                {
                    AntennaPower(1);
                }
                else
                {
                    ret = ERROR_EXSTOP;
                    break;
                }
                
                i      += 2;
                cmdlen -= 2;
            }
            else
            {
                ret = ERROR_OBJECT_LENGTH;
                break;
            }
            pcsc.preCmd = CMD_DO_TURNON_RF;
        }
        else if((cmdBuf[i] == CMD_DO_PARAMETERS) && (cmdBuf[i + 1] == 0x6D))    // get parameters data object
        {
            if((cmdlen >= (cmdBuf[i + 2] + 3)))
            {
                if((cmdBuf[i + 2] < 0x02) || ((cmdBuf[i + 2] % 2) != 0))
                {
                    ret = ERROR_OBJECT_LENGTH;
                    break;
                }
                n = cmdBuf[i + 2];
                cmdlen -= 3;
                i += 3;
                while(n)
                {
                    ret = ParamDataObject(cmdBuf + i, resBuf + j, &tempLen, 0);
                    if(ret != ERROR_NO_GENERAL)
                        break;
                    cmdlen -= 2;
                    i += 2;
                    j += tempLen;
                    n -= 2;
                }
                if(ret != ERROR_NO_GENERAL)
                    break;
            }
            else
            {
                ret = ERROR_OBJECT_LENGTH;
                break;
            }
            pcsc.preCmd = CMD_DO_PARAMETERS;
        }
        else if((cmdBuf[i] == CMD_DO_PARAMETERS) && (cmdBuf[i + 1] == 0x6E))    // set parameters data object
        {
            if(cmdlen >= (cmdBuf[i + 2] + 3))
            {
                if((cmdBuf[i + 2] < 0x03) || ((cmdBuf[i + 2] % 3) != 0))
                {
                    ret = ERROR_OBJECT_LENGTH;
                    break;
                }
                
                n       = cmdBuf[i + 2];
                cmdlen -= 3;
                i      += 3;
                
                while(n)
                {
                    ret = ParamDataObject(cmdBuf + i, resBuf + j, &tempLen, 1);

                    if(ret != ERROR_NO_GENERAL)
                    {
                        break;
                    }
                    if(cmdBuf[i + 3] == 0x0A)
                    {
                        cmdlen -= 6;
                        i      += 6;
                        n      -= 6;
                    }
                    else
                    {
                        cmdlen -= 3;
                        i      += 3;
                        n      -= 3;
                    }
                    
                    j +=  tempLen;
                }
                if(ret != ERROR_NO_GENERAL)
                {
                    break;
                }
            }
            else
            {
                ret = ERROR_OBJECT_LENGTH;
                break;
            }
            pcsc.preCmd = CMD_DO_PARAMETERS;
        }	
        //------------- ----Manage End-------------------------------//
        
        //-------------Transparant Exchange Command-----------------------//
        else if(((cmdBuf[i] & 0xF0) == 0x90) && (cmdtype == TRANSPARENT_EXCHANGE) && (pcsc.trsStatus==0x01))
        {
            if(cmdBuf[i] == CMD_DO_TRANSCEIVE_FLAG)
            {
                if((cmdBuf[i + 1] == 0x02) && (cmdlen >= 4))
                {
                    pcsc.fgTxRx = MAKEWORD(cmdBuf[i + 3], cmdBuf[i + 2]);
                    pcsc.fgStatus = 0x01;
                    i += 4;
                    cmdlen -= 4;
                }
                else
                {
                    ret = ERROR_OBJECT_LENGTH;
                    break;
                }
                pcsc.preCmd = CMD_DO_TRANSCEIVE_FLAG;
            }
            else if(cmdBuf[i] == CMD_DO_TRANS_BIT_FRAM)
            {
                if((cmdBuf[i + 1] == 0x01) && (cmdlen >= 3))
                {
                    pcsc.lastTxValBits = cmdBuf[i + 2];
                    i += 3;
                    cmdlen -= 3;
                }
                else
                {
                    ret = ERROR_OBJECT_LENGTH;
                    break;
                }
                pcsc.preCmd = CMD_DO_TRANS_BIT_FRAM;
            }
            else if(cmdBuf[i] == CMD_DO_REC_BIT_FRAM)
            {
                if((cmdBuf[i + 1] == 0x01) && (cmdlen >= 3))
                {
                    pcsc.lastRxValBits = cmdBuf[i + 2];
                    i += 3;
                    cmdlen -= 3;
                }
                else
                {
                    ret = ERROR_OBJECT_LENGTH;
                    break;
                }
                pcsc.preCmd = CMD_DO_REC_BIT_FRAM;
            }
            else if(cmdBuf[i] == CMD_DO_TRANSMIT)
            {
                if((cmdBuf[i + 1] > 0x00) && (cmdlen > (cmdBuf[i + 1] + 2)))
                {
                    SetTimer100us(3);
                    tempLen = cmdBuf[i + 1];
                    ret = PcscTrsprtExchange(CMD_TRANSMIT, cmdBuf + i + 2, tempLen, 0, 0);
                    cmdlen -= (tempLen + 2);
                    i += tempLen + 2;
                    ret = ERROR_NO_GENERAL;
                }
                else
                {
                    ret = ERROR_OBJECT_LENGTH;
                    break;
                }
                pcsc.preCmd = CMD_DO_TRANSMIT;
            }
            else if(cmdBuf[i] == CMD_DO_RECEIVE)
            {
                if((cmdBuf[i + 1] == 0x00) && (cmdlen >= 0x02))
                {
                    if((cmdlen >= 9) && (cmdBuf[i + 2]== CMD_DO_TIMER) && (cmdBuf[i + 3] == 0x46) && (cmdBuf[i + 4] == 0x04))
                    {
                        timeVal =  MAKEUINT32(cmdBuf[i + 8], cmdBuf[i + 7], cmdBuf[i + 6], cmdBuf[i + 5]);
                    }
                    if(timeVal)
                    {
                        RcLongTimerSet(timeVal);
                    }
                    else
                    {
                        if(picc.states == PICC_ACTIVATED)
                        {
                            PcdSetTimeout(picc.FWI);
                        }
                        else
                        {
                            PcdSetTimeout(0);
                        }
                    }

                    ret = PcscTrsprtExchange(CMD_RECEIVE, 0, 0, resBuf + j, &tempLen);
                    if(ret == ERROR_NO_GENERAL)
                    {
                        j += tempLen;
                    }
                    else
                    {
                        break;
                    }
                    cmdlen -= 2;
                    i += 2;
                }
                else
                {
                    ret = ERROR_OBJECT_LENGTH;
                    break;
                }
                pcsc.preCmd = CMD_DO_RECEIVE;
            }	
            else if(cmdBuf[i] == CMD_DO_TRANSCEIVE_DATA)
            {
                if(cmdlen >= (cmdBuf[i + 1] + 0x02))
                {
                    if(timeVal)
                    {
                        //vPCDSetTmo(TCL_PARAM.FWI);
                        RcLongTimerSet(timeVal);
                    }
                    else
                    {
                        if(picc.states == PICC_ACTIVATED)
                        {
                            PcdSetTimeout(picc.FWI);
                        }
                        else
                        {
                            PcdSetTimeout(0);
                        }
                    }
                    tempLen = cmdBuf[i+1];
                    ret = PcscTrsprtExchange(CMD_TRANSCEIVE, cmdBuf + i + 2, tempLen, resBuf + j, &tempLen);
                    if(ret == ERROR_NO_GENERAL)
                    {
                        j += tempLen;
                    }
                    else
                    {
                        break;
                    }
                    cmdlen -= (tempLen + 2);
                    i      += tempLen + 2;
                }
                else
                {
                    ret = ERROR_OBJECT_LENGTH;
                    break;
                }
                pcsc.preCmd = CMD_DO_TRANSCEIVE_DATA;
            }
        }
        //------------- ----Transparent  End-------------------------------//
        
        //------------------Switch Protocol Command-----------------------//
        else if((cmdBuf[i] == CMD_DO_SWITCH_PROTOCOL) && (pcsc.trsStatus == 0x01))
        {
            if((cmdBuf[i + 1] == 0x02) && (cmdlen >= 4))
            {
                switch(cmdBuf[i + 2])
                {
                    case SWCH_ISO14443_TYPEA:
                        if((cmdBuf[i + 3] == SWCH_TO_LAYER_2)\
                            || ((cmdBuf[i + 3] & 0xF0)== SWCH_TO_LAYER_2X))
                        {
                            PiccReset();
                            PcdConfigIso14443Type(CONFIGTYPEA, TYPEA_106TX);
                            PcdConfigIso14443Type(CONFIGNOTHING, TYPEA_106RX);
                            ret = ERROR_NO_GENERAL;
                        }
                        else if((cmdBuf[i + 3] == SWCH_TO_LAYER_3)\
                                    || ((cmdBuf[i + 3] & 0xF0)== SWCH_TO_LAYER_3X))
                        {
                            if(picc.states == PICC_ACTIVATED)
                            {
                                DeselectRequest();
                            }
                            else if(picc.states == PICC_POWEROFF)
                            {
                                AntennaPower(1);
                                Delay1ms(8);
                            }
                            else
                            {
                                PiccHaltA();
                            }
                            Delay1us(600);
                            ret = TypeASelect();
                            if(ret == ERROR_NO)
                            {
                                resBuf[j++] = CMD_DO_SWITCH_PROTOCOL;
                                resBuf[j++] = 0x01;
                                resBuf[j++] = picc.SAK;
                                ret = ERROR_NO_GENERAL;
                            }
                            else
                            {
                                ret = ERROR_NOICCRES;
                            }
                        }
                        else if((cmdBuf[i + 3] == SWCH_TO_LAYER_4) || (cmdBuf[i + 3] == SWCH_TO_HIGHLEVEL_PROTOCOL))
                        {
                            if(picc.states == PICC_ACTIVATED)
                            {
                                DeselectRequest();
                            }
                            else if(picc.states == PICC_POWEROFF)
                            {
                                AntennaPower(1);
                                Delay1ms(8);
                            }
                            else
                            {
                                PiccHaltA();
                            }
                            PollTypeATags();
                            if(picc.type != PICC_ABSENT)
                            {
                                resBuf[j++] = 0x5F;
                                resBuf[j++] = 0x51;
                                PcscAtrBuild(resBuf + j + 1, &tempLen);
                                resBuf[j++] = tempLen;
                                j +=  tempLen;
                                ret = ERROR_NO_GENERAL;
                            }
                            else
                            {
                                ret = ERROR_NOICCRES;
                            }
                        }
                        else 
                        {
                            ret = ERROR_NOINF;
                        }
                        break;
                        
                    case SWCH_ISO14443_TYPEB:
                        if((cmdBuf[i + 3] == SWCH_TO_LAYER_2)\
                            || ((cmdBuf[i + 3] & 0xF0) == SWCH_TO_LAYER_2X))
                        {
                            PiccReset();
                            PcdConfigIso14443Type(CONFIGTYPEB, TYPEB_106TX);
                            PcdConfigIso14443Type(CONFIGNOTHING, TYPEB_106RX);
                            ret = ERROR_NO_GENERAL;
                        }
                        else if((cmdBuf[i + 3] == SWCH_TO_LAYER_3)\
                            || ((cmdBuf[i + 3] & 0xF0) == SWCH_TO_LAYER_3X))
                        {
                            if(picc.states == PICC_ACTIVATED)
                            {
                                DeselectRequest();
                            }
                            else if(picc.states == PICC_POWEROFF)
                            {
                                AntennaPower(1);
                                Delay1ms(6);
                            }
                            else
                            {
                                PiccHaltB(picc.sn);
                            }
                            ret = PiccRequestB(PICC_WUPB, 0);
                            if (ret == ERROR_NOTAG) 
                            {
                                Delay1us(100);
                                ret = PiccRequestB(PICC_WUPB, 0); // ReqB with 1 slot
                            }
                            if(ret == ERROR_NO)
                            {
                                resBuf[j++] = CMD_DO_SWITCH_PROTOCOL;
                                resBuf[j++] = 0x03;
                                resBuf[j++] = picc.ATQB[9];
                                resBuf[j++] = picc.ATQB[10];
                                resBuf[j++] = picc.ATQB[11];
                                ret = ERROR_NO_GENERAL;
                            }
                            else
                            {
                                ret = ERROR_NOICCRES;
                            }
                        }
                        else if((cmdBuf[i + 3] == SWCH_TO_LAYER_4) || (cmdBuf[i + 3] == SWCH_TO_HIGHLEVEL_PROTOCOL))
                        {
                            if(picc.states == PICC_ACTIVATED)
                            {
                                DeselectRequest();
                            }
                            else if(picc.states == PICC_POWEROFF)
                            {
                                AntennaPower(1);
                                Delay1ms(6);
                            }
                            else
                            {
                                PiccHaltB(picc.sn);
                            }
                            PollTypeBTags();
                            if(picc.type != PICC_ABSENT)
                            {
                                resBuf[j++] = 0x5F;
                                resBuf[j++] = 0x51;
                                PcscAtrBuild(resBuf + j + 1, &tempLen);
                                resBuf[j++] = tempLen;
                                j +=  tempLen;
                                ret = ERROR_NO_GENERAL;
                            }
                            else
                            {
                                ret = ERROR_NOICCRES;
                            }
                        }
                        else 
                        {
                            ret = ERROR_NOINF;
                        }
                        break;
                        
                    case SWCH_ISO15693:
                        ret = ERROR_NOINF;
                        break;
                        
                    case SWCH_FELICA:
                        ret = ERROR_NOINF;
                        break;
                        
                    case SWCH_ICODE_EPC_UID:
                        ret = ERROR_NOINF;
                        break;
                        
                    case SWCH_ICODE_1:
                        ret = ERROR_NOINF;
                        break;
                        
                    case SWCH_HF_EPC_G2_ISO18000_3:
                        ret = ERROR_NOINF;
                        break;
                        
                    case SWCH_INNOVATRON:
                        ret = ERROR_NOINF;
                        break;
                        
                    default:
                        ret = ERROR_NOINF;
                        break;
                        
                }
                if(ret != ERROR_NO_GENERAL)
                {
                    break;
                }
                i      += 4;
                cmdlen -= 4;
            }
            else
            {
                ret = ERROR_OBJECT_LENGTH;
                break;
            }
        }
        //------------------Switch Protocol End-----------------------//
        else
        {
            if(cmdlen == 0x01)
            {
                ret = ERROR_NO_GENERAL;
                break;
            }
            else
            {
                ret = ERROR_NOINF;
                break;
            }
        }
    }

    if((ret == ERROR_OBJECT_LENGTH) && (cmdlen == 0x01))
    {
        ret = ERROR_NO_GENERAL;
        if(cmdBuf[i])
        {
            j = (j < cmdBuf[i]) ? j : cmdBuf[i];
        }
    }

    resBuf[0] = 0xC0;
    resBuf[1] = 0x03;
    if(ret == ERROR_NO_GENERAL)
    {
        resBuf[2] = 0x00;
        resBuf[3] = 0x90;
        resBuf[4] = 0x00;
    }
    else
    {
        resBuf[2] = ObjectNo;
        switch (ret)
        {
            case ERROR_WARNING:
                resBuf[3] = 0x62;
                resBuf[4] = 0x82;
                break;
                
            case ERROR_NOINF:
                resBuf[3] = 0x63;
                resBuf[4] = 0x00;
                break;
                
            case ERROR_EXSTOP:
                resBuf[3] = 0x63;
                resBuf[4] = 0x01;
                break;
                
            case ERROR_OBJECT_NOTSUPPORTED:
                resBuf[3] = 0x6A;
                resBuf[4] = 0x81;
                break;
                
            case ERROR_OBJECT_LENGTH:
                resBuf[3] = 0x67;
                resBuf[4] = 0x00;	
                break;
                
            case ERROR_OBJECT_VALUE:
                resBuf[3] = 0x6A;
                resBuf[4] = 0x80;
                break;
                
            case ERROR_NOIFDRES:
                resBuf[3] = 0x64;
                resBuf[4] = 0x00;
                break;
                
            case ERROR_NOICCRES:
                resBuf[3] = 0x64;
                resBuf[4] = 0x01;
                break;
                
            default:
                resBuf[3] = 0x6F;
                resBuf[4] = 0x00;	
                break;
        }
    }
    
    //ISO7816 SW1SW2
    resBuf[j++] = 0x90;
    resBuf[j++] = 0x00;
    *resLen = j;
    ret = SLOT_NO_ERROR;
    
    return(ret);
}




