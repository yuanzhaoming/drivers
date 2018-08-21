/********************************copyright ythuitong by wit_yuan**************************/

#include "ds1302.h"	 
#include "stm32f10x.h"
#include "timer2.h"

#define USE_UCOSII 	1

//////////////////////////////////////////////////////////////////////////
//			 	函数名		:		ds1302_init		
//			   	功能		:		ds1302初始化部分
//				参数		:		void
//				作者		:		wit_yuan
//				时间		:		2014-08-08
////////////////////////////////////////////////////////////////////////////
void ds1302_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE );	  
	GPIO_InitStructure.GPIO_Pin = (DS1302_SCK_PIN | DS1302_IO_PIN | DS1302_CE_PIN);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;    
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DS1302_PORT, &GPIO_InitStructure);

	DS1302_WriteByte(WrControl,0x00);  //关闭写保护，可以写入数据了	

	Delay_10us(10);
	if(DS1302_ReadByte(RdTrickleCharge) != 0xA6)
	{
		Delay_10us(10);
		DS1302_WriteByte(WrTrickleCharge,0xA6);
	}	
	Delay_10us(10);
	DS1302_WriteByte(WrControl,0x80);  //开启写保护，禁止写入数据
}


void DS1302_IO_OUT()
{
    GPIO_InitTypeDef GPIO_InitStructure;                                                                
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;           
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			
    GPIO_Init(GPIOE, &GPIO_InitStructure);         
}

void DS1302_IO_IN()
{
    GPIO_InitTypeDef GPIO_InitStructure;                                                                
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;           
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;      
			
    GPIO_Init(GPIOE, &GPIO_InitStructure);         
}



//DS1302写入8bit
void DS1302_Write8bit(unsigned char code)
{
	unsigned char i;
	DS1302_IO_OUT();						//输出模式
	DS1302_CLRSCK();						//SCK = 0
	for(i=0;i<8;i++)
	{
		  Delay_10us(5);
		  if(code&0x01) 
				(DS1302_SETIO());			//I/0  = 1
		  else 
				(DS1302_CLRIO());			//I/0 = 0
		  Delay_10us(5);
		  
		  DS1302_SETSCK();				//SCLK  = 1
		  Delay_10us(5);
															
		  DS1302_CLRSCK();				//SCLK = 0
		  code = code >> 1;
	}
}

//DS1302读取8bit的数据
unsigned char DS1302_Read8bit(void)
{
	unsigned char i,code;
	unsigned char temp;
	DS1302_IO_IN();
	code = 0;
	DS1302_CLRSCK();						//SCLK = 0

	Delay_10us(5);

	for(i=0;i<8;i++)
	{
			
		  code = code >>1;
			
		  if(DS1302_READ_SDA())
			{
				code = code | 0x80;
			}	
					
				
					
		  Delay_10us(5);
		  DS1302_SETSCK();			//SCLK = 1
		  Delay_10us(5);
	
		  DS1302_CLRSCK();			//SCLK = 0

	}
	
	temp = code /16;
	code = code % 16;
	code = code + temp * 10;			//数据的相关转换
	
	return code;
}



//读取DS1302指定的1Byte
unsigned char DS1302_ReadByte(unsigned char con)
{
	unsigned char code;
	DS1302_CLRCE();		   //关闭DS1302					//CE = 0
	Delay_10us(5);       
	DS1302_CLRSCK();													//SCLK = 0
	Delay_10us(5);
	DS1302_SETCE();          //使能DS1302			//CE = 1;
	Delay_10us(5);
	DS1302_Write8bit(con);   //读取代码				//发送地址
	code = DS1302_Read8bit();  //返回读取数字
	
	//printf("code = %d\r\n" ,code );
	Delay_10us(5);
	DS1302_SETSCK();													//SCLK = 1
	Delay_10us(5);
	DS1302_CLRCE();         //关闭DS1302			//CE = 0
	return code;
}


//写DS1302指定的1Byte
void DS1302_WriteByte(unsigned char con,unsigned char code)
{
	DS1302_CLRCE();         		//关闭DS1302		//CE = 0
	Delay_10us(5);
	DS1302_CLRSCK();														//SCK = 0
	Delay_10us(5);
	
	DS1302_SETCE();          		//使能DS1302		//CE = 1
	Delay_10us(5);
	DS1302_Write8bit(con);     	//写控制命令		//发送地址
	DS1302_Write8bit(code); 		//写入数据			//发送数据
	Delay_10us(5);
	DS1302_SETSCK();
	Delay_10us(5);
	DS1302_CLRCE();          //关闭DS1302

}


//写入时间
void DS1302_WriteTime(TIME_TypeDef* time)
{
	DS1302_WriteByte(WrControl,0x00);  		//关闭写保护，可以写入数据
		
	DS1302_WriteByte(WrYear,time->year);
	DS1302_WriteByte(WrMonth,time->month);
	DS1302_WriteByte(WrDate,time->date);
	DS1302_WriteByte(WrWeek,time->week);
	DS1302_WriteByte(WrHour,time->hour);
	DS1302_WriteByte(WrMin,time->min);
	DS1302_WriteByte(WrSec,time->sec);

	DS1302_WriteByte(WrControl,0x80);  		//开启写保护，禁止写入数据

}

//读出时间
void DS1302_ReadTime(TIME_TypeDef* time)
{
	time->year 	= DS1302_ReadByte(RdYear);
	time->month = DS1302_ReadByte(RdMonth);	
	time->date 	= DS1302_ReadByte(RdDate);
	time->week 	= DS1302_ReadByte(RdWeek);

	time->hour 	= DS1302_ReadByte(RdHour);
	time->min 	= DS1302_ReadByte(RdMin);
	time->sec 	= DS1302_ReadByte(RdSec);
}
	
	
unsigned char drv_time[20]="\0";

void time_convert(TIME_TypeDef *time_get)
{
	drv_time[0] = '2';															//2
	drv_time[1] = '0';															//0
	drv_time[2] = time_get->year / 10 + '0';		//1
	drv_time[3] = time_get->year % 10 + '0';        //4
	drv_time[4] = '-';	                            //-
	drv_time[5] = time_get->month / 10 + '0';				//0
	drv_time[6] = time_get->month % 10 + '0';       //4
	drv_time[7] = '-';															//-
	
	drv_time[8] = time_get->date / 10 + '0';        //1
	drv_time[9] = time_get->date % 10 + '0';	      //0
	drv_time[10] = ' ';                             // 
	drv_time[11] = time_get->hour / 10 + '0';       //1
	drv_time[12] = time_get->hour % 10 + '0';       //4
	drv_time[13] = ':';															//:
	drv_time[14] = time_get->min / 10 + '0';        //2
	drv_time[15] = time_get->min % 10 + '0';      	//1
	drv_time[16] = ':';															//:
	drv_time[17] = time_get->sec / 10 + '0';		    //3
	drv_time[18] = time_get->sec % 10 + '0';	      //0
	drv_time[19] = '\0';															// 
	
	printf("%s\r\n",drv_time);
}

//显示当前时间
void showDs1302Time()
{
	TIME_TypeDef show_time;	
	DS1302_ReadTime(&show_time);	
	time_convert(&show_time);	
}


void getDs1302Time(u8 *time)
{
	TIME_TypeDef time_get;
	
	#if USE_UCOSII
	OSSchedLock();		
	DS1302_ReadTime(&time_get);
	OSSchedUnlock();
	#endif
	

	time[0] = '2';														
	time[1] = '0';														
	time[2] = time_get.year / 10 + '0';		
	time[3] = time_get.year % 10 + '0';                                 
	time[4] = time_get.month / 10 + '0';				
	time[5] = time_get.month % 10 + '0';       
	time[6] = time_get.date / 10 + '0';       
	time[7] = time_get.date % 10 + '0';	      
                           
	time[8] = time_get.hour / 10 + '0';       
	time[9] = time_get.hour % 10 + '0';       								
	time[10] = time_get.min / 10 + '0';        
	time[11] = time_get.min % 10 + '0';      														
	time[12] = time_get.sec / 10 + '0';		    
	time[13] = time_get.sec % 10 + '0';	     
	time[14] = '\0';
}


/************************************end of file 2014-08-08*********************************/





