/***************************copyright ythuitong by wit_yuan 2014-10-24********************/
//////////////////////////////////////////////////////////////////////////////////////////
//				   		�ļ���			:		usart2.c
//						����			:       ���豸�ն˷���������豸!
//						����			:		wit_yuan
//						��дʱ��		:		2014-10-24
//						����ʱ��		:		2014-10-24
//						�޸�����		:		��
//						Ӳ������		:	    rx------------PA3
//												tx------------PA2
/////////////////////////////////////////////////////////////////////////////////////////////
#include "includes.h"
#include <stdarg.h>

unsigned char USART2_RECEIVE_DATA[512];
unsigned char USART2_SEND_DATA[512];
//////////////////////////////////////////////////////////////////////////////////////////////
// 				������			��			USART2_Config
// 				����  			��			�ײ�Ӳ���ĳ�ʼ��
// 				����  			��			��
// 				����  			: 			��
//				����			:			wit_yuan
// 				��дʱ��  		��			2014-10-24
//				�޸�ʱ��		:			2014-10-24
//				�޸�����		:			��
//////////////////////////////////////////////////////////////////////////////////////////////
void USART2_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure; 
	DMA_InitTypeDef DMA_InitStructure; 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 ;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
	GPIO_Init(GPIOA, &GPIO_InitStructure);  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);			 
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
	//////////////////�жϵ��������////////////////////////////
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	//USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); //�����жϷ�ʽ 
  	USART_Cmd(USART2, ENABLE);

	DMA_DeInit(DMA1_Channel6); 
	DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40004404;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)USART2_RECEIVE_DATA;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 512;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel6, &DMA_InitStructure);

	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
	DMA_Cmd(DMA1_Channel6, ENABLE);
	USART_ITConfig(USART2, USART_IT_IDLE , ENABLE);	
}

/////////////////////////////////////////////////////////////////////////////
// 				������			��			USART2_Putc
// 				����  			��			�����ֽ�����
// 				����			:			
// 				����	  		��			wit_yuan
//				��дʱ��		��			2014-10-24
//				�޸�ʱ��		:			2014-10-24
//				�޸�����		:			��
//////////////////////////////////////////////////////////////////////////////
void USART2_Putc(unsigned char c)
{
	
    USART_SendData(USART2, c);
    while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET );
}

/***************************copyright ythuitong by wit_yuan 2014-10-24******end of file**************/


