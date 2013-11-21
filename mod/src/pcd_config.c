




#include "common.h"
#include "pn512.h"
#include "pcd_config.h"




u8 RCRegFactor[19] = 
{
    0x12, 0x85, 0x85, 0x85, 0x85, 0x85, 0x85, 0x85, 0x85,
    0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x3F, 0x3F
};




//*******************************************
// ISO 14443 TypeA config data
//*******************************************
static const u8 CfgTbl_AGen[] =
{
    ModeReg,       0x39,        //CRCPreset = 6363H
    BitFramingReg, 0x00,
    GsNOnReg,      0xFF,
    GsNOffReg,     0x6F,
    ManualRCVReg,  0x00,
    TxAutoReg,     0x43,
    DemodReg,      0x4D,
    0x00
};

static const u8 CfgTbl_A106Tx[] =
{
    TxModeReg,0x80,        //ISO/IEC 14443A/MIFARE and 106 kbit, TxCRCEn On
    ModWidthReg,0x26,
    0x00
};

static const u8 CfgTbl_A106Rx[] =
{
    RxModeReg,0x80,        //ISO/IEC 14443A/MIFARE and 106 kbit, RxCRCEn On
    0x00
};

static const u8 CfgTbl_A212Tx[] =
{
    TxModeReg,0x90,
    ModWidthReg,0x13,
    0x00
};

static const u8 CfgTbl_A212Rx[] =
{
    RxModeReg,0x90,
    0x00
};

static const u8 CfgTbl_A424Tx[] = 
{
    TxModeReg,0xA0,
    ModWidthReg,0x0A,
    0x00
};

static const u8 CfgTbl_A424Rx[] =
{
    RxModeReg,0xA0,
    0x00
};

static const u8 CfgTbl_A848Tx[] =
{
    TxModeReg,0xB0,
    ModWidthReg,0x05,
    0x00
};

static const u8 CfgTbl_A848Rx[] = 
{
    RxModeReg,0xB0,
    0x00
};


//*******************************************
// ISO 14443 TypeB config data
//*******************************************
static const u8 CfgTbl_BGen[] =
{
    ModeReg,       0x3B,
    BitFramingReg, 0x00,
    GsNOnReg,      0xFF,
    TypeBReg,      0x10,
    DemodReg,      0x4D,
    ManualRCVReg,  0x10,
    0x00
};

static const u8 CfgTbl_B106Tx[] =
{
    TxModeReg,0x83,
    TxAutoReg,0x03,
    0x00
};

static const u8 CfgTbl_B106Rx[] = 
{
    RxModeReg,0x83,
    RxSelReg,0x84,
    0x00
};

static const u8 CfgTbl_B212Tx[] =
{
    TxModeReg,0x93,
    TxAutoReg,0x03,
    0x00
};


static const u8 CfgTbl_B212Rx[] = 
{
    RxModeReg,0x93,
    RxSelReg,0x84,
    0x00
};

static const u8 CfgTbl_B424Tx[] =
{
    TxModeReg,0xA3,
    TxAutoReg,0x03,
    0x00
};

static const u8 CfgTbl_B424Rx[] = 
{
    RxModeReg,0xA3,
    RxSelReg,0x82,
    0x00
};

static const u8 CfgTbl_B848Tx[] =
{
    TxModeReg,0xB3,
    TxAutoReg,0x03,
    0x00
};

static const u8 CfgTbl_B848Rx[] = 
{
    RxModeReg,0xB3,
    RxSelReg,0x82,
    0x00
};


void pcd_config_iso14443_card(u8 flagConfig, u8 cardType)
{
    u8 regAddr;
    u8 i;
    const u8 *pTable;
    


    if(flagConfig)
    {
        if(flagConfig == CONFIGTYPEA)
        {
            pTable = CfgTbl_AGen;
            pn512_reg_write(CWGsPReg, RCRegFactor[TypeACWGsP]);
        }
        else
        {
            pTable = CfgTbl_BGen;
            pn512_reg_write(CWGsPReg, RCRegFactor[TypeBCWGsP]);
            pn512_reg_write(ModGsPReg, RCRegFactor[BModeIndex]);
        }
        i = 0;
        regAddr = pTable[i++];
        while(regAddr) 
        {
            pn512_reg_write(regAddr,pTable[i++]);
            regAddr = pTable[i++];
        }
    }
    switch(cardType)
    {
        case TYPEA_106TX:
            pTable = CfgTbl_A106Tx;
            break;
        case TYPEA_212TX:
            pTable = CfgTbl_A212Tx;
            break;
        case TYPEA_424TX:
            pTable = CfgTbl_A424Tx;
            break;
        case TYPEA_848TX:
            pTable = CfgTbl_A848Tx;
            break;
        case TYPEB_106TX:
            pTable = CfgTbl_B106Tx;
            break;
        case TYPEB_212TX:
            pTable = CfgTbl_B212Tx;
            break;
        case TYPEB_424TX:
            pTable = CfgTbl_B424Tx;
            break;
        case TYPEB_848TX:
            pTable = CfgTbl_B848Tx;
            break;
        case TYPEA_106RX:
            pn512_reg_write(RFCfgReg, RCRegFactor[ARFAmpCfg106]);
            pn512_reg_write(RxThresholdReg, RCRegFactor[RxAThres106]);
            pTable = CfgTbl_A106Rx;
            break;
        case TYPEA_212RX:
            pn512_reg_write(RFCfgReg, RCRegFactor[ARFAmpCfg212]);
            pn512_reg_write(RxThresholdReg, RCRegFactor[RxAThres212]);
            pTable = CfgTbl_A212Rx;
            break;
        case TYPEA_424RX:
            pn512_reg_write(RFCfgReg, RCRegFactor[ARFAmpCfg424]);
            pn512_reg_write(RxThresholdReg, RCRegFactor[RxAThres424]);
            pTable = CfgTbl_A424Rx;
            break;
        case TYPEA_848RX:
            pn512_reg_write(RFCfgReg, RCRegFactor[ARFAmpCfg848]);
            pn512_reg_write(RxThresholdReg, RCRegFactor[RxAThres848]);
            pTable = CfgTbl_A848Rx;
            break;
        case TYPEB_106RX:
            pn512_reg_write(RFCfgReg, RCRegFactor[BRFAmpCfg106]);
            pn512_reg_write(RxThresholdReg, RCRegFactor[RxBThres106]);
            pTable = CfgTbl_B106Rx;
            break;
        case TYPEB_212RX:
            pn512_reg_write(RFCfgReg, RCRegFactor[BRFAmpCfg212]);
            pn512_reg_write(RxThresholdReg, RCRegFactor[RxBThres212]);
            pTable = CfgTbl_B212Rx;
            break;
        case TYPEB_424RX:
            pn512_reg_write(RFCfgReg, RCRegFactor[BRFAmpCfg424]);
            pn512_reg_write(RxThresholdReg, RCRegFactor[RxBThres424]);
            pTable = CfgTbl_B424Rx;
            break;
        default:
            pn512_reg_write(RFCfgReg, RCRegFactor[BRFAmpCfg848]);
            pn512_reg_write(RxThresholdReg, RCRegFactor[RxBThres848]);
            pTable = CfgTbl_B848Rx;
            break;
    }
    
    i = 0;
    regAddr = pTable[i++];
    while(regAddr) 
    {
        pn512_reg_write(regAddr,pTable[i++]);
        regAddr = pTable[i++];
    }
}






