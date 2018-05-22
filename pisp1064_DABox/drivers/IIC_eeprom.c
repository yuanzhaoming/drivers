#include <finsh.h>
#include <rtthread.h>
#include "iic_eeprom.h"
#include "stm32f10x.h"

static void Delay_us(int time)
{
	int i = 0;
	unsigned int j;
	for(i = 0 ; i < 8 ; i ++)
	{
	  	for(j = 0 ; j < time; j ++)
			__nop();
	}
}
	    
//////////////////////////////////////////////////////////////////////////
//			 	������		:		IIC_Init	
//			   	����		:		��ʼ��i2c
//				����		:		void
//				����		:		wit_yuan
//				ʱ��		:		2018-07-30
//				
//				Ӳ������	:       PB6 SCL
//									PB7 SDA
////////////////////////////////////////////////////////////////////////////
void eeprom_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD ;   //��©���

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_6); 	//PB6,PB7 �����
	GPIO_SetBits(GPIOB,GPIO_Pin_7); 	//PB6,PB7 �����	    
}
//////////////////////////////////////////////////////////////////////////
//			 	������		:		IIC_Start	
//			   	����		:		i2c��ʼ�ź�
//				����		:		void
//				����		:		wit_yuan
//				ʱ��		:		2018-07-30
////////////////////////////////////////////////////////////////////////////
void IIC_Start(void)
{
	SDA_OUT();     	//������һ��SCL״̬��0������1��Ҫ��SDA�ȶ����������������Ϊ1��
	IIC_SDA=1;
	IIC_SCL=1;
	Delay_us(2);
		  	  
 	IIC_SDA=0;		//SDA�����ݱ仯���Ӷ����Ա�֤��������start�źš� 
	Delay_us(2);
	
	IIC_SCL=0;		//��I2C����ǯס����һ��ʱ��SDA��������ߵ͵�ƽ��
	Delay_us(4);
}	  
//////////////////////////////////////////////////////////////////////////
//			 	������		:		IIC_Stop	
//			   	����		:		i2c�����ź�
//				����		:		void
//				����		:		wit_yuan
//				ʱ��		:		2014-11-07
////////////////////////////////////////////////////////////////////////////
void IIC_Stop(void)
{
	SDA_OUT();
	IIC_SDA=0;		//��һ��״̬��SCL=0,���״̬����SDA=0,Ŀ������״̬�ܷ�ת
	Delay_us(2);	

	IIC_SCL=1;		//��ʱ������SCL=1,Ȼ��Ϳ����������ȶ��ڸ�״̬�¡�
 	Delay_us(2);	

	IIC_SDA=1;
	Delay_us(4);							   	
}
//////////////////////////////////////////////////////////////////////////
//			 	������		:		IIC_Wait_Ack	
//			   	����		:		�ȴ�i2c��Ӧ���ź�
//				����		:		void
//				����		:		wit_yuan
//				ʱ��		:		2014-11-07
////////////////////////////////////////////////////////////////////////////
//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
u8 IIC_Wait_Ack(void)
{
	u16 ucErrTime=0;

	IIC_SCL=0;
	Delay_us(2);							  		   		

	SDA_IN();      				//SDA����Ϊ����  	   
		 
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;			//��ʱ���������ݴ���������
		}
	}
	IIC_SCL=1;
	Delay_us(1);


	IIC_SCL=0;//ʱ�����0 
	Delay_us(2);
					   
	return 0;  
} 
//////////////////////////////////////////////////////////////////////////
//			 	������		:		IIC_Ack	
//			   	����		:		����i2c��ackӦ���ź�
//				����		:		void
//				����		:		wit_yuan
//				ʱ��		:		2014-11-07
////////////////////////////////////////////////////////////////////////////
//����ACKӦ��
void IIC_Ack(void)
{
	IIC_SCL=0;
	Delay_us(2);

	//added by wit_yuan 2016-09-16
	SDA_OUT();
	IIC_SDA = 1;
	Delay_us(1);


	IIC_SDA=0;
	Delay_us(1);


	IIC_SCL=1;
	Delay_us(1);
	IIC_SCL=0;
	Delay_us(4);

	//////add 2016-09-16 by wit_yuan///////////
	IIC_SDA = 1;
	Delay_us(1);
}
//������ACKӦ��		    
void IIC_NAck(void)
{
	SDA_OUT();
	IIC_SCL=0;
	Delay_us(2);

	IIC_SDA=1;
	Delay_us(2);

	IIC_SCL=1;
	Delay_us(2);
	IIC_SCL=0;
	Delay_us(2);
}					 				     
//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��			  
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;
	//Delay_us(1);  
	SDA_OUT(); 	    
    IIC_SCL=0;//����ʱ�ӿ�ʼ���ݴ���
	Delay_us(2);
    for(t=0;t<8;t++)
    {
		if((txd&0x80)>>7)
			IIC_SDA=1;
		else
			IIC_SDA=0;
		txd<<=1; 	  
		Delay_us(2);   
		IIC_SCL=1;
		Delay_us(2); 
		IIC_SCL=0;	
		Delay_us(2);
    }	 
} 	    
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
unsigned char IIC_Read_Byte( void )
{
	unsigned char i,u_receive=0;
	SDA_IN();//SDA����Ϊ����
    for(i=0;i<8;i++ )
	{
        IIC_SCL=0; 
        Delay_us(4);
		IIC_SCL=1;
		Delay_us(1);
        u_receive<<=1;
        if(READ_SDA)
			u_receive++;   
    }

    return u_receive;
}			  



unsigned char cs4385_write(unsigned char c_slave_address7bit,unsigned char c_reg_address,unsigned char u_data)
{
	unsigned char u_wait_err = 0;

	IIC_Start();
	IIC_Send_Byte(c_slave_address7bit << 1);    
	u_wait_err |= IIC_Wait_Ack();

	IIC_Send_Byte(c_reg_address);   
	u_wait_err |= IIC_Wait_Ack();
	
	IIC_Send_Byte(u_data);
	u_wait_err |= IIC_Wait_Ack();

	IIC_Stop();	

	if(	u_wait_err == 0)
		return 0;
	return 1;
}

unsigned char cs4385_read(unsigned char c_slave_address7bit,unsigned char c_reg_address)
{	
	unsigned char u_temp;
	unsigned char u_wait_err = 0;

	IIC_Start();
	IIC_Send_Byte(c_slave_address7bit << 1);    
	u_wait_err |= IIC_Wait_Ack();

	IIC_Send_Byte(c_reg_address);    
	u_wait_err |= IIC_Wait_Ack();

	IIC_Start();
	IIC_Send_Byte((c_slave_address7bit << 1)+1);    
	u_wait_err |= IIC_Wait_Ack();
		
	u_temp = IIC_Read_Byte( );
	IIC_NAck();//����Ҫ��Ӧ	

	IIC_Stop();

	return  u_temp;
}


unsigned char iic_eeprom_at24cm02_write( unsigned int address,unsigned char *buffer,unsigned int len)
{
	unsigned char u_temp;
	unsigned char u_wait_err = 0;
	int i = 0;
	/*start*/	
	IIC_Start();
	/*device address*/
	IIC_Send_Byte((EEPROM_SLAVE_ADDRESS | ((address>>16)&0x3)) << 1);    
	u_wait_err |= IIC_Wait_Ack();

	/*destination address*/
	IIC_Send_Byte(((address >> 8) & 0xff)); 
	u_wait_err |= IIC_Wait_Ack();

	IIC_Send_Byte((address & 0xff)); 
	u_wait_err |= IIC_Wait_Ack();

 	for(i = 0 ; i < len ; i ++){
		IIC_Send_Byte(buffer[i]); 
		u_wait_err |= IIC_Wait_Ack();
	}
   	IIC_Stop();

	return (u_wait_err!=0)?1:0;
}

 
unsigned char iic_eeprom_at24cm02_read( unsigned int address,unsigned char *buffer,unsigned int len)
{
	unsigned char u_temp;
	unsigned char u_wait_err = 0;
	int i = 0;
	/*start*/	
	IIC_Start();
	/*device address*/
	IIC_Send_Byte((EEPROM_SLAVE_ADDRESS | ((address>>16)&0x3)) << 1);    
	u_wait_err |= IIC_Wait_Ack();
 	if(u_wait_err!=0){
		rt_kprintf("read error ... 1\r\n");
	}
	/*destination address*/
	IIC_Send_Byte(((address >> 8) & 0xff)); 
	u_wait_err |= IIC_Wait_Ack();
	IIC_Send_Byte((address & 0xff)); 
	u_wait_err |= IIC_Wait_Ack();

	if(u_wait_err!=0){
		rt_kprintf("read error ... 2\r\n");
	}

	/*a new start*/
 	IIC_Start();
	IIC_Send_Byte(((EEPROM_SLAVE_ADDRESS | ((address>>16)&0x3)) << 1) | 1);  
	u_wait_err |= IIC_Wait_Ack();	
	if(u_wait_err!=0){
		rt_kprintf("read error ... 3\r\n");
	}
	if(len > 1){
	   	for(i = 0 ; i < len-1 ; i ++){
			buffer[i] = IIC_Read_Byte( );
			IIC_Ack();
		}
		buffer[len-1] = IIC_Read_Byte( );
	    IIC_NAck();//����Ҫ��Ӧ	
	}
	else{
		buffer[0] = IIC_Read_Byte( );
  		IIC_NAck();//����Ҫ��Ӧ
	}	

   	IIC_Stop();

	return (u_wait_err!=0)?1:0;
}


static void e2_read_disp( unsigned int address )
{
	int len = 256;
	char buffer[256];
	unsigned char u_temp;
	unsigned char u_wait_err = 0;
	int count = 0;
	int i = 0;
 
	/*start*/	
	IIC_Start();
	/*device address*/
	IIC_Send_Byte((EEPROM_SLAVE_ADDRESS | ((address>>16)&0x3)) << 1);    
	u_wait_err |= IIC_Wait_Ack();
 	if(u_wait_err!=0){
		rt_kprintf("read error ... 1\r\n");
	}
	/*destination address*/
	IIC_Send_Byte(((address >> 8) & 0xff)); 
	u_wait_err |= IIC_Wait_Ack();
	IIC_Send_Byte((address & 0xff)); 
	u_wait_err |= IIC_Wait_Ack();

	if(u_wait_err!=0){
		rt_kprintf("read error ... 2\r\n");
	}

	/*a new start*/
 	IIC_Start();
	IIC_Send_Byte(((EEPROM_SLAVE_ADDRESS | ((address>>16)&0x3)) << 1) | 1);  
	u_wait_err |= IIC_Wait_Ack();	
	if(u_wait_err!=0){
		rt_kprintf("read error ... 3\r\n");
	}
	 
   	for(i = 0 ; i < len-1 ; i ++){
		buffer[i] = IIC_Read_Byte( );
		IIC_Ack();
	}
	buffer[len-1] = IIC_Read_Byte( );
    IIC_NAck();//����Ҫ��Ӧ	
   	IIC_Stop();

	for(i = 0 ; i < 256 ; i ++){
		rt_kprintf("%02x ",buffer[i]);	
		count++;
		if((count % 16)==0){
			rt_kprintf("\r\n");
		} 	
	}
	 
}
FINSH_FUNCTION_EXPORT(e2_read_disp, read eeprom data with specified address);

