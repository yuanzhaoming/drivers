#include "spi_task.h"
#include <rtthread.h>
#include <finsh.h>

#define SPI_DEBUG

#ifdef SPI_DEBUG
#define spi_debug(fmt, ...) rt_kprintf("[spi_task.c][Line:%d]"fmt"\n", __LINE__, ##__VA_ARGS__)
#else
#define spi_debug(fmt, ...)
#endif


ALIGN(RT_ALIGN_SIZE)
rt_uint8_t spi_stack[ 512 ];
struct rt_thread spi_thread;

#define SPI_BUF_MAX   1024
static int spi_recv[SPI_BUF_MAX];
static int spi_rd_pointer=0;
static int spi_wr_pointer=0;


int get_spi_buf_full(void)
{
	return ((spi_wr_pointer+1)%SPI_BUF_MAX==spi_rd_pointer);
}

int get_spi_buf_empty(void)
{
	return (spi_wr_pointer== spi_rd_pointer);
}

int get_spi_buf(unsigned int *buf,int len)
{
	int i = 0;
	int length=0;
	for(i = 0 ; i < len ; i ++){
		if(get_spi_buf_empty()==0){	
			 buf[i] = spi_recv[spi_rd_pointer];
			 spi_rd_pointer ++;
			 spi_rd_pointer = spi_rd_pointer % SPI_BUF_MAX;

			 length ++;
		}
		else{	
		 	break;
		}	
	}
	return length;	
}




void write_spi_buf(unsigned int data)
{
	if(get_spi_buf_full()==0){
		spi_recv[spi_wr_pointer] = data;
		spi_wr_pointer ++;
		spi_wr_pointer = spi_wr_pointer % SPI_BUF_MAX;
	}
}

#if 0
void PC_write4Bytes(unsigned int data)
{
	if(get_spi_buf_full()==0){
		spi_recv[spi_wr_pointer] = data;
		spi_wr_pointer ++;
		spi_wr_pointer = spi_wr_pointer % SPI_BUF_MAX;
	}
}
FINSH_FUNCTION_EXPORT(PC_write4Bytes, simulate pc to write 4 bytes);
#endif




void spi_thread_entry(void* parameter)
{
	int spi_rd_len = 0;
    unsigned int spi_app_recv;
    unsigned int spi_app_send;

	while(1){
		spi_rd_len = get_spi_buf(&spi_app_recv,1);
		if(spi_rd_len > 0){
			spi_debug("send:0x%0x",spi_app_recv);

			DSP_W4Bytes(spi_app_recv);
			/*spi to cfg*/
			spi_to_cfg(spi_app_recv);
		}

		rt_thread_delay( 2 );
	}
}
