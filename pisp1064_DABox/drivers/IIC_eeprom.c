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
//			 	函数名		:		IIC_Init	
//			   	功能		:		初始化i2c
//				参数		:		void
//				作者		:		wit_yuan
//				时间		:		2018-07-30
//				
//				硬件连线	:       PB6 SCL
//									PB7 SDA
////////////////////////////////////////////////////////////////////////////
void eeprom_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD ;   //开漏输出

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_6); 	//PB6,PB7 输出高
	GPIO_SetBits(GPIOB,GPIO_Pin_7); 	//PB6,PB7 输出高	    
}
//////////////////////////////////////////////////////////////////////////
//			 	函数名		:		IIC_Start	
//			   	功能		:		i2c起始信号
//				参数		:		void
//				作者		:		wit_yuan
//				时间		:		2018-07-30
////////////////////////////////////////////////////////////////////////////
void IIC_Start(void)
{
	SDA_OUT();     	//由于上一个SCL状态是0或者是1，要让SDA稳定输出，都可以设置为1。
	IIC_SDA=1;
	IIC_SCL=1;
	Delay_us(2);
		  	  
 	IIC_SDA=0;		//SDA线数据变化，从而可以保证发出的是start信号。 
	Delay_us(2);
	
	IIC_SCL=0;		//将I2C总线钳住，下一个时间SDA可以输出高低电平。
	Delay_us(4);
}	  
//////////////////////////////////////////////////////////////////////////
//			 	函数名		:		IIC_Stop	
//			   	功能		:		i2c结束信号
//				参数		:		void
//				作者		:		wit_yuan
//				时间		:		2014-11-07
////////////////////////////////////////////////////////////////////////////
void IIC_Stop(void)
{
	SDA_OUT();
	IIC_SDA=0;		//上一个状态的SCL=0,这个状态设置SDA=0,目的是让状态能反转
	Delay_us(2);	

	IIC_SCL=1;		//该时刻设置SCL=1,然后就可以让数据稳定在改状态下。
 	Delay_us(2);	

	IIC_SDA=1;
	Delay_us(4);							   	
}
//////////////////////////////////////////////////////////////////////////
//			 	函数名		:		IIC_Wait_Ack	
//			   	功能		:		等待i2c的应答信号
//				参数		:		void
//				作者		:		wit_yuan
//				时间		:		2014-11-07
////////////////////////////////////////////////////////////////////////////
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 IIC_Wait_Ack(void)
{
	u16 ucErrTime=0;

	IIC_SCL=0;
	Delay_us(2);							  		   		

	SDA_IN();      				//SDA设置为输入  	   
		 
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;			//超时，表明数据传输有问题
		}
	}
	IIC_SCL=1;
	Delay_us(1);


	IIC_SCL=0;//时钟输出0 
	Delay_us(2);
					   
	return 0;  
} 
//////////////////////////////////////////////////////////////////////////
//			 	函数名		:		IIC_Ack	
//			   	功能		:		产生i2c的ack应答信号
//				参数		:		void
//				作者		:		wit_yuan
//				时间		:		2014-11-07
////////////////////////////////////////////////////////////////////////////
//产生ACK应答
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
//不产生ACK应答		    
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
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;
	//Delay_us(1);  
	SDA_OUT(); 	    
    IIC_SCL=0;//拉低时钟开始数据传输
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
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
unsigned char IIC_Read_Byte( void )
{
	unsigned char i,u_receive=0;
	SDA_IN();//SDA设置为输入
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
	IIC_NAck();//不需要响应	

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
	    IIC_NAck();//不需要响应	
	}
	else{
		buffer[0] = IIC_Read_Byte( );
  		IIC_NAck();//不需要响应
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
    IIC_NAck();//不需要响应	
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

