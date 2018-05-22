/**
  ******************************************************************************
  * @file    xxx.c 
  * @author  Waveshare Team
  * @version 
  * @date    xx-xx-2014			   
  * @brief   xxxxx.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, WAVESHARE SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "delay.h"
#include "inc.h"

#include "stm32f10x.h"

void Delay_us(uint32_t time)
{
	int i = 0;
	unsigned int j;
	for(i = 0 ; i < 8 ; i ++)
	{
	  	for(j = 0 ; j < time; j ++)
			__nop();
	}
}

void Delay_ms(uint16_t value)
{
	Delay_us(value*1000);
}

#if 0

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t  s_chFacMicro = 0;
static uint16_t s_hwFacMill = 0;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
 * @brief initializes delay.
 * @param wSystemCoreClock comes from system core clock.
 * @retval None
 */
void Delay_init(uint32_t wSystemCoreClock)	 
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);	 
	s_chFacMicro = wSystemCoreClock / 8000000;	  //SystemCoreClock
	s_hwFacMill = (uint16_t)s_chFacMicro * 1000;//   
}								    

/**
 * @brief delay for n us.
 * @param wMicro = nus / (SystemCoreClock / 8000000)
 * @retval None 
 */
void Delay_us(uint32_t wMicro)
{		
	uint32_t wTemp;
	
	if (wMicro == 0)
	{
		return;
	}
	
	SysTick->LOAD = wMicro * s_chFacMicro;   		 
	SysTick->VAL = 0x00;         
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk ;        
	do {
		wTemp = SysTick->CTRL;
	}
	while(wTemp & 0x01 && !(wTemp & (1 << 16)));   
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;       
	SysTick->VAL = 0X00;       
}

/**
 * @brief delay for n ms.
 * @param hwMill = (nms / (SystemCoreClock / 8000000)) * 1000
 * @retval None
 */
void Delay_ms(uint16_t hwMill)
{	 		  	  
	uint32_t wTemp;
	
	if (hwMill == 0)
	{
		return;
	}
	
	SysTick->LOAD = (uint32_t)hwMill * s_hwFacMill;
	SysTick->VAL = 0x00;           
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk ;         
	do {
		wTemp = SysTick->CTRL;
	}
	while(wTemp & 0x01 && !(wTemp & (1 << 16)));
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;       
	SysTick->VAL = 0X00;       
} 
#endif
/*-------------------------------END OF FILE-------------------------------*/
