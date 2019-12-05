///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <Utility.c>
//   @author Max.Kao@ite.com.tw
//   @date   2014/09/25
//   @fileversion: ITE_MHLRX_SAMPLE_V1.11
//******************************************/
#include <stdio.h>

#include "config.h"
#include "it6801.h"
#include "i2c.h"
#include "Utility.h"


/*
idata USHORT ucTickCount=0;
idata USHORT loopTicCount = 0;
idata USHORT prevTickCount;
idata USHORT MsdelayCnt =0;
*/

void init_printf(void)
{
    printf("init_printf is OK \n");
}

/*
void system_tick(void) interrupt 3 {

	TR1=0; // temporarily stop timer 0

	TH1=Tick1ms / 256;
	TL1=Tick1ms % 256;

	ucTickCount++;

	TR1=1; // restart the timer
}
*/
/*
USHORT getloopTicCount()
{
    idata USHORT loopms;

    if(loopTicCount>ucTickCount)
    {
        loopms =  (0xffff-(loopTicCount-ucTickCount));
    }
    else
    {
        loopms =  (ucTickCount-loopTicCount);
    }
    loopTicCount = ucTickCount;
//    MHLRX_DEBUG_PRINTF(" loop ms  = %u \n",loopms));
    return  loopms;

}
*/

/*

USHORT CalTimer(USHORT SetupCnt)
{
    if(SetupCnt>ucTickCount)
    {
        return (0xffff-(SetupCnt-ucTickCount));
    }
    else
    {
        return (ucTickCount-SetupCnt);
    }
}

int TimeOutCheck(USHORT timer, USHORT x)
{
    if(CalTimer(timer)>=x)
    {
        return TRUE ;
    }
    return FALSE ;
}


BOOL IsTimeOut(USHORT x)
{
    if(CalTimer(prevTickCount) >= x )
    {
        prevTickCount = ucTickCount ;
        return TRUE ;
    }
    return FALSE ;
}

USHORT GetCurrentVirtualTime()
{
    return ucTickCount ;
}
*/

void delay1ms(USHORT ms)
{
   int i;
   for (i=0; i<ms; i++)
   {
       usleep(1000);
   }
}

