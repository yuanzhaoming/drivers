#include "address_485.h"
#include <stm32f10x.h>

#define RS485_BUF_MAX  1024
/*
*    SW_D2<-->PE12    
*	 SW_D1<-->PE13
*	 SW_D0<-->PE14
*
*
*/
void rt_hw_address_485_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE ,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_14;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
}

unsigned char get_485_address(void)
{
 	return (GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_12)<<2) | (GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_13)<<1) | (GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_14)<<0);
}

/**
*	  init rs485 en pin
*
*	  PE11
*/
void rt_hw_rs_485_rd_wr_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE ,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

	GPIO_ResetBits(GPIOE, GPIO_Pin_11);
}

void hw_rs_485_rd_wr(int dir)
{
	/*read*/
	if(dir == 0){
		GPIO_ResetBits(GPIOE, GPIO_Pin_11);
	}
	/*write*/
	if(dir == 1){
		GPIO_SetBits(GPIOE, GPIO_Pin_11);
	}
}

/*
*
* 		uart_tx <------------> PA2
*		uart_rx <------------> PA3
*
*/
void rt_hw_rs485_uart_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
	USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;  
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2,&USART_InitStructure);
	USART_Cmd(USART2,ENABLE);  

	
    /* Enable the USART2 Interrupt */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
}

static char RS485_recv[RS485_BUF_MAX];
static int rs485_rd_pointer=0;
static int rs485_wr_pointer=0;

int get_rs485_buf_full(void)
{
	return ((rs485_wr_pointer+1)%RS485_BUF_MAX==rs485_rd_pointer);
}

int get_rs485_buf_empty(void)
{
	return (rs485_wr_pointer== rs485_rd_pointer);
}

int get_rs485_buf(char *buf,int len)
{
	int i = 0;
	int length=0;
	for(i = 0 ; i < len ; i ++){
		if(get_rs485_buf_empty()==0){	
			 buf[i] = RS485_recv[rs485_rd_pointer];
			 rs485_rd_pointer ++;
			 rs485_rd_pointer = rs485_rd_pointer % RS485_BUF_MAX;

			 length ++;
		}
		else{	
		 	break;
		}	
	} 
   	
	return length;	
}

void usart2_puts(u8 *str)
{
	while(*str)
	{
		USART_SendData(USART2, *str++);
	 	while (!(USART2->SR & USART_FLAG_TXE));
	}
}

void usart2_put(u8 ch)
{
	USART_SendData(USART2, ch);
	while(!USART_GetFlagStatus(USART2, USART_FLAG_TC));

}



void USART2_IRQHandler(void)
{
	//char c;
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);

    	//c = USART_ReceiveData(USART2);

		if(get_rs485_buf_full()==0){
			 RS485_recv[rs485_wr_pointer] = USART2->DR;
			 rs485_wr_pointer ++;
			 rs485_wr_pointer = rs485_wr_pointer % RS485_BUF_MAX;
		}		
    }
}

