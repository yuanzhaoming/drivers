#include "rs485_task.h"
#include <rtthread.h>

ALIGN(RT_ALIGN_SIZE)
rt_uint8_t rs485_stack[ 512 ];
struct rt_thread rs485_thread;

#define RS485_APP_MAXSIZE   1024


static unsigned char rs485_app_recv_buffer[RS485_APP_MAXSIZE];
static unsigned char rs485_app_send_buffer[RS485_APP_MAXSIZE];

void rs485_thread_entry(void* parameter)
{ 
	int rs485_rd_len = 0;
   	int i = 0;

	hw_rs_485_rd_wr(0);
	rt_thread_delay(10);
	rt_kprintf("rs485 task start...\n"); 

    while (1)
    {
		rs485_rd_len = get_rs485_buf(rs485_app_recv_buffer,20);
		if(rs485_rd_len > 0){

			rt_kprintf("rs485_app_recv_buffer:%d\n",rs485_rd_len);
			for(i = 0 ; i < 20 ; i ++){	
				rt_kprintf("0x%0x ",rs485_app_recv_buffer[i]);
			}
			rt_kprintf("\r\n");

			#if 1
			rt_thread_delay( 1);
			hw_rs_485_rd_wr(1);
			rt_thread_delay( 1);
			usart2_puts("rs485_app_recv_buffer:\n");
			for(i = 0 ; i < 20 ; i ++){	
				usart2_put(rs485_app_recv_buffer[i]);  
			}
			hw_rs_485_rd_wr(0);
			#endif
		}

	
	
        rt_thread_delay( RT_TICK_PER_SECOND);
    }
}














