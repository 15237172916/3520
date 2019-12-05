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

static HI_RET atoul(IN CHAR *str,OUT pU32 pulValue);
static HI_RET atoulx(IN CHAR *str,OUT pU32 pulValue);

/*****************************************************************************
 Prototype    : StrToNumber
 Calls        : isdigit
                
 Called  By   : 
                
 History        : 
   Author       : t41030
   Modification : Created function
*****************************************************************************/

static HI_RET StrToNumber(IN CHAR *str , OUT pU32 pulValue)
{
    if ( *str == '0' && (*(str+1) == 'x' || *(str+1) == 'X') )
    {
        if (*(str+2) == '\0')
        {
            return HI_FAILURE;
        }
        else
        {
            return atoulx(str+2,pulValue);
        }
    }
    else
    {
        return atoul(str,pulValue);
    }
}

/*****************************************************************************
 Prototype    : atoul
 Calls        : isdigit
                
 Called  By   : 
                
 History        : 
   Author       : t41030
   Modification : Created function
*****************************************************************************/
static HI_RET atoul(IN CHAR *str,OUT pU32 pulValue)
{
    U32 ulResult=0;

    while (*str)
    {
        if (isdigit((int)*str))
        {
            if ((ulResult<429496729) || ((ulResult==429496729) && (*str<'6')))
            {
                ulResult = ulResult*10 + (*str)-48;
            }
            else
            {
                *pulValue = ulResult;
                return HI_FAILURE;
            }
        }
        else
        {
            *pulValue=ulResult;
            return HI_FAILURE;
        }
        str++;
    }
    *pulValue=ulResult;
    return HI_SUCCESS;
}



/*****************************************************************************
 Prototype    : atoulx
 Calls        : toupper
                isdigit
                
 Called  By   : 
                
 History        : 
   Author       : t41030
   Modification : Created function
*****************************************************************************/
#define ASC2NUM(ch) (ch - '0')
#define HEXASC2NUM(ch) (ch - 'A' + 10)

static HI_RET  atoulx(IN CHAR *str,OUT pU32 pulValue)
{
    U32   ulResult=0;
    UCHAR ch;

    while (*str)
    {
        ch=toupper(*str);
        if (isdigit(ch) || ((ch >= 'A') && (ch <= 'F' )))
        {
            if (ulResult < 0x10000000)
            {
                ulResult = (ulResult << 4) + ((ch<='9')?(ASC2NUM(ch)):(HEXASC2NUM(ch)));
            }
            else
            {
                *pulValue=ulResult;
                return HI_FAILURE;
            }
        }
        else
        {
            *pulValue=ulResult;
            return HI_FAILURE;
        }
        str++;
    }
    
    *pulValue=ulResult;
    return HI_SUCCESS;
}


void print_r_usage(void)
{
	printf("usage: i2c_read <i2c_num> <device_addr> <reg_addr> <end_reg_addr>"
			"<reg_width> <data_width> <reg_step>. sample: \n");
	printf("------------------------------------------------------------------------------------\n");
	printf("\t\ti2c_read 0x1 0x56 0x0 0x10 2 2. \n");
	printf("\t\ti2c_read 0x1 0x56 0x0 0x10 2 2 2. \n");
	printf("\t\ti2c_read 0x1 0x56 0x0 0x10. default reg_width, data_width, reg_step is 1. \n");
}

static HI_RET i2c_read(int argc , char argv[][10], unsigned char *p_data)
{
	int fd = -1;
	int ret;
	unsigned int device_addr, reg_addr, reg_addr_end;
    int value;
	
	if ((argc != 3) && (argc != 4))
    {
    	printf("usage: %s <device_addr> <reg_addr>. sample: %s 0x56 0x0\n", argv[0], argv[0]);
        return -1;
    }
	
	fd = open("/dev/gpioi2c", 0);
    if (fd<0)
    {
    	printf("Open gpioi2c dev error!\n");
    	return -1;
    }
    
    if (StrToNumber(argv[1], &device_addr))
    {    	
    	return -1;
    }
       
    if (StrToNumber(argv[2], &reg_addr))
    {    
    	return -1;
    }
    
    if (3 == argc)
    {
        if (0x100 > reg_addr)
        {
            value = ((device_addr&0xff)<<24) | ((reg_addr&0xff)<<16);   
            ret = ioctl(fd, GPIO_I2C_READ_BYTE, &value);            
            *p_data = value&0xff;
            //printf("GPIO_I2C_READ_BYTE, dev_addr=0x%x reg_addr=0x%x value=0x%x\n", device_addr, reg_addr, *p_data);
        }
        else
    	{
            value = ((device_addr&0xff)<<24) | ((reg_addr&0xffff)<<8);    
            
            ret = ioctl(fd, GPIO_I2C_READ_DWORD, &value);        
            *p_data = value;
            //printf("GPIO_I2C_READ_DWORD, dev_addr=0x%x reg_addr=0x%x value=0x%x\n", device_addr, reg_addr, *p_data);
        }
    }
    else if (4 == argc)
    {
        int cur_addr;
        if (StrToNumber(argv[3], &reg_addr_end))
        {
            return -1;
        }
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
    }
    close(fd);
    return 0;
}

#if 0
static HI_RET i2c_read(int argc , char argv[][10], unsigned char *p_data)
{
	//printf("argc: %d \n", argc);
	//printf("argv: %p \n", argv);
	//printf("*argv: %s", argv[0]);
	int fd = -1;
	char file_name[0x20];
	int ret;
	unsigned int tmp, i;
	unsigned int i2c_num, device_addr, reg_addr, reg_addr_end;
	unsigned int data;
	unsigned int reg_width  = 1;
	unsigned int data_width = 1;
	unsigned int reg_step   = 1;
	char recvbuf[4];
	int cur_addr;

	if (argc < READ_MIN_CNT) {
		print_r_usage();
		return -1;
	}

	for (i = 1; i < argc; i++) {
		if (StrToNumber(argv[i], (OUT pU32)&tmp)) {
			print_r_usage();
			return -1;
		}

		switch (i) {
		case 1: i2c_num = tmp;
			break;
		case 2: device_addr = tmp;
			break;
		case 3: reg_addr = tmp;
			reg_addr_end = tmp;
			break;
		case 4: reg_addr_end = tmp;
			if (reg_addr_end < reg_addr) {
				printf("end addr(0x%2x) should bigger than start addr(0x%2x)\n",
						reg_addr_end, reg_addr);
				print_r_usage();
				return -1;
			}
			break;
		case 5: reg_width = tmp;
			break;
		case 6: data_width = tmp;
			break;
		case 7: reg_step = tmp;
			break;
		default: break;
		}
	}

	//printf("i2c_num:0x%x, dev_addr:0x%2x; reg_addr:0x%2x; reg_addr_end:0x%2x; reg_width: %d; data_width: %d; reg_step:0x%2x. \n\n",
	//		i2c_num, device_addr, reg_addr, reg_addr_end, reg_width, data_width, reg_step);

	sprintf(file_name, "/dev/i2c-%u", i2c_num);
	fd = open(file_name, O_RDWR);
	if (fd<0) {
		printf("Open %s error!\n", file_name);
		return -1;
	}

	ret = ioctl(fd, I2C_SLAVE_FORCE, device_addr);
	if (ret < 0) {
		printf("CMD_SET_DEV error!\n");
		close(fd);
		return -1;
	}

	if (reg_width == 2)
		ret = ioctl(fd, I2C_16BIT_REG, 1);
	else
		ret = ioctl(fd, I2C_16BIT_REG, 0);

	if (ret < 0) {
		printf("CMD_SET_REG_WIDTH error!\n");
		close(fd);
		return -1;
	}

	if (data_width == 2)
		ret = ioctl(fd, I2C_16BIT_DATA, 1);
	else
		ret = ioctl(fd, I2C_16BIT_DATA, 0);

	if (ret < 0) {
		printf("CMD_SET_DATA_WIDTH error!\n");
		close(fd);
		return -1;
	}

	for (cur_addr = reg_addr; cur_addr < reg_addr_end + reg_step; cur_addr += reg_step) {
		if (reg_width == 2) {
			recvbuf[0] = cur_addr & 0xff;
			recvbuf[1] = (cur_addr >> 8) & 0xff;
		} else {
			recvbuf[0] = cur_addr & 0xff;
		}

		ret = read(fd, recvbuf, reg_width);
		if (ret < 0) {
			printf("CMD_I2C_READ error!\n");
			close(fd);
			return -1;
		}

		if (data_width == 2) {
			data = recvbuf[0] | (recvbuf[1] << 8);
		} else
			data = recvbuf[0];

		//printf("0x%x 0x%x\n", cur_addr, data);
		*p_data = (unsigned char)data;
		p_data++;
	}

	close(fd);
	//printf("data: %x \n", data);
	return 0;
}
#endif
void print_w_usage(void)
{
	printf("usage: i2c_write <i2c_num> <device_addr> <reg_addr> <value> <reg_width> <data_width>. sample:\n");
	printf("----------------------------------------------------------------------------\n");
	printf("\t\ti2c_write 0x1 0x56 0x0 0x28 2 2. \n");
	printf("\t\ti2c_write 0x1 0x56 0x0 0x28. default reg_width and data_width is 1. \n");
}

static HI_RET i2c_write(int argc , char argv[][10])
{
	int fd = -1;
	int ret =0;
    int value;
	unsigned int device_addr, reg_addr, reg_value, data1, data2;
		
	if(argc < 4)
    {
    	printf("usage: %s <device_addr> <reg_addr> <value>. sample: %s 0x56 0x0 0x28\n", argv[0], argv[0]);
        return -1;
    }
	
	fd = open("/dev/gpioi2c", 0);
    if(fd<0)
    {
    	printf("Open gpioi2c error!\n");
    	return -1;
    }
    
    if (StrToNumber(argv[1], &device_addr))
    {    	
    	return 0;
    }
    
    if (StrToNumber(argv[2], &reg_addr))
    {    
    	return 0;
    }
    
    if (StrToNumber(argv[3], &reg_value))
    {    
    	return 0;
    }
    if(argc==4)
    {
        //printf("device_addr:0x%x; reg_addr:0x%x; reg_value:0x%x.\n", device_addr, reg_addr, reg_value);
        
        value = ((device_addr&0xff)<<24) | ((reg_addr&0xff)<<16) | (reg_value&0xffff);
        
        ret = ioctl(fd, GPIO_I2C_WRITE_BYTE, &value);
	}
	else
    {
    	cx25838_regs_data value1;
    	StrToNumber(argv[4], &data1);
    	StrToNumber(argv[5], &data2);
    	value1.chip=device_addr;
    	value1.addr=reg_addr;
    	value1.lobit=reg_value;
    	value1.hibit=data1;
    	value1.data=data2;
    	
    	ret = ioctl(fd, GPIO_I2C_WRITE_DWORD, &value1);
	}
    close(fd);
    return 0;
}
#if 0
static HI_RET i2c_write(int argc , char argv[][10])
{
	int fd = -1;
	char file_name[0x20];
	int ret =0;
	unsigned int tmp, i, index = 0;
	unsigned int i2c_num, device_addr, reg_addr, reg_value;
	unsigned int reg_width  = 1;
	unsigned int data_width = 1;
	char buf[4];

	if (argc < WRITE_MIN_CNT) {
		print_w_usage();
		return -1;
	}

	for (i = 1; i < argc; i++) {
		if (StrToNumber(argv[i], (OUT pU32)&tmp)) {
			print_w_usage();
			return -1;
		}

		switch (i) {
		case 1: i2c_num = tmp;
			break;
		case 2: device_addr = tmp;
			break;
		case 3: reg_addr = tmp;
			break;
		case 4: reg_value = tmp;
			break;
		case 5: reg_width = tmp;
			break;
		case 6: data_width = tmp;
			break;
		default: break;
		}
	}

	//printf("dev_addr:0x%2x; reg_addr:0x%2x; reg_value:0x%2x; reg_width: %d; data_width: %d.\n",
	//		device_addr, reg_addr, reg_value, reg_width, data_width);

	sprintf(file_name, "/dev/i2c-%u", i2c_num);
	fd = open(file_name, O_RDWR);
	if (fd<0) {
		printf("Open %s error!\n", file_name);
		return -1;
	}

	ret = ioctl(fd, I2C_SLAVE_FORCE, device_addr);
	if (ret < 0) {
		printf("CMD_SET_DEV error!\n");
		close(fd);
		return -1;
	}

	if (reg_width == 2)
		ret = ioctl(fd, I2C_16BIT_REG, 1);
	else
		ret = ioctl(fd, I2C_16BIT_REG, 0);

	if (ret < 0) {
		printf("CMD_SET_REG_WIDTH error!\n");
		close(fd);
		return -1;
	}

	if (data_width == 2)
		ret = ioctl(fd, I2C_16BIT_DATA, 1);
	else
		ret = ioctl(fd, I2C_16BIT_DATA, 0);

	if (ret < 0) {
		printf("CMD_SET_DATA_WIDTH error!\n");
		close(fd);
		return -1;
	}

	if (reg_width == 2) {
		buf[index] = reg_addr & 0xff;
		index++;
		buf[index] = (reg_addr >> 8) & 0xff;
		index++;
	} else {
		buf[index] = reg_addr & 0xff;
		index++;
	}

	if (data_width == 2) {
		buf[index] = reg_value & 0xff;
		index++;
		buf[index] = (reg_value >> 8) & 0xff;
		index++;
	} else {
		buf[index] = reg_value & 0xff;
		index++;
	}

	ret = write(fd, buf, (reg_width + data_width));
	if(ret < 0)
	{
		printf("I2C_WRITE error!\n");
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}
#endif

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
	char write_cmd[7][10] = {0};

	for (i=0; i<byteno; i++)
	{
		//printf("usage: i2c_write <i2c_num> <device_addr> <reg_addr> <value> <reg_width> <data_width>. sample:\n");
		
		//sprintf(write_cmd[1], "%d", 0); //i2c_num
		sprintf(write_cmd[1], "0x%x", address); //device_addr
		sprintf(write_cmd[2], "0x%x", offset); //reg_addr
		sprintf(write_cmd[3], "0x%x", *p_data); //value
		//sprintf(write_cmd[5], "%d", 1); //reg_width
		//sprintf(write_cmd[6], "%d", 1); //data_width

		ret = i2c_write( 4, write_cmd);
		if (ret < 0)
		{
			return 0;
		}
		p_data++;
		offset++;
	}
	
    return 1;

    /*
     BYTE data i;

     i2c_8051_start(device);

     i2c_8051_write(address&0xFE,device);
     if( i2c_8051_wait_ack(device)==1 )    {
         i2c_8051_end(device);
     return 0;
      }

     i2c_8051_write(offset,device);
     if( i2c_8051_wait_ack(device)==1 )    {
         i2c_8051_end(device);
     return 0;
     }

     for(i=0; i<byteno-1; i++) {
          i2c_8051_write(*p_data,device);
          if( i2c_8051_wait_ack(device)==1 ) {
              i2c_8051_end(device);
         return 0;
         }
         p_data++;
     }

     i2c_8051_write(*p_data,device);
     if( i2c_8051_wait_ack(device)==1 )    {
          i2c_8051_end(device);
     return 0;
     }
     else {
          i2c_8051_end(device);
     return 1;
     }
     */
}

#endif 

#if 1
BOOL i2c_read_byte( BYTE address,BYTE offset,BYTE byteno,BYTE *p_data,BYTE device )
{
   // printf("usage: i2c_read <i2c_num> <device_addr> <reg_addr> <end_reg_addr>"
			//"<reg_width> <data_width> <reg_step>. sample: \n");
	char read_cmd[8][10] = {0};
	//BYTE tmp = 0, i;
	unsigned long ret;
	//sprintf(read_cmd[1], "%d", 0); //i2c_num
	sprintf(read_cmd[1], "0x%x", address); //device_addr
	sprintf(read_cmd[2], "0x%x", offset); //reg_addr
	sprintf(read_cmd[3], "0x%x", offset+(byteno-1)); //end_reg_addr
	//sprintf(read_cmd[5], "%d", 1); //reg_width
	//sprintf(read_cmd[6], "%d", 1); //data_width
	//sprintf(read_cmd[7], "%d", 1); //reg_step

	ret = i2c_read( 4, read_cmd, p_data);
	if (ret < 0)
	{
		return 0;
	}
	#if 0
	for (i=0; i<byteno; i++)
	{
		sprintf(read_cmd[1], "%d", 0); //i2c_num
		sprintf(read_cmd[2], "0x%x", address); //device_addr
		sprintf(read_cmd[3], "0x%x", offset); //reg_addr
		sprintf(read_cmd[4], "0x%x", offset); //end_reg_addr
		sprintf(read_cmd[5], "%d", 1); //reg_width
		sprintf(read_cmd[6], "%d", 1); //data_width
		sprintf(read_cmd[7], "%d", 1); //reg_step
		//printf("%d \n", i2c_read( 7, read_cmd));
		ret = i2c_read( 7, read_cmd, p_data);
		if (ret < 0)
		{
			return 0;
		}
		//printf("ret: %x \n", ret);
		//*p_data = (unsigned char)ret;
		//printf("*p_data: %x \n", *p_data);
		//p_data++;
		//offset++;
		//printf("offset: %x \n", offset);
	}
	#endif

    /*
    BYTE data i;

     i2c_8051_start(device);

     i2c_8051_write(address&0xFE,device);
     if( i2c_8051_wait_ack(device)==1 ) {
         i2c_8051_end(device);
         return 0;
     }

     i2c_8051_write(offset,device);
     if( i2c_8051_wait_ack(device)==1 ) {
         i2c_8051_end(device);
         return 0;
     }

     i2c_8051_start(device);

     i2c_8051_write(address|0x01,device);
     if( i2c_8051_wait_ack(device)==1 ) {
         i2c_8051_end(device);
         return 0;
     }

     for(i=0; i<byteno-1; i++) {
         *p_data=i2c_8051_read(device);
         i2c_8051_send_ack(LOW,device);

         p_data++;
     }

     *p_data=i2c_8051_read(device);
     i2c_8051_send_ack(HIGH,device);
     i2c_8051_end(device);
    */
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
