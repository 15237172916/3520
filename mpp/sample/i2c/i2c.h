#ifndef _I2C_H_
#define _I2C_H_
#include "typedef.h"


#define STRFMT_ADDR32    "%#010lX"
#define STRFMT_ADDR32_2  "0x%08lX
#define IN
#define OUT


typedef unsigned long HI_RET;
typedef unsigned long U32;

//typedef static LOCALFUNC;
//typedef unsigned char UCHAR;
//typedef char CHAR;


BOOL i2c_write_byte( BYTE address,BYTE offset,BYTE byteno,BYTE *p_data,BYTE device );
BOOL i2c_read_byte( BYTE address,BYTE offset,BYTE byteno,BYTE *p_data,BYTE device );

#ifdef Enable_IT6802_CEC
#include "debug.h"
BYTE IT6802_CEC_ReadI2C_Byte(BYTE RegAddr);
SYS_STATUS IT6802_CEC_WriteI2C_Byte(BYTE offset,BYTE buffer );
#endif

#endif

