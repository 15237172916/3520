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

void it6801_main(void);


#endif