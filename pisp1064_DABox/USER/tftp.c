#include <rtthread.h>
#include <dfs_posix.h>
#include <lwip/sockets.h>	
#include <finsh.h>
  
#include "tftp.h" 
#include "spi_instruction.h"
#include "file.h"   




#define TFTP_PORT			69
/* opcode */
#define TFTP_RRQ			1 	/* read request */
#define TFTP_WRQ			2	/* write request */
#define TFTP_DATA			3	/* data */
#define TFTP_ACK			4	/* ACK */
#define TFTP_ERROR			5	/* error */

rt_uint8_t tftp_buffer[512 + 4];

int g_file_size = 0;
int g_file_start_addr = 0;


static void eeprom_udelay(unsigned int time)
{
	int i = 0;
	unsigned int j;
	for(i = 0 ; i < 8 ; i ++)
	{
	  	for(j = 0 ; j < time; j ++)
			__nop();
	}
}


/* tftp client */
//void tftp_get(const char* host, const char* filename,int startAddr)
void tftp_get( )
{
	int sock_fd, sock_opt;
//	int i = 0;
	struct sockaddr_in tftp_addr, from_addr;
	rt_uint32_t length;
	socklen_t fromlen;

	unsigned char *host = "192.168.25.10";
	unsigned char *filename = "dabox.ldr";
	int startAddr = TFTP_DATA_ADDR;
		 
	int i_write_file = 0;
	int now_addr;

	g_file_start_addr = startAddr;
	g_file_size = 0;

	rt_kprintf("------using tftp to get dsp loader file------------\r\n");

	/* connect to tftp server */
    inet_aton((const char *)host, (struct in_addr*)&(tftp_addr.sin_addr));
    tftp_addr.sin_family = AF_INET;
    tftp_addr.sin_port = htons(TFTP_PORT);
    
    sock_fd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if (sock_fd < 0)
	{
	    //close(fd);
	    rt_kprintf("can't create a socket\n");
	    return ;
	}
	
	/* set socket option */
	sock_opt = 5000; /* 5 seconds */
	lwip_setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &sock_opt, sizeof(sock_opt));

	/* make tftp request */
	tftp_buffer[0] = 0;			/* opcode */
	tftp_buffer[1] = TFTP_RRQ; 	/* RRQ */
	length = rt_sprintf((char *)&tftp_buffer[2], "%s", filename) + 2;
	tftp_buffer[length] = 0; length ++;
	length += rt_sprintf((char*)&tftp_buffer[length], "%s", "octet");
	tftp_buffer[length] = 0; length ++;

	fromlen = sizeof(struct sockaddr_in);
	
	/* send request */	
	lwip_sendto(sock_fd, tftp_buffer, length, 0, 
		(struct sockaddr *)&tftp_addr, fromlen);
	
	do
	{
		length = lwip_recvfrom(sock_fd, tftp_buffer, sizeof(tftp_buffer), 0, 
			(struct sockaddr *)&from_addr, &fromlen);
		//rt_kprintf("-------not connected-%d-------\n",length);
		if((int)length < 0 )
		{
			lwip_close(sock_fd); 
			return;
		}

		if (length > 0)
		{
			//write(fd, (char*)&tftp_buffer[4], length - 4);
			//��???
			//now_addr = startAddr + 512 * i_write_file;
		   	//if((i_write_file % 8) == 0)
			//{
				//????flash????
				//SPI_FLASH_SectorErase(now_addr);
			   	//rt_kprintf("erase..0x%06x \n",now_addr);

			//}
			//SPI_FLASH_BufferWrite(&tftp_buffer[4], now_addr, length - 4);
			//i_write_file ++;

			now_addr = startAddr + 256 * i_write_file;
			iic_eeprom_at24cm02_write(now_addr,&tftp_buffer[4],256);
			eeprom_udelay(8000);
			iic_eeprom_at24cm02_write(now_addr+256,&tftp_buffer[4+256],256);
			eeprom_udelay(8000);
			i_write_file = i_write_file + 2;

			#if 0
			for(i = 0 ; i < length - 4 ; i ++)
			{
				rt_kprintf("%02x ",tftp_buffer[i + 4]);
				if((i + 4) % 16 == 0)
					rt_kprintf("\r\n");	
			}
			rt_kprintf("\r\n");
			#endif

			rt_kprintf("#");

			/* make ACK */			
			tftp_buffer[0] = 0; tftp_buffer[1] = TFTP_ACK; /* opcode */
			/* send ACK */
			lwip_sendto(sock_fd, tftp_buffer, 4, 0, 
				(struct sockaddr *)&from_addr, fromlen);

			g_file_size = g_file_size + length - 4;
		}
	} while (length == 516);

	if (length == 0) 
		rt_kprintf("timeout\n");
	else 
		rt_kprintf("\ndone\n");

	iic_eeprom_at24cm02_write(TFTP_FILE_SIZE_ADDR,(unsigned char *)&g_file_size,4);

	rt_kprintf("filesize:%d B\n",g_file_size);
	//close(fd);
	lwip_close(sock_fd);
}
FINSH_FUNCTION_EXPORT(tftp_get, get file from tftp server);



void stm_dsp_code( void )
{
   	int i_size ;
	int i = 0 , j = 0;
	int i_read_block_size = 4;
	unsigned char buff[4];

	spi_dsp_config();
	/*reset dsp first...*/
	dsp_reset();
	//SPI_FLASH_BufferRead((unsigned char *)&i_size, TFTP_ADDR , 4);

	iic_eeprom_at24cm02_read(TFTP_FILE_SIZE_ADDR,(unsigned char *)&i_size,4);
	g_file_start_addr = TFTP_DATA_ADDR;

	if((i_size == 0xffffffff) || (i_size == 0) || (i_size == -1))
	{
		rt_kprintf("!!bin error!! \n");
		return;
	}
	rt_kprintf("bin: %d B \n",i_size);
	eeprom_udelay(10000);

	SPI_DSP_CS_LOW();	 
	for(i = 0 ; i < i_size / i_read_block_size  ; i ++ )
	{	
		//SPI_FLASH_BufferRead(buff, g_file_start_addr + i * 4, 4);
		//SPI_DSP_SendBytes(buff,4);
		//SPI_FLASH_BufferRead(buff, g_file_start_addr + i * i_read_block_size, i_read_block_size);
		//SPI_DSP_SendBytes_using_tftp(buff,i_read_block_size);
		//rt_kprintf("#");
		iic_eeprom_at24cm02_read(g_file_start_addr + i * 4,buff,4);

		for(j = 0 ; j < 4 ; j ++){
			SPI_DSP_SendByte(buff[j]);
		}
		eeprom_udelay(10);
	}
	eeprom_udelay(1000);
	SPI_DSP_CS_HIGH();
	eeprom_udelay(1000);
//	rt_kprintf("\n--------------dsp950 loade boot file finished--------\n");
}
FINSH_FUNCTION_EXPORT(stm_dsp_code, stm32 send program to dsp);


void stm_erase_dcode( void )
{
   	int i_size ;
	int start_addr=TFTP_DATA_ADDR;
	unsigned char buff[256];
	memset(buff,0,sizeof(buff));
	iic_eeprom_at24cm02_read(TFTP_FILE_SIZE_ADDR,(unsigned char *)&i_size,4);

	if((i_size == 0xffffffff) || (i_size == 0) || (i_size == -1))
	{
		rt_kprintf("!!bin error!! \n");
		return;
	}
	rt_kprintf("bin: %d B \n",i_size);

	for(start_addr = TFTP_DATA_ADDR ; start_addr < TFTP_DATA_ADDR+ (i_size/256)*256 + 256 ; start_addr += 256 )
	{	
		iic_eeprom_at24cm02_write(start_addr,buff,256);
		eeprom_udelay(6000);
	}
	eeprom_udelay(1000);
	iic_eeprom_at24cm02_write(TFTP_FILE_SIZE_ADDR,buff,256); 
	eeprom_udelay(3000);

	rt_kprintf("erase finished...\n");
//	rt_kprintf("\n--------------dsp950 loade boot file finished--------\n");
}
FINSH_FUNCTION_EXPORT(stm_erase_dcode, stm32 erase dsp program );

void flash_dump(int startAddr,int i_len)
{
	int i = 0;
	
	if(i_len > 512)
	{
		rt_kprintf("len is bigger than 512 \n");
		return;
	}

	//SPI_FLASH_BufferRead(&tftp_buffer[0], startAddr, i_len);
 
	for(i = 0 ; i < i_len ; i ++)
	{
		rt_kprintf("0x%02x ",tftp_buffer[i]);
		if( ((i + 1) % 16) == 0)
		{
			rt_kprintf("\r\n");
		} 
	}
}
FINSH_FUNCTION_EXPORT(flash_dump, dump data for w25q16);


void tftp_put(const char* host, const char* dir, const char* filename)
{
	int fd, sock_fd, sock_opt;
	struct sockaddr_in tftp_addr, from_addr;
	rt_uint32_t length, block_number = 0;
	socklen_t fromlen;

	/* make local file name */
	rt_snprintf((char*)tftp_buffer, sizeof(tftp_buffer),
		"%s/%s", dir, filename);

	/* open local file for write */
	fd = open((char*)tftp_buffer, O_RDONLY, 0);
	if (fd < 0)
	{
		rt_kprintf("can't open local filename\n");
		return;
	}

	/* connect to tftp server */
    inet_aton(host, (struct in_addr*)&(tftp_addr.sin_addr));
    tftp_addr.sin_family = AF_INET;
    tftp_addr.sin_port = htons(TFTP_PORT);

    sock_fd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if (sock_fd < 0)
	{
	    close(fd);
	    rt_kprintf("can't create a socket\n");
	    return ;
	}

	/* set socket option */
	sock_opt = 5000; /* 5 seconds */
	lwip_setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &sock_opt, sizeof(sock_opt));

	/* make tftp request */
	tftp_buffer[0] = 0;			/* opcode */
	tftp_buffer[1] = TFTP_WRQ; 	/* WRQ */
	length = rt_sprintf((char *)&tftp_buffer[2], "%s", filename) + 2;
	tftp_buffer[length] = 0; length ++;
	length += rt_sprintf((char*)&tftp_buffer[length], "%s", "octet");
	tftp_buffer[length] = 0; length ++;

	fromlen = sizeof(struct sockaddr_in);
	
	/* send request */	
	lwip_sendto(sock_fd, tftp_buffer, length, 0, 
		(struct sockaddr *)&tftp_addr, fromlen);

	/* wait ACK 0 */	
	length = lwip_recvfrom(sock_fd, tftp_buffer, sizeof(tftp_buffer), 0, 
		(struct sockaddr *)&from_addr, &fromlen);
	if (!(tftp_buffer[0] == 0 &&
		tftp_buffer[1] == TFTP_ACK &&
		tftp_buffer[2] == 0 &&
		tftp_buffer[3] == 0))
	{
		rt_kprintf("tftp server error\n");
		close(fd);
		return;
	}

	block_number = 1;
	
	while (1)
	{
		length = read(fd, (char*)&tftp_buffer[4], 512);
		if (length > 0)
		{
			/* make opcode and block number */
			tftp_buffer[0] = 0; tftp_buffer[1] = TFTP_DATA;
			tftp_buffer[2] = (block_number >> 8) & 0xff;
			tftp_buffer[3] = block_number & 0xff;

			lwip_sendto(sock_fd, tftp_buffer, length + 4, 0, 
				(struct sockaddr *)&from_addr, fromlen);
		}
		else
		{
			rt_kprintf("done\n");
			break; /* no data yet */
		}

		/* receive ack */
		length = lwip_recvfrom(sock_fd, tftp_buffer, sizeof(tftp_buffer), 0, 
			(struct sockaddr *)&from_addr, &fromlen);
		if (length > 0)
		{
			if ((tftp_buffer[0] == 0 &&
				tftp_buffer[1] == TFTP_ACK &&
				tftp_buffer[2] == (block_number >> 8) & 0xff) &&
				tftp_buffer[3] == (block_number & 0xff))
			{
				block_number ++;
				rt_kprintf("#");
			}
			else 
			{
				rt_kprintf("server respondes with an error\n");
				break;
			}
		}
		else if (length == 0)
		{
			rt_kprintf("server timeout\n");
			break;
		}
	}

	close(fd);
	lwip_close(sock_fd);
}
FINSH_FUNCTION_EXPORT(tftp_put, put file to tftp server);





