
#include "common.h"




#define CRC_A    1
#define CRC_B    2


void CRCReg_Update(u8 ch, u16 *IpwCrc)
{


    ch ^= (u8)(*IpwCrc & 0x00FF);
    ch ^= (ch << 4);
    *IpwCrc = (*IpwCrc >> 8)^((u16)ch << 8)^((u16)ch << 3)^((u16)ch >> 4);
}

void ComputeCrc(u8 CRCType, u8 *Data, u32 Length, u8 *TransmitFirst, u8 *TransmitSecond)
{
    u16 wCrc;


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
    
    *TransmitFirst = (u8)(wCrc & 0xFF);
    *TransmitSecond = (u8)((wCrc >> 8) & 0xFF);
}

