

//#include <linux/spi/spi.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>


#include "common.h"
#include "pn512.h"
#include "debug.h"
#include "pcd_config.h"
#include "spi.c"



#define SPI_DEVICE_NAME					"pn512"
#define SPI_DEVICE_RESET_GPIO			136
#define SPI_DEVICE_IRQ_GPIO				157


#define PN512_FIFO_MAX_DEEPTH			64
#define PN512_FIFO_MAX_FIFO_ALLOWED		62
#define PN512_FIFO_WATER_LEVEL			58



struct pn512_common *pn512 = NULL;






int pn512_reg_write(u8 reg, u8 value)
{
    u8 cmdBuf[2];
	int ret;


	spin_lock(&pn512->pn512_lock);

    reg <<= 1;
    CLEAR_BIT(reg, BIT(7));
    cmdBuf[0] = reg;
    cmdBuf[1] = value;

	ret = spi_write(pn512->spi_device, cmdBuf, 2);
    if(ret < 0)
    {
		ERROR_TO("writting register to pn512 failed\n");
       	goto done;
    }

	ret = 0;

done:
	spin_unlock(pn512->pn512_lock);
    return(0);
}

u8 pn512_reg_read(u8 reg)
{
    u8 tempReg;
    u8 tempVal = 0x00;
	int ret;


//	TRACE_TO("enter %s\n", __func__);

	spin_lock(&pn512->pn512_lock);
    
    tempReg = (reg << 1);
    SET_BIT(tempReg, BIT(7));

	ret = spi_write_then_read(pn512->spi_device, &tempReg, 1, &tempVal, 1);
    if(ret < 0)
    {
        ERROR_TO("reading register from pn512 failed\n");    
    }

	spin_unlock(pn512->pn512_lock);

//	TRACE_TO("exit %s, [%02X] = %02X\n", __func__, reg,tempVal);
	
    return(tempVal);
}

void pn512_reg_clear(u8 reg, u8 bitMask)
{
	u8 tempValue;

	tempValue = pn512_reg_read(reg);
	tempValue &= ~bitMask;
	pn512_reg_write(reg, tempValue);
}

void pn512_reg_set(u8 reg, u8 bitMask)
{
	u8 tempValue;
    
	tempValue = pn512_reg_read(reg);
	tempValue |= bitMask;
	pn512_reg_write(reg, tempValue);
}


int pn512_fifo_write(u8 *data, u32 len)
{
    u8 buf[65];
	int ret;
    

	spin_lock(&pn512->pn512_lock);
	
    if (len > PN512_FIFO_MAX_DEEPTH)
    {
        ret = -EINVAL;
		goto done;
    }

	if(len <= 0)
	{
		ret = 0;
		goto done;
	}
    
    buf[0] = (FIFODataReg << 1) & 0x7E;     // accroding to PN512: bit 7 = 0, write; bit 0 = 0
    memcpy(buf + 1, data, len);

	ret = spi_write(pn512->spi_device, buf, len + 1);
	if(ret < 0)
		goto done;

	ret = 0;
	
done:
	spin_unlock(pn512->pn512_lock);
    return(ret);
}


int pn512_fifo_read(u8 *data, u32 len)
{
    u32 i;
    u8 buf[64];
	int ret;
    

	spin_lock(&pn512->pn512_lock);
	
    if (len > PN512_FIFO_MAX_DEEPTH)
    {
       	ret = -EINVAL;
		goto done;
    }
    
    if (len <= 0)
    {
        ret = 0;
		goto done;
    }
    
    memset(buf, (FIFODataReg << 1) | 0x80, len);    // accroding to PN512: bit 7 = 1, read; bit 0 = 0


    for(i = 0; i < len; i++)
    {
    	ret = spi_write_then_read(pn512->spi_device, buf+i, 1, data+i, 1);
        if(ret < 0)
        {
	        goto done;
        }
    }

	ret = 0;

done:
	spin_unlock(pn512->pn512_lock);
    return(ret);
}


void set_pn512_timer(u16 timeOut)
{
    pn512_reg_write(TModeReg, 0x82);                        //  TAuto=1,TAutoRestart=0,TPrescaler=677=2a5h
    pn512_reg_write(TPrescalerReg, 0xA5);                   //  Indicate 100us per timeslot
    pn512_reg_write(TReloadVal_Hi, (u8)(timeOut>>8));  	// 
    pn512_reg_write(TReloadVal_Lo, (u8)timeOut);        //
    pn512_reg_write(CommIRqReg, 0x01);                      // Clear the TimerIrq bit
}

void turn_on_antenna(void)
{
	pn512_reg_write(TxControlReg, 0x83);
}

void turn_off_antenna(void)
{
	pn512_reg_write(TxControlReg, 0x80);
}
void pn512_process_done(struct pn512_request *req)
{
	struct pn512_common *pn512 = container_of(req, struct pn512_common, request);

//	TRACE_TO("enter %s\n", __func__);
	pn512_reg_write(CommandReg, CMD_IDLE);
	pn512_reg_set(ControlReg, TStopNow);
	pn512_reg_clear(TModeReg, TAuto);
	pn512_reg_write(CommIRqReg, 0x7F);


	complete(&pn512->pn512_complete);

//	TRACE_TO("exit %s\n", __func__);
}


void pn512_process_request(struct pn512_request *req)
{
	struct pn512_common *pn512 = container_of(req, struct pn512_common, request);


//	TRACE_TO("enter %s\n", __func__);

	pn512->intr_enable_mask = IRqInv|ErrIEn|TimerIEn;
	req->done = pn512_process_done;

	spin_lock(&pn512->pn512_lock);

	pn512_reg_write(BitFramingReg, req->bit_frame);
	pn512_reg_write(FIFOLevelReg, FlushBuffer);		// flush fifo

	if(req->time_out)
	{
//		if(req->timer_start_auto)
			set_pn512_timer((u16)req->time_out);

		if(req->timer_start_now)
			pn512_reg_set(ControlReg, TStartNow);
	}

	if(req->command == CMD_MFAUTHENT)
		pn512->intr_enable_mask |= IdleIEn;

#if 0
	if(req->direction == RECEIVE)
	{
		pn512->intr_enable_mask |= RxIEn|HiAlertIEn;
		pn512_reg_set(REG_COMMIRQ, BIT_HIALERTIRQ);
	}
	else
	{
		if(req->length > PN512_FIFO_MAX_FIFO_ALLOWED)
		{
			
			pn512_fifo_write(&req->buf[req->actual], PN512_FIFO_MAX_FIFO_ALLOWED);
			req->actual += PN512_FIFO_MAX_FIFO_ALLOWED;
			req->length -= PN512_FIFO_MAX_FIFO_ALLOWED;
			pn512->intr_enable_mask |= LoAlertIEn;
			pn512_reg_set(REG_COMMIRQ, BIT_LOALERTIRQ);
		}
		else
		{
			pn512_fifo_write(&req->buf[req->actual], req->length);
			req->actual = req->length = 0;
			pn512->intr_enable_mask |= TxIEn;
			if(req->direction == TRANSCEIVE)
				pn512->intr_enable_mask |= RxIEn;
		}
	}

	pn512_reg_write(REG_COMMAND, req->command);

	if(req->direction == TRANSCEIVE)
	{
		pn512_reg_set(REG_BITFRAMING, BIT_STARTSEND);
	}
#else
	if(req->direction == RECEIVE)
	{
		pn512->intr_enable_mask |= RxIEn|HiAlertIEn;
		pn512_reg_set(CommIRqReg, HiAlertIRq);

		pn512_reg_write(CommandReg, req->command);
	}
	else 
	{
		if(req->length > PN512_FIFO_MAX_FIFO_ALLOWED)
		{
			u8 temp_len;

			
			pn512_fifo_write(req->buf, PN512_FIFO_MAX_FIFO_ALLOWED);
			req->actual = PN512_FIFO_MAX_FIFO_ALLOWED;
			req->length -= PN512_FIFO_MAX_FIFO_ALLOWED;

			pn512_reg_write(CommandReg, req->command);

			pn512_reg_set(BitFramingReg, StartSend);

			while(req->length)
			{
        		if(pn512_reg_read(FIFOLevelReg) < pn512->water_level)      //
        		{
//        			TRACE_TO("water=%d\n", pn512->water_level);
        			temp_len = (PN512_FIFO_MAX_FIFO_ALLOWED-pn512->water_level);
        			temp_len = req->length < temp_len ? req->length : temp_len;
            		pn512_fifo_write(&req->buf[req->actual], temp_len);
					req->actual += temp_len;
					req->length -= temp_len;
        		}	
			}

			req->actual = req->length = 0;

			pn512->intr_enable_mask |= TxIEn;
			if(req->direction == TRANSCEIVE)
				pn512->intr_enable_mask |= RxIEn;

		}
		else
		{
			pn512_fifo_write(req->buf, req->length);
			req->actual = req->length = 0;
			
			pn512->intr_enable_mask |= TxIEn;
			if(req->direction == TRANSCEIVE)
				pn512->intr_enable_mask |= RxIEn;

			pn512_reg_write(CommandReg, req->command);
		
			pn512_reg_set(BitFramingReg, StartSend);

		}



	}
	

#endif
	
	// enable pn512 irq
	pn512_reg_set(CommIEnReg, pn512->intr_enable_mask);
	
	wait_for_completion(&pn512->pn512_complete);

	// disable pn512 irq
	pn512_reg_write(CommIEnReg, IRqInv);
	
	spin_unlock(&pn512->pn512_lock);
	
}

static void pn512_irq_work_queue(struct work_struct *work)
{
	struct pn512_common *pn512 = container_of(work, struct pn512_common, wq);
	struct pn512_request *req = &pn512->request;
	u8 comm_irq;


	pn512_reg_write(CommIEnReg, IRqInv);
	
//	TRACE_TO("enter %s\n", __func__);
	
	comm_irq = pn512_reg_read(CommIRqReg);

	// reset irq mark
	pn512_reg_write(CommIRqReg, comm_irq);
	comm_irq &= pn512->intr_enable_mask;

	// indicate the timer decrements the TimerValue Register to zero
	if(comm_irq & TimerIRq)
	{
		// stop timer
		req->error_code = -ERROR_NOTAG;
		
		req->done(req);
		
		goto done;
	}

	// indicate any error bit in the Error Register is set
	if(comm_irq & ErrIRq)
	{
		u8 error = pn512_reg_read(ErrorReg);

		if(BITISSET(error, CollErr))	
			req->error_code = -ERROR_COLL;								// collision detected
		else 
		{
			if(BITISSET(error, ParityErr)) 
				req->error_code = -ERROR_PARITY;						// parity error
		}
		
		if(BITISSET(error, ProtocolErr))
			req->error_code = -ERROR_PROTOCOL;					  // framing error
		
		if(BITISSET(error, BufferOvfl) )
		{  
			pn512_reg_write(FIFOLevelReg, 0x80);						  // FIFO overflow
			req->error_code = -ERROR_BUFOVFL;
		}
		
		if(BITISSET(error, CRCErr))
			req->error_code = -ERROR_CRC;

	}

	// indicate the last bit of the transmitted data was sent out
	if(comm_irq & TxIRq)
	{
		if(!req->length)
		{
			// transfer has complete
			req->tx_done = 1;
			if(req->direction == TRANSMIT)
			{
				req->done(req);

				goto done;
			}
			else
			{	
				pn512->intr_enable_mask &= ~(LoAlertIEn|TxIEn);
				pn512->intr_enable_mask |= RxIEn|HiAlertIEn;
				pn512_reg_write(CommIEnReg, pn512->intr_enable_mask);
			}
		}
	}

	// indicate bit HiAlert in register Status1Reg is set
	if(comm_irq & HiAlertIRq)
	{
		u8 level;

		if(!req->tx_done)
			return;

		// receiving stage
		level = pn512_reg_read(FIFOLevelReg);
		pn512_fifo_read(&req->buf[req->actual], level);
		req->actual += level;

	}

	// indicate the receiver detects the end of a valid datastream
	if(comm_irq & RxIRq)
	{
		// receiving has complete
		u8 level;


		level = pn512_reg_read(FIFOLevelReg);

		pn512->intr_enable_mask &= ~HiAlertIEn;
		pn512_reg_write(CommIEnReg, pn512->intr_enable_mask);

		req->rx_last_bits = pn512_reg_read(ControlReg)&RxLastBitsMask;

		
		pn512_fifo_read(&req->buf[req->actual], level);
		req->actual += level;
		if(req->rx_last_bits)
			req->bit_numbers = (req->actual-1)*8 + req->rx_last_bits;
		else
			req->bit_numbers = req->actual*8;
		
		req->done(req);

		goto done;
	}

	if(comm_irq & IdleIRq)
	{
		req->done(req);
		goto done;
	}


#if 0
	// bit LoAlert in register Status1Reg is set
	if(comm_irq & BIT_LOALERTIRQ)
	{
		UINT8 lim_len;

		if(!req->tx_done)
		{
			// transfer stage
			lim_len = PN512_FIFO_MAX_DEEPTH - pn512->water_level;
			if(req->length > lim_len)
			{
				pn512_fifo_write(&req->buf[req->actual], lim_len);
				req->actual += lim_len;
				req->length -= lim_len;

				if((pn512_reg_read(REG_STATUS2)&ModemStateMask) == 0x01)
				{
					pn512_reg_set(REG_BITFRAMING, BIT_STARTSEND);
				}

			}
			else if(req->length)
			{
				pn512_fifo_write(&req->buf[req->actual], req->length);
				req->actual = req->length = 0;
				
				pn512->intr_enable_mask &= ~LoAlertIEn;
				pn512->intr_enable_mask |= TxIEn;
				if(req->direction == TRANSCEIVE)
					pn512->intr_enable_mask |= RxIEn;
				
				pn512_reg_write(CommIEnReg, pn512->intr_enable_mask);				

					pn512_reg_set(REG_BITFRAMING, BIT_STARTSEND);

			}
	
		}

	}
#endif

done:

//	TRACE_TO("exit %s\n", __func__);
	pn512_reg_write(CommIEnReg, pn512->intr_enable_mask);
}

static irqreturn_t pn512_interrupt(int irq, void *dev)
{
	struct pn512_common *pn512 = dev;


	schedule_work(&pn512->wq);
	
	return IRQ_HANDLED;	
}

#if 0
static int pn512_probe(struct spi_device *spi)
{
    int ret;
	const struct spi_device_id	*id_table = spi_get_device_id(spi);
	struct pn512_common		*pn512 = (struct pn512_common *)id_table->driver_data;


//	TRACE_TO("enter %s\n", __func__);

	pn512->spi_device = spi;
	
	ret = gpio_request(pn512->reset_pin, "pn512_reset");
	if(ret)
	{
		ERROR_TO("fail to request GPIO%d for pn512 reset pin\n", pn512->reset_pin);
		goto err;
	}

	// hardware reset pn512
	ret = gpio_direction_output(pn512->reset_pin, 1);
	if(ret)
	{
		ERROR_TO("operation failed\n");
		goto err2;
	}	
	udelay(400);
	gpio_set_value(pn512->reset_pin, 0);
	udelay(400);
	gpio_set_value(pn512->reset_pin, 1);
	udelay(400);
 
	// software reset pn512
    ret = pn512_reg_write(REG_COMMAND, CMD_SOFTRESET);
    if(ret < 0)
		goto err2;
    
    mdelay(100);
  
    ret = pn512_reg_write(REG_TXCONTROL, 0x00);         //Turn off the Antenna
    if(ret < 0)
		goto err2;

    ret = pn512_reg_write(REG_COMMAND, 0x00);           // Switch on the analog part of the receiver 
    if(ret < 0)
		goto err2;
    
    ret = pn512_reg_write(REG_CONTROL, 0x10);           // Set PN512 in initiator mode
    if(ret < 0)
		goto err2;

    ret = pn512_reg_write(REG_FIFOLEVEL, 0x80);         // flush FIFO
    if(ret < 0)
		goto err2;

	ret = pn512_reg_write(REG_WATERLEVEL, pn512->water_level);         // 
    if(ret < 0)
		goto err2;



	pn512_reg_write(CommIEnReg, IRqInv);	// disable interrupts
	pn512_reg_write(REG_DIVIEN, 0x00);
	pn512_reg_write(REG_COMMIRQ, 0x7F); // clean all the irq flag bits
	pn512_reg_write(REG_DIVIRQ, 0x1F);
	

	INIT_WORK(&pn512->wq, pn512_irq_work_queue);

	if((ret = request_irq(spi->irq, pn512_interrupt, 
					IRQF_TRIGGER_FALLING, SPI_DEVICE_NAME, pn512)) < 0)
	{
		ERROR_TO("can't request irq for pn512 interrupt!\n");
		goto err3;
	}
	TRACE_TO("exit %s\n", __func__);

    return(0);

err3:
//	disable_irq(OMAP_GPIO_IRQ(pn512->intr_pin));
err2:
//	gpio_free(pn512->intr_pin);
//err1:
	gpio_free(pn512->reset_pin);
err:
//	TRACE_TO("exit %s\n", __func__);
	
	return ret;
}

static INT32 __devexit pn512_remove(struct spi_device *spi)
{
	destroy_work_on_stack(&pn512->wq);
	free_irq(spi->irq, pn512);
//	free_irq(OMAP_GPIO_IRQ(pn512->intr_pin), pn512);
//	disable_irq(OMAP_GPIO_IRQ(pn512->intr_pin));

	gpio_free(pn512->reset_pin);
//	gpio_free(pn512->intr_pin);

    return(0);
}

static struct spi_device_id pn512_id_table[] =
{
    [0]	=
		{
			.name	 = SPI_DEVICE_NAME, 
			.driver_data	=	0,
		},

	[1]	=
    	{

		},
};

static struct spi_driver pn512_driver =
{
    .driver = 
    {
        .name  = SPI_DEVICE_NAME,
        .bus   = &spi_bus_type,
        .owner = THIS_MODULE,
    },
 
    .id_table  = pn512_id_table,
    
    .probe = pn512_probe,
    .remove = __devexit_p(pn512_remove),
};
#endif

int pn512_init(struct pn512_request **req)
{
	int ret;



//	TRACE_TO("enter %s\n", __func__);
	
	pn512 = kzalloc(sizeof *pn512, GFP_KERNEL);
	if(!pn512)
	{
		ERROR_TO("fail to requesting memory for pn512");
		ret = -ENOMEM;
		goto err1;
	}



//	pn512_id_table[0].driver_data = (kernel_ulong_t)pn512;
//	pn512->spi_driver = &pn512_driver;
	pn512->reset_pin = SPI_DEVICE_RESET_GPIO;
	pn512->intr_pin = SPI_DEVICE_IRQ_GPIO;
	pn512->water_level = PN512_FIFO_WATER_LEVEL;

	spin_lock_init(&pn512->pn512_lock);

	init_completion(&pn512->pn512_complete);

#if 1

	ret = spi_init();
	if(ret)
		goto err2;

	ret = gpio_request(pn512->reset_pin, "pn512_reset");
	if(ret)
	{
		ERROR_TO("fail to request GPIO%d for pn512 reset pin\n", pn512->reset_pin);
		goto err2;
	}

	// hardware reset pn512
	ret = gpio_direction_output(pn512->reset_pin, 1);
	if(ret)
	{
		ERROR_TO("operation failed\n");
		goto err2;
	}	
	udelay(400);
	gpio_set_value(pn512->reset_pin, 0);
	udelay(400);
	gpio_set_value(pn512->reset_pin, 1);
	udelay(400);

 	pn512_reg_write(CommIRqReg, 0x10);
	// software reset pn512
    ret = pn512_reg_write(CommandReg, CMD_SOFTRESET);
    if(ret < 0)
		goto err2;
    
//    mdelay(100);
	while((pn512_reg_read(CommIRqReg)&IdleIRq) == 0);

//	TRACE_TO("cut here\n");

//	pn512_reg_read(REG_MODE);
//	pn512_reg_read(REG_MODE);

    ret = pn512_reg_write(TxControlReg, 0x00);         //Turn off the Antenna
    if(ret < 0)
		goto err2;

    ret = pn512_reg_write(CommandReg, 0x00);           // Switch on the analog part of the receiver 
    if(ret < 0)
		goto err2;
    
    ret = pn512_reg_write(ControlReg, 0x10);           // Set PN512 in initiator mode
    if(ret < 0)
		goto err2;

    ret = pn512_reg_write(FIFOLevelReg, 0x80);         // flush FIFO
    if(ret < 0)
		goto err2;

	ret = pn512_reg_write(WaterLevelReg, pn512->water_level);         // flush FIFO
    if(ret < 0)
		goto err2;

	
//	while(1){turn_on_antenna();}

	pn512_reg_write(CommIEnReg, 0x80);	// disable interrupts
	pn512_reg_write(DivIEnReg, 0x00);
	pn512_reg_write(CommIRqReg, 0x7F); // clean all the irq flag bits
	pn512_reg_write(DivIRqReg, 0x1F);

	INIT_WORK(&pn512->wq, pn512_irq_work_queue);

	ret = gpio_request(pn512->intr_pin, "pn512 interrupt");
	if(ret)
	{
		ERROR_TO("fail to request GPIO%d for pn512 reset pin\n", pn512->intr_pin);
		goto err3;
	}
	set_irq_type(OMAP_GPIO_IRQ(pn512->intr_pin), IRQF_TRIGGER_FALLING);
	enable_irq(gpio_to_irq(pn512->intr_pin));
	if((ret = request_irq(OMAP_GPIO_IRQ(pn512->intr_pin), pn512_interrupt, 
					IRQF_TRIGGER_FALLING, SPI_DEVICE_NAME, pn512)) < 0)
	{
		ERROR_TO("can't request irq for pn512 interrupt!\n");
		goto err4;
	}

#else
    if((ret = spi_register_driver(pn512->spi_driver)) < 0)
    {
        ERROR_TO("Fail to registe the driver PN512!!!\n");
        goto err2;
    }
#endif

	*req = &pn512->request;

//	TRACE_TO("exit %s\n", __func__);

//	pn512_reg_read(REG_MODE);
	
    return(0);


err4:
	gpio_free(pn512->intr_pin);
err3:
	gpio_free(pn512->reset_pin);	
err2:
	kfree(pn512);
err1:
	
//	TRACE_TO("enter %s\n", __func__);
	
	return	ret;
}

int pn512_uninit(void)
{
    pn512_reg_write(TxControlReg, 0x00);    // turn off antenna

	free_irq(OMAP_GPIO_IRQ(pn512->intr_pin), pn512);
	disable_irq(OMAP_GPIO_IRQ(pn512->intr_pin));

	gpio_free(pn512->intr_pin);
	gpio_free(pn512->reset_pin);
    
//    spi_unregister_driver(pn512->spi_driver);
	spi_uninit();

	kfree(pn512);
	
    return(0);
}
