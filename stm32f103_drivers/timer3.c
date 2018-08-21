/***************************copyright ythuitong by wit_yuan 2014-10-24********************/
#include "includes.h"
//////////////////////////////////////////////////////////////////////////////////////////////
//				   		文件名			:		timer3.c
//						功能			:       对定时的配置
//						作者			:		wit_yuan
//						编写时间		:		2014-10-30
//						更新时间		:		2014-11-04
//						修改内容		:		无
/////////////////////////////////////////////////////////////////////////////////////////////////
static volatile unsigned TimingDelay;

extern Controler my_controler;	   									//中控信息
extern End_Device	my_end_device[END_DEVICE_NUMBERS + 1];			//终端节点
extern Ready_data	my_ready_433_data[END_DEVICE_NUMBERS + 1];		//终端设备需要的数据
//////////////////////////////////////////////////////////////////////////////////////////////
//				   		函数名			:		TIM3_Init
//						功能			:       定时器3的操作
//						作者			:		wit_yuan
//						编写时间		:		2014-10-30
//						更新时间		:		2014-10-30
//						修改内容		:		无
/////////////////////////////////////////////////////////////////////////////////////////////////
void TIM3_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	TIM_DeInit(TIM3);
	//分频系数是10000 times
	TIM_TimeBaseStructure.TIM_Period = 1000 ;
	//分频系数为72,就是72000000/72/1000=1000Hz;
	TIM_TimeBaseStructure.TIM_Prescaler=72-1;//division number   is 32 
	//设置时钟分频
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; //or TIM_CKD_DIV2 or TIM_CKD_DIV4
	//递增计数
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);
	//清除TIM3溢出中断标志
	TIM_ClearFlag(TIM3,TIM_FLAG_Update);
	//TIM3溢出中断使能
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
	//使能TIM3
	TIM_Cmd(TIM3,ENABLE);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
//////////////////////////////////////////////////////////////////////////////////////////////
//				   		函数名			:		TIM3_IRQHandler
//						功能			:       定时器3的中断
//						作者			:		wit_yuan
//						编写时间		:		2014-10-30
//						更新时间		:		2014-11-04
//						修改内容		:		无
/////////////////////////////////////////////////////////////////////////////////////////////////
void TIM3_IRQHandler(void)
{ 
//   	int i = 0;
//	static u32 timer3_tick = 0;
    if ( TIM_GetITStatus(TIM3 , TIM_IT_Update) != RESET ) 
	{     
		TIM_ClearITPendingBit(TIM3 , TIM_FLAG_Update); //清除标志 
//		if(timer3 == 2000)   //2s 
//		{ 					
//			timer3 = 0;
//		}		
//		if(my_controler.sound_light_switcher[0] == '1')
//		{
//			timer3_tick ++; 
//			if(timer3_tick == 1000)   //2s 
//			{
//				for( i = 0 ; i <= END_DEVICE_NUMBERS ; i ++)
//				{			
//					//if(my_ready_433_data[i].exist[0] == '1')
//					{
//						if(my_ready_433_data[i].speaker == 1)
//						{
//							alarm(ON);	
//							//delay_1ms(500);
//							//alarm(OFF);
//							my_ready_433_data[i].speaker = 0;
//							//OSTimeDlyHMSM(0, 0, 0 , 500);
//							break;
//						}
//					}
//				}
//			}
//			if(timer3_tick == 1500)
//			{
//				alarm(OFF);
//				timer3_tick = 0;
//			}
//		}

		TimingDelay_Decrement();	
	 }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//				   		函数名			:		TimingDelay_Decrement
//						功能			:       
//						作者			:		wit_yuan
//						编写时间		:		2014-10-30
//						更新时间		:		2014-10-30
//						修改内容		:		无
/////////////////////////////////////////////////////////////////////////////////////////////////
void TimingDelay_Decrement(void)
{
	if (TimingDelay != 0x00)
	{ 
		TimingDelay--;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//				   		函数名			:		TimingDelay_Decrement
//						功能			:       
//						作者			:		wit_yuan
//						编写时间		:		2014-10-30
//						更新时间		:		2014-10-30
//						修改内容		:		无
/////////////////////////////////////////////////////////////////////////////////////////////////
void Delay_1ms(volatile unsigned int  nTime)
{ 
	TimingDelay = nTime;	
	while(TimingDelay != 0);
}

/***************************copyright ythuitong by wit_yuan 2014-10-30**end of file******************/
