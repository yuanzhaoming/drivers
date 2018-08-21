/***************************copyright ythuitong by wit_yuan 2014-10-30********************/
//////////////////////////////////////////////////////////////////////////////////////////
//				   		�ļ���			:		usart3.c
//						����			:       �����ݴ��͵�������
//						����			:		wit_yuan
//						��дʱ��		:		2014-10-30
//						����ʱ��		:		2014-10-30
//						�޸�����		:		��
//						Ӳ������		:	    rx------------PA10
//												tx------------PA9
/////////////////////////////////////////////////////////////////////////////////////////////
#include "includes.h"
#include <stdarg.h>
#include "stm32f103.h"
//m72d��Ӧ��������Ϣ
extern gsm_info my_gsm_info;

extern	uint8_t  gsm_message_center[19];

extern Controler my_controler;
extern GSM_STATUS my_gsm_status;
//////////////////////////////////////////////////////////////////////////////////////////////
// 				������			��			USART3_Config
// 				����  			��			�ײ�Ӳ���ĳ�ʼ��
// 				����  			��			��
// 				����  			: 			��
//				����			:			wit_yuan
// 				��дʱ��  		��			2014-10-30
//				�޸�ʱ��		:			2014-10-30
//				�޸�����		:			��
//////////////////////////////////////////////////////////////////////////////////////////////
void USART3_Config(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;  
	
	/*ʹ�ܸ���ʱ�ӣ�ʹ�ܴ���3ʱ��*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE , ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3 , ENABLE);
	/*PB10 ��TX*/ 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    /*PB11 ��RX */ 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    /*PB3 ����Դ */ 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); 
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;	
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);
	//GPIO_ResetBits(GPIOB,GPIO_Pin_3);
	GPIO_SetBits(GPIOB,GPIO_Pin_3);
    /*PE1 ��BOOT */ 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
   	GPIO_SetBits(GPIOE,GPIO_Pin_1);
	/*����USART3�Ĺ���ģʽ*/
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;								
	USART_InitStructure.USART_StopBits = USART_StopBits_1;								
	USART_InitStructure.USART_Parity = USART_Parity_No;								
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;							
    USART_Init(USART3, &USART_InitStructure);//��ʼ������ USARTx�Ĵ���
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); //�����жϷ�ʽ
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure); 
    USART_Cmd(USART3, ENABLE);
}

/////////////////////////////////////////////////////////////////////////////
// 				������			��			USART3_Putc
// 				����  			��			�����ֽ�����
// 				����			:			
// 				����	  		��			wit_yuan
//				��дʱ��		��			2014-10-30
//				�޸�ʱ��		:			2014-10-30
//				�޸�����		:			��
//////////////////////////////////////////////////////////////////////////////
void USART3_Putc(uint8_t c)
{
    USART_SendData(USART3, c);
    while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET );
}
/////////////////////////////////////////////////////////////////////////////
// 				������			��			USART3_Puts
// 				����  			��			�����ֽ�����
// 				����			:			
// 				����	  		��			wit_yuan
//				��дʱ��		��			2014-10-30
//				�޸�ʱ��		:			2014-10-30
//				�޸�����		:			��
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
 * ��������itoa
 * ����  ������������ת�����ַ���
 * ����  ��-radix =10 ��ʾ10���ƣ��������Ϊ0
 *         -value Ҫת����������
 *         -buf ת������ַ���
 *         -radix = 10
 * ���  ����
 * ����  ����
 * ����  ����USART1_printf()����
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
 * ��������USART3_printf
 * ����  ����ʽ�������������C���е�printf��������û���õ�C��
 * ����  ��-USARTx ����ͨ��������ֻ�õ��˴���1����USART1
 *		     -Data   Ҫ���͵����ڵ����ݵ�ָ��
 *			   -...    ��������
 * ���  ����
 * ����  ���� 
 * ����  ���ⲿ����
 *         ����Ӧ��USART1_printf( USART1, "\r\n this is a demo \r\n" );
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

	while ( *Data != 0)     // �ж��Ƿ񵽴��ַ���������
	{				                          
		if ( *Data == 0x5c )  //'\'
		{									  
			switch ( *++Data )
			{
				case 'r':							          //�س���
					USART_SendData(USARTx, 0x0d);
					Data ++;
					break;

				case 'n':							          //���з�
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
				case 's':										  //�ַ���
					s = va_arg(ap, const char *);
          for ( ; *s; s++) 
					{
						USART_SendData(USARTx,*s);
						while( USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET );
          }
					Data++;
          break;

        case 'd':										//ʮ����
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
// 				������			��			USART3_IRQHandler
// 				����  			��			����3�жϣ�����m72d������
// 				����			:			
// 				����	  		��			wit_yuan
//				��дʱ��		��			2014-10-30
//				�޸�ʱ��		:			2014-11-03
//				�޸�����		:			��
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
		//����ͨ��һ���ж����������е��Դ�ӡ��Ϣ��
		if(my_controler.debug == 1)
		{
			USART_SendData(USART1,data_temp);
		}

		usart3_buffer[usart3_i] = data_temp;

		if((usart3_buffer[usart3_i] == '\n'))
		{
			//���sim���Ƿ����					 
			if(strncmp((char *)usart3_buffer,"+CPIN: READY",strlen("+CPIN: READY")) == 0)
			{
				my_gsm_info.sim_on_off = 1;		
			}
			 //����Ƿ�ע��������
			if(strncmp((char *)usart3_buffer,"+CREG: 0,1",strlen("+CREG: 0,1")) == 0)
			{
				my_gsm_info.have_net = 1;		
			}
			if(strncmp((char *)usart3_buffer,"+CREG: 0,5",strlen("+CREG: 0,5")) == 0)
			{
				my_gsm_info.have_net = 1;		
			}
			//���sim�����յ�������������û��  received
			if(strncmp((char *)usart3_buffer,"OK",strlen("OK")) == 0)
			{
				my_gsm_info.ok = 1;	
			}
			//���յ�sim��ģ�鷵�ص�ʱ������
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

			//���sim�����ź�״̬
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
			//���sim�����յ�������������û��  received
			if(strncmp((char *)usart3_buffer,"received",strlen("received")) == 0)
			{
				my_gsm_info.returned = 1;	
			}
			//�ε���sim��
			if(strncmp((char *)usart3_buffer,"+CPIN: NOT READY",sizeof("+CPIN: NOT READY") - 1) == 0  )
			{
				my_gsm_info.sim_on_off = 0;
			}
			//�յ�����ok��ָ����
			if(strncmp((char *)usart3_buffer,"SEND OK",strlen("SEND OK")) == 0  )
			{
			//	my_gsm_status.send_ok = 1;
				
			}
			//�յ������Ѿ����ӵ���Ϣ��
			if(strncmp((char *)usart3_buffer,"ALREADY CONNECT",strlen("ALREADY CONNECT")) == 0  )
			{
				my_gsm_status.already_connect = 1;
			}

			//�յ������Ѿ����ӵ���Ϣ��
			if(strncmp((char *)usart3_buffer,"+CMGS:",strlen("+CMGS:")) == 0  )
			{
				my_gsm_status.message_ok = 1;
			}
						
			//δ��鵽sim��
			if(strncmp((char *)usart3_buffer,"+CME ERROR:",sizeof("+CME ERROR:") - 1) == 0  )
			{
				my_gsm_info.sim_on_off = 0;			
			}

			//�������ĺ����ȡ 2014-08-27	8613010161500    089168  683110101605F0
			if(strncmp((char *)usart3_buffer,"+CSCA: \"+",strlen("+CSCA: \"+")) == 0  )
			{
				//����������ĺ���	
				for(i = 0 ; i < 13 ; i ++)
				{
				 	message_center[ i ] = usart3_buffer[9 + i];
				}
				message_center[ 13 ] = 'F';
				 //����ת���ݴ��ݸ�temp
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

