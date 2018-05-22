#include "echo.h"  
#include "rtdef.h"
   
struct rx_msg  
{  
    rt_device_t dev;  
    rt_size_t   size;  
};  
   
static struct rt_messagequeue  rx_mq;  
static char uart_rx_buffer[64];  
static char msg_pool[2048];  
   
// ���������ص�����  
rt_err_t uart_input(rt_device_t dev, rt_size_t size)  
{  
    struct rx_msg msg;  
    msg.dev = dev;  
    msg.size = size;  
     
        // ���������ݷ�����Ϣ����  
    rt_mq_send(&rx_mq, &msg, sizeof(struct rx_msg));
	
	rt_kprintf("indicated echo\r\n");  
     
    return RT_EOK;  
}  
   
// ������ں���  
void usr_echo_thread_entry(void* parameter)  
{  
    struct rx_msg msg;  
     
    rt_device_t device;  
    rt_err_t result = RT_EOK;  
     
        // ��RTϵͳ�л�ȡ����1�豸  
    device = (rt_device_t)rt_device_find("uart1");  
    if (device != RT_NULL)  
    {  
                           // ָ�����մ������ݵĻص�����  
        rt_device_set_rx_indicate(device, uart_input);  
                           // �Զ�д��ʽ���豸  
        rt_device_open(device, RT_DEVICE_OFLAG_RDWR);  
    }  
     
    while(1)  
    {  
                           // ����Ϣ�����л�ȡ���ص�����������Ϣ�����е�����  
        result = rt_mq_recv(&rx_mq, &msg, sizeof(struct rx_msg), 50);  
        if (result == -RT_ETIMEOUT)  
        {  
            // timeout, do nothing  
        }  
         
        if (result == RT_EOK)  
        {  
            rt_uint32_t rx_length;  
            
            rx_length = (sizeof(uart_rx_buffer) - 1) > msg.size ?  
                msg.size : sizeof(uart_rx_buffer) - 1;  
            
            rx_length = rt_device_read(msg.dev, 0, &uart_rx_buffer[0], rx_length);  
            uart_rx_buffer[rx_length] = '\0';  
            // ������д�ص�����1  
            rt_device_write(device, 0, &uart_rx_buffer[0], rx_length); 
			
			
			 
        }  

		//rt_device_write(device, 0, "wewfwfwe\r\n", 10);  
    }  
}  
// �������̳�ʼ������  
void usr_echo_init()  
{  
    rt_thread_t thread ;  
     
    rt_err_t result;   
      // ������Ϣ���У�������д洢�ռ�  
    result = rt_mq_init(&rx_mq, "mqt", &msg_pool[0], 128 - sizeof(void*), sizeof(msg_pool), RT_IPC_FLAG_FIFO);  
     
    if (result != RT_EOK)   
    {   
        rt_kprintf("init message queue failed.\n");   
        return;   
    }   
    // ���������߳�  
    thread = (rt_thread_t)rt_thread_create("devt",  
        usr_echo_thread_entry, RT_NULL,  
        1024, 25, 7);  
    // ���������߳�  
    if (thread != RT_NULL)  
        rt_thread_startup(thread);  

}  



