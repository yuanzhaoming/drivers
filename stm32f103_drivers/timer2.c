/********************************copyright ythuitong by wit_yuan**************************/
#include "timer2.h"
#include "stm32f10x.h"

static volatile unsigned TimingDelay;
//////////////////////////////////////////////////////////////////////////
//			 	函数名		:		TIM2_Init		
//			   	功能		:		定时器2初始化
//				参数		:		void
//				作者		:		wit_yuan
//				时间		:		2014-11-07
////////////////////////////////////////////////////////////////////////////
void TIM2_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	TIM_DeInit(TIM2);
	//分频系数是10000 times
	TIM_TimeBaseStructure.TIM_Period=10;
	//分频系数为72,就是72000000/72/10=100000Hz;	 = 10us
	TIM_TimeBaseStructure.TIM_Prescaler=72-1;//division number   is 32 
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


void Delay_10us(volatile unsigned int  nTime)
{ 
	TimingDelay = nTime;	

	while(TimingDelay != 0);
}


/*
 * 函数名：TimingDelay_Decrement
 * 描述  ：获取节拍程序
 * 输入  ：无
 * 输出  ：无
 * 调用  ：在 SysTick 中断函数 SysTick_Handler()调用
 */  
static void Timer2_Decrement(void)
{
	if (TimingDelay != 0x00)
	{ 
		TimingDelay--;
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////
//				   		函数名			:		TIM2_IRQHandler
//						功能			:       定时器2的中断,作为简单的10us延时程序
//						作者			:		wit_yuan
//						编写时间		:		2015-01-20
//						更新时间		:		2015-01-20
//						修改内容		:		无
/////////////////////////////////////////////////////////////////////////////////////////////////
void TIM2_IRQHandler(void)
{
    if ( TIM_GetITStatus(TIM2 , TIM_IT_Update) != RESET ) 
	{     
		TIM_ClearITPendingBit(TIM2 , TIM_FLAG_Update); //清除标志
	   	Timer2_Decrement();
	}
}




/************************************end of file 2014-08-08*********************************/
