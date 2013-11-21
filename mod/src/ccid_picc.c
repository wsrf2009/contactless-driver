
#include "ccid.h"



static u8 ccid_get_picc_status(void)
{
	if(BITISCLEAR(common->picc.status, PRESENT))
		return PICC_ICCSTATUS_NOICC;
	else if(BITISCLEAR(common->picc.status, ACTIVATED))
		return PICC_ICCSTATUS_INACTIVE;
	else
		return PICC_ICCSTATUS_ACTIVE;
}

static void ccid_picc_power_on(struct ccid_msg_data *picc_data)
{
	int ret = 0;

	
	ret = picc_power_on(&common->picc, picc_data->rBuf, &picc_data->rLen);
	
	picc_data->errCode = ret;
	picc_data->cardStatus = ccid_get_picc_status();

}

static void ccid_picc_power_off(struct ccid_msg_data *picc_data)
{
	picc_power_off(&common->picc);

	picc_data->errCode = PICC_ERRORCODE_NONE;
	picc_data->param = 0;

	picc_data->cardStatus = ccid_get_picc_status();

}

static void ccid_picc_get_slot_status(struct ccid_msg_data *picc_data)
{
	picc_data->errCode = PICC_ERRORCODE_NONE;
	picc_data->param = 0;

	picc_data->cardStatus = ccid_get_picc_status();

}

static void ccid_picc_xfr_block(struct ccid_msg_data *picc_data)
{
	u8 level = (u8)picc_data->param;
	int ret = 0;

//	printk("slen = %08X, rlen = %08X\n", picc_data->sLen, picc_data->rLen);
	ret = picc_command_exchange(&common->picc, picc_data->sBuf, picc_data->sLen,
								picc_data->rBuf, &picc_data->rLen, 
								&level);
//	printk("slen = %08X, rlen = %08X\n", picc_data->sLen, picc_data->rLen);
	picc_data->errCode = ret;
	picc_data->param = 0;

	picc_data->cardStatus = ccid_get_picc_status();	
}

static void ccid_picc_get_parameters(struct ccid_msg_data *picc_data)
{
	// bmFindexDindex	
	picc_data->rBuf[0] = 0x11;	
	// bmTCCKST1	
	picc_data->rBuf[1] = 0x10;	
	// bGuardTimeT1	
	picc_data->rBuf[2] = 0x01;	
	// bmWaitingIntegersT1	
	picc_data->rBuf[3] = 0x4D;	
	// bClockStop	
	picc_data->rBuf[4] = 0x00;
	
	// bIFSC	
	if(fsdi_to_fsd[common->picc.FSCI] > (u16)0xFE)		
		picc_data->rBuf[5] = 0xFE;	
	else		
		picc_data->rBuf[5] = (u8)fsdi_to_fsd[common->picc.FSCI];	
	// bNadValue	
	picc_data->rBuf[6] = 0x00;	
	// bProtocolNum	
	picc_data->param = 0x01;	
	picc_data->rLen = 0x07;	
	picc_data->errCode = PICC_ERRORCODE_NONE;	
	picc_data->cardStatus = ccid_get_picc_status();

}

static void ccid_picc_set_parameters(struct ccid_msg_data *picc_data)
{
	ccid_picc_get_parameters(picc_data);
}

static void ccid_picc_reset_parameters(struct ccid_msg_data *picc_data)
{
	ccid_picc_get_parameters(picc_data);
}

static void ccid_picc_escape(struct ccid_msg_data *picc_data)
{

}

static struct ccid_operations ccid_picc_ops =
{
	.card_power_on = ccid_picc_power_on,
	.card_power_off = ccid_picc_power_off,
	.get_slot_status = ccid_picc_get_slot_status,
	.xfr_block = ccid_picc_xfr_block,
	.get_parameters = ccid_picc_get_parameters,
	.set_parameters = ccid_picc_set_parameters,
	.reset_parameters = ccid_picc_reset_parameters,
	.escape = ccid_picc_escape,
};

int picc_interrput_in(u8 slot_status)
{
	if(!common->slot_changed_notify)
		return(-ENXIO);
	
	return(common->slot_changed_notify(common->private_data, slot_status));
}

void ccid_picc_init(struct ccid_operations **ops, 
						u32 max_IFSD, 
						int (*picc_intr_in)(void *, u8),
						void *pri_data)
{
	picc_param_init(common, max_IFSD);

	common->slot_changed_notify = picc_intr_in;

	common->private_data = pri_data;
	
	*ops = &ccid_picc_ops;
}

void ccid_picc_uninit(void)
{
	picc_param_init(common, 256);

	common->slot_changed_notify = NULL;

	common->private_data = NULL;
}

EXPORT_SYMBOL(ccid_picc_init);

EXPORT_SYMBOL(ccid_picc_uninit);


















