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

extern struct it6802_dev_data it6802DEV;
extern VTiming CurVTiming;

#define AUDFS_22p05KHz  4
#define AUDFS_44p1KHz 0
#define AUDFS_88p2KHz 8
#define AUDFS_176p4KHz    12

#define AUDFS_24KHz  6
#define AUDFS_48KHz  2
#define AUDFS_96KHz  10
#define AUDFS_192KHz 14

#define AUDFS_32KHz  3
#define AUDFS_OTHER    1

static int _GET_IT6801_INFO(pIT6801_INFO_S pIT6801Info, pVTiming_S pVideoInfo, pIT6801_DEV_S pDevInfo)
{
	printf("---------pVideoInfo->VActive: %d -----------\n", pVideoInfo->VActive);
   	printf("*********pVideoInfo->HActive: %d ***********\n", pVideoInfo->HActive);
	//Video
	do 
	{
		pIT6801Info->V_Active = pVideoInfo->VActive;
		pIT6801Info->H_Active = pVideoInfo->HActive;
		
		//sleep(1);
	}while ((0 == pIT6801Info->V_Active) || (0 == pIT6801Info->H_Active));
	
	//Audio
	switch (pDevInfo->m_RxAudioCaps.SampleFreq)
	{
		case AUDFS_22p05KHz:
			pIT6801Info->AudioBitWidth_IN = HDMI_SAMPLE_RATE_22050;
			break;
		case AUDFS_44p1KHz:
			pIT6801Info->AudioBitWidth_IN = HDMI_SAMPLE_RATE_44100;
			break; 
		case AUDFS_88p2KHz:
			pIT6801Info->AudioBitWidth_IN = HDMI_SAMPLE_RATE_88200;
			break; 
		case AUDFS_176p4KHz:
			pIT6801Info->AudioBitWidth_IN = HDMI_SAMPLE_RATE_176400;
			break; 
		case AUDFS_24KHz:
			pIT6801Info->AudioBitWidth_IN = HDMI_SAMPLE_RATE_24000;
			break; 
		case AUDFS_48KHz:
			pIT6801Info->AudioBitWidth_IN = HDMI_SAMPLE_RATE_48000;
			break; 
		case AUDFS_96KHz:
			pIT6801Info->AudioBitWidth_IN = HDMI_SAMPLE_RATE_96000;
			break; 
		case AUDFS_192KHz:
			pIT6801Info->AudioBitWidth_IN = HDMI_SAMPLE_RATE_192000;
			break;
		case AUDFS_32KHz:
			pIT6801Info->AudioBitWidth_IN = HDMI_SAMPLE_RATE_32000;
			break; 
		case AUDFS_OTHER:
			pIT6801Info->AudioBitWidth_IN = HDMI_SAMPLE_RATE_44100;
			break; 	
		default:
			pIT6801Info->AudioBitWidth_IN = HDMI_SAMPLE_RATE_44100;
			break; 
	}
	//printf("/n/n===============pIT6801Info->AudioBitWidth_IN: %d \n\n", pIT6801Info->AudioBitWidth_IN);
	return 0;
}

void GetIt6801Info(pIT6801_INFO_S pIT6801Info)
{
	_GET_IT6801_INFO(pIT6801Info, &CurVTiming, &it6802DEV);
	printf("---------CurVTiming->V_Active: %d -----------\n", CurVTiming.VActive);
    printf("*********CurVTiming->H_Active: %d ***********\n", CurVTiming.HActive);
	printf("---------pIT6801Info->V_Active: %d -----------\n", pIT6801Info->V_Active);
    printf("*********pIT6801Info->H_Active: %d ***********\n", pIT6801Info->H_Active);
}
void main(void)
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
		//sleep(1);
		//PRINTF_IT8601_INFO(CurVTiming);

    }
}




