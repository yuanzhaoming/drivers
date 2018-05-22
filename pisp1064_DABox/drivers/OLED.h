/*********************copyright wit_yuan qj 2016-10-09*********************/
#ifndef _OLED_h_
#define _OLED_h_


#define DISP_ON           0xAF
#define DISP_OFF          0xAE  

#define SPI_OLED_CS_LOW()       GPIO_ResetBits(GPIOA, GPIO_Pin_15)
#define SPI_OLED_CS_HIGH()      GPIO_SetBits(GPIOA, GPIO_Pin_15)

#define OLED_RST_LOW()       GPIO_ResetBits(GPIOB, GPIO_Pin_8)
#define OLED_RST_HIGH()      GPIO_SetBits(GPIOB, GPIO_Pin_8)

#define OLED_DC_LOW()       GPIO_ResetBits(GPIOB, GPIO_Pin_9)
#define OLED_DC_HIGH()      GPIO_SetBits(GPIOB, GPIO_Pin_9)

  

#endif
/****************************end of file*********************************/


