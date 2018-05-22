#include "timer2.h"
#include "stm32f10x.h"
void TIM2_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	TIM_DeInit(TIM2);
	//分频系数是10000 times
	TIM_TimeBaseStructure.TIM_Period=10000;
	//分频系数为72,就是72000000/7200/10000=1Hz;	 = 1s
	TIM_TimeBaseStructure.TIM_Prescaler=7200-1;//division number   is 32 
	//设置时钟分频
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; //or TIM_CKD_DIV2 or TIM_CKD_DIV4
	//递增计数
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);
	//清除TIM2溢出中断标志
	TIM_ClearFlag(TIM2,TIM_FLAG_Update);
	//TIM2溢出中断使能
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
	//使能TIM2
	TIM_Cmd(TIM2,ENABLE);
	
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
	NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


static unsigned int time_eclapse = 0;

void TIM2_IRQHandler(void)
{
    if ( TIM_GetITStatus(TIM2 , TIM_IT_Update) != RESET ) 
	{     
		TIM_ClearITPendingBit(TIM2 , TIM_FLAG_Update); //清除标志
	   	time_eclapse++;
	}
}

static char sys_time[30];

char* get_sys_time(void)
{
	int hour,minute,second;
	unsigned int total_second;
	total_second = time_eclapse;

	hour = total_second / 3600;
	minute = (time_eclapse - hour*3600) / 60; 
	second =  (time_eclapse - hour*3600-minute*60);   
	
	sprintf(sys_time,"%2d.%02d.%02d",hour,minute,second);	
	
	return sys_time;
}


