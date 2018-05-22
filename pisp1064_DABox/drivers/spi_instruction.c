#include <stm32f10x.h>
#include <rtthread.h>
#include <finsh.h>

#include "rt_stm32f10x_spi.h"
#include "spi_flash_w25qxx.h"
#include "rt_stm32f10x_spi.h"
#include "spi.h"
#include "spi_instruction.h"

#define SPI_READ_MAX  100

static void spi_delay_us(int time)
{
	int i = 0;
	unsigned int j;
	for(i = 0 ; i < 8 ; i ++)
	{
	  	for(j = 0 ; j < time; j ++)
			__nop();
	}
}

/*
* 		执行与DSP通信与配置DA
*		硬件接线:	
*				PA04/SPI1_NSS	DSP_CS
*				PA05/SPI1_SCK	DSP_SCK
*				PA06/SPI1_MISO	DSP_MISO
*				PA07/SPI1_MOSI	DSP_MOSI
*
*
*               PE07            DSP_DA_CS
*				PA05            DSP_DA_CLK
*               PA07            DSP_DA_MOSI
*
*
*	  			PC04            DSP_GPIO
*/
/*******************************************************************************
*******************************************************************************/
void SPI_DSP_Init(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;					 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE , ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);		
	/*!< Configure SPI_DSP_SPI pins: SCK */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/*!< Configure SPI_DSP_SPI pins: MISO */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/*!< Configure SPI_DSP_SPI pins: MOSI */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/*!< Configure SPI_DSP_SPI_CS_PIN pin: Card CS pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/* Deselect the FLASH: Chip Select high */
	SPI_DSP_CS_HIGH();

	/*!< Configure SPI_DA_SPI_CS_PIN pin: Card CS pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	/* Deselect the FLASH: Chip Select high */
	SPI_DA_CS_HIGH();

	/*default for slave boot*/
	/* SPI1 configuration */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge; /*both 1 == mode3*/
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_LSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);
	/* Enable SPI1  */
	SPI_Cmd(SPI1, ENABLE);	  


	/*DSP_GPIO*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC ,ENABLE);
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

unsigned char DSP_GPIO_STATUS(void)
{
	return 	GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4);
}



void spi_da_config(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
	/* SPI1 configuration */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge; /*both 1 == mode3*/
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);
	/* Enable SPI1  */
	SPI_Cmd(SPI1, ENABLE);			    
}

void spi_dsp_config(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
	/* SPI1 configuration */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge; /*both 1 == mode3*/
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_LSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);
	/* Enable SPI1  */
	SPI_Cmd(SPI1, ENABLE);
}

/*
* 	stm32硬件spi发送数据
*
*
*/
unsigned char SPI_DSP_SendByte(unsigned char byte)
{
	/* Loop while DR register in not emplty */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	/* Send byte through the SPI1 peripheral */
	SPI_I2S_SendData(SPI1, byte);
	/* Wait to receive a byte */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	/* Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData(SPI1);
}

/*
* 		读取DSP传输过来的32位的数据，采用轮询方式
*
*/
u32 SPI_DSP_ReadBytes(void)
{
	unsigned int c_rcv1;
	unsigned int c_rcv2;
	unsigned int c_rcv3;
	unsigned int c_rcv4;

	SPI_DSP_CS_LOW();

	c_rcv1 = SPI_DSP_SendByte(0x00); 
	c_rcv2 = SPI_DSP_SendByte(0x00); 
	c_rcv3 = SPI_DSP_SendByte(0x00); 
	c_rcv4 = SPI_DSP_SendByte(0x00); 

	SPI_DSP_CS_HIGH();
  	return (c_rcv1 << 24) | (c_rcv2 << 16 ) | (c_rcv3 << 8) | c_rcv4;
}

#if 0
/*
*
*
*  	 使用硬件spi给dsp发送数据
*
*/
void SPI_DSP_SendBytes(unsigned char *buffer, unsigned int len)
{
	int i = 0;
	unsigned char c_value;

  	SPI_DSP_CS_LOW();

	for(i = 0 ; i < len ; i ++)
	{
		c_value = SPI_DSP_SendByte(buffer[i]);
	}
	SPI_DSP_CS_HIGH();
}
#endif

int dsp_read_u32_with_timeout(unsigned int *data,int time_out)
{
	int timeout = 0;
	do{
		if(DSP_GPIO_STATUS()==1){
			*data = SPI_DSP_SendByte(0x00); 
			*data <<= 8;
			*data |= SPI_DSP_SendByte(0x00);
			*data <<= 8; 
			*data |= SPI_DSP_SendByte(0x00); 
			*data <<= 8; 
			*data |= SPI_DSP_SendByte(0x00); 
			//rt_kprintf("stm recv:0x%08x...\n",*data); 
			return 0;			
		}
		else{
			timeout ++;
			//spi_delay_us(10);
			rt_thread_delay( 1 );	  
		}
	}while(timeout<time_out);	// if dsp is in debugging then use the 200ms timeout
	return -1;
}

int dsp_read_u32(unsigned int *data)
{
	if(DSP_GPIO_STATUS()==1){
		*data = SPI_DSP_SendByte(0x00); 
		*data <<= 8;
		*data |= SPI_DSP_SendByte(0x00);
		*data <<= 8; 
		*data |= SPI_DSP_SendByte(0x00); 
		*data <<= 8; 
		*data |= SPI_DSP_SendByte(0x00); 
		return 0;			
	}
	return -1;
}


static unsigned int spi_read_data[SPI_READ_MAX];




void DSP_W4Bytes(unsigned int data)
{
	unsigned int c_value;
	int ret = 0;
	int read_times = 0;
	int i = 0;

	SPI_DSP_CS_LOW(); 
	spi_delay_us(200);

	if(DSP_GPIO_STATUS()==0){
		rt_kprintf("stm send:0x%08x...\n",data); 
		SPI_DSP_SendByte((data>>24)&0xff);
		SPI_DSP_SendByte((data>>16)&0xff);
		SPI_DSP_SendByte((data>>8)&0xff);
		SPI_DSP_SendByte(data&0xff);
	 
	 	//the first time read
		ret = dsp_read_u32_with_timeout(&spi_read_data[read_times],2000);
		if(ret < 0){
	   		rt_kprintf("dsp is timeout:0x%0x...\n",spi_read_data[0]);
			SPI_DSP_CS_HIGH();
			//spi_delay_us(20); 	
			rt_thread_delay( 1 );	  
			return;
		}
		//have more data to be read.
	 	while(1){
			if(	spi_read_data[read_times] !=0)
				read_times++; 
			//spi_delay_us(10000); 	
			rt_thread_delay( 1 );	  
			ret = dsp_read_u32_with_timeout(&spi_read_data[read_times],200);
			
			if(ret < 0)
				break;	 
			if(read_times > SPI_READ_MAX-1){
			   rt_kprintf("exccessed the max number...\n");
			   break;
			}
		}
	}				  
	else{
		return;
	}

   	if(read_times > 0){
		rt_kprintf("stm recv len:%d\n",read_times); 
		for( i = 0 ; i <  read_times ; i ++){
			rt_kprintf("0x%0x\n",spi_read_data[i]);
		}
		rt_kprintf("\n"); 
	}

	if(read_times>=100){
		for( i = 0 ; i <  read_times ; i ++){
			if(	spi_read_data[i] == 0){
				return;
			}
		}	
	}
	//send data via socket
	spi_send_to_tcp(data,spi_read_data,read_times);

out:
	SPI_DSP_CS_HIGH();
	spi_delay_us(200); 	
} 
FINSH_FUNCTION_EXPORT(DSP_W4Bytes, send four bytes to dsp);

















