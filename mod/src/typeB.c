






#include <linux/string.h>


#include "common.h"
#include "typeB.h"
#include "picc.h"
#include "pn512.h"
#include "pn512app.h"
#include "delay.h"
#include "part4.h"
#include "debug.h"



//                 T Y P E B   F U N C T I O N S
//////////////////////////////////////////////////////////////////////
//                    R E Q U E S T   B
//////////////////////////////////////////////////////////////////////
UINT8 PiccRequestB(UINT8 reqCmd, UINT8 N)
{
    UINT8 ret;
    UINT8 nBytesReceived;
    UINT8 i;
    

    //-----------Initialization---------
    SetRegBit(REG_TXMODE, BIT_TXCRCEN);            //TXCRC enable
    SetRegBit(REG_RXMODE, BIT_RXCRCEN);            //RXCRC enable, RxMulitple
    ClearRegBit(REG_STATUS2, BIT_MFCRYPTO1ON);     // Disable crypto 1 unit
    
    //----------Load REQB/WUPB into FIFO--------
    FIFOFlush();
    RegWrite(REG_FIFODATA, 0x05);                   // APf: the anticollision prefix byte , 0x05
    RegWrite(REG_FIFODATA, 0x00);                   // AFI: application family identifier, 0x00--- all PICC shall process the REQB/WUPB
    RegWrite(REG_FIFODATA, reqCmd | (N & 0x07));    //PARAM
    //----------Command Execute------------
    
    SetTimer100us(50);

    ret = PcdHandlerCmd(CMD_TRANSCEIVE, BIT_CRCERR);
    
    //---------Error handling------------
    if(ret == ERROR_NO) 
    {
        nBytesReceived = RegRead(REG_FIFOLEVEL);
        if((nBytesReceived == 12) || (nBytesReceived == 13))    // basic ATQB or extended ATQB
        {
            for (i = 0; i < nBytesReceived; i++) 
            {
                picc.ATQB[i] = RegRead(REG_FIFODATA);
            }

            PrtMsg(DBGL1, "%s: pupi = {%02X, %02X, %02X, %02X}\n", __FUNCTION__, picc.ATQB[1], picc.ATQB[2], picc.ATQB[3], picc.ATQB[4]);

            picc.ATQBLen = i;
            if(picc.ATQB[0] != 0x50)
            {
                ret = ERROR_WRONGPARAM;
            }
            else
            {
                picc.states = PICC_READY;
            }
        }
        else
        {
            ret = ERROR_BYTECOUNT;
        }
    } 
    
    return(ret);
}  

//////////////////////////////////////////////////////////////////////
//                    S L O T - M A R K E R
//////////////////////////////////////////////////////////////////////
static UINT8 PiccSlotMarker(UINT8 N)
{
    UINT8 ret = ERROR_NO;
    UINT8 nBytesReceived;
    UINT8 i;
    

    SetRegBit(REG_TXMODE, BIT_TXCRCEN);    //TXCRC enable
    SetRegBit(REG_RXMODE, BIT_RXCRCEN);    //RXCRC enable
    
    FIFOFlush();        // empty FIFO
    if(N == 0 || N > 15) 
    {
        ret = ERROR_WRONGPARAM;
    }
    else 
    {
        RegWrite(REG_FIFODATA, (N << 4) | 0x05);   // APn: N | 0x05
        SetTimer100us(10);
        ret = PcdHandlerCmd(CMD_TRANSCEIVE, BIT_CRCERR);
        if(ret==ERROR_NO) 
        {
            nBytesReceived = RegRead(REG_FIFOLEVEL);
            if((nBytesReceived == 12) || (nBytesReceived == 13))    // basic ATQB or extended ATQB
            {
                for(i=0; i < nBytesReceived; i++) 
                {
                    picc.ATQB[i] = RegRead(REG_FIFODATA);
                }
                picc.ATQBLen = i;
                if(picc.ATQB[0] != 0x50)
                {
                    ret = ERROR_WRONGPARAM;
                }
                else
                {
                    picc.states = PICC_READY;
                }
            }
            else
            {
                ret = ERROR_BYTECOUNT;
            }
        } 
    }
    
    return(ret);
}  

static void ATQBAnalysis(UINT8 *ATQB)
{

    CLEAR_BIT(picc.fgTCL, BIT_PCDBLOCKNUMBER);
    
    memcpy(picc.sn, ATQB + 1, 4);            // copy PUPI
    picc.snLen = 4;
    
    picc.speed = ATQB[9];
    
    picc.FSCI = ATQB[10] >> 4;
    if(picc.FSCI > 8)
    {
        picc.FSCI = 8;
    }
    
    if(ATQB[11] & 0x01)
    {
        // CID supported by the PICC
        SET_BIT(picc.fgTCL, BIT_CIDPRESENT);
        picc.CID = GetCID(picc.sn);
    }
    else
    {
        CLEAR_BIT(picc.fgTCL, BIT_CIDPRESENT);
        picc.CID = 0;
    }
    
    picc.FWI = ATQB[11] >> 4;
    if(picc.FWI > 14)
    {
        picc.FWI = 4;          // compliant to ISO14443-3:2011
    }

    if(ATQB[10] & 0x01)
    {
        picc.sPart4 = 0x01;        // PICC compliant with ISO/IEC 14443-4
    }
    else
    {
        picc.sPart4 = 0x00;        // PICC not compliant with ISO/IEC 14443-4
    }

    // b3:b2 in Protocol_Type defines the minimum of TR2. compliant to ISO14443-4: 2011
    if((ATQB[10] & 0x06) == 0x00)
    {
        picc.SFGI = 0;    // 10etu + 32 / fs
    }
    else if((ATQB[10] & 0x06) == 0x02)
    {
        picc.SFGI = 1;    // 10etu + 128 / fs
    }
    else if((ATQB[10] & 0x06) == 0x04)
    {
        picc.SFGI = 1;   // 10etu + 256 / fs
    }
    else
    {
        picc.SFGI = 2;    // 10etu + 512 / fs    
    }

    if(picc.ATQBLen == 13)
    {
        // extended ATQB supported by PICC, it defines a specific guard time replacing TR2
        // SGT is needed by the PICC before it is ready to receive the next frame after it has sent the Answer to ATTRIB command
        picc.SFGI = ATQB[12] >> 4;
        if(picc.SFGI > 14)
        {
            picc.SFGI = 0;
        }
    }
}

static UINT8 PcdAttribB(UINT8 *pupi, UINT8 *param, UINT8 *resp)
{
    UINT8 ret = ERROR_NO;
    UINT8 nBytesReceived;
    UINT8 i;
    

    SetRegBit(REG_TXMODE, BIT_TXCRCEN);            //TXCRC enable
    SetRegBit(REG_RXMODE, BIT_RXCRCEN);            //RXCRC enable, RxMulitple
    
    FIFOFlush();        // empty FIFO
    RegWrite(REG_FIFODATA, 0x1D);            // 0x1D: Attrib command
    for(i = 0; i < 4; i++) 
    {
        RegWrite(REG_FIFODATA, pupi[i]);     // PUPI: Identifier
    }
    for(i = 0; i < 4; i++) 
    {
        RegWrite(REG_FIFODATA, param[i]);    // Param: Param 1 to 4
    }

    SET_BIT(picc.fgTCL, BIT_TYPEBATTRIB);
    PcdSetTimeout(picc.FWI);
    ret = PcdHandlerCmd(CMD_TRANSCEIVE, BIT_CRCERR);
    CLEAR_BIT(picc.fgTCL, BIT_TYPEBATTRIB);

    nBytesReceived = RegRead(REG_FIFOLEVEL);
    for (i = 0; i < nBytesReceived; i++) 
    {
        resp[i] = RegRead(REG_FIFODATA);
    }

    if((resp[0] & 0x0F) != picc.CID)
    {
        ret = ERROR_CID;
    }

    if((ret == ERROR_CRC) && (nBytesReceived == 1)) 
    {
        // CRC Error
        ret = ERROR_WRONGPARAM;
    }
    
    return(ret);
}


//////////////////////////////////////////////////////////////////////
//                    A T T R I B
//////////////////////////////////////////////////////////////////////
static UINT8 StandardAttribB(void)
{
    UINT8 ret = ERROR_NO;
    UINT8 speedParam;
    

    ATQBAnalysis(picc.ATQB);
    
    speedParam = PiccSpeedCheck();
    
    picc.attrPara[0] = 0x00;                                // param 1:  TR0 = 64 /fs, TR1 = 80 / fs, SOF required, EOF required
    picc.attrPara[1] = (speedParam << 4) | pcd.FSDI;    // param 2: 
    picc.attrPara[2] = picc.sPart4;                         // param 3:
    picc.attrPara[3] = pcd.CID & 0x0f;                  // param 4:

    ret = PcdAttribB(picc.sn, picc.attrPara, picc.resATTRIB);
    if(ret == ERROR_NO)
    {
        picc.states = PICC_ACTIVATED;
        PiccHighSpeedConfig(speedParam, TYPEB_106TX);
    }
    return(ret);
} 


//////////////////////////////////////////////////////////////////////
//                    H A L T
//////////////////////////////////////////////////////////////////////
UINT8 PiccHaltB(UINT8 *pupi)
{
    UINT8 ret = ERROR_NO;
    UINT8 i;
    
    PrtMsg(DBGL4, "%s: start\n", __FUNCTION__);

    if (picc.states != PICC_POWEROFF)
    {

        //-----------Initialization---------
        SetRegBit(REG_TXMODE, BIT_TXCRCEN);        // TXCRC enable 
        SetRegBit(REG_RXMODE, BIT_RXCRCEN);        // RXCRC enable
        FIFOFlush();
        
        //----------Load argument data to FIFO--------
        RegWrite(REG_FIFODATA, 0x50);      // HALT command
        for (i = 0; i < 4; i++) 
        {
            RegWrite(REG_FIFODATA, pupi[i]);    // Identifier
        }
        SetTimer100us(15);
        ret = PcdHandlerCmd(CMD_TRANSCEIVE, BIT_CRCERR);
        if ((RegRead(REG_FIFOLEVEL) != 1) || (RegRead(REG_FIFODATA) != 0))
        {
            ret = ERROR_NOTAG;
        }
        else
        {
            picc.states= PICC_IDLE;

        }
    }

    PrtMsg(DBGL4, "%s: exit\n", __FUNCTION__);

    return(ret);
} 




void PollTypeBTags(void)
{
    UINT8 ret;
    UINT8 i;


    PrtMsg(DBGL1, "%s: start\n", __FUNCTION__);    

    /* reset speed settings to 106Kbps */
    PcdConfigIso14443Type(CONFIGTYPEB, TYPEB_106TX);
    PcdConfigIso14443Type(CONFIGNOTHING, TYPEB_106RX);
    
    /*** check for any card in the field ***/
    ret = PiccRequestB(PICC_WUPB, N_1_SLOT);    // ReqB with 1 slot

    if (ret == ERROR_NOTAG)
    {
        // if no card detected, quit the loop
        Delay1us(100);
        ret = PiccRequestB(PICC_WUPB, N_1_SLOT);     // ReqB with 1 slot
        if (ret == ERROR_NOTAG)
        {
            picc.type = PICC_ABSENT;
            return;
        }
    }

    if (ret != ERROR_NO)
    {
        // If collision occurs, performs type B anticollision
        // Anti-collision using slot-marker
        Delay1us(200);
        if((ret = PiccRequestB(PICC_WUPB, N_4_SLOT)) != ERROR_NO) // ReqB with 4 slot
        {
            for (i = 1; i < 4; i++)
            {
                if ((ret = PiccSlotMarker(i)) == ERROR_NO)
                {
                    break;
                }
            }
        }
    }
    if (ret == ERROR_NO) 
    {
        Delay1us(600);
        ret = StandardAttribB();
        if(ret == ERROR_NO)
        {
            picc.type = PICC_TYPEB_TCL;
            picc.FSC = FSCConvertTbl[picc.FSCI] - 3;    // FSC excluding EDC and PCB
            if(BITISSET(picc.fgTCL, BIT_CIDPRESENT))
            {
                picc.FSC--;        // FSC excluding CID
            }
        }
        else
        {
            if(DeselectRequest() != ERROR_NO)
            {
                PiccHaltB(picc.sn);
            }
            picc.type = PICC_ABSENT;
        }
    }
    else
    {
        picc.type = PICC_ABSENT;
    }
}


