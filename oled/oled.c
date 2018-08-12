#include "oled.h"
#include "oled_font.h"

#define OLED_ADDRESS    0x78

static void delay_us(int time)
{
	int i = 0 , j = 0;
	for(i = 0 ; i < 10; i ++){
		for(j = 0 ; j < time; j ++){
				;
		}
	}
}

static void delay_ms(int time)
{
	delay_us(1000*time);
}


void I2C_WriteByte(unsigned char addr,unsigned char data)
{
	int ret;			  
	IIC_Start();
	IIC_Send_Byte(OLED_ADDRESS);
	
	ret |= IIC_Wait_Ack();
	
	IIC_Send_Byte(addr);   
	ret |= IIC_Wait_Ack();
	
	IIC_Send_Byte(data);
	ret |= IIC_Wait_Ack();	
	
	IIC_Stop();
}



void WriteCmd(unsigned char I2C_Command) 
{
  I2C_WriteByte(0x00, I2C_Command);
}
 
void WriteDat(unsigned char I2C_Data) 
{
  I2C_WriteByte(0x40, I2C_Data);
}	

void OLED_SetPos(unsigned char x, unsigned char y) 
{ 
	WriteCmd(0xb0+y);
	WriteCmd(((x&0xf0)>>4)|0x10);
	WriteCmd((x&0x0f)|0x01);
}

void OLED_Fill(unsigned char fill_Data)//????
{
	unsigned char m,n;
	for(m=0;m<8;m++)
	{
		WriteCmd(0xb0+m);       //page0-page1
		WriteCmd(0x00);     //low column start address
		WriteCmd(0x10);     //high column start address
		for(n=0;n<128;n++)
		{
				WriteDat(fill_Data);
		}
	}
}

void OLED_CLS(void)//
{
    OLED_Fill(0x00);
}

void OLED_ON(void)
{
    WriteCmd(0X8D);  //ÉèÖÃµçºÉ±Ã
    WriteCmd(0X14);  //¿ªÆôµçºÉ±Ã
    WriteCmd(0XAF);  //OLED»½ÐÑ
}

void OLED_OFF(void)
{
    WriteCmd(0X8D);  //ÉèÖÃµçºÉ±Ã
    WriteCmd(0X10);  //¹Ø±ÕµçºÉ±Ã
    WriteCmd(0XAE);  //OLEDÐÝÃß
}

/**
  *     init the oled
  * @param   none
  * @retval  none
  */
void OLED_Init(void)
{
	delay_ms(1000);     // 1s, 

	WriteCmd(0xAE); //display off
	WriteCmd(0x20); //Set Memory Addressing Mode    
	WriteCmd(0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	WriteCmd(0xb0); //Set Page Start Address for Page Addressing Mode,0-7
	WriteCmd(0xc8); //Set COM Output Scan Direction
	WriteCmd(0x00); //---set low column address
	WriteCmd(0x10); //---set high column address
	WriteCmd(0x40); //--set start line address
	WriteCmd(0x81); //--set contrast control register
	WriteCmd(0xff); //???? 0x00~0xff
	WriteCmd(0xa1); //--set segment re-map 0 to 127
	WriteCmd(0xa6); //--set normal display
	WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
	WriteCmd(0x3F); //
	WriteCmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	WriteCmd(0xd3); //-set display offset
	WriteCmd(0x00); //-not offset
	WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
	WriteCmd(0xf0); //--set divide ratio
	WriteCmd(0xd9); //--set pre-charge period
	WriteCmd(0x22); //
	WriteCmd(0xda); //--set com pins hardware configuration
	WriteCmd(0x12);
	WriteCmd(0xdb); //--set vcomh
	WriteCmd(0x20); //0x20,0.77xVcc
	WriteCmd(0x8d); //--set DC-DC enable
	WriteCmd(0x14); //
	WriteCmd(0xaf); //--turn on oled panel
	
	OLED_CLS();
	OLED_ON();	
}

/**
*        show the ascii. 6*8 or 8*16 to choice   
* 	@param  x,y x:0~127  y:0~7  ch[] the char to show, Textsize:(1:6*8  2:8*16)                  
* 	@retval none
*
*/
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
{
	unsigned char c = 0,i = 0,j = 0;
	switch(TextSize)
	{
		case 1:
		{
			while(ch[j] != '\0')
			{
					c = ch[j] - 32;
					if(x > 126)
					{
							x = 0;
							y++;
					}
					OLED_SetPos(x,y);
					for(i=0;i<6;i++)
							WriteDat(F6x8[c][i]);
					x += 6;
					j++;
			}
		}break;
		case 2:
		{
			while(ch[j] != '\0')
			{
					c = ch[j] - 32;
					if(x > 120)
					{
							x = 0;
							y++;
					}
					OLED_SetPos(x,y);
					for(i=0;i<8;i++)
							WriteDat(F8X16[c*16+i]);
					OLED_SetPos(x,y+1);
					for(i=0;i<8;i++)
							WriteDat(F8X16[c*16+i+8]);
					x += 8;
					j++;
			}
		}break;
	}
}

/**
*  @breif    show chinese
*  @param    x,y,N x:0~127  y:0~7   N:the index of the chinese                
*  @retval   none
*/
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N)
{
	unsigned char wm=0;
	unsigned int  adder=32*N;
	OLED_SetPos(x , y);
	for(wm = 0;wm < 16;wm++)
	{
			WriteDat(F16x16[adder]);
			adder += 1;
	}
	OLED_SetPos(x,y + 1);
	for(wm = 0;wm < 16;wm++)
	{
			WriteDat(F16x16[adder]);
			adder += 1;
	}
}


/**
*
*		draw bitmap:
*		@param x0,y0 the start point x0:0~127  y0:0~7
*   @param x1,y1 the end   point x1:1~128  y1:1~8
*   @retval none
*
*		eg:
*			OLED_DrawBMP(0,0,128,8,(unsigned char *)BMP1);
*/
void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[])
{
	unsigned int j=0;
	unsigned char x,y;

  if(y1%8==0)
		y = y1/8;
  else
    y = y1/8 + 1;
	for(y=y0;y<y1;y++)
	{
			OLED_SetPos(x0,y);
	for(x=x0;x<x1;x++)
			{
					WriteDat(BMP[j++]);
			}
	}
}



