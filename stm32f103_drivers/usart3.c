/***************************copyright ythuitong by wit_yuan 2014-10-30********************/
//////////////////////////////////////////////////////////////////////////////////////////
//				   		文件名			:		usart3.c
//						功能			:       将数据传送到服务器
//						作者			:		wit_yuan
//						编写时间		:		2014-10-30
//						更新时间		:		2014-10-30
//						修改内容		:		无
//						硬件连接		:	    rx------------PA10
//												tx------------PA9
/////////////////////////////////////////////////////////////////////////////////////////////
#include "includes.h"
#include <stdarg.h>
#include "stm32f103.h"
//m72d相应的内容信息
extern gsm_info my_gsm_info;

extern	uint8_t  gsm_message_center[19];

extern Controler my_controler;
extern GSM_STATUS my_gsm_status;
//////////////////////////////////////////////////////////////////////////////////////////////
// 				函数名			：			USART3_Config
// 				功能  			：			底层硬件的初始化
// 				参数  			：			无
// 				返回  			: 			无
//				作者			:			wit_yuan
// 				编写时间  		：			2014-10-30
//				修改时间		:			2014-10-30
//				修改内容		:			无
//////////////////////////////////////////////////////////////////////////////////////////////
void USART3_Config(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;  
	
	/*使能复用时钟，使能串口3时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE , ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3 , ENABLE);
	/*PB10 做TX*/ 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    /*PB11 做RX */ 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    /*PB3 做电源 */ 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); 
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;	
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);
	//GPIO_ResetBits(GPIOB,GPIO_Pin_3);
	GPIO_SetBits(GPIOB,GPIO_Pin_3);
    /*PE1 做BOOT */ 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
   	GPIO_SetBits(GPIOE,GPIO_Pin_1);
	/*配置USART3的工作模式*/
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;								
	USART_InitStructure.USART_StopBits = USART_StopBits_1;								
	USART_InitStructure.USART_Parity = USART_Parity_No;								
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;							
    USART_Init(USART3, &USART_InitStructure);//初始化外设 USARTx寄存器
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); //允许中断方式
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure); 
    USART_Cmd(USART3, ENABLE);
}

/////////////////////////////////////////////////////////////////////////////
// 				函数名			：			USART3_Putc
// 				功能  			：			传输字节数据
// 				参数			:			
// 				作者	  		：			wit_yuan
//				编写时间		：			2014-10-30
//				修改时间		:			2014-10-30
//				修改内容		:			无
//////////////////////////////////////////////////////////////////////////////
void USART3_Putc(uint8_t c)
{
    USART_SendData(USART3, c);
    while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET );
}
/////////////////////////////////////////////////////////////////////////////
// 				函数名			：			USART3_Puts
// 				功能  			：			传输字节数据
// 				参数			:			
// 				作者	  		：			wit_yuan
//				编写时间		：			2014-10-30
//				修改时间		:			2014-10-30
//				修改内容		:			无
//////////////////////////////////////////////////////////////////////////////
void USART3_Puts(char * str)
{ 
    while(*str)
    {
        USART_SendData(USART3, *str++);
        while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
    }
}











/*
 * 函数名：itoa
 * 描述  ：将整形数据转换成字符串
 * 输入  ：-radix =10 表示10进制，其他结果为0
 *         -value 要转换的整形数
 *         -buf 转换后的字符串
 *         -radix = 10
 * 输出  ：无
 * 返回  ：无
 * 调用  ：被USART1_printf()调用
 */
static char *itoa(int value, char *string, int radix)
{
    int     i, d;
    int     flag = 0;
    char    *ptr = string;

    /* This implementation only works for decimal numbers. */
    if (radix != 10)
    {
        *ptr = 0;
        return string;
    }

    if (!value)
    {
        *ptr++ = 0x30;
        *ptr = 0;
        return string;
    }

    /* if this is a negative value insert the minus sign. */
    if (value < 0)
    {
        *ptr++ = '-';

        /* Make the value positive. */
        value *= -1;
    }

    for (i = 10000; i > 0; i /= 10)
    {
        d = value / i;

        if (d || flag)
        {
            *ptr++ = (char)(d + 0x30);
            value -= (d * i);
            flag = 1;
        }
    }

    /* Null terminate the string. */
    *ptr = 0;

    return string;

} /* NCL_Itoa */

char *i16toa(int value, char *string, int radix)
{
    int     i, d;
    int     flag = 0;
    char    *ptr = string;

    /* This implementation only works for decimal numbers. */
    if (radix != 16)
    {
        *ptr = 0;
        return string;
    }

    if (!value)
    {
        *ptr++ = 0x30;
        *ptr = 0;
        return string;
    }

    /* if this is a negative value insert the minus sign. */
    if (value < 0)
    {
        *ptr++ = '-';

        /* Make the value positive. */
        value *= -1;
    }

    for (i = 0x1000; i > 0; i /= 16)
    {
        d = value / i;

        if (d || flag)
        {
            if(d>=10)  *ptr++ = (char)(d + 0x37);
		    else       *ptr++ = (char)(d + 0x30);
            value -= (d * i);
            flag = 1;
        }
    }

    /* Null terminate the string. */
    *ptr = 0;

    return string;

} /* NCL_Itoa */
/*
 * 函数名：USART3_printf
 * 描述  ：格式化输出，类似于C库中的printf，但这里没有用到C库
 * 输入  ：-USARTx 串口通道，这里只用到了串口1，即USART1
 *		     -Data   要发送到串口的内容的指针
 *			   -...    其他参数
 * 输出  ：无
 * 返回  ：无 
 * 调用  ：外部调用
 *         典型应用USART1_printf( USART1, "\r\n this is a demo \r\n" );
 *            		 USART1_printf( USART1, "\r\n %d \r\n", i );
 *            		 USART1_printf( USART1, "\r\n %s \r\n", j );
 */
void USART3_printf(USART_TypeDef* USARTx, uint8_t *Data,...)
{
	const char *s;
  int d;   
  char buf[16];

  va_list ap;
  va_start(ap, Data);

	while ( *Data != 0)     // 判断是否到达字符串结束符
	{				                          
		if ( *Data == 0x5c )  //'\'
		{									  
			switch ( *++Data )
			{
				case 'r':							          //回车符
					USART_SendData(USARTx, 0x0d);
					Data ++;
					break;

				case 'n':							          //换行符
					USART_SendData(USARTx, 0x0a);	
					Data ++;
					break;
				
				default:
					Data ++;
				    break;
			}			 
		}
		else if ( *Data == '%')
		{									  //
			switch ( *++Data )
			{				
				case 's':										  //字符串
					s = va_arg(ap, const char *);
          for ( ; *s; s++) 
					{
						USART_SendData(USARTx,*s);
						while( USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET );
          }
					Data++;
          break;

        case 'd':										//十进制
          d = va_arg(ap, int);
          itoa(d, buf, 10);
          for (s = buf; *s; s++) 
					{
						USART_SendData(USARTx,*s);
						while( USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET );
          }
					Data++;
          break;
				 default:
						Data++;
				    break;
			}		 
		} /* end of else if */
		else USART_SendData(USARTx, *Data++);
		while( USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET );
	}
}
/////////////////////////////////////////////////////////////////////////////
// 				函数名			：			USART3_IRQHandler
// 				功能  			：			串口3中断，接收m72d的数据
// 				参数			:			
// 				作者	  		：			wit_yuan
//				编写时间		：			2014-10-30
//				修改时间		:			2014-11-03
//				修改内容		:			无
//////////////////////////////////////////////////////////////////////////////
u8 usart3_buffer[1024];

void USART3_IRQHandler(void)
{
	u8 data_temp;
	static int usart3_i;
	int i = 0;
	uint8_t message_center[15];
	uint8_t temp[20];
//	OSIntEnter(); 
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(USART3,USART_IT_RXNE);
		data_temp = USART_ReceiveData(USART3);

		if(usart3_i == 1024)
		{
		   usart3_i = 0;
		}	  
		//可以通过一个判断条件，进行调试打印信息。
		if(my_controler.debug == 1)
		{
			USART_SendData(USART1,data_temp);
		}

		usart3_buffer[usart3_i] = data_temp;

		if((usart3_buffer[usart3_i] == '\n'))
		{
			//检测sim卡是否插上					 
			if(strncmp((char *)usart3_buffer,"+CPIN: READY",strlen("+CPIN: READY")) == 0)
			{
				my_gsm_info.sim_on_off = 1;		
			}
			 //检测是否注册上网络
			if(strncmp((char *)usart3_buffer,"+CREG: 0,1",strlen("+CREG: 0,1")) == 0)
			{
				my_gsm_info.have_net = 1;		
			}
			if(strncmp((char *)usart3_buffer,"+CREG: 0,5",strlen("+CREG: 0,5")) == 0)
			{
				my_gsm_info.have_net = 1;		
			}
			//检测sim卡接收到服务器的数据没有  received
			if(strncmp((char *)usart3_buffer,"OK",strlen("OK")) == 0)
			{
				my_gsm_info.ok = 1;	
			}
			//接收到sim卡模块返回的时间数据
			#if M72D_MODULE
			if(strncmp((char *)usart3_buffer,"+CCLK: \"",strlen("+CCLK: \"")) == 0 )
			{
				handle_gsm_time(&usart3_buffer[strlen("+CCLK: \"")]);	
			}
			#endif
			#if SIM900A_MODULE
			if(strncmp((char *)&usart3_buffer[37],"UTC(NIST) *",strlen("UTC(NIST) *")) == 0 )
			{
				handle_gsm_time(&usart3_buffer[6]);	
			}

			if(strncmp((char *)&usart3_buffer,"SDATIME",strlen("SDATIME")) == 0 )
			{
				handle_gsm_time(usart3_buffer);	
			}

			#endif

			//检测sim卡的信号状态
			if(strncmp((char *)usart3_buffer,"+CSQ: ",strlen("+CSQ: ")) == 0)
			{
				if(usart3_buffer[7] == ',')
				{
				   my_gsm_info.sim_signal_strength = (usart3_buffer[6] - '0') * 1;
				}
				if(usart3_buffer[8] == ',')
				{
				   my_gsm_info.sim_signal_strength = (usart3_buffer[6] - '0') * 10 + (usart3_buffer[7] - '0') * 1;
				}			 	
			}
			//检测sim卡接收到服务器的数据没有  received
			if(strncmp((char *)usart3_buffer,"received",strlen("received")) == 0)
			{
				my_gsm_info.returned = 1;	
			}
			//拔掉了sim卡
			if(strncmp((char *)usart3_buffer,"+CPIN: NOT READY",sizeof("+CPIN: NOT READY") - 1) == 0  )
			{
				my_gsm_info.sim_on_off = 0;
			}
			//收到返回ok的指令了
			if(strncmp((char *)usart3_buffer,"SEND OK",strlen("SEND OK")) == 0  )
			{
			//	my_gsm_status.send_ok = 1;
				
			}
			//收到返回已经连接的信息了
			if(strncmp((char *)usart3_buffer,"ALREADY CONNECT",strlen("ALREADY CONNECT")) == 0  )
			{
				my_gsm_status.already_connect = 1;
			}

			//收到返回已经连接的信息了
			if(strncmp((char *)usart3_buffer,"+CMGS:",strlen("+CMGS:")) == 0  )
			{
				my_gsm_status.message_ok = 1;
			}
						
			//未检查到sim卡
			if(strncmp((char *)usart3_buffer,"+CME ERROR:",sizeof("+CME ERROR:") - 1) == 0  )
			{
				my_gsm_info.sim_on_off = 0;			
			}

			//短信中心号码获取 2014-08-27	8613010161500    089168  683110101605F0
			if(strncmp((char *)usart3_buffer,"+CSCA: \"+",strlen("+CSCA: \"+")) == 0  )
			{
				//处理短信中心号码	
				for(i = 0 ; i < 13 ; i ++)
				{
				 	message_center[ i ] = usart3_buffer[9 + i];
				}
				message_center[ 13 ] = 'F';
				 //将反转数据传递给temp
				for(i = 0 ;i < 13 ; i = i + 2)
				{
					temp[i + 1] = message_center[i];
					temp[i] = message_center[i + 1];
				}
				gsm_message_center[0] = '0';
				gsm_message_center[1] = '8';
				gsm_message_center[2] = '9';
				gsm_message_center[3] = '1';

				for(i = 0 ;i < 14 ; i ++)
				{
					gsm_message_center[i + 4] = temp[i];         	
				}
				gsm_message_center[18] = '\0';

				my_gsm_info.csca = 1;

			}

			usart3_i = 0;
			memset(usart3_buffer,'\0', sizeof(usart3_buffer));
		} 
		else
		{
		  	usart3_i ++ ;
		}                                                            
	}
//	OSIntExit();
	USART_ClearFlag(USART3,  USART_FLAG_RXNE);
}	

/***************************copyright ythuitong by wit_yuan 2014-10-24******end of file**************/

