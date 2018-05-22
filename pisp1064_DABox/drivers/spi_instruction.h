#ifndef _stm32_spi_instruction_dsp_h_
#define _stm32_spi_instruction_dsp_h_
#include "stm32f10x.h"

#define SPI_DSP_CS_LOW()       GPIO_ResetBits(GPIOA, GPIO_Pin_4)
#define SPI_DSP_CS_HIGH()      GPIO_SetBits(GPIOA, GPIO_Pin_4)


#define SPI_DA_CS_LOW()       GPIO_ResetBits(GPIOE, GPIO_Pin_7)
#define SPI_DA_CS_HIGH()      GPIO_SetBits(GPIOE, GPIO_Pin_7)
#endif



