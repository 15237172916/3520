#ifndef _IT6801_H_
#define _IT6801_H_

#define IT6802A0_HDMI_ADDR 0x94	//Hardware Fixed I2C address of IT6802 HDMI
#define IT6802B0_HDMI_ADDR 0x90	//Hardware Fixed I2C address of IT6802 HDMI
#define MHL_ADDR 0xE0	//Software programmable I2C address of IT6802 MHL
#define EDID_ADDR 0xA8	//Software programmable I2C address of IT6802 EDID RAM
#define CEC_ADDR 0xC8	//Software programmable I2C address of IT6802 CEC


#define DP_ADDR 0x90
#define ADC_ADDR 0x90



#define HDMI_DEV  0
#define DP_DEV	  0

#define RXDEV           0
#define MAXRXDEV        1

#if 1
typedef enum IT6801_SCAN_MODE_E
{
    IT6801_SCAN_PROGRESSIVE	= 0,
    IT6801_SCAN_INTERLACED	= 1,
	IT6801_SCAN_UNKNOWN		= 2,
} IT6801_SCAN_MODE_E;


typedef enum IT6801_SAMPLE_RATE_E
{
	HDMI_SAMPLE_RATE_UNKNOWN	= 0,	 /* unknown sample rate*/
    HDMI_SAMPLE_RATE_8000   = 8000,	 /* 8K samplerate*/
    HDMI_SAMPLE_RATE_12000  = 12000,	 /* 12K samplerate*/
    HDMI_SAMPLE_RATE_11025  = 11025,	 /* 11.025K samplerate*/
    HDMI_SAMPLE_RATE_16000  = 16000,	 /* 16K samplerate*/
    HDMI_SAMPLE_RATE_22050  = 22050,	 /* 22.050K samplerate*/
    HDMI_SAMPLE_RATE_24000  = 24000,	 /* 24K samplerate*/
    HDMI_SAMPLE_RATE_32000  = 32000,	 /* 32K samplerate*/
    HDMI_SAMPLE_RATE_44100  = 44100,	 /* 44.1K samplerate*/
    HDMI_SAMPLE_RATE_48000  = 48000,	 /* 48K samplerate*/
    HDMI_SAMPLE_RATE_64000  = 64000,	 /* 64K samplerate*/
	HDMI_SAMPLE_RATE_88200	= 88200, /* 88.2K samplerate*/
    HDMI_SAMPLE_RATE_96000  = 96000,	 /* 96K samplerate*/
    HDMI_SAMPLE_RATE_176400  = 176400, /* 176.4K samplerate*/
    HDMI_SAMPLE_RATE_192000  = 192000, /* 192K samplerate*/
    HDMI_SAMPLE_RATE_BUTT
} IT6801_SAMPLE_RATE_E;

typedef enum IT6801_BIT_WIDTH_E
{
	HDMI_BIT_WIDTH_UNKNOWN = 0, 	/* unknown bit width*/
    HDMI_BIT_WIDTH_8   = 8,   /* 8bit width */
    HDMI_BIT_WIDTH_16  = 16,   /* 16bit width*/
	HDMI_BIT_WIDTH_17  = 17,   /* 17bit width*/
	HDMI_BIT_WIDTH_18  = 18,   /* 18bit width*/
    HDMI_BIT_WIDTH_19  = 19,   /* 19bit width*/
    HDMI_BIT_WIDTH_20  = 20,   /* 20bit width*/
    HDMI_BIT_WIDTH_21  = 21,   /* 21bit width*/
    HDMI_BIT_WIDTH_22  = 22,   /* 22bit width*/
    HDMI_BIT_WIDTH_23  = 23,   /* 23bit width*/
    HDMI_BIT_WIDTH_24  = 24,   /* 24bit width*/
	HDMI_BIT_WIDTH_25  = 25,   /* 25bit width*/
	HDMI_BIT_WIDTH_26  = 26,   /* 26bit width*/
	HDMI_BIT_WIDTH_27  = 27,   /* 27bit width*/
	HDMI_BIT_WIDTH_28  = 28,   /* 28bit width*/
	HDMI_BIT_WIDTH_29  = 29,   /* 29bit width*/
	HDMI_BIT_WIDTH_30  = 30,   /* 30bit width*/
	HDMI_BIT_WIDTH_31  = 31,   /* 31bit width*/
    HDMI_BIT_WIDTH_32  = 32,   /* 32bit width*/
    HDMI_BIT_WIDTH_BUTT
} IT6801_BIT_WIDTH_E;

typedef enum AUDIO_STATUS_E
{
	AUDIO_STATUS_ERROR = 0,
    AUDIO_STATUS_ON   = 1,   
    AUDIO_STATUS_OFF  = 2,   
    AUDIO_STATUS_MUTE  = 3,  
} IT6801_AUDIO_STATUS_E;

typedef struct IT6801_INFO
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
	IT6801_SCAN_MODE_E	ScanMode;
	IT6801_SAMPLE_RATE_E	AudioSampleRate_IN;
	IT6801_BIT_WIDTH_E		AudioBitWidth_IN;	
	IT6801_SAMPLE_RATE_E	AudioSampleRate_OUT;
	IT6801_BIT_WIDTH_E		AudioBitWidth_OUT;	
	IT6801_AUDIO_STATUS_E 	AudioStatus;
}IT6801_INFO_S,*pIT6801_INFO_S;
#endif

void GetIt6801Info(pIT6801_INFO_S IT6801Info);
void it6801_main(void);


#endif