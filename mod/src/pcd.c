/*
* Name: PCD source file
* Date: 2012/12/04
* Author: Alex Wang
* Version: 1.0
*/


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/semaphore.h>
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
//#include <linux/sched.h>   //wake_up_process()
//#include <linux/kthread.h> //kthread_create(),kthread_run()


#include "common.h"
#include "picc.h"
#include "debug.h"
#include "pn512.h"
#include "delay.h"

struct pcd_dev
{
    struct cdev cdev;
};

typedef struct
{
    UINT8 *p_iBuf;
    UINT8 *p_oBuf;
    UINT32  iDataLen;
    UINT32  oDataLen;
}PCD_PARAM;


#define  Card_PowerOn     0x01
#define  Card_PowerOff    0x02
#define  Card_XfrAPDU     0x03

//extern volatile bool pollReady;

static struct semaphore pcd_mutex;
static UINT8 sem_inc = 0;
struct workqueue_struct *pcdPoll = NULL;

void RunPiccPoll(struct work_struct *work);
DECLARE_DELAYED_WORK(cardPoll, RunPiccPoll);




static INT64 PCD_ioctl(struct file *filp, UINT32 cmd, UINT64 arg) 
{

//    UINT8 CardIdx = cmd & 0x0F;
    UINT8 IFDCMD = (cmd >> 4) & 0x0F;
    PCD_PARAM KerParam;
    PCD_PARAM *UsrParam = (PCD_PARAM *)arg;
    UINT8 *p_iData;
    UINT8 *p_oData;
    INT64  ret = 0;
    UINT8 level = 0;
    


    PrtMsg(DBGL4, "welcome to the function: %s, IFDCMD = %X\n", __FUNCTION__, IFDCMD);

//    CLEAR_BIT(pcd.fgPoll, BIT_POLLCARD);

    if(down_interruptible(&pcd_mutex))    // acquire the semaphore
    {
        ret = (-ERESTARTSYS);
        goto err;
    }
    PrtMsg(DBGL4, "11111111111111111111111\n");
    if((!UsrParam) || (copy_from_user(&KerParam, UsrParam, sizeof(KerParam))))
    {
        ret = (-EFAULT);          // bad address
        goto err;
    }

    switch(IFDCMD)
    {
        case Card_PowerOn:
        {
            if(!KerParam.p_oBuf)
            {
                ret = (-EFAULT);       // bad address
                goto err;
            }
            p_oData = kmalloc(KerParam.oDataLen, GFP_KERNEL);
            if(!p_oData)
            {
                ret = (-EFAULT);       // bad address
                goto err;                
            }
            if(PiccPowerON(p_oData, (UINT16*)&KerParam.oDataLen))
            {
                ret = (-ENXIO);        // device error
                goto err;
            }
            if(copy_to_user(KerParam.p_oBuf, p_oData, KerParam.oDataLen))
            {
                ret = (-EFAULT);       // bad address
                goto err;
            }
            if(copy_to_user(&UsrParam->oDataLen, &KerParam.oDataLen, sizeof(KerParam.oDataLen)))
            {
                ret = (-EFAULT);       // bad address
                goto err;
            }
            break; 
        }

        case Card_PowerOff:
        {
            PiccPowerOff();

            break;
        }

        case Card_XfrAPDU:
        {
            if((KerParam.iDataLen <= 0) || (KerParam.oDataLen <= 0) || (!KerParam.p_iBuf) || (!KerParam.p_oBuf))
            {
                ret = (-EFAULT);       // bad address
                goto err;
            }
            p_iData = kmalloc(KerParam.iDataLen, GFP_KERNEL);
            p_oData = kmalloc(KerParam.oDataLen, GFP_KERNEL);
            if((!p_iData) || (!p_oData) || (copy_from_user(p_iData, KerParam.p_iBuf, KerParam.iDataLen)))
            {
                ret = (-EFAULT);       // bad address
                goto err;
            }
            if(PiccXfrDataExchange(p_iData, KerParam.iDataLen, p_oData, (UINT16*)&KerParam.oDataLen, &level))
            {
                ret = (-ENXIO);        // device error
                goto err;
            }
            if((KerParam.oDataLen <= 0) || (copy_to_user(KerParam.p_oBuf, p_oData, (unsigned long)KerParam.oDataLen)))
            {
                ret = (-EFAULT);       // bad address
                goto err;
            }
            if(copy_to_user(&UsrParam->oDataLen, &KerParam.oDataLen, sizeof(KerParam.oDataLen)))
            {
                ret = (-EFAULT);       // bad address
                goto err;
            }

            if(p_iData)
            {
                kfree(p_iData);
            }
            if(p_oData)
            {
                kfree(p_oData);
            }
            break;
        }

        default:
            break;
    }

err:
    PrtMsg(DBGL4, "2222222222222222\n");
    up(&pcd_mutex);                    // release the semaphore
//    SET_BIT(pcd.fgPoll, BIT_POLLCARD);

    PrtMsg(DBGL4, "%s: exit\n", __FUNCTION__);

    return(ret);

}


static INT32 PCD_Open(struct inode *inode, struct file *filp)
{
    struct pcd_dev *dev;

    

    PrtMsg(DBGL1, "Welcome to entry to the function: %s\n", __FUNCTION__);
    if(sem_inc > 0)    return(-ERESTARTSYS);
    sem_inc++;

    dev = container_of(inode->i_cdev, struct pcd_dev, cdev);
    filp->private_data = dev;

    return(0);
}
static INT32 PCD_release(struct inode *inode, struct file *filp)
{
    sem_inc--;
    return(0);
}


void RunPiccPoll(struct work_struct *work)
{
    PrtMsg(DBGL3, "%s: start\n", __FUNCTION__);

    queue_delayed_work(pcdPoll, &cardPoll, (pcd.pollDelay * HZ) / 1000);

    if(down_trylock(&pcd_mutex))    
    {
        PrtMsg(DBGL3, "%s: can't got the semaphore 'pcd_mutex'\n", __FUNCTION__);
        return;
    }

    if(BITISSET(pcd.fgPoll, BIT_AUTOPOLL) && BITISSET(pcd.fgPoll, BIT_POLLCARD))
    {
        PiccPoll();
    }

    up(&pcd_mutex);

    PrtMsg(DBGL3, "%s: exit\n", __FUNCTION__);
}


static struct file_operations pcd_fops=
{
    .owner = THIS_MODULE,
    .open = PCD_Open,
    .unlocked_ioctl = PCD_ioctl,
    .release = PCD_release
};

static struct miscdevice pcd_misc=
{
    .minor = 221,
    .name = "pcd",
    .fops = &pcd_fops
};

static INT32 PCDInit(void)
{
    Pn512Init();
    PiccInit();
  
    return(0);
}

static INT32 PCDUninit(void)
{
    Pn512Uninit();

    return(0);
}

static INT32 PCD_Init(void)
{
    PrtMsg(DBGL1, "%s: start to install PCD driver!\n", __FUNCTION__);
    sema_init(&pcd_mutex, 0);    // initial a semaphore, and lock it
    if(PCDInit())
    {
        up(&pcd_mutex);
        PrtMsg(DBGL1, "%s: Fail to initial PCD device\n",__FUNCTION__);
        return (-1);
    }
    
    if(misc_register(&pcd_misc))
    {
        PrtMsg(DBGL1, "%s: Fail to register device\n",__FUNCTION__);
        goto err1;
    }



    pcdPoll = create_singlethread_workqueue("Polling PICC");
    if(pcdPoll == NULL)
    {
        PrtMsg(DBGL1, "%s: Can't create work queue 'pcdPoll'", __FUNCTION__);
        goto err2;
    }
    RunPiccPoll(0);

    up(&pcd_mutex);                  // release the semaphore
    PrtMsg(DBGL1, "%s: Success to install PCD device\n",__FUNCTION__);
    return (0);

err2:
    misc_deregister(&pcd_misc);
err1:
    PCDUninit();
    up(&pcd_mutex);
    return(-1);
}

static void PCD_Exit(void)
{
    PrtMsg(DBGL1, "%s: start to uninstall PCD driver", __FUNCTION__);

    if (down_interruptible(&pcd_mutex)) 
    {
        return;
    }
   
    if(!cancel_delayed_work(&cardPoll)) 
    {
        flush_workqueue(pcdPoll);
    }
    destroy_workqueue(pcdPoll);

    PCDUninit();
    misc_deregister(&pcd_misc);
    up(&pcd_mutex);
    
    PrtMsg(DBGL1, "%s: Success to uninstall PCD driver", __FUNCTION__);

    return;
}

module_init(PCD_Init);
module_exit(PCD_Exit);
MODULE_DESCRIPTION("Contactless Card Driver");
MODULE_AUTHOR("Alex Wang");
MODULE_LICENSE("GPL");


