/*
* Name: Delay source file
* Date: 2012/12/04
* Author: Alex Wang
* Version: 1.0
*/


#include <linux/delay.h>

#include "common.h"
#include "pn512.h"



void Delay1us(UINT32 delay)
{
    udelay(delay);
}

void Delay256us(UINT8 delay)
{
    volatile UINT8 i;


    while (delay--)
    {
        Delay1us(200);
        for(i = 0; i < 134; i++);
    }
}

void Delay256P2us(UINT8 delay)
{
    UINT8 i;

    
    for(i = 0; i < delay; i++)
    {
        Delay256us(0xFF);
        Delay256us(0x01);
    }
}

void Delay256P3us(UINT8 delay)
{
    UINT8 i;
    
    
    for(i = 0; i < delay; i++)
    {
        Delay256P2us(0xFF);
        Delay256P2us(0x01);
    }
}


void Delay1ms(UINT32 delay)
{
    mdelay(delay);
}

void Delay1s(UINT8 delay)
{
    UINT8 i;


    for(i = 0; i < delay; i++)
    {
        mdelay(1000);
    }
}
void SetTimer100us(UINT16 timeOut)
{
    SetPN512Timer(timeOut);
}


