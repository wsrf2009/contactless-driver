


#include "common.h"
#include "topaz.h"



void CRCReg_Update(UINT8 ch, UINT16 *IpwCrc)
{


    ch ^= (UINT8)(*IpwCrc & 0x00FF);
    ch ^= (ch << 4);
    *IpwCrc = (*IpwCrc >> 8)^((UINT16)ch << 8)^((UINT16)ch << 3)^((UINT16)ch >> 4);
}

void ComputeCrc(UINT8 CRCType, UINT8 *Data, UINT16 Length, UINT8 *TransmitFirst, UINT8 *TransmitSecond)
{
    UINT16 wCrc;


    switch(CRCType) 
    {
        case CRC_A:
            wCrc = 0x6363;        // ITU-V.41
            break;
        case CRC_B:
            wCrc = 0xFFFF;        // ISO/IEC 13239 (formerly ISO/IEC 3309)
            break;
        default:
            return;
    }

    while(Length--)
    {
        CRCReg_Update(*Data++, &wCrc);
    }

    if (CRCType == CRC_B)
    {
        wCrc = ~wCrc;         // ISO/IEC 13239 (formerly ISO/IEC 3309)
    }
    
    *TransmitFirst = (UINT8)(wCrc & 0xFF);
    *TransmitSecond = (UINT8)((wCrc >> 8) & 0xFF);
}

