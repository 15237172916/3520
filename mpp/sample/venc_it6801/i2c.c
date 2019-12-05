#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "sample_comm.h"
#include "i2c.h"
#include "typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/* /dev/i2c-X ioctl commands.  The ioctl's parameter is always an
 * unsigned long, except for:
 *      - I2C_FUNCS, takes pointer to an unsigned long
 *      - I2C_RDWR, takes pointer to struct i2c_rdwr_ioctl_data
 *      - I2C_SMBUS, takes pointer to struct i2c_smbus_ioctl_data
 */
#define I2C_RETRIES     0x0701  /* number of times a device address should
				   be polled when not acknowledging */
#define I2C_TIMEOUT     0x0702  /* set timeout in units of 10 ms */
/* NOTE: Slave address is 7 or 10 bits, but 10-bit addresses
 * are NOT supported! (due to code brokenness)
 */
#define I2C_SLAVE       0x0703  /* Use this slave address */
#define I2C_SLAVE_FORCE 0x0706  /* Use this slave address, even if it
				   is already in use by a driver! */
#define I2C_TENBIT      0x0704  /* 0 for 7 bit addrs, != 0 for 10 bit */

#define I2C_FUNCS       0x0705  /* Get the adapter functionality mask */

#define I2C_RDWR        0x0707  /* Combined R/W transfer (one STOP only) */

#define I2C_PEC         0x0708  /* != 0 to use PEC with SMBus */
#define I2C_SMBUS       0x0720  /* SMBus transfer */
#define I2C_16BIT_REG   0x0709  /* 16BIT REG WIDTH */
#define I2C_16BIT_DATA  0x070a  /* 16BIT DATA WIDTH */

#define READ_MIN_CNT 4
#define WRITE_MIN_CNT 5


static HI_RET i2c_read(unsigned int device_addr, unsigned int reg_addr, unsigned int reg_addr_end, unsigned char *p_data)
{
	int fd = -1;
	int ret;
	
    int value;
	//unsigned int reg_addr_end
	fd = open("/dev/gpioi2c", 0);
    if (fd<0)
    {
    	printf("Open gpioi2c dev error!\n");
    	return -1;
    }
   
    int cur_addr;
   
    if (reg_addr_end < reg_addr)
    {
        printf("end addr(0x%x) should bigger than start addr(0x%x)\n",
            reg_addr_end, reg_addr);
        return -1;
    }
    //printf("device_addr:0x%x; reg_addr_start:0x%x; reg_addr_end:0x%x.\n", 
    //    device_addr, reg_addr, reg_addr_end);
    for (cur_addr=reg_addr; cur_addr<reg_addr_end+1; cur_addr++)
    {
        value = ((device_addr&0xff)<<24) | ((cur_addr&0xff)<<16);   
        ret = ioctl(fd, GPIO_I2C_READ_BYTE, &value);            
        *p_data = value&0xff;
        //printf("GPIO_I2C_READ_BYTE, dev_addr=0x%x reg_addr=0x%x value=0x%x\n", device_addr, cur_addr, *p_data);
        p_data++;
    }
    
    close(fd);
    return 0;
}

static HI_RET i2c_write(unsigned int device_addr, unsigned int reg_addr, unsigned int reg_value)
{
	int fd = -1;
	int ret =0;
    int value;
	
	fd = open("/dev/gpioi2c", 0);
    if(fd<0)
    {
    	printf("Open gpioi2c error!\n");
    	return -1;
    }
    
    //printf("device_addr:0x%x; reg_addr:0x%x; reg_value:0x%x.\n", device_addr, reg_addr, reg_value);
    
    value = ((device_addr&0xff)<<24) | ((reg_addr&0xff)<<16) | (reg_value&0xffff);
    
    ret = ioctl(fd, GPIO_I2C_WRITE_BYTE, &value);
    if (ret < 0)
    {
        printf("i2c_write ioctl error \n");
        return -1;
    }

    close(fd);
    return 0;
}


/*
 * function : i2c data transport 
 * parameter:
 *      address: device address
 *      offset: registers address
 *      byteno: data length
 *      p_data: data head point
 *      device: I2C port number
*/
#if 1
BOOL i2c_write_byte( BYTE address,BYTE offset,BYTE byteno,BYTE *p_data,BYTE device )
{
    int ret, i;
	unsigned char Read;
	for (i=0; i<byteno; i++)
	{
		//printf("usage: i2c_write <i2c_num> <device_addr> <reg_addr> <value> <reg_width> <data_width>. sample:\n");
		
        //printf("address: 0x%x, offset: 0x%x, data: 0x%x", address, offset, *p_data);
        ret = i2c_write(address, offset, *p_data);
		if (ret < 0)
		{
            printf("i2c_write error \n");
			return 0;
		}
		i2c_read(address, offset, offset, &Read);
        //printf("     ---read : 0x%x \n", Read);
		p_data++;
		offset++;
	}
	
    return 1;
}

#endif 

#if 1
BOOL i2c_read_byte( BYTE address,BYTE offset,BYTE byteno,BYTE *p_data,BYTE device )
{
   // printf("usage: i2c_read <i2c_num> <device_addr> <reg_addr> <end_reg_addr>"
			//"<reg_width> <data_width> <reg_step>. sample: \n");
	
	unsigned long ret;
    unsigned int end_reg_addr = offset+(byteno-1); 
	
	ret = i2c_read( address, offset, end_reg_addr, p_data);
	if (ret < 0)
	{
        printf("i2c_read error \n");
		return 0;
	}
	
    return 1;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

///////////////////////////////////////////////////////////////////////////////////////
//IIC control Functions
//
///////////////////////////////////////////////////////////////////////////////////////

#ifdef Enable_IT6802_CEC
BYTE IT6802_CEC_ReadI2C_Byte(BYTE RegAddr)
{
	 BYTE  p_data;
	 BOOL	FLAG;

	FLAG=i2c_read_byte(CEC_ADDR,RegAddr,1,&p_data,IT6802CECGPIOid);

	  if(FLAG==0)
	{
	 		CEC_DEBUG_PRINTF(("IT6802_CEC I2C ERROR !!!"));
			CEC_DEBUG_PRINTF(("=====  Read Reg0x%X=  \n",RegAddr));

	}

	 return p_data;
}


SYS_STATUS IT6802_CEC_WriteI2C_Byte(BYTE offset,BYTE buffer )
{
	 BOOL  flag;

	 flag=i2c_write_byte(CEC_ADDR,offset,1,&buffer,IT6802CECGPIOid);

	 return !flag;
}

#endif
