


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/slab.h> 
#include <linux/err.h>     //IS_ERR()
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/poll.h>
#include <linux/sched.h>   //wake_up_process()


#include "common.h"
#include "picc.h"
#include "debug.h"


struct pcd_param
{
    u8 *p_iBuf;
    u8 *p_oBuf;
    u32  iDataLen;
    u32  oDataLen;
    u32 statusCode;
};


#define  Card_PowerOn     0x01
#define  Card_PowerOff    0x02
#define  Card_XfrAPDU     0x03



void run_picc_poll(struct work_struct *work);
DECLARE_DELAYED_WORK(card_Poll, run_picc_poll);


struct pcd_common
{
	struct pcd_device		pcd;
	struct picc_device		picc;

	struct semaphore	mutex;
	u8	sem_inc;
	struct workqueue_struct 	*polling;

	int 		(*slot_changed_notify)(void *, u8);
	void		*private_data;

};

struct pcd_common		*common = NULL;



#include "picc.c"
#include "ccid_picc.c"


static long pcd_ioctl(struct file *filp, u32 cmd, unsigned long arg) 
{
	struct pcd_common *common = filp->private_data;
    u8 pcd_cmd = (cmd >> 4) & 0xFF;
    struct pcd_param KerParam;
    struct pcd_param *UsrParam = (struct pcd_param *)arg;
    u8 *p_iData;
    u8 *p_oData;
    u32  ret = 0;
    u8 level = 0;
    

    if(down_interruptible(&common->mutex))    // acquire the semaphore
    {
        ret = -ERESTARTSYS;
        goto err;
    }

    if((!UsrParam) || (copy_from_user(&KerParam, UsrParam, sizeof(KerParam))))
    {
        ret = -EFAULT;          // bad address
        goto err;
    }

    switch(pcd_cmd)
    {		
        case Card_PowerOn:
        {
            if(!KerParam.p_oBuf) 
			{
                ret = -EFAULT;       // bad address
                goto err;
            }
			
            p_oData = kmalloc(KerParam.oDataLen, GFP_KERNEL);
			
            if(!p_oData) 
			{
                ret = -EFAULT;       // bad address
                goto err;                
            }

			if((ret = picc_power_on(&common->picc, p_oData, &KerParam.oDataLen)) != 0)	
				goto err2;

			if(copy_to_user(KerParam.p_oBuf, p_oData, KerParam.oDataLen)) 
			{
                ret = -EFAULT;       // bad address
                goto err2;
            }

			if(copy_to_user(&UsrParam->oDataLen, &KerParam.oDataLen, sizeof(KerParam.oDataLen)))
			{
                ret = -EFAULT;       // bad address
                goto err2;
            }
			
			kfree(p_oData);
			
            break; 
        }

        case Card_PowerOff:
        {
            picc_power_off(&common->picc);
			ret = 0; 
            break;
        }

        case Card_XfrAPDU:
        {
            if((KerParam.iDataLen <= 0) || (KerParam.oDataLen <= 0) || (!KerParam.p_iBuf) || (!KerParam.p_oBuf)) 
			{
                ret = -EFAULT;       // bad address
                goto err;
            }
			
            p_iData = kmalloc(KerParam.iDataLen, GFP_KERNEL);
            p_oData = kmalloc(KerParam.oDataLen, GFP_KERNEL);
			
            if((!p_iData) || (!p_oData) || (copy_from_user(p_iData, KerParam.p_iBuf, KerParam.iDataLen)))
            {
                ret = -EFAULT;       // bad address
                goto err1;
            }
			
            if((ret = picc_command_exchange(&common->picc, p_iData, KerParam.iDataLen, p_oData, &KerParam.oDataLen, &level)) != 0)	
				goto err;

            if((KerParam.oDataLen <= 0) || (copy_to_user(KerParam.p_oBuf, p_oData, (unsigned long)KerParam.oDataLen)))
            {
                ret = -EFAULT;       // bad address
                goto err1;
            }
			
            if(copy_to_user(&UsrParam->oDataLen, &KerParam.oDataLen, sizeof(KerParam.oDataLen)))
            {
                ret = -EFAULT;       // bad address
                goto err1;
            }

			kfree(p_iData);
			kfree(p_oData);
			
            break;
        }

        default:
            break;
    }

	up(&common->mutex); 
	return(0);


err1:
	if(p_iData)		kfree(p_iData);
err2:
	if(p_oData)		kfree(p_oData);
err:
    up(&common->mutex);                    // release the semaphore
    UsrParam->statusCode = ret;
    return(ret);
}


static int pcd_open(struct inode *inode, struct file *filp)
{
    if(common->sem_inc > 0)    return(-ERESTARTSYS);
    common->sem_inc++;

    filp->private_data = common;

    return(0);
}
static int pcd_release(struct inode *inode, struct file *filp)
{
	struct pcd_common *common = filp->private_data;

	
	common->sem_inc--;
	
    return(0);
}

extern int picc_interrput_in(u8 slot_status);

void run_picc_poll(struct work_struct *work)
{


    if(down_trylock(&common->mutex))    
    {
        goto done;
    }

    if(BITISSET(common->pcd.flags_polling, AUTO_POLLING) && BITISSET(common->pcd.flags_polling, POLLING_CARD_ENABLE))
    {	
        picc_polling_tags(&common->picc);

		if(BITISSET(common->picc.status, SLOT_CHANGE))
		{
			if(!picc_interrput_in(common->picc.status & PRESENT))
				CLEAR_BIT(common->picc.status, SLOT_CHANGE);
		}
    }



    up(&common->mutex);

done:
	
	queue_delayed_work(common->polling, &card_Poll, (common->pcd.poll_interval * HZ) / 1000);


}


static struct file_operations pcd_fops=
{
    .owner = THIS_MODULE,
    .open = pcd_open,
    .unlocked_ioctl = pcd_ioctl,
    .release = pcd_release
};

static struct miscdevice pcd_misc=
{
    .minor = 221,
    .name = "pcd",
    .fops = &pcd_fops
};

static int pcd_init(void)
{
	int ret;

	
    TRACE_TO("enter %s\n", __func__);
	
	common = kzalloc(sizeof *common, GFP_KERNEL);
	if (!common)
	{
		ret = -ENOMEM;
		goto err1;
	}

    sema_init(&common->mutex, 0);    // initial a semaphore, and lock it

	ret = picc_init(common);
	if(ret)
		goto err2;

	ret = misc_register(&pcd_misc);
    if(ret)
    {
        ERROR_TO("fail to register device\n");
        goto err3;
    }

    common->polling = create_singlethread_workqueue("polling picc");
    if(!common->polling)
    {
        ERROR_TO("can't create work queue 'pcdPoll'\n");
		ret = -EFAULT;
        goto err4;
    }
    run_picc_poll(0);

    up(&common->mutex);    

    TRACE_TO("exit %s\n", __func__);
	
    return (0);

err4:
    misc_deregister(&pcd_misc);
err3:
	picc_uninit();
err2:	
    up(&common->mutex);
	kfree(common);
err1:	
	
	TRACE_TO("exit %s\n", __func__);
    return ret;
}

static void pcd_exit(void)
{
	TRACE_TO("enter %s\n", __func__);
	
    if (down_interruptible(&common->mutex)) 
    {
        return;
    }
   
    if(!cancel_delayed_work(&card_Poll)) 
    {
        flush_workqueue(common->polling);
    }
    destroy_workqueue(common->polling);

    picc_uninit();
	
    misc_deregister(&pcd_misc);
	
    up(&common->mutex);

	kfree(common);
    
	TRACE_TO("exit %s\n", __func__);
	
    return;
}

module_init(pcd_init);
module_exit(pcd_exit);
MODULE_DESCRIPTION("Contactless Card Driver");
MODULE_AUTHOR("Alex Wang");
MODULE_LICENSE("GPL");


