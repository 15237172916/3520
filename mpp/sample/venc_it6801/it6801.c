#include <stdio.h>
#include <string.h>

#include "config.h"
#include "it6801.h"
#include "i2c.h"
#include "Utility.h"
#include "stdio.h"
#include "version.h"
#include "Mhlrx.h"
#include "Mhlrx_reg.h"

char code Firmware_date[] = __DATE__;
char code Firmware_time[] = __TIME__;


void InitMessage()
{
	init_printf();
	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	printf("           %s \n",VERSION_STRING);
	printf("           %s \n",Firmware_date);
	printf("           %s \n",Firmware_time);
#if defined(_IT6802_)
	printf("           IT6802 \n");
#elif defined(_IT6803_)
	printf("           IT6803 \n");
#else
	printf("           IT6801 \n");
#endif
	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");

}


void it6801_main(void)
{
    InitMessage();
    it6802HPDCtrl(1,0);	// HDMI port , set HPD = 0
	IT6802_fsm_init();
	/*----------------------------------*/
	//Hold_Pin=1;
	//while(!Hold_Pin)
	{
		// HotPlug(1);	//clear port 1 HPD=0 for Enable EDID update
		//xxxxx 2013-0801
		it6802HPDCtrl(1,1);	// HDMI port , set HPD = 1
		//xxxxx

	}
	
    while (1)
    {
        //printf("it6801 is running \n");
        usleep(150000);
        IT6802_fsm();
    }
}



