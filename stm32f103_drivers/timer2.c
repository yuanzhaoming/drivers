/********************************copyright ythuitong by wit_yuan**************************/
#include "timer2.h"
#include "stm32f10x.h"

static volatile unsigned TimingDelay;
//////////////////////////////////////////////////////////////////////////
//			 	������		:		TIM2_Init		
//			   	����		:		��ʱ��2��ʼ��
//				����		:		void
//				����		:		wit_yuan
//				ʱ��		:		2014-11-07
////////////////////////////////////////////////////////////////////////////
void TIM2_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	TIM_DeInit(TIM2);
	//��Ƶϵ����10000 times
	TIM_TimeBaseStructure.TIM_Period=10;
	//��Ƶϵ��Ϊ72,����72000000/72/10=100000Hz;	 = 10us
	TIM_TimeBaseStructure.TIM_Prescaler=72-1;//division number   is 32 
	//����ʱ�ӷ�Ƶ
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; //or TIM_CKD_DIV2 or TIM_CKD_DIV4
	//��������
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);
	//���TIM2����жϱ�־
	TIM_ClearFlag(TIM2,TIM_FLAG_Update);
	//TIM2����ж�ʹ��
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
	//ʹ��TIM2
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
 * ��������TimingDelay_Decrement
 * ����  ����ȡ���ĳ���
 * ����  ����
 * ���  ����
 * ����  ���� SysTick �жϺ��� SysTick_Handler()����
 */  
static void Timer2_Decrement(void)
{
	if (TimingDelay != 0x00)
	{ 
		TimingDelay--;
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////
//				   		������			:		TIM2_IRQHandler
//						����			:       ��ʱ��2���ж�,��Ϊ�򵥵�10us��ʱ����
//						����			:		wit_yuan
//						��дʱ��		:		2015-01-20
//						����ʱ��		:		2015-01-20
//						�޸�����		:		��
/////////////////////////////////////////////////////////////////////////////////////////////////
void TIM2_IRQHandler(void)
{
    if ( TIM_GetITStatus(TIM2 , TIM_IT_Update) != RESET ) 
	{     
		TIM_ClearITPendingBit(TIM2 , TIM_FLAG_Update); //�����־
	   	Timer2_Decrement();
	}
}




/************************************end of file 2014-08-08*********************************/
