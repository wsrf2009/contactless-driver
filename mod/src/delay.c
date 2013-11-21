

#include <linux/delay.h>

#include "common.h"
#include "pn512.h"



void Delay1us(u32 delay)
{
    udelay(delay);
}

void Delay256us(u8 delay)
{
    volatile u8 i;


    while (delay--)
    {
        Delay1us(200);
        for(i = 0; i < 134; i++);
    }
}

void Delay256P2us(u8 delay)
{
    u8 i;

    
    for(i = 0; i < delay; i++)
    {
        Delay256us(0xFF);
        Delay256us(0x01);
    }
}

void Delay256P3us(u8 delay)
{
    u8 i;
    
    
    for(i = 0; i < delay; i++)
    {
        Delay256P2us(0xFF);
        Delay256P2us(0x01);
    }
}


void Delay1ms(u32 delay)
{
    mdelay(delay);
}

void Delay1s(u8 delay)
{
    u8 i;


    for(i = 0; i < delay; i++)
    {
        mdelay(1000);
    }
}
void SetTimer100us(u16 timeOut)
{
    set_pn512_timer(timeOut);
}


