#ifndef __USART3_H
#define __USART3_H

#include "stm32f10x.h"
#include <stdio.h>


#define True  1
#define False 0

#define M72D(a)	if (a)	\
					GPIO_SetBits(GPIOB,GPIO_Pin_3);\
					else		\
					GPIO_ResetBits(GPIOB,GPIO_Pin_3)

#define SIM900A(a)	if (a)	\
					GPIO_ResetBits(GPIOB,GPIO_Pin_3);\
					else		\
					GPIO_SetBits(GPIOB,GPIO_Pin_3)



void USART3_Config(void);	
//int fputc(int ch, FILE *f);
void USART3_Putc(uint8_t c);
void USART3_Puts(char * str);
void USART3_IRQHandler(void);
char *i16toa(int value, char *string, int radix);
void USART3_printf(USART_TypeDef* USARTx, uint8_t *Data,...);
void NVIC_Configuration3(void);
#endif
