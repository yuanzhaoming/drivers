/***************************copyright ythuitong by wit_yuan 2014-10-24********************/
#include "includes.h"
//////////////////////////////////////////////////////////////////////////////////////////////
//				   		�ļ���			:		timer3.c
//						����			:       �Զ�ʱ������
//						����			:		wit_yuan
//						��дʱ��		:		2014-10-30
//						����ʱ��		:		2014-11-04
//						�޸�����		:		��
/////////////////////////////////////////////////////////////////////////////////////////////////
static volatile unsigned TimingDelay;

extern Controler my_controler;	   									//�п���Ϣ
extern End_Device	my_end_device[END_DEVICE_NUMBERS + 1];			//�ն˽ڵ�
extern Ready_data	my_ready_433_data[END_DEVICE_NUMBERS + 1];		//�ն��豸��Ҫ������
//////////////////////////////////////////////////////////////////////////////////////////////
//				   		������			:		TIM3_Init
//						����			:       ��ʱ��3�Ĳ���
//						����			:		wit_yuan
//						��дʱ��		:		2014-10-30
//						����ʱ��		:		2014-10-30
//						�޸�����		:		��
/////////////////////////////////////////////////////////////////////////////////////////////////
void TIM3_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	TIM_DeInit(TIM3);
	//��Ƶϵ����10000 times
	TIM_TimeBaseStructure.TIM_Period = 1000 ;
	//��Ƶϵ��Ϊ72,����72000000/72/1000=1000Hz;
	TIM_TimeBaseStructure.TIM_Prescaler=72-1;//division number   is 32 
	//����ʱ�ӷ�Ƶ
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; //or TIM_CKD_DIV2 or TIM_CKD_DIV4
	//��������
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);
	//���TIM3����жϱ�־
	TIM_ClearFlag(TIM3,TIM_FLAG_Update);
	//TIM3����ж�ʹ��
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
	//ʹ��TIM3
	TIM_Cmd(TIM3,ENABLE);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
//////////////////////////////////////////////////////////////////////////////////////////////
//				   		������			:		TIM3_IRQHandler
//						����			:       ��ʱ��3���ж�
//						����			:		wit_yuan
//						��дʱ��		:		2014-10-30
//						����ʱ��		:		2014-11-04
//						�޸�����		:		��
/////////////////////////////////////////////////////////////////////////////////////////////////
void TIM3_IRQHandler(void)
{ 
//   	int i = 0;
//	static u32 timer3_tick = 0;
    if ( TIM_GetITStatus(TIM3 , TIM_IT_Update) != RESET ) 
	{     
		TIM_ClearITPendingBit(TIM3 , TIM_FLAG_Update); //�����־ 
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
//				   		������			:		TimingDelay_Decrement
//						����			:       
//						����			:		wit_yuan
//						��дʱ��		:		2014-10-30
//						����ʱ��		:		2014-10-30
//						�޸�����		:		��
/////////////////////////////////////////////////////////////////////////////////////////////////
void TimingDelay_Decrement(void)
{
	if (TimingDelay != 0x00)
	{ 
		TimingDelay--;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//				   		������			:		TimingDelay_Decrement
//						����			:       
//						����			:		wit_yuan
//						��дʱ��		:		2014-10-30
//						����ʱ��		:		2014-10-30
//						�޸�����		:		��
/////////////////////////////////////////////////////////////////////////////////////////////////
void Delay_1ms(volatile unsigned int  nTime)
{ 
	TimingDelay = nTime;	
	while(TimingDelay != 0);
}

/***************************copyright ythuitong by wit_yuan 2014-10-30**end of file******************/
