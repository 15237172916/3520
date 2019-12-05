///*****************************************
//   @file   <HDMIRX.h>
//   @author limingqiang@zhiyang-tech.com
//   @date   2018/12/10
//   @fileversion: HDMIRX_V2.00
//******************************************/
#ifndef _HDMIRX_H_
#define _HDMIRX_H_

#define HDMIRX_DEV_NAME "HDMIRX"
#define HDMIRX_DEV_CHN1_NAME "HDMIRX_CHN1"
#define HDMIRX_DEV_CHN2_NAME "HDMIRX_CHN2"
#define HDMIRX_DEV_CHN3_NAME "HDMIRX_CHN3"
#define HDMIRX_DEV_CHN4_NAME "HDMIRX_CHN4"
#define HDMIRX_DEV_CHN5_NAME "HDMIRX_CHN5"
#define HDMIRX_DEV_CHN6_NAME "HDMIRX_CHN6"
#define HDMIRX_DEV_CHN7_NAME "HDMIRX_CHN7"
#define HDMIRX_DEV_CHN8_NAME "HDMIRX_CHN8"


#define HDMIRX_DEVICE "/dev/HDMIRX"
#define HDMIRX_DEVICE_CHN1 "/dev/HDMIRX_CHN1"
#define HDMIRX_DEVICE_CHN2 "/dev/HDMIRX_CHN2"
#define HDMIRX_DEVICE_CHN3 "/dev/HDMIRX_CHN3"
#define HDMIRX_DEVICE_CHN4 "/dev/HDMIRX_CHN4"
#define HDMIRX_DEVICE_CHN5 "/dev/HDMIRX_CHN5"
#define HDMIRX_DEVICE_CHN6 "/dev/HDMIRX_CHN6"
#define HDMIRX_DEVICE_CHN7 "/dev/HDMIRX_CHN7"
#define HDMIRX_DEVICE_CHN8 "/dev/HDMIRX_CHN8"


#define IOC_GET_HDMIRX_INFO		0x10

#define IOC_SET_AUDIO_MUTE		0x20
#define IOC_SET_AUDIO_ON		0x21
#define IOC_CHANGE_AUDIO_WIDTH	0x22
#define IOC_SET_AUDIO_OFF		0x23
#define IOC_SEL_AUDIO_CHN		0x24


typedef enum ZY_HDMIRX_SCAN_MODE_E
{
    ZY_HDMIRX_SCAN_PROGRESSIVE	= 0,
    ZY_HDMIRX_SCAN_INTERLACED	= 1,
	ZY_HDMIRX_SCAN_UNKNOWN		= 2,
} HDMIRX_SCAN_MODE_E;

typedef enum ZY_HDMIRX_SAMPLE_RATE_E
{
	ZY_HDMI_SAMPLE_RATE_UNKNOWN	= 0,	 /* unknown sample rate*/
    ZY_HDMI_SAMPLE_RATE_8000   = 8000,	 /* 8K samplerate*/
    ZY_HDMI_SAMPLE_RATE_12000  = 12000,	 /* 12K samplerate*/
    ZY_HDMI_SAMPLE_RATE_11025  = 11025,	 /* 11.025K samplerate*/
    ZY_HDMI_SAMPLE_RATE_16000  = 16000,	 /* 16K samplerate*/
    ZY_HDMI_SAMPLE_RATE_22050  = 22050,	 /* 22.050K samplerate*/
    ZY_HDMI_SAMPLE_RATE_24000  = 24000,	 /* 24K samplerate*/
    ZY_HDMI_SAMPLE_RATE_32000  = 32000,	 /* 32K samplerate*/
    ZY_HDMI_SAMPLE_RATE_44100  = 44100,	 /* 44.1K samplerate*/
    ZY_HDMI_SAMPLE_RATE_48000  = 48000,	 /* 48K samplerate*/
    ZY_HDMI_SAMPLE_RATE_64000  = 64000,	 /* 64K samplerate*/
	ZY_HDMI_SAMPLE_RATE_88200	= 88200, /* 88.2K samplerate*/
    ZY_HDMI_SAMPLE_RATE_96000  = 96000,	 /* 96K samplerate*/
    ZY_HDMI_SAMPLE_RATE_176400  = 176400, /* 176.4K samplerate*/
    ZY_HDMI_SAMPLE_RATE_192000  = 192000, /* 192K samplerate*/
    ZY_HDMI_SAMPLE_RATE_BUTT
} HDMIRX_SAMPLE_RATE_E;

typedef enum ZY_HDMIRX_BIT_WIDTH_E
{
	ZY_HDMI_BIT_WIDTH_UNKNOWN = 0, 	/* unknown bit width*/
    ZY_HDMI_BIT_WIDTH_8   = 8,   /* 8bit width */
    ZY_HDMI_BIT_WIDTH_16  = 16,   /* 16bit width*/
	ZY_HDMI_BIT_WIDTH_17  = 17,   /* 17bit width*/
	ZY_HDMI_BIT_WIDTH_18  = 18,   /* 18bit width*/
    ZY_HDMI_BIT_WIDTH_19  = 19,   /* 19bit width*/
    ZY_HDMI_BIT_WIDTH_20  = 20,   /* 20bit width*/
    ZY_HDMI_BIT_WIDTH_21  = 21,   /* 21bit width*/
    ZY_HDMI_BIT_WIDTH_22  = 22,   /* 22bit width*/
    ZY_HDMI_BIT_WIDTH_23  = 23,   /* 23bit width*/
    ZY_HDMI_BIT_WIDTH_24  = 24,   /* 24bit width*/
	ZY_HDMI_BIT_WIDTH_25  = 25,   /* 25bit width*/
	ZY_HDMI_BIT_WIDTH_26  = 26,   /* 26bit width*/
	ZY_HDMI_BIT_WIDTH_27  = 27,   /* 27bit width*/
	ZY_HDMI_BIT_WIDTH_28  = 28,   /* 28bit width*/
	ZY_HDMI_BIT_WIDTH_29  = 29,   /* 29bit width*/
	ZY_HDMI_BIT_WIDTH_30  = 30,   /* 30bit width*/
	ZY_HDMI_BIT_WIDTH_31  = 31,   /* 31bit width*/
    ZY_HDMI_BIT_WIDTH_32  = 32,   /* 32bit width*/
    ZY_HDMI_BIT_WIDTH_BUTT
} HDMIRX_BIT_WIDTH_E;

typedef enum ZY_AUDIO_STATUS_E
{
	ZY_AUDIO_STATUS_ERROR = 0,
    ZY_AUDIO_STATUS_ON   = 1,   
    ZY_AUDIO_STATUS_OFF  = 2,   
    ZY_AUDIO_STATUS_MUTE  = 3,  
} HDMIRX_AUDIO_STATUS_E;

typedef struct ZY_HDMIRX_INFO
{
	unsigned char	Chip_ID;
	unsigned char	Channel_ID;
	unsigned long	PCLK;
	unsigned short	H_Total; 
	unsigned short	H_Active;
	unsigned short	H_Syncwidth;
	unsigned short	H_Bkporch;
	unsigned char	H_Syncpol;
	unsigned short	V_Total;
	unsigned short	V_Active;
	unsigned short	V_Syncwidth;
	unsigned short	V_Bkporch;
	unsigned short	V_Syncpol;
	unsigned short	Color;
	unsigned char	FrmRate;
	HDMIRX_SCAN_MODE_E	ScanMode;
	HDMIRX_SAMPLE_RATE_E	AudioSampleRate_IN;
	HDMIRX_BIT_WIDTH_E		AudioBitWidth_IN;	
	HDMIRX_SAMPLE_RATE_E	AudioSampleRate_OUT;
	HDMIRX_BIT_WIDTH_E		AudioBitWidth_OUT;	
	HDMIRX_AUDIO_STATUS_E 	AudioStatus;
}HDMIRX_INFO,*pHDMIRX_INFO;

typedef struct _STC_HDMIRX_INFO_
{
	unsigned char Chip_ID;
	unsigned char Channel_ID;
	unsigned char PCLK_byte0;
	unsigned char PCLK_byte1;
	unsigned char PCLK_byte2;
	unsigned char PCLK_byte3;
	unsigned char H_Total_LSB; 
	unsigned char H_Total_MSB; 
	unsigned char H_Active_LSB;
	unsigned char H_Active_MSB;
	unsigned char H_Syncwidth;
	unsigned char H_Bkporch_LSB;
	unsigned char H_Bkporch_MSB;
	unsigned char H_Syncpol;
	unsigned char V_Total_LSB;
	unsigned char V_Total_MSB;
	unsigned char V_Active_LSB;
	unsigned char V_Active_MSB;
	unsigned char V_Syncwidth;
	unsigned char V_Bkporch;
	unsigned char V_Syncpol;
	unsigned char ScanMode;
	unsigned char AudioSampleRate_IN_LSB;
	unsigned char AudioSampleRate_IN_MSB;
	unsigned char AudioBitWidth_IN;	
	unsigned char AudioSampleRate_OUT_LSB;
	unsigned char AudioSampleRate_OUT_MSB;
	unsigned char AudioBitWidth_OUT;
	unsigned char AudioStatus;
	unsigned char check_sum;
}STC_HDMIRX_INFO,*pSTC_HDMIRX_INFO;

#define	HDMIRX_VERSION	("\r\n------ ZY_HDMIRX_VERSION_V2.00 !!! ------\r\n")

#endif 






















