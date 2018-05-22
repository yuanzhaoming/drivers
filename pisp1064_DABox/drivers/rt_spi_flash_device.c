#include <rtthread.h>
#include <board.h>

//#ifdef RT_USING_LWIP
//#include "stm32_eth.h"
//#endif /* RT_USING_LWIP */
//
#ifdef RT_USING_SPI
#include "rt_stm32f10x_spi.h"
//
//#if defined(RT_USING_DFS) && defined(RT_USING_DFS_ELMFAT)
//#include "msd.h"
//#endif /* RT_USING_DFS */
//
#define RT_USING_SPI1 
/*
 * SPI1_MOSI: PA7
 * SPI1_MISO: PA6
 * SPI1_SCK : PA5
 * SPI1_CS	: PA4  
*/
static void rt_hw_spi_init(void)
{
#ifdef RT_USING_SPI1
	static struct stm32_spi_bus stm32_spi;
    /* register spi bus */
    {
        GPIO_InitTypeDef GPIO_InitStructure;
        /* Enable GPIO clock */
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO,ENABLE);
        GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        stm32_spi_register(SPI1, &stm32_spi, "spi1");
    }

    /* attach cs */
    {
        static struct rt_spi_device spi_device;
        static struct stm32_spi_cs  spi_cs;
        GPIO_InitTypeDef GPIO_InitStructure;
        /* spi1: PA4 */
        spi_cs.GPIOx = GPIOA;
        spi_cs.GPIO_Pin = GPIO_Pin_4;
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Pin = spi_cs.GPIO_Pin;
        GPIO_Init(spi_cs.GPIOx, &GPIO_InitStructure);
		GPIO_SetBits(spi_cs.GPIOx, spi_cs.GPIO_Pin);
        rt_spi_bus_attach_device(&spi_device, "spi-elm", "spi1", (void*)&spi_cs);

	///	rt_kprintf("rt-thread spi init\r\n");
	///	rt_kprintf("GPIO_Pin_4 = %d\r\n",GPIO_Pin_4);

    }
#endif /* RT_USING_SPI1 */
}
#endif /* RT_USING_SPI */


void rt_platform_init(void)
{
#ifdef RT_USING_SPI
    rt_hw_spi_init();
#endif 


}

void rt_spi_flash_device_init(void)
{ 
	rt_hw_spi_init();
#if defined(RT_USING_DFS) && defined(RT_USING_DFS_ELMFAT) 
    w25qxx_init("flash0", "spi-elm");
#endif /* RT_USING_DFS && RT_USING_DFS_ELMFAT */
}























