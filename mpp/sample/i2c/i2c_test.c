#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include "i2c.h"

#define GPIO_I2C_MAGIC_BASE	'I'
#define GPIO_I2C_READ_BYTE   _IOR(GPIO_I2C_MAGIC_BASE,0x01,int)
#define GPIO_I2C_WRITE_BYTE  _IOW(GPIO_I2C_MAGIC_BASE,0x02,int)

#define GPIO_I2C_READ_DWORD   _IOR(GPIO_I2C_MAGIC_BASE,0x03,int)
#define GPIO_I2C_WRITE_DWORD  _IOR(GPIO_I2C_MAGIC_BASE,0x04,int)


#if 0

#include <stdio.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "strfunc.h"
#include "gpio_i2c.h"
#endif

int main()
{
	int fd = -1;
	int ret;
	unsigned int device_addr, reg_addr, reg_addr_end;
    int reg_value, value;
	
	
	fd = open("/dev/gpioi2c", 0);
    if (fd<0)
    {
    	printf("Open gpioi2c dev error!\n");
    	return -1;
    }
	reg_addr = 0x68;
	device_addr = 0x94;
while (1)
{
	sleep(1);


    if (1)
    {
        if (0x100 > reg_addr)
        {
            value = ((device_addr&0xff)<<24) | ((reg_addr&0xff)<<16);   
            ret = ioctl(fd, GPIO_I2C_READ_BYTE, &value);            
            reg_value = value&0xff;
            printf("GPIO_I2C_READ_BYTE, dev_addr=0x%x reg_addr=0x%x value=0x%x\n", device_addr, reg_addr, reg_value);
        }
        else
    	{
            value = ((device_addr&0xff)<<24) | ((reg_addr&0xffff)<<8);    
            
            ret = ioctl(fd, GPIO_I2C_READ_DWORD, &value);        
            reg_value = value;
            printf("GPIO_I2C_READ_DWORD, dev_addr=0x%x reg_addr=0x%x value=0x%x\n", device_addr, reg_addr, reg_value);
        }
    }
    else
    {
        int cur_addr;
        
        if (reg_addr_end < reg_addr)
        {
            printf("end addr(0x%x) should bigger than start addr(0x%x)\n",
                reg_addr_end, reg_addr);
            return 0;
        }
        printf("device_addr:0x%x; reg_addr_start:0x%x; reg_addr_end:0x%x.\n", 
            device_addr, reg_addr, reg_addr_end);
        for (cur_addr=reg_addr; cur_addr<reg_addr_end+1; cur_addr++)
        {
            value = ((device_addr&0xff)<<24) | ((cur_addr&0xff)<<16);   
            ret = ioctl(fd, GPIO_I2C_READ_BYTE, &value);            
            reg_value = value&0xff;
            printf("GPIO_I2C_READ_BYTE, dev_addr=0x%x reg_addr=0x%x value=0x%x\n", device_addr, cur_addr, reg_value);
        }
    }
}
    return 0;
}

#if 0

int main(void)
{
	int fd, ret;
	int p_data;
	int i;
	

	fd = open("/dev/gpioi2c", 0);
	if (fd<0) 
	{
		printf("Open i2c error!\n");
		return -1;
	}
	printf("device address : %x \n", 0x92);

	while (1)
	{
		ret = ioctl(fd, GPIO_I2C_READ_BYTE, 0x92);
		if (ret < 0) {
			printf("CMD_SET_DEV error!\n");
			//close(fd);
			//printf("ret: %d \n", ret);
			//return -1;
		}
		else
		{
			printf("device %x succed \n", i);
		}
		usleep(10000);
	}
	
	//printf("device address : %x \n", device_addr);
	#if 0
	for (i=0x10; i<0x100; i++)
	{
		printf("device address : %x \n", i);
		ret = ioctl(fd, I2C_SLAVE_FORCE, i);
		if (ret < 0) {
			printf("CMD_SET_DEV error!\n");
			//close(fd);
			//printf("ret: %d \n", ret);
			//return -1;
		}
		else
		{
			printf("device %x succed \n", i);
		}
		
		
	}
	#endif
	
	return 0;
}
#endif