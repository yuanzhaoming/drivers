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

typedef struct _CLIENT{        	//�ͻ��˽ṹ��
	int       fd;        		//�ͻ���socket������
	struct sockaddr_in addr;    //�ͻ��˵�ַ��Ϣ�ṹ��                                  
} CLIENT;

CLIENT client[CLIENT_NUMS];     		//FD_SETSIZEΪselect����֧�ֵ��������������

static int g_i_connected_id = 0;
static int g_i_connected_flag = 0;

/*
* 		DSP��stm32�Ͻ���ͨ�ŵ���Ӧ������:
*			g_mute:
*			g_input_type:
*			g_volume:
*/

int g_mute;	     	//����
int g_input_type;	//��������
int g_volume;	 	//����

extern int g_i_key_led_handle_count;
extern int g_i_store_flag;

int g_set_volume_to_dsp_flag = 0;//��pc���úõ��������ݷ��ظ�dsp.

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
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));    //����socket����
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
   //��ȡϵͳ��������
   //read_reboot_times();
   //��ȡ�����е�ǰ������
   //read_w25q16();
   #if 1
   	while(stop != RT_TRUE)
   	{
		fd_set  rset,   allset;    	//select������ļ�����������
		int     maxfd  ;
		int     nready;
		
		sin_size = sizeof(struct sockaddr_in);
		maxfd =  sock;
		FD_ZERO(&allset);        	//���
		FD_SET(sock, &allset);    	//������socket����select��������������		 
		sin_size = sizeof(struct sockaddr_in);
		for (i = 0; i < CLIENT_NUMS; i++) {
			client[i].fd = -1;  
		}
      	while(1)
		{
			rset = allset; 
			nready = select(maxfd+1, &rset, NULL, NULL, NULL);    //����select
			//net_debug(get_debug_value(),"select saw rset actions and the readfset num is %d. ",nready );  
			if (FD_ISSET(sock, &rset)) 
			{      //����Ƿ����¿ͻ�������
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
				//����в�������һ���˿ڣ���Ҫ�رյ�����Ϊ����ֻ֧��һ���ͻ��ˡ�

				for (i = 0; i < CLIENT_NUMS; i++)
				{
					if (client[i].fd >= 0) 
					{
						//���жϲ���
						g_i_connected_flag = 0;
						g_i_connected_id = 0; 
						//�ر�
						lwip_close(client[i].fd);
						rt_kprintf("close other client:%d \n",i); 
						FD_CLR(client[i].fd, &allset);    //�Ӽ���������ɾ����socket����
						client[i].fd = -1;        //����Ԫ�����ʼֵ����ʾû�ͻ�������	
					}	
				}

				for (i = 0; i < CLIENT_NUMS; i++)
				{
					if (client[i].fd < 0) 
					{
						client[i].fd = connected;    //����ͻ���������
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
				FD_SET(connected, &allset);   //����socket���ӷ���select��������
				if (connected > maxfd)  
					maxfd = connected;   //ȷ��maxfd�����������
				if (i > maxi)          //�������Ԫ��ֵ
					maxi = i; 
				if (--nready <= 0) 
					continue;      //���û���¿ͻ������ӣ�����ѭ?
			}

			for (i = 0; i <= maxi; i++) 
			{     
				if ( (sockfd = client[i].fd) < 0)       //����ͻ���������С��0����û�пͻ������ӣ������һ��
					continue;
				if (FD_ISSET(sockfd, &rset)) 
				{        //���˿ͻ���socket�Ƿ�������    
					//net_debug(get_debug_value(),"recv occured for connect fd[%d].",i);
					if ((bytes_received = recv(sockfd, recv_data, RECEIVE_DATA_LEN,0)) <= 0) 
					{ //�ӿͻ���socket�����ݣ�����0��ʾ�����ж�
						//���жϲ���
						g_i_connected_flag = 0;
						g_i_connected_id = 0; 
						lwip_close(sockfd);        //�ر�socket����
						//net_debug(get_debug_value(),"Client(  ) closed connection.%d ",bytes_received);
						FD_CLR(sockfd, &allset);    //�Ӽ���������ɾ����socket����
						client[i].fd = -1;        //����Ԫ�����ʼֵ����ʾû�ͻ�������
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
						break;       //���û���¿ͻ��������ݣ�����forѭ���ص�whileѭ��
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


 