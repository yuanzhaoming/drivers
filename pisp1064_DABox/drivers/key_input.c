/******************** (C) COPYRIGHT 2016***************************
 * �ļ���  ��key.c
 * ����    ��
**********************************************************************************/
#include "key_input.h" 

#define KEY_MAX_NUMBER 1
#define USE_HARDWARE_DEBUG_KEY_INPUT 0
//static int g_i_key_press_count[KEY_MAX]= 0;	//��⵽�������µĴ���
static int g_i_key_press_time[KEY_MAX_NUMBER] = 0; //��⵽�������µ�ʱ��
static int g_i_key_press_flag[KEY_MAX_NUMBER] = 0; //��⵽�������µı�־
static int g_i_key_pressed[KEY_MAX_NUMBER]	= 0;	//����ȷʵ�����˵ı�־


/*
* 		������������ť:
*					   VOL0    <----------->     PE3
*					   VOL1    <----------->     PE4	 
*					   push    <----------->     PE0
*                      led     <----------->     PE1
*
*/
#define key1_rcc                    RCC_APB2Periph_GPIOE
#define key1_gpio                   GPIOE
#define key1_pin                    (GPIO_Pin_0)


void hal_front_panel_led(int val)
{
	if(val == 0){
		GPIO_ResetBits(GPIOE, GPIO_Pin_1);
	}
	if(val == 1){
		GPIO_SetBits(GPIOE, GPIO_Pin_1);
	}
}


/*get the key status:pressed or released*/
int rt_read_key_value(int i_key_number)
{
	if(i_key_number > KEY_MAX_NUMBER)
		return -1;

	if(	i_key_number == 1)
		return  GPIO_ReadInputDataBit(key1_gpio, key1_pin);
}


void hal_front_panel_init(void)
{
	/*key init*/
    GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure; 
	EXTI_InitTypeDef EXTI_InitStructure;
    RCC_APB2PeriphClockCmd(key1_rcc,ENABLE);
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin   = key1_pin;
    GPIO_Init(key1_gpio, &GPIO_InitStructure);

	/*led init*/
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	hal_front_panel_led(1);

	/*vol init*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE ,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1); 
    NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource3);
   	
   	EXTI_InitStructure.EXTI_Line    = EXTI_Line3;
	EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //Falling�½��� Rising����
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	EXTI_ClearITPendingBit(EXTI_Line3);
}

static int vol = 0;

int get_vol(void)
{
	return vol;
}

void EXTI3_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line3) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line3);

		if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4)==0){
			 rt_kprintf("vol + ...\r\n");
			 if(vol < 24)
			 	vol ++;
		}
		else
		{
			 rt_kprintf("vol - ...\r\n");
			 if(vol > 0)
			 	vol --;
		}		
    }
}


/*
*
* 	������Ϊ����������ĵײ�������
*		��Ҫ����Ϊ:��ⰴ�����£����ڳ����Ȳ�������ʱ����֧�֡�
*
*	�ϲ�������ֻ��Ҫ���߼��ϵ��жϼ���
*
*
*	�������ִ������		
*	�״α�дʱ��:	2016-10-22
*	����		: 	Ԭ����
*		
*	�ò������ݵĴ����߼�Ϊ:
*		ֻҪ��⵽һ���������£����ʱ����ʼ��ʱ����һ����ʱ��֮�󣬲鿴�����Ƿ��¡�
*		���ð������£����������ȷʵ���£�������Ҫ�ȴ������ͷŲ���ִ�к���ĳ���
*		���ð���δ�����£�����ΪΪ��������������������±�־��
*/
int rt_handle_key_input(void)
{
	int i = 0;

	//ͨ������ֵ���ж��Ƿ��°�����,ע��Ҫ�ж�ʱ��
	for(i = 0 ; i < KEY_MAX_NUMBER ; i ++)
	{
		if(rt_read_key_value(i+1) == 0)
		{
			g_i_key_press_flag[i] = 1;
			#if USE_HARDWARE_DEBUG_KEY_INPUT
		   	rt_kprintf("key_%d pressing\r\n",i+1);
			#endif
		}	

		if(g_i_key_press_flag[i])
		{
			//��һ�����ü�ⰴ�����µ�ʱ��
			g_i_key_press_time[i] ++;	

	   		if(g_i_key_press_time[i] >= 5)//����һ����ʱ��֮�������жϰ����Ƿ���
			{
				if(rt_read_key_value(i+1) == 0)
				{
					g_i_key_pressed[i] = 1;
					#if USE_HARDWARE_DEBUG_KEY_INPUT
					rt_kprintf("key_%d really pressed\r\n",i+1);
					#endif
				}
			}

			//�ȴ������ͷ�
			if((rt_read_key_value(i+1) == 1))
			{
				g_i_key_press_flag[i] = 0;
				g_i_key_press_time[i] = 0;
				g_i_key_pressed[i] = 0; 	
			}
		}
	}
   	
	for(i = 0 ; i < KEY_MAX_NUMBER ; i ++)
	{
		if(g_i_key_pressed[i] == 1)
		{
		 	return (i + 1);
		}
	}
	return 0;
}


/******************* (C) COPYRIGHT 2016 QinJing *****END OF FILE****/
