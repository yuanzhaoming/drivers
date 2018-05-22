#include "pcm4104.h"
#include "spi_instruction.h"

typedef struct{
	int control;
	int value;
}PCM4104_t;


PCM4104_t pcm4104_t[]={
  	{AUDIO_CTLL_REG,1},
	{CHAN1_ATTENUATION_REG,255},
};

static void pcm4104_delay_us(int time)
{
	int i = 0;
	unsigned int j;
	for(i = 0 ; i < 8 ; i ++)
	{
	  	for(j = 0 ; j < time; j ++)
			__nop();
	}
}

void hal_pcm4104_i2s_mode(void)
{
	int i;
   	spi_da_config();
   	pcm4104_delay_us(100);
	for(i = 0 ; i < sizeof(pcm4104_t)/sizeof(pcm4104_t[0]); i++){
	 	   
	  	SPI_DA_CS_LOW();
		pcm4104_delay_us(100);
		SPI_DSP_SendByte(pcm4104_t[i].control | PCM4104_WRITE);
		SPI_DSP_SendByte(pcm4104_t[i].value);
		pcm4104_delay_us(100);
		SPI_DA_CS_HIGH();
		pcm4104_delay_us(100);		    	 
	}
}

