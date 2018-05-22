#include "dsp_reset.h"
#include <stm32f10x.h>

static void dsp_reset_udelay(unsigned int time)
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
* 		PE6    <----->   reset DSP
*
*
*/
void rt_dsp_reset_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
}

void rt_hw_dsp_reset(char value)
{
	if(value == 0){
	 	GPIO_ResetBits(GPIOE, GPIO_Pin_6);
	}
	if(value == 1){
	 	GPIO_SetBits(GPIOE, GPIO_Pin_6);
	}
}


void dsp_reset(void)
{
	rt_hw_dsp_reset(0);
	dsp_reset_udelay(1000);
	rt_hw_dsp_reset(1);
	dsp_reset_udelay(1000);
}
