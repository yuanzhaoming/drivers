/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 * 2013-07-12     aozima       update for auto initial.
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include <board.h>
#include <rtthread.h>

#include "do_data.h"
#include "address_485.h"
#include "file.h"

#ifdef  RT_USING_COMPONENTS_INIT
#include <components.h>
#endif  /* RT_USING_COMPONENTS_INIT */

#ifdef RT_USING_DFS
/* dfs filesystem:ELM filesystem init */
#include <dfs_elm.h>
/* dfs Filesystem APIs */
#include <dfs_fs.h>
#endif

#ifdef RT_USING_RTGUI
#include <rtgui/rtgui.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/driver.h>
#include <rtgui/calibration.h>
#endif

#include "led.h"
#include "oled.h"

/*rs485 thread*/
extern rt_uint8_t rs485_stack[ 512 ];
extern struct rt_thread rs485_thread;
extern void rs485_thread_entry(void* parameter);
/*spi thread*/
extern rt_uint8_t spi_stack[ 512 ];
extern struct rt_thread spi_thread;
extern void spi_thread_entry(void* parameter);



#define USE_OLED  1

static void soft_udelay(unsigned int time)
{
	int i = 0;
	unsigned int j;
	for(i = 0 ; i < 8 ; i ++)
	{
	  	for(j = 0 ; j < time; j ++)
			__nop();
	}
}

 


#define USE_KEY_INPUT 1
#if  USE_KEY_INPUT
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t key_input_stack[ 256 ];
static struct rt_thread key_input_thread;
//#define USE_DEBOUNCING 1
#define USE_RELEASE_KEY_INPUT             1

#define USE_INTERNAL_FLASH                0//定义是否使用内部flash空间
#define WRITE_INTERNAL_PLACE              0x0807F000

static int g_i_key1_press_ok = 0;
static int g_i_have_key_pressed = 0;
static int g_i_lcd_show_value = -1; 	//是在lcd1602中使用的量
#define USE_KEY_APP_DEBUG 0
//使用信号量，这样可以将按键操作的内容发送给DSP和显示屏
/*
* 	 
*			 			  
*/

#if USE_RELEASE_KEY_INPUT
/*
*		该部分为按键处理程序的应用层程序
*
*		应用层处理逻辑为:
*			 两个按键为互斥关系，需要一个按键处理完成，才能处理另外一个。
*
*		这个在底层驱动中，是没有要求互斥关系的。
*/
static void key_input_thread_entry(void* parameter)
{
	int key_input_number;

    while (1)
    {
		//检测是那一个按键按下了。
		key_input_number = rt_handle_key_input();

		if(g_i_have_key_pressed == 0)
		{
			if(g_i_key1_press_ok == 0)
			{
				if(	key_input_number == 1)
				{
					rt_kprintf("key1_input ok\r\n");
					{
						static int mute_led_count=0;
						hal_front_panel_led(mute_led_count);
						mute_led_count ++;
						mute_led_count = mute_led_count % 2;
					}

					g_i_key1_press_ok = 1; 	
					g_i_have_key_pressed = 1;
				}
			}
		}
		else
		{
			if(g_i_key1_press_ok == 1)
			{
			 	//第一个按键按下了,等待释放
				if(rt_read_key_value(1) == 1)
				{
					//释放了，才做处理
					#if USE_KEY_APP_DEBUG
					rt_kprintf("key1_input release\r\n");
					#endif
					g_i_key1_press_ok = 0;
					g_i_have_key_pressed = 0;	
				}	
			}
		}

 
	
		rt_thread_delay( RT_TICK_PER_SECOND / 100);
		#endif
    }
}
#endif




ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t led_stack[ 512 ];
static struct rt_thread led_thread;

 
 
static void led_thread_entry(void* parameter)
{
    unsigned int count=0;
	//硬件初始化
    rt_hw_led_init();
	
    while (1)
    {
        /* led1 on */
        count++;
        rt_hw_led_on(1);
		//hal_front_panel_led(1);
        rt_thread_delay( RT_TICK_PER_SECOND ); /* sleep 0.5 second and switch to other thread */
        /* led1 off */
        rt_hw_led_off(1);
		//	hal_front_panel_led(0);
        rt_thread_delay( RT_TICK_PER_SECOND);
    }
}


ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t oled_stack[ 512 ];
static struct rt_thread oled_thread;

static void oled_thread_entry(void* parameter)
{
    static unsigned int oled_count=0;
	static char oled_buf[40];
	int vol_value;
    while (1)
    {
#if USE_OLED
		 oled_count ++;
		 if(oled_count == 1000){
		 	oled_count = 0;	
		 }
 		//sprintf(oled_buf,"  count:%d ",oled_count);
		//OLED_Print(2,0,oled_buf);
	   	
		if(vol_value != get_vol()){
			vol_value = get_vol();
			sprintf(oled_buf,"   vol:%d ",vol_value);
			//OLED_ShowStr(80,35,oled_buf,1);
			OLED_Print(2,0,oled_buf);
		}	
#endif
        rt_thread_delay( RT_TICK_PER_SECOND/10);
    }
}


#ifdef RT_USING_RTGUI
rt_bool_t cali_setup(void)
{
    rt_kprintf("cali setup entered\n");
    return RT_FALSE;
}

void cali_store(struct calibration_data *data)
{
    rt_kprintf("cali finished (%d, %d), (%d, %d)\n",
               data->min_x,
               data->max_x,
               data->min_y,
               data->max_y);
}
#endif /* RT_USING_RTGUI */

void rt_init_thread_entry(void* parameter)
{
#ifdef RT_USING_COMPONENTS_INIT
    /* initialization RT-Thread Components */
    rt_components_init();
#endif
						 
#ifdef  RT_USING_FINSH
    finsh_set_device(RT_CONSOLE_DEVICE_NAME);
#endif  

    /* Filesystem Initialization */
#if defined(RT_USING_DFS) && defined(RT_USING_DFS_ELMFAT)
//    /* mount sd card fat partition 1 as root directory */
//    if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
//    {
//        rt_kprintf("File System initialized!\n");
//    }
//    else
//        rt_kprintf("File System initialzation failed!\n");
	{
//		dfs_init();
//		elm_init(); 
#ifdef RT_USING_DFS_ELMFAT	
	    /* mount sd card fat partition 1 as root directory */  
//	    if (dfs_mount("flash0", "/", "elm", 0, 0) == 0)  
//	    {  
//	        rt_kprintf("flash0 mount to /.\n");  
//	    }  
//	    else  
//	        rt_kprintf("flash0 mount to / failed.\n"); 
#endif  
    }    
#endif  /* RT_USING_DFS */

#ifdef RT_USING_RTGUI
    {
        extern void rt_hw_lcd_init();
        extern void rtgui_touch_hw_init(void);

        rt_device_t lcd;

        /* init lcd */
        rt_hw_lcd_init();

        /* init touch panel */
        rtgui_touch_hw_init();

        /* re-init device driver */
        rt_device_init_all();

        /* find lcd device */
        lcd = rt_device_find("lcd");

        /* set lcd device as rtgui graphic driver */
        rtgui_graphic_set_device(lcd);

#ifndef RT_USING_COMPONENTS_INIT
        /* init rtgui system server */
        rtgui_system_server_init();
#endif

        calibration_set_restore(cali_setup);
        calibration_set_after(cali_store);
        calibration_init();
    }
#endif /* #ifdef RT_USING_RTGUI */

//读取内部flash的值，进而可以设置一些变量的值
#if USE_INTERNAL_FLASH
{
	unsigned int i_internal_value = *(volatile unsigned long *)WRITE_INTERNAL_PLACE;

	rt_kprintf("old input type:%d\n",i_internal_value);

	if((i_internal_value>5)||(i_internal_value<0))
		i_internal_value = 0;		

	rt_kprintf("inter input type:%d\n",i_internal_value);
}
#endif

#if 1
	//rt_hw_rtc_init();
#ifdef RT_USING_LWIP
		eth_system_device_init();

		rt_hw_stm32_eth_init();
		lwip_sys_init();
		rt_kprintf("TCP/IP initialized!\n");

		//开启服务器程序，接收数据开始
		tcpserv((void *) NULL);
#endif
#endif
	
	rt_thread_delay(50);
}




int rt_application_init(void)
{
    rt_thread_t init_thread;
	char id_485[10];
	char wanos_logo[20];


    rt_err_t result;
	soft_udelay(500000);
	/*dsp reset gpio must be high at last*/
	//dsp_reset();	  //now put this to tftp_send_dsp(). 
	TIM2_Init();
	/*485 address*/
	rt_hw_address_485_init();
	rt_kprintf("get_485_address()=0x%0x\n",get_485_address());

	rt_dsp_reset_gpio_init();
#if USE_OLED
	/*oled init*/
	OLED_Init();
	//sprintf(id_485,"  id:%d ",get_485_address());
    //OLED_Print(0,0,id_485);
	OLED_Print(0,0,"   wanos");
 
#endif
	/*front panel init*/
	hal_front_panel_init();

	/*eeprom init*/
	eeprom_Init();
	#if 0
	{
		int i = 0;
		unsigned char eeprom_read_buf[256];
		unsigned char eeprom_write_buf[256]={0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a};
		int ret;
		
		//ret = iic_eeprom_at24cm02_write(0x0,eeprom_write_buf,256);
		//rt_kprintf("write_ret=0x%0x\n",ret);
		//soft_udelay(5000);

		ret = iic_eeprom_at24cm02_read(0x20,eeprom_read_buf,10);
		rt_kprintf("read_ret=0x%0x\n",ret);
		for(i = 0 ; i < 10 ; i ++){
			rt_kprintf("0x%0x ",eeprom_read_buf[i]);
		}
		rt_kprintf("\r\n");
	}
	#endif

	/*spi slave boot*/
	SPI_DSP_Init();
	/*start dsp code*/
	stm_dsp_code();
	/*config pcm4104*/
	hal_pcm4104_i2s_mode();

	test_big_little_edian();
	/*read cfg*/
	reset_struct_value();
	file_read();
	

	//验证加密芯片
	//ATSHA204m_mac_test();
	//sha204_authentication();

	/*read 485*/
	rt_hw_rs_485_rd_wr_init();
	rt_hw_rs485_uart_init();

    /* init led thread */
    result = rt_thread_init(&led_thread,
                            "led",
                            led_thread_entry,
                            RT_NULL,
                            (rt_uint8_t*)&led_stack[0],
                            sizeof(led_stack),
                            20,
                            5);

    if (result == RT_EOK)
    {
        rt_thread_startup(&led_thread);
    }

    /* init rs485 thread */
    result = rt_thread_init(&rs485_thread,
                            "rs485",
                            rs485_thread_entry,
                            RT_NULL,
                            (rt_uint8_t*)&rs485_stack[0],
                            sizeof(rs485_stack),
                            21,
                            5);

    if (result == RT_EOK)
    {
        rt_thread_startup(&rs485_thread);
    }

    /* init key thread */
    result = rt_thread_init(&key_input_thread,
                            "key",
                            key_input_thread_entry,
                            RT_NULL,
                            (rt_uint8_t*)&key_input_stack[0],
                            sizeof(key_input_stack),
                            18,
                            10);
    if (result == RT_EOK)
    {
        rt_thread_startup(&key_input_thread);
    }


    /* init spi thread */
    result = rt_thread_init(&spi_thread,
                            "spi",
                            spi_thread_entry,
                            RT_NULL,
                            (rt_uint8_t*)&spi_stack[0],
                            sizeof(spi_stack),
                            17,
                            1);
    if (result == RT_EOK)
    {
        rt_thread_startup(&spi_thread);
    }


    /* init lcd thread */
    result = rt_thread_init(&oled_thread,
                            "oled",
                            oled_thread_entry,
                            RT_NULL,
                            (rt_uint8_t*)&oled_stack[0],
                            sizeof(oled_stack),
                            21,
                            2);
    if (result == RT_EOK)
    {
        rt_thread_startup(&oled_thread);
    }
 


#if (RT_THREAD_PRIORITY_MAX == 32)
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   2048, 8, 20);
#else
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   2048, 80, 20);
#endif

    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);

    return 0;
}
/*@}*/



