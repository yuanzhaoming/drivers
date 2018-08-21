#ifndef _STM32F103_DS1302_H_
#define _STM32F103_DS1302_H_


//*****************DS1302控制命令*******************
#define   RdSec  							0x81
#define   RdMin  							0x83
#define   RdHour  						0x85
#define   RdDate  						0x87
#define   RdMonth  						0x89
#define   RdWeek  						0x8b
#define   RdYear  						0x8d
#define   RdControl          	0x8f
#define   RdTrickleCharge 		0x91
#define   RdClockBurst  			0xbf
#define   WrSec  							0x80
#define   WrMin  							0x82
#define   WrHour  						0x84
#define   WrDate  						0x86
#define   WrMonth  						0x88
#define   WrWeek  						0x8a
#define   WrYear  						0x8c
#define   WrControl         	0x8e
#define   WrTrickleCharge 		0x90
#define   WrClockBurst  			0xbe


//相对应的IO口配置

#define DS1302_PORT GPIOE

#define DS1302_SCK_PIN GPIO_Pin_10		//对应的IO口
#define DS1302_IO_PIN GPIO_Pin_11
#define DS1302_CE_PIN GPIO_Pin_12



#define DS1302_CLRSCK() (GPIO_ResetBits(GPIOE, GPIO_Pin_10))		//寄存器IO口操作状态
#define DS1302_SETSCK() (GPIO_SetBits(GPIOE, GPIO_Pin_10))

#define DS1302_CLRIO() (GPIO_ResetBits(GPIOE, GPIO_Pin_11) )
#define DS1302_SETIO() (GPIO_SetBits(GPIOE, GPIO_Pin_11)  )

#define DS1302_CLRCE() (GPIO_ResetBits(GPIOE, GPIO_Pin_12))
#define DS1302_SETCE() (GPIO_SetBits(GPIOE, GPIO_Pin_12))


void DS1302_IO_OUT(void );
void DS1302_IO_IN( void);


//#define DS1302_IO_IN()  DS1302_IO_IN()  //操作输入输出状态
//#define DS1302_IO_OUT() DS1302_IO_OUT()

#define DS1302_READ_SDA()    (GPIO_ReadInputDataBit(DS1302_PORT, DS1302_IO_PIN))



//定义时间结构体
typedef struct
{
	unsigned char  year;
	unsigned char month;
	unsigned char date;
	unsigned char week;
	unsigned char hour;
	unsigned char min;
	unsigned char sec;
}TIME_TypeDef;	

//内部函数
void DS1302_Write8bit(unsigned char code);
unsigned char DS1302_Read8bit(void);
//外部函数
extern void ds1302_init (void);
extern unsigned char DS1302_ReadByte(unsigned char con);
extern void DS1302_WriteByte(unsigned char con,unsigned char code);

extern void DS1302_WriteTime(TIME_TypeDef* time);
extern void DS1302_ReadTime(TIME_TypeDef* time);


void time_convert(TIME_TypeDef *time_get);

#endif

