

#include <linux/slab.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/err.h>


#define	OMAP_MCSPI1_BASE		0x48098000
#define	OMAP_MCSPI2_BASE		0x4809A000
#define	OMAP_MCSPI3_BASE		0x480B8000
#define	OMAP_MCSPI4_BASE		0x480BA000

#define revision_reg			(spi_base_addr+0x00)

#define OMAP_SPI_CLOCKACTIVITY_BOTH_ON	(3<<8)
#define OMAP_SPI_SIDLEMODE_KEEP_ON		(1<<3)
#define	OMAP_SPI_SOFTRESET				(1<<1)
#define sys_config_reg			(spi_base_addr+0x10)

#define OMAP_SPI_RESETDONE				(1<<0)
#define sys_status_reg			(spi_base_addr+0x14)

#define irq_status_reg			(spi_base_addr+0x18)
#define irq_enable_reg			(spi_base_addr+0x1C)
#define wakeup_enable			(spi_base_addr+0x20)
#define syst_reg				(spi_base_addr+0x24)

#define OMAP_SPI_SINGLE_CHANNEL_EN		(1<<0)
#define modul_ctrl_reg			(spi_base_addr+0x28)

#define OMAP_SPI_FFER_EN				(1<<28)
#define OMAP_SPI_FFEW_EN				(1<<27)
#define OMAP_SPI_FORCE					(1<<20)
#define OMAP_SPI_TURBO					(1<<19)
#define	OMAP_SPI_DPE0					(1<<16)
#define OMAP_SPI_RECEIVE_ONLY			(1<<12)
#define OMAP_SPI_TRANSMIT_ONLY			(2<<12)
#define OMAP_SPI_TRM_MASK				(3<<12)
#define OMAP_SPI_WL_8BITS				(7<<7)
#define OMAP_SPI_WL_9BITS				(8<<7)
#define OMAP_SPI_EPOL_LOW				(1<<6)
#define OMAP_SPI_CLKD_6M				(3<<2)
#define chx_conf_reg			(spi_base_addr+0x2C)

#define OMAP_SPI_TXS					(1<<1)
#define OMAP_SPI_RXS					(1<<0)
#define chx_stat_reg			(spi_base_addr+0x30)

#define OMAP_SPI_EN_CHANNEL				(1<<0)
#define chx_ctrl_reg			(spi_base_addr+0x34)

#define tx_reg					(spi_base_addr+0x38)
#define rx_reg					(spi_base_addr+0x3C)



void __iomem	*spi_base_addr;



static int  spi_write(struct spi_device *device, u8 *in_buf, u32 in_len)
{
	u32 	temp;
	unsigned long flags;
	

	if(in_len <= 0)
		return -EINVAL;

//	local_irq_save(flags);

	temp = __raw_readl(chx_ctrl_reg);
	__raw_writel(temp|OMAP_SPI_EN_CHANNEL, chx_ctrl_reg);
	temp = __raw_readl(chx_ctrl_reg);
	
	temp = OMAP_SPI_FORCE|OMAP_SPI_DPE0|OMAP_SPI_TRANSMIT_ONLY|OMAP_SPI_WL_8BITS|OMAP_SPI_EPOL_LOW|OMAP_SPI_CLKD_6M;
	__raw_writel(temp, chx_conf_reg);

	while((__raw_readl(chx_stat_reg)&OMAP_SPI_TXS) == 0);

	local_irq_save(flags);
	
	while(in_len)
	{
		__raw_writel(*in_buf++, tx_reg);
		in_len--;

		while((__raw_readl(chx_stat_reg)&OMAP_SPI_TXS) == 0);
	}

	local_irq_restore(flags);

	temp = __raw_readl(chx_conf_reg);
	temp &= ~OMAP_SPI_FORCE;
	__raw_writel(temp, chx_conf_reg);

	temp = __raw_readl(chx_ctrl_reg);
	temp &= ~OMAP_SPI_EN_CHANNEL;
	__raw_writel(temp, chx_ctrl_reg);

//	local_irq_restore(flags);

	return 0;
}

static int spi_write_then_read(struct spi_device *device, u8 *in_buf, u32 in_len,
										u8 *out_buf, u32 out_len)
{
	u32 temp;
	unsigned long flags;


//	local_irq_save(flags);

	temp = __raw_readl(chx_ctrl_reg);
	__raw_writel(temp|OMAP_SPI_EN_CHANNEL, chx_ctrl_reg);
	temp = __raw_readl(chx_ctrl_reg);
	
	temp = OMAP_SPI_FORCE|OMAP_SPI_DPE0|OMAP_SPI_WL_8BITS|OMAP_SPI_EPOL_LOW|OMAP_SPI_CLKD_6M;
	__raw_writel(temp, chx_conf_reg);

	while((__raw_readl(chx_stat_reg)&OMAP_SPI_TXS) == 0);
	
	local_irq_save(flags);
	
	if(in_len)
	{
		__raw_writel(*in_buf++, tx_reg);
		in_len--;

		while((__raw_readl(chx_stat_reg)&OMAP_SPI_TXS) == 0);

		while((__raw_readl(chx_stat_reg)&OMAP_SPI_RXS) == 0);
		temp = __raw_readl(rx_reg);

	}
	else
		return -EINVAL;


	while(in_len)
	{
		__raw_writel(*in_buf++, tx_reg);
		in_len--;
		while((__raw_readl(chx_stat_reg)&OMAP_SPI_TXS) == 0);


		while((__raw_readl(chx_stat_reg)&OMAP_SPI_RXS) == 0);
		temp = __raw_readl(rx_reg);
		*out_buf++ = temp;

	}


	__raw_writel(0, tx_reg);
	while((__raw_readl(chx_stat_reg)&OMAP_SPI_TXS) == 0);

	
	while((__raw_readl(chx_stat_reg)&OMAP_SPI_RXS) == 0);
	temp = __raw_readl(rx_reg);
	*out_buf++ = temp;

	local_irq_restore(flags);

	temp = __raw_readl(chx_conf_reg);
	temp &= ~OMAP_SPI_FORCE;
	__raw_writel(temp, chx_conf_reg);

	temp = __raw_readl(chx_ctrl_reg);
	temp &= ~OMAP_SPI_EN_CHANNEL;
	__raw_writel(temp, chx_ctrl_reg);

//	local_irq_restore(flags);

	return 0;
}

int spi_init(void)
{
	int ret;
	u32 *reg;


	spi_base_addr = ioremap(OMAP_MCSPI3_BASE, 4096);
	if(!spi_base_addr)
	{
		ret = -EFAULT;
		ERROR_TO("fail to remap spi4 register address\n");
		goto err;
	}

	reg = ioremap(0x48004A00, 1);
	*reg |= 1<<20;		// enable functional clock
	reg = ioremap(0x48004A10, 1);
	*reg |= 1<<20;		// enable interface clock


	__raw_writel(OMAP_SPI_SOFTRESET, sys_config_reg);
	while(!(__raw_readl(sys_status_reg) & OMAP_SPI_RESETDONE));

	
	__raw_writel(OMAP_SPI_SINGLE_CHANNEL_EN, modul_ctrl_reg);

	__raw_writel(OMAP_SPI_CLOCKACTIVITY_BOTH_ON|OMAP_SPI_SIDLEMODE_KEEP_ON, sys_config_reg);


	return 0;

err:
	
	return ret;
}

void spi_uninit(void)
{
	u32 *reg;


	reg = ioremap(0x48004A00, 1);
	*reg &= ~(1<<20);		// disable functional clock
	reg = ioremap(0x48004A10, 1);
	*reg &= ~(1<<20);

	iounmap(spi_base_addr);

}





