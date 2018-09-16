#include <rtthread.h>
#include <lwip/sockets.h>  
#include "tcpserver.h"
#include "enc28j60.h"

#define RECEIVE_DATA_LEN  	1024	 
#define PORT				50006

#define CLIENT_NUMS         3

unsigned char recv_data[RECEIVE_DATA_LEN];

#define HTONL(v)	 ( ((v) << 24) | (((v) >> 24) & 0xff) | (((v) << 8) & 0xff0000) | (((v) >> 8) & 0xff00) )

#define net_debug(value,fmt,...) \
	do{\
		if(value&0x01){\
		 		rt_kprintf("[%s],[%d]"fmt"\n",__FUNCTION__,__LINE__,##__VA_ARGS__);\
		}\
		else\
		{\
			 ;\
		}\
	}while(0);

typedef struct _CLIENT{        	//客户端结构体
	int       fd;        		//客户端socket描述符
	struct sockaddr_in addr;    //客户端地址信息结构体                                  
} CLIENT;

CLIENT client[CLIENT_NUMS];     		//FD_SETSIZE为select函数支持的最大描述符个数

static int g_i_connected_id = 0;
static int g_i_connected_flag = 0;

/*
* 		DSP和stm32上进行通信的相应设置项:
*			g_mute:
*			g_input_type:
*			g_volume:
*/

int g_mute;	     	//静音
int g_input_type;	//输入类型
int g_volume;	 	//音量

extern int g_i_key_led_handle_count;
extern int g_i_store_flag;

int g_set_volume_to_dsp_flag = 0;//将pc设置好的音量数据返回给dsp.

static int i_eth_interrupt_flag = 0;

void set_eth_interrupt_flag()
{
	i_eth_interrupt_flag = 1;	
}

int get_eth_interrupt_flag()
{
	return i_eth_interrupt_flag;
}


void clear_eth_interrupt_flag(void)
{
    rt_interrupt_enter();   	
	i_eth_interrupt_flag = 0;
	rt_interrupt_leave();
}


void tcpserv(void* parameter)
{
   //char *recv_data; 
	rt_uint32_t sin_size;
	int count = 0;
	int i = 0;
	int maxi = -1;
//	int newClientId = 0;
	int sock, connected, bytes_received,sockfd;
	struct sockaddr_in server_addr, client_addr;
	rt_bool_t stop = RT_FALSE;  
   //recv_data = rt_malloc(1024);  
   //if (recv_data == RT_NULL)
   //{
   //   rt_kprintf("No memory\n");
   //   return;
   //}
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
	   rt_kprintf("Socket error\n");
		//rt_free(recv_data);
	   return;
	}
	{
		int opt = SO_REUSEADDR;
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));    //设置socket属性
	}

   server_addr.sin_family = AF_INET;
   server_addr.sin_port = htons(PORT); 
   server_addr.sin_addr.s_addr = INADDR_ANY;
   rt_memset(&(server_addr.sin_zero),8, sizeof(server_addr.sin_zero));

 
   if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
   {  
       rt_kprintf("Unable to bind\n");
       //rt_free(recv_data);
       return;
   }
   if (listen(sock, CLIENT_NUMS) == -1)
   {
       rt_kprintf("Listen error\n");
       /* release recv buffer */
       //rt_free(recv_data);
       return;
   }
//   rt_kprintf("\nTCPServer Waiting for client on port %d...\n",PORT);
   //获取系统启动次数
   //read_reboot_times();
   //获取缓存中当前的数据
   //read_w25q16();
   #if 1
   	while(stop != RT_TRUE)
   	{
		fd_set  rset,   allset;    	//select所需的文件描述符集合
		int     maxfd  ;
		int     nready;
		
		sin_size = sizeof(struct sockaddr_in);
		maxfd =  sock;
		FD_ZERO(&allset);        	//清空
		FD_SET(sock, &allset);    	//将监听socket加入select检测的描述符集合		 
		sin_size = sizeof(struct sockaddr_in);
		for (i = 0; i < CLIENT_NUMS; i++) {
			client[i].fd = -1;  
		}
    while(1)
		{
			rset = allset; 
			nready = select(maxfd+1, &rset, NULL, NULL, NULL);    //调用select
			//net_debug(get_debug_value(),"select saw rset actions and the readfset num is %d. ",nready );  
			if (FD_ISSET(sock, &rset)) 
			{      //检测是否有新客户端请求
				rt_kprintf("accept a connection.\n");
				connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
				if(connected == -1)
				{
					rt_kprintf(" accept error \n");
					continue;
				}
				rt_kprintf("I got a connection from (%s , %d),count:%d \n",
			          inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port),count);
				count ++; 
				//如果有残留的上一个端口，需要关闭掉，因为这里只支持一个客户端。

				for (i = 0; i < CLIENT_NUMS; i++)
				{
					if (client[i].fd >= 0) 
					{
						//关中断操作
						g_i_connected_flag = 0;
						g_i_connected_id = 0; 
						//关闭
						lwip_close(client[i].fd);
						rt_kprintf("close other client:%d \n",i); 
						FD_CLR(client[i].fd, &allset);    //从监听集合中删除此socket连接
						client[i].fd = -1;        //数组元素设初始值，表示没客户端连接	
					}	
				}

				for (i = 0; i < CLIENT_NUMS; i++)
				{
					if (client[i].fd < 0) 
					{
						client[i].fd = connected;    //保存客户端描述符
						client[i].addr = client_addr; 
						//net_debug(get_debug_value(),"You got a connection from %s.  ",inet_ntoa(client[i].addr.sin_addr) );
						break;
					}
				}  
			//	net_debug(get_debug_value(),"add new connect fd.");
				g_i_connected_flag = 1;
				g_i_connected_id = connected; 
				if (i == CLIENT_NUMS)          
					rt_kprintf("too many clients");
				FD_SET(connected, &allset);   //将新socket连接放入select监听集合
				if (connected > maxfd)  
					maxfd = connected;   //确认maxfd是最大描述符
				if (i > maxi)          //数组最大元素值
					maxi = i; 
				if (--nready <= 0) 
					continue;      //如果没有新客户端连接，继续循?
			}

			for (i = 0; i <= maxi; i++) 
			{     
				if ( (sockfd = client[i].fd) < 0)       //如果客户端描述符小于0，则没有客户端连接，检测下一个
					continue;
				if (FD_ISSET(sockfd, &rset)) 
				{        //检测此客户端socket是否有数据    
					//net_debug(get_debug_value(),"recv occured for connect fd[%d].",i);
					if ((bytes_received = recv(sockfd, recv_data, RECEIVE_DATA_LEN,0)) <= 0) 
					{ //从客户端socket读数据，等于0表示网络中断
						//关中断操作
						g_i_connected_flag = 0;
						g_i_connected_id = 0; 
						lwip_close(sockfd);        //关闭socket连接
						//net_debug(get_debug_value(),"Client(  ) closed connection.%d ",bytes_received);
						FD_CLR(sockfd, &allset);    //从监听集合中删除此socket连接
						client[i].fd = -1;        //数组元素设初始值，表示没客户端连接
					} 
					else
					{
						recv_data[bytes_received] = '\0';
						if((bytes_received % 4 == 0) && (bytes_received != 0 ))
					   	{	 
							 rt_kprintf("tcp:%d bytes \r\n",bytes_received);
							 tcp_recv_data(recv_data,bytes_received);
							//handle_net_data(connected,recv_data,bytes_received);
					   	}						
					}
					if (--nready <= 0)     
						break;       //如果没有新客户端有数据，跳出for循环回到while循环
				}
			}  
		} 
	}
   #endif
   lwip_close(sock); 
   //rt_free(recv_data);  
   return ;
}

/*
* 		tcp send data
*
*
*/
void tcp_send_data(unsigned char *buf,int len)
{
	int i = 0;
	int client_id;
	/*if no socket exists*/
	if(g_i_connected_flag==0){
	 	return ;
	}
	/*find the exist socket*/
	for (i = 0; i < CLIENT_NUMS; i++) {
		if(client[i].fd != -1){
			client_id = client[i].fd;
		}  
	}	
	/*send data to client socket*/ 
   	send(client_id,buf,len,0);     
}


 