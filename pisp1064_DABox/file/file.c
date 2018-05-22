#include "file.h"
#include "protocol.h"

/**
*	   this file do the things below:
* 
*                      app
*         				|
*   				    |
*					   cfg
*					    |
*						|
*                     buffer
*                       |
*						|
*                      hardware
*
*              we define hardware-->buffer is  reading.
*                        buffer---->cfg is reading.
*              we define cfg---->buffer is writing.
*                        buffer--->hardware is writing.
*                        app------>cfg is protocol translating ,it is also writing.
*
*/
//#define FILE_DEBUG 
#define FILE_INFO

#ifdef FILE_DEBUG
#define file_debug(fmt, ...) rt_kprintf("[file.c][Line:%d]"fmt"\n", __LINE__, ##__VA_ARGS__)
#else
#define file_debug(fmt, ...)
#endif

#ifdef FILE_INFO
#define file_info(fmt, ...) rt_kprintf("[file.c][Line:%d]"fmt"\n", __LINE__, ##__VA_ARGS__)
#else
#define file_info(fmt, ...)
#endif

#define REVERSE_U32(x)  (((x&0xff)<<24) | ((x&0xff00)<<8) | ((x&0xff0000)>>8) | ((x&0xff000000)>>24))


static union 
{
	struct
	{
#pragma align 4
		unsigned int param3:13;
		unsigned int param2:5;
		unsigned int param1:5;
		unsigned int param_sub_type:4;
		unsigned int param_type:4;
		unsigned int instru_functions:1;
	}x;
#pragma align 4
	unsigned int spi_instruction;
}SPI_CODE;


extern SPI_HANDLE_T SPI_HANDLE_t[];
extern SPI_DATA_T SPI_DATA_t;
#define DELAY_TIME 4000

static void file_udelay(unsigned int time)
{
	int i = 0;
	unsigned int j;
	for(i = 0 ; i < 8 ; i ++)
	{
	  	for(j = 0 ; j < time; j ++)
			__nop();
	}
}

unsigned int memory_cfg_buf[MEMORY_CFG_BUF];
unsigned int reversed_memory_cfg_buf[MEMORY_CFG_BUF];
static unsigned int general_start_address = GENERAL_PARAM;
static unsigned int output_start_address;
static unsigned int eq_start_address;

static unsigned int general_nums=0;
static unsigned int output_nums=0;
static unsigned int eq_nums=0;

unsigned int file_crc=0;

typedef struct{
	char *name;
	char type;
	unsigned int (*control)(int directions,int dev,char *name);
	int size;	  /*instructions nums*/
}FILE_OPS;



unsigned int file_type3_chan_gain(char *name,int directions,int dev);
unsigned int file_type3_all_chan_gain(char *name,int directions,int dev);
unsigned int file_type3_chan_delay(char *name,int directions,int dev);
unsigned int file_type3_all_chan_delay(char *name,int directions,int dev);
unsigned int file_type3_gradual_max(char *name,int directions,int dev);
unsigned int file_type3_gradual_min(char *name,int directions,int dev);
unsigned int file_type3_high_cut(char *name,int directions,int dev);
unsigned int file_type3_low_cut(char *name,int directions,int dev);
unsigned int file_type3_division_nums(char *name,int directions,int dev);
unsigned int file_type3_divison_chans(char *name,int directions,int dev);
unsigned int file_type3_divison_point(char *name,int directions,int dev);

unsigned int file_type4_chans_geq(char *name,int directions,int dev);
unsigned int file_type4_chans_peq_type(char *name,int directions,int dev);
unsigned int file_type4_chans_peq_freq(char *name,int directions,int dev);
unsigned int file_type4_chans_peq_q(char *name,int directions,int dev);
unsigned int file_type4_chans_peq_gain(char *name,int directions,int dev);

unsigned int file_type4_all_chans_geq_gain(char *name,int directions,int dev);

FILE_OPS file_ops[]={
	/*it is for pc,we use it and not for protocol*/
	/*output */
	{"chan_gain",TYPE_OUTPUT,file_type3_chan_gain,AUDIO_CHANS},
	{"all_gain",TYPE_OUTPUT,file_type3_all_chan_gain,1},
	{"chan_delay",TYPE_OUTPUT,file_type3_chan_delay,AUDIO_CHANS},
	{"all_delay",TYPE_OUTPUT,file_type3_all_chan_delay,1},
	{"gradual_max",TYPE_OUTPUT,file_type3_gradual_max,1},
	{"gradual_min",TYPE_OUTPUT,file_type3_gradual_min,1},
	{"high_cut_freq",TYPE_OUTPUT,file_type3_high_cut,AUDIO_CHANS},
	{"low_cut_freq",TYPE_OUTPUT,file_type3_low_cut,AUDIO_CHANS},
	/*general */
 	{"division nums",TYPE_GENERAL,file_type3_division_nums,AUDIO_CHANS},
   	{"division chans",TYPE_GENERAL,file_type3_divison_chans,AUDIO_CHANS*3},
	{"frequency point",TYPE_GENERAL,file_type3_divison_point,AUDIO_CHANS*2},	
	/*geq */
	{"chans geq",TYPE_EQ,file_type4_chans_geq,AUDIO_CHANS*GEQ_BANDS},
	/*peq */
	{"chans peq type",TYPE_EQ,file_type4_chans_peq_type,AUDIO_CHANS*PEQ_NUMS},
	{"chans peq freq",TYPE_EQ,file_type4_chans_peq_freq,AUDIO_CHANS*PEQ_NUMS},
	{"chans peq q",TYPE_EQ,file_type4_chans_peq_q,AUDIO_CHANS*PEQ_NUMS},
	{"chans peq gain",TYPE_EQ,file_type4_chans_peq_gain,AUDIO_CHANS*PEQ_NUMS},

	{"chans all gain",TYPE_EQ,file_type4_all_chans_geq_gain,GEQ_BANDS},
};


/*get general address*/
unsigned int get_general_address(void)
{
	return 	general_start_address;
}
/*get output address*/
unsigned int get_output_address(void)
{
	int i;
	unsigned int start_address = general_start_address;
	for(i = 0 ; i < sizeof(file_ops)/sizeof(file_ops[0]); i ++){
		if(TYPE_OUTPUT==file_ops[i].type){
			break;
		}
		start_address += file_ops[i].size * 4;
	}
	return 	start_address;
}
/*get eq address*/
unsigned int get_eq_address(void)
{
	int i;
	unsigned int start_address = general_start_address;
	for(i = 0 ; i < sizeof(file_ops)/sizeof(file_ops[0]); i ++){
		if(TYPE_EQ==file_ops[i].type){
			break;
		}
		start_address += file_ops[i].size * 4;
	}
	return 	start_address;
}

/*
*		write to dev
*	  @param dev device such as eeprom
*     @param address where to write
*     @param buffer the data to be write
*     @param len the buffer len to write
*/
void file_write_dev(int dev,unsigned int start_address,unsigned char *buffer,unsigned int len)
{
	unsigned char *p = (unsigned char *)start_address;
	int i = 0;
	if(dev == DEV_MEMORY_TO_EEPROM){
	   iic_eeprom_at24cm02_write(start_address,buffer,len);
	}
	if(dev == DEV_CFG_TO_MEMORY){
		for(i = 0 ; i < 4 ; i ++){
			*p = buffer[i];
			p++;	
		}			    
	}
}

/*
*			read from dev
*	  @param dev device such as eeprom
*     @param address where to read
*     @param buffer the data to be read
*     @param len the buffer len to read
*/
void file_read_dev(int dev,unsigned int start_address,unsigned char *buffer,unsigned int len)
{

	if(dev == DEV_EEPROM_TO_MEMORY){
		iic_eeprom_at24cm02_read(start_address,buffer,len);
	}
	if(dev == DEV_MEMORY_TO_CFG){
		//do nothing.			    
	}
}

/*1.save or read division nums*/
unsigned int file_type3_division_nums(char *name,int directions,int dev)
{
	unsigned int start_address = 0;
	int nums = 0;
	int i = 0;

	for(i = 0 ; i < sizeof(file_ops)/sizeof(file_ops[0]); i ++){
		if(strcmp(file_ops[i].name,name,strlen(file_ops[i].name))==0){
			break;
		}else{
			start_address += file_ops[i].size * 4;
		}
	}

	for(i = 0 ; i <  TYPE_MAX ; i ++){
		if(	file_ops[i].type == TYPE_GENERAL){
			start_address += get_general_address();
			break;
		}
		if(	file_ops[i].type == TYPE_OUTPUT){
			start_address += get_output_address();
			break;
		}
		if(	file_ops[i].type == TYPE_EQ){
			start_address += get_eq_address();
			break;
		}
	}

	nums = (start_address - general_start_address)/4;
	file_debug("nums %s,start:%d",file_ops[i].name,nums);
	/*write*/
	if(directions == D_WRITE){	
		for(i = 0 ; i < AUDIO_CHANS ; i ++){
			if(	dev == DEV_CFG_TO_MEMORY){
				{
					SPI_CODE.x.instru_functions= 0 ;
					SPI_CODE.x.param_type      = 3 ; 
					SPI_CODE.x.param_sub_type  = 11;
					SPI_CODE.x.param1          = i ;
					SPI_CODE.x.param2          = 0 ;
					SPI_CODE.x.param3          = SPI_DATA_t.type3_freq_divnums[i];
				}
				file_write_dev(dev,(unsigned int)&memory_cfg_buf[nums],(unsigned char *)&SPI_CODE.spi_instruction,4);

				file_debug("write memory:0x%0x",SPI_CODE.spi_instruction);
			}

			if(	dev == DEV_MEMORY_TO_EEPROM){
				file_write_dev(dev, start_address ,(unsigned char *)&memory_cfg_buf[nums],4);
				file_debug("write eeprom:0x%0x",memory_cfg_buf[nums]);
				file_udelay(DELAY_TIME);
			}
			file_udelay(DELAY_TIME);
			start_address = start_address + 4; 
			nums ++;
		}
	}

	/*read*/
	if(directions == D_READ){
		/*1.read division nums*/
		for(i = 0 ; i < AUDIO_CHANS ; i ++){
			if(dev== DEV_EEPROM_TO_MEMORY){
				file_read_dev(dev,start_address, (unsigned char *)&memory_cfg_buf[nums] ,4);
				file_debug("read eeprom:0x%0x",memory_cfg_buf[nums]);
				file_udelay(DELAY_TIME);
			}
			if(dev== DEV_MEMORY_TO_CFG){
				file_read_dev(dev,(unsigned int)&memory_cfg_buf[nums], (unsigned char *)&SPI_CODE.spi_instruction ,4);
				SPI_DATA_t.type3_freq_divnums[i] = SPI_CODE.x.param3;
				file_debug("read memory:0x%0x",memory_cfg_buf[nums]);
			}
			start_address = start_address + 4;
			file_udelay(DELAY_TIME);
			nums ++; 
		}		
	}
	file_debug("file_type3_divison_nums ");
	return nums;
}

unsigned int file_type3_divison_chans(char *name,int directions,int dev)
{
	unsigned int start_address = 0;
	int nums = 0;
	int i = 0 ,j = 0;

	for(i = 0 ; i < sizeof(file_ops)/sizeof(file_ops[0]); i ++){
		if(strcmp(file_ops[i].name,name,strlen(file_ops[i].name))==0){
			break;
		}else{
			start_address += file_ops[i].size * 4;
		}
	}

	for(i = 0 ; i <  TYPE_MAX ; i ++){
		if(	file_ops[i].type == TYPE_GENERAL){
			start_address += get_general_address();
			break;
		}
		if(	file_ops[i].type == TYPE_OUTPUT){
			start_address += get_output_address();
			break;
		}
		if(	file_ops[i].type == TYPE_EQ){
			start_address += get_eq_address();
			break;
		}
	}

	nums = (start_address - general_start_address)/4;
	file_debug("nums %s,start:%d",file_ops[i].name,nums);
	/*write*/
	if(directions == D_WRITE){	
		for(i = 0 ; i < AUDIO_CHANS ; i ++){
			for(j = 0 ; j < 3 ; j ++){
				if(	dev == DEV_CFG_TO_MEMORY){
					{
						SPI_CODE.x.instru_functions= 0 ;
						SPI_CODE.x.param_type      = 3 ; 
						SPI_CODE.x.param_sub_type  = 13;
						SPI_CODE.x.param1          = i ;
						SPI_CODE.x.param2          = j ;
						SPI_CODE.x.param3          = SPI_DATA_t.type3_output_chans[i][j];
					}
					file_write_dev(dev,(unsigned int)&memory_cfg_buf[nums],(unsigned char *)&SPI_CODE.spi_instruction,4);
	
					file_debug("write memory:0x%0x",SPI_CODE.spi_instruction);
				}
	
				if(	dev == DEV_MEMORY_TO_EEPROM){
					file_write_dev(dev, start_address ,(unsigned char *)&memory_cfg_buf[nums],4);
					file_debug("write eeprom:0x%0x",memory_cfg_buf[nums]);
					file_udelay(DELAY_TIME);
				}
				file_udelay(DELAY_TIME);
				start_address = start_address + 4; 
				nums ++;
			}
		}
	}

	/*read*/
	if(directions == D_READ){
		/*1.read division nums*/
		for(i = 0 ; i < AUDIO_CHANS ; i ++){
			for(j = 0 ; j < 3 ; j ++){
				if(dev== DEV_EEPROM_TO_MEMORY){
					file_read_dev(dev,start_address, (unsigned char *)&memory_cfg_buf[nums] ,4);
					file_debug("read eeprom:0x%0x",memory_cfg_buf[nums]);
					file_udelay(DELAY_TIME);
				}
				if(dev== DEV_MEMORY_TO_CFG){
					file_read_dev(dev,(unsigned int)&memory_cfg_buf[nums], (unsigned char *)&SPI_CODE.spi_instruction ,4);
					SPI_DATA_t.type3_output_chans[i][j] = SPI_CODE.x.param3;
					file_debug("read memory:0x%0x",memory_cfg_buf[nums]);
				}
				start_address = start_address + 4;
				file_udelay(DELAY_TIME);
				nums ++;
			} 
		}		
	}
	file_debug("file_type3_divison_chans ");	
	return nums;
}



unsigned int file_type3_divison_point(char *name,int directions,int dev)
{
	unsigned int start_address = 0;
	int nums = 0;
	int i = 0 ,j = 0;

	for(i = 0 ; i < sizeof(file_ops)/sizeof(file_ops[0]); i ++){
		if(strcmp(file_ops[i].name,name,strlen(file_ops[i].name))==0){
			break;
		}else{
			start_address += file_ops[i].size * 4;
		}
	}

	for(i = 0 ; i <  TYPE_MAX ; i ++){
		if(	file_ops[i].type == TYPE_GENERAL){
			start_address += get_general_address();
			break;
		}
		if(	file_ops[i].type == TYPE_OUTPUT){
			start_address += get_output_address();
			break;
		}
		if(	file_ops[i].type == TYPE_EQ){
			start_address += get_eq_address();
			break;
		}
	}

	nums = (start_address - general_start_address)/4;
	file_debug("nums %s,start:%d",file_ops[i].name,nums);
	/*write*/
	if(directions == D_WRITE){	
		for(i = 0 ; i < AUDIO_CHANS ; i ++){
			for(j = 0 ; j < 2 ; j ++){
				if(	dev == DEV_CFG_TO_MEMORY){
					{
						SPI_CODE.x.instru_functions= 0 ;
						SPI_CODE.x.param_type      = 3 ; 
						SPI_CODE.x.param_sub_type  = 12;
						SPI_CODE.x.param1          = i ;
						SPI_CODE.x.param2          = j ;
						SPI_CODE.x.param3          = SPI_DATA_t.type3_freq_point[i][j];
					}
					file_write_dev(dev,(unsigned int)&memory_cfg_buf[nums],(unsigned char *)&SPI_CODE.spi_instruction,4);
	
					file_debug("write memory:0x%0x",SPI_CODE.spi_instruction);
				}
	
				if(	dev == DEV_MEMORY_TO_EEPROM){
					file_write_dev(dev, start_address ,(unsigned char *)&memory_cfg_buf[nums],4);
					file_debug("write eeprom:0x%0x",memory_cfg_buf[nums]);
					file_udelay(DELAY_TIME);
				}
				file_udelay(DELAY_TIME);
				start_address = start_address + 4; 
				nums ++;
			}
		}
	}

	/*read*/
	if(directions == D_READ){
		/*1.read division nums*/
		for(i = 0 ; i < AUDIO_CHANS ; i ++){
			for(j = 0 ; j < 2 ; j ++){
				if(dev== DEV_EEPROM_TO_MEMORY){
					file_read_dev(dev,start_address, (unsigned char *)&memory_cfg_buf[nums] ,4);
					file_info("read eeprom:0x%0x",memory_cfg_buf[nums]);
					file_udelay(DELAY_TIME);
				}
				if(dev== DEV_MEMORY_TO_CFG){
					file_read_dev(dev,(unsigned int)&memory_cfg_buf[nums], (unsigned char *)&SPI_CODE.spi_instruction ,4);
					SPI_DATA_t.type3_freq_point[i][j] = SPI_CODE.x.param3;
					file_debug("read memory:0x%0x",memory_cfg_buf[nums]);
				}
				start_address = start_address + 4;
				file_udelay(DELAY_TIME);
				nums ++;
			} 
		}		
	}
	file_debug("file_type3_divison_point ");	
	return nums;
}


/*chan gain*/
unsigned int file_type3_chan_gain(char *name,int directions,int dev)
{
	unsigned int start_address = 0;
	int nums = 0;
	int i = 0 ,j = 0;

	for(i = 0 ; i < sizeof(file_ops)/sizeof(file_ops[0]); i ++){
		if(strcmp(file_ops[i].name,name,strlen(file_ops[i].name))==0){
			break;
		}else{
			start_address += file_ops[i].size * 4;
		}
	}

	for(i = 0 ; i <  TYPE_MAX ; i ++){
		if(	file_ops[i].type == TYPE_GENERAL){
			start_address += get_general_address();
			break;
		}
		if(	file_ops[i].type == TYPE_OUTPUT){
			start_address += get_output_address();
			break;
		}
		if(	file_ops[i].type == TYPE_EQ){
			start_address += get_eq_address();
			break;
		}
	}

	nums = (start_address - general_start_address)/4;
	file_debug("nums %s,start:%d",file_ops[i].name,nums);
	/*write*/
	if(directions == D_WRITE){	
		for(i = 0 ; i < AUDIO_CHANS ; i ++){
			if(	dev == DEV_CFG_TO_MEMORY){
				{
					SPI_CODE.x.instru_functions= 0 ;
					SPI_CODE.x.param_type      = 3 ; 
					SPI_CODE.x.param_sub_type  = 1;
					SPI_CODE.x.param1          = i ;
					SPI_CODE.x.param2          = 0 ;
					SPI_CODE.x.param3          = (SPI_DATA_t.type3_gain[i] & 0x1fff) ;
				}
				file_write_dev(dev,(unsigned int)&memory_cfg_buf[nums],(unsigned char *)&SPI_CODE.spi_instruction,4);

				file_debug("write memory:0x%0x",SPI_CODE.spi_instruction);
			}

			if(	dev == DEV_MEMORY_TO_EEPROM){
				file_write_dev(dev, start_address ,(unsigned char *)&memory_cfg_buf[nums],4);
				file_debug("write eeprom:0x%0x",memory_cfg_buf[nums]);
				file_udelay(DELAY_TIME);
			}
			file_udelay(DELAY_TIME);
			start_address = start_address + 4; 
			nums ++;
		}
	}

	/*read*/
	if(directions == D_READ){
		/*1.read division nums*/
		for(i = 0 ; i < AUDIO_CHANS ; i ++){
			 
			if(dev== DEV_EEPROM_TO_MEMORY){
				file_read_dev(dev,start_address, (unsigned char *)&memory_cfg_buf[nums] ,4);
				file_info("read eeprom:0x%0x",memory_cfg_buf[nums]);
				file_udelay(DELAY_TIME);
			}
			if(dev== DEV_MEMORY_TO_CFG){
				file_read_dev(dev,(unsigned int)&memory_cfg_buf[nums], (unsigned char *)&SPI_CODE.spi_instruction ,4);
				SPI_DATA_t.type3_gain[i]  = (signed char)(SPI_CODE.x.param3 & 0x1fff);
				file_debug("read memory:0x%0x",memory_cfg_buf[nums]);
			}
			start_address = start_address + 4;
			file_udelay(DELAY_TIME);
			nums ++;
			  
		}		
	}
	file_debug("file_type3_chan_gain ");	
	return nums;
}



/*file_type3_all_chan_gain*/
unsigned int file_type3_all_chan_gain(char *name,int directions,int dev)
{
	unsigned int start_address = 0;
	int nums = 0;
	int i = 0 ,j = 0;

	for(i = 0 ; i < sizeof(file_ops)/sizeof(file_ops[0]); i ++){
		if(strcmp(file_ops[i].name,name,strlen(file_ops[i].name))==0){
			break;
		}else{
			start_address += file_ops[i].size * 4;
		}
	}

	for(i = 0 ; i <  TYPE_MAX ; i ++){
		if(	file_ops[i].type == TYPE_GENERAL){
			start_address += get_general_address();
			break;
		}
		if(	file_ops[i].type == TYPE_OUTPUT){
			start_address += get_output_address();
			break;
		}
		if(	file_ops[i].type == TYPE_EQ){
			start_address += get_eq_address();
			break;
		}
	}

	nums = (start_address - general_start_address)/4;
	file_debug("nums %s,start:%d",file_ops[i].name,nums);
	/*write*/
	if(directions == D_WRITE){	
		for(i = 0 ; i < 1 ; i ++){
			if(	dev == DEV_CFG_TO_MEMORY){
				{
					SPI_CODE.x.instru_functions= 0 ;
					SPI_CODE.x.param_type      = 3 ; 
					SPI_CODE.x.param_sub_type  = 1;
					SPI_CODE.x.param1          = 0 ;
					SPI_CODE.x.param2          = 2 ;
					SPI_CODE.x.param3          = (SPI_DATA_t.type3_all_gain)&0x1fff ;
				}
				file_write_dev(dev,(unsigned int)&memory_cfg_buf[nums],(unsigned char *)&SPI_CODE.spi_instruction,4);

				file_debug("write memory:0x%0x",SPI_CODE.spi_instruction);
			}

			if(	dev == DEV_MEMORY_TO_EEPROM){
				file_write_dev(dev, start_address ,(unsigned char *)&memory_cfg_buf[nums],4);
				file_debug("write eeprom:0x%0x",memory_cfg_buf[nums]);
				file_udelay(DELAY_TIME);
			}
			file_udelay(DELAY_TIME);
			start_address = start_address + 4; 
			nums ++;
		}
	}

	/*read*/
	if(directions == D_READ){
		/*1.read division nums*/
		for(i = 0 ; i < 1 ; i ++){
			 
			if(dev== DEV_EEPROM_TO_MEMORY){
				file_read_dev(dev,start_address, (unsigned char *)&memory_cfg_buf[nums] ,4);
				file_info("read eeprom:0x%0x",memory_cfg_buf[nums]);
				file_udelay(DELAY_TIME);
			}
			if(dev== DEV_MEMORY_TO_CFG){
				file_read_dev(dev,(unsigned int)&memory_cfg_buf[nums], (unsigned char *)&SPI_CODE.spi_instruction ,4);
				SPI_DATA_t.type3_all_gain  = (signed char)(SPI_CODE.x.param3 & 0x1fff);
				file_debug("read memory:0x%0x",memory_cfg_buf[nums]);
			}
			start_address = start_address + 4;
			file_udelay(DELAY_TIME);
			nums ++;
			  
		}		
	}
	file_debug("file_type3_all_chan_gain ");	
	return nums;
}



/*file_type3_chan_delay*/
unsigned int file_type3_chan_delay(char *name,int directions,int dev)
{
	unsigned int start_address = 0;
	int nums = 0;
	int i = 0 ,j = 0;

	for(i = 0 ; i < sizeof(file_ops)/sizeof(file_ops[0]); i ++){
		if(strcmp(file_ops[i].name,name,strlen(file_ops[i].name))==0){
			break;
		}else{
			start_address += file_ops[i].size * 4;
		}
	}

	for(i = 0 ; i <  TYPE_MAX ; i ++){
		if(	file_ops[i].type == TYPE_GENERAL){
			start_address += get_general_address();
			break;
		}
		if(	file_ops[i].type == TYPE_OUTPUT){
			start_address += get_output_address();
			break;
		}
		if(	file_ops[i].type == TYPE_EQ){
			start_address += get_eq_address();
			break;
		}
	}

	nums = (start_address - general_start_address)/4;
	file_debug("nums %s,start:%d",file_ops[i].name,nums);
	/*write*/
	if(directions == D_WRITE){	
		for(i = 0 ; i < AUDIO_CHANS ; i ++){
			if(	dev == DEV_CFG_TO_MEMORY){
				{
					SPI_CODE.x.instru_functions= 0 ;
					SPI_CODE.x.param_type      = 3 ; 
					SPI_CODE.x.param_sub_type  = 2;
					SPI_CODE.x.param1          = i ;
					SPI_CODE.x.param2          = 0 ;
					SPI_CODE.x.param3          = SPI_DATA_t.type3_delay[i];
				}
				file_write_dev(dev,(unsigned int)&memory_cfg_buf[nums],(unsigned char *)&SPI_CODE.spi_instruction,4);

				file_debug("write memory:0x%0x",SPI_CODE.spi_instruction);
			}

			if(	dev == DEV_MEMORY_TO_EEPROM){
				file_write_dev(dev, start_address ,(unsigned char *)&memory_cfg_buf[nums],4);
				file_debug("write eeprom:0x%0x",memory_cfg_buf[nums]);
				file_udelay(DELAY_TIME);
			}
			file_udelay(DELAY_TIME);
			start_address = start_address + 4; 
			nums ++;
		}
	}

	/*read*/
	if(directions == D_READ){
		/*1.read division nums*/
		for(i = 0 ; i < AUDIO_CHANS ; i ++){
			 
			if(dev== DEV_EEPROM_TO_MEMORY){
				file_read_dev(dev,start_address, (unsigned char *)&memory_cfg_buf[nums] ,4);
				file_info("read eeprom:0x%0x",memory_cfg_buf[nums]);
				file_udelay(DELAY_TIME);
			}
			if(dev== DEV_MEMORY_TO_CFG){
				file_read_dev(dev,(unsigned int)&memory_cfg_buf[nums], (unsigned char *)&SPI_CODE.spi_instruction ,4);
				SPI_DATA_t.type3_delay[i]  = SPI_CODE.x.param3;
				file_debug("read memory:0x%0x",memory_cfg_buf[nums]);
			}
			start_address = start_address + 4;
			file_udelay(DELAY_TIME);
			nums ++;
			  
		}		
	}
	file_debug("file_type3_chan_delay ");	
	return nums;
}



/*file_type3_all_chan_delay*/
unsigned int file_type3_all_chan_delay(char *name,int directions,int dev)
{
	unsigned int start_address = 0;
	int nums = 0;
	int i = 0 ,j = 0;

	for(i = 0 ; i < sizeof(file_ops)/sizeof(file_ops[0]); i ++){
		if(strcmp(file_ops[i].name,name,strlen(file_ops[i].name))==0){
			break;
		}else{
			start_address += file_ops[i].size * 4;
		}
	}

	for(i = 0 ; i <  TYPE_MAX ; i ++){
		if(	file_ops[i].type == TYPE_GENERAL){
			start_address += get_general_address();
			break;
		}
		if(	file_ops[i].type == TYPE_OUTPUT){
			start_address += get_output_address();
			break;
		}
		if(	file_ops[i].type == TYPE_EQ){
			start_address += get_eq_address();
			break;
		}
	}

	nums = (start_address - general_start_address)/4;
	file_debug("nums %s,start:%d",file_ops[i].name,nums);
	/*write*/
	if(directions == D_WRITE){	
		for(i = 0 ; i < 1 ; i ++){
			if(	dev == DEV_CFG_TO_MEMORY){
				{
					SPI_CODE.x.instru_functions= 0 ;
					SPI_CODE.x.param_type      = 3 ; 
					SPI_CODE.x.param_sub_type  = 2;
					SPI_CODE.x.param1          = 0 ;
					SPI_CODE.x.param2          = 2 ;
					SPI_CODE.x.param3          = SPI_DATA_t.type3_all_delay;
				}
				file_write_dev(dev,(unsigned int)&memory_cfg_buf[nums],(unsigned char *)&SPI_CODE.spi_instruction,4);

				file_debug("write memory:0x%0x",SPI_CODE.spi_instruction);
			}

			if(	dev == DEV_MEMORY_TO_EEPROM){
				file_write_dev(dev, start_address ,(unsigned char *)&memory_cfg_buf[nums],4);
				file_debug("write eeprom:0x%0x",memory_cfg_buf[nums]);
				file_udelay(DELAY_TIME);
			}
			file_udelay(DELAY_TIME);
			start_address = start_address + 4; 
			nums ++;
		}
	}

	/*read*/
	if(directions == D_READ){
		/*1.read division nums*/
		for(i = 0 ; i < 1 ; i ++){
			 
			if(dev== DEV_EEPROM_TO_MEMORY){
				file_read_dev(dev,start_address, (unsigned char *)&memory_cfg_buf[nums] ,4);
				file_info("read eeprom:0x%0x",memory_cfg_buf[nums]);
				file_udelay(DELAY_TIME);
			}
			if(dev== DEV_MEMORY_TO_CFG){
				file_read_dev(dev,(unsigned int)&memory_cfg_buf[nums], (unsigned char *)&SPI_CODE.spi_instruction ,4);
				SPI_DATA_t.type3_all_delay  = SPI_CODE.x.param3;
				file_debug("read memory:0x%0x",memory_cfg_buf[nums]);
			}
			start_address = start_address + 4;
			file_udelay(DELAY_TIME);
			nums ++;
			  
		}		
	}
	file_debug("file_type3_all_chan_delay ");	
	return nums;
}

/*ffile_type3_gradual_max*/
unsigned int file_type3_gradual_max(char *name,int directions,int dev)
{
	unsigned int start_address = 0;
	int nums = 0;
	int i = 0 ,j = 0;

	for(i = 0 ; i < sizeof(file_ops)/sizeof(file_ops[0]); i ++){
		if(strcmp(file_ops[i].name,name,strlen(file_ops[i].name))==0){
			break;
		}else{
			start_address += file_ops[i].size * 4;
		}
	}

	for(i = 0 ; i <  TYPE_MAX ; i ++){
		if(	file_ops[i].type == TYPE_GENERAL){
			start_address += get_general_address();
			break;
		}
		if(	file_ops[i].type == TYPE_OUTPUT){
			start_address += get_output_address();
			break;
		}
		if(	file_ops[i].type == TYPE_EQ){
			start_address += get_eq_address();
			break;
		}
	}

	nums = (start_address - general_start_address)/4;
	file_debug("nums %s,start:%d",file_ops[i].name,nums);
	/*write*/
	if(directions == D_WRITE){	
		for(i = 0 ; i < 1 ; i ++){
			if(	dev == DEV_CFG_TO_MEMORY){
				{
					SPI_CODE.x.instru_functions= 0 ;
					SPI_CODE.x.param_type      = 3 ; 
					SPI_CODE.x.param_sub_type  = 4;
					SPI_CODE.x.param1          = 0 ;
					SPI_CODE.x.param2          = 0 ;
					SPI_CODE.x.param3          = SPI_DATA_t.type3_gradual_max;
				}
				file_write_dev(dev,(unsigned int)&memory_cfg_buf[nums],(unsigned char *)&SPI_CODE.spi_instruction,4);

				file_debug("write memory:0x%0x",SPI_CODE.spi_instruction);
			}

			if(	dev == DEV_MEMORY_TO_EEPROM){
				file_write_dev(dev, start_address ,(unsigned char *)&memory_cfg_buf[nums],4);
				file_debug("write eeprom:0x%0x",memory_cfg_buf[nums]);
				file_udelay(DELAY_TIME);
			}
			file_udelay(DELAY_TIME);
			start_address = start_address + 4; 
			nums ++;
		}
	}

	/*read*/
	if(directions == D_READ){
		/*1.read division nums*/
		for(i = 0 ; i < 1 ; i ++){
			 
			if(dev== DEV_EEPROM_TO_MEMORY){
				file_read_dev(dev,start_address, (unsigned char *)&memory_cfg_buf[nums] ,4);
				file_info("read eeprom:0x%0x",memory_cfg_buf[nums]);
				file_udelay(DELAY_TIME);
			}
			if(dev== DEV_MEMORY_TO_CFG){
				file_read_dev(dev,(unsigned int)&memory_cfg_buf[nums], (unsigned char *)&SPI_CODE.spi_instruction ,4);
				SPI_DATA_t.type3_gradual_max  = SPI_CODE.x.param3;
				file_debug("read memory:0x%0x",memory_cfg_buf[nums]);
			}
			start_address = start_address + 4;
			file_udelay(DELAY_TIME);
			nums ++;
			  
		}		
	}
	file_debug("file_type3_gradual_max");	
	return nums;
}


/*file_type3_gradual_min*/
unsigned int file_type3_gradual_min(char *name,int directions,int dev)
{
	unsigned int start_address = 0;
	int nums = 0;
	int i = 0 ,j = 0;

	for(i = 0 ; i < sizeof(file_ops)/sizeof(file_ops[0]); i ++){
		if(strcmp(file_ops[i].name,name,strlen(file_ops[i].name))==0){
			break;
		}else{
			start_address += file_ops[i].size * 4;
		}
	}

	for(i = 0 ; i <  TYPE_MAX ; i ++){
		if(	file_ops[i].type == TYPE_GENERAL){
			start_address += get_general_address();
			break;
		}
		if(	file_ops[i].type == TYPE_OUTPUT){
			start_address += get_output_address();
			break;
		}
		if(	file_ops[i].type == TYPE_EQ){
			start_address += get_eq_address();
			break;
		}
	}

	nums = (start_address - general_start_address)/4;
	file_debug("nums %s,start:%d",file_ops[i].name,nums);
	/*write*/
	if(directions == D_WRITE){	
		for(i = 0 ; i < 1 ; i ++){
			if(	dev == DEV_CFG_TO_MEMORY){
				{
					SPI_CODE.x.instru_functions= 0 ;
					SPI_CODE.x.param_type      = 3 ; 
					SPI_CODE.x.param_sub_type  = 4;
					SPI_CODE.x.param1          = 1 ;
					SPI_CODE.x.param2          = 0 ;
					SPI_CODE.x.param3          = SPI_DATA_t.type3_gradual_min;
				}
				file_write_dev(dev,(unsigned int)&memory_cfg_buf[nums],(unsigned char *)&SPI_CODE.spi_instruction,4);

				file_debug("write memory:0x%0x",SPI_CODE.spi_instruction);
			}

			if(	dev == DEV_MEMORY_TO_EEPROM){
				file_write_dev(dev, start_address ,(unsigned char *)&memory_cfg_buf[nums],4);
				file_debug("write eeprom:0x%0x",memory_cfg_buf[nums]);
				file_udelay(DELAY_TIME);
			}
			file_udelay(DELAY_TIME);
			start_address = start_address + 4; 
			nums ++;
		}
	}

	/*read*/
	if(directions == D_READ){
		/*1.read division nums*/
		for(i = 0 ; i < 1 ; i ++){
			 
			if(dev== DEV_EEPROM_TO_MEMORY){
				file_read_dev(dev,start_address, (unsigned char *)&memory_cfg_buf[nums] ,4);
				file_info("read eeprom:0x%0x",memory_cfg_buf[nums]);
				file_udelay(DELAY_TIME);
			}
			if(dev== DEV_MEMORY_TO_CFG){
				file_read_dev(dev,(unsigned int)&memory_cfg_buf[nums], (unsigned char *)&SPI_CODE.spi_instruction ,4);
				SPI_DATA_t.type3_gradual_min  = SPI_CODE.x.param3;
				file_debug("read memory:0x%0x",memory_cfg_buf[nums]);
			}
			start_address = start_address + 4;
			file_udelay(DELAY_TIME);
			nums ++;
			  
		}		
	}
	file_debug("file_type3_gradual_min");	
	return nums;
}


unsigned int file_type3_high_cut(char *name,int directions,int dev)
{	
	unsigned int start_address = 0;
	int nums = 0;
	int i = 0;
	for(i = 0 ; i < sizeof(file_ops)/sizeof(file_ops[0]); i ++){
		if(strcmp(file_ops[i].name,name,strlen(file_ops[i].name))==0){
			break;
		}else{
			start_address += file_ops[i].size * 4;
		}	 
	}
	for(i = 0 ; i <  TYPE_MAX ; i ++){
		if(	file_ops[i].type == TYPE_GENERAL){
			start_address += get_general_address();
			break;
		}
		if(	file_ops[i].type == TYPE_OUTPUT){
			start_address += get_output_address();
			break;
		}
		if(	file_ops[i].type == TYPE_EQ){
			start_address += get_eq_address();
			break;
		}
	}

	nums = (start_address - general_start_address)/4;
	file_debug("nums %s,start:%d",file_ops[i].name,nums);
	/*write*/
	if(directions == D_WRITE){	
		for(i = 0 ; i < AUDIO_CHANS ; i ++){
			if(	dev == DEV_CFG_TO_MEMORY){
				{
					SPI_CODE.x.instru_functions= 0 ;
					SPI_CODE.x.param_type      = 3 ; 
					SPI_CODE.x.param_sub_type  = 3;
					SPI_CODE.x.param1          = i ;
					SPI_CODE.x.param2          = 0 ;
					SPI_CODE.x.param3          = SPI_DATA_t.type3_high_cut[i];
				}
				file_write_dev(dev,(unsigned int)&memory_cfg_buf[nums],(unsigned char *)&SPI_CODE.spi_instruction,4);

				file_debug("write memory:0x%0x",SPI_CODE.spi_instruction);
				file_udelay(DELAY_TIME);
			}

			if(	dev == DEV_MEMORY_TO_EEPROM){
				file_write_dev(dev, start_address ,(unsigned char *)&memory_cfg_buf[nums],4);
				file_debug("write eeprom:0x%0x",memory_cfg_buf[nums]);
				file_udelay(DELAY_TIME);
			}
			file_udelay(DELAY_TIME);
			start_address = start_address + 4; 
			nums ++;
		}
	}

	/*read*/
	if(directions == D_READ){
		/*1.read division nums*/
		for(i = 0 ; i < AUDIO_CHANS ; i ++){
			if(dev== DEV_EEPROM_TO_MEMORY){
				file_read_dev(dev,start_address, (unsigned char *)&memory_cfg_buf[nums] ,4);
				file_debug("read eeprom:0x%0x",memory_cfg_buf[nums]);
				file_udelay(DELAY_TIME);
			}
			if(dev== DEV_MEMORY_TO_CFG){
				file_read_dev(dev,(unsigned int)&memory_cfg_buf[nums], (unsigned char *)&SPI_CODE.spi_instruction ,4);
				SPI_DATA_t.type3_high_cut[i] = SPI_CODE.x.param3;
				file_debug("read memory:0x%0x",memory_cfg_buf[nums]);
			}
			start_address = start_address + 4;
			file_udelay(DELAY_TIME);
			nums ++; 
		}		
	}
	file_debug("file_type3_high_cut ");	
	return nums;
}


unsigned int file_type3_low_cut(char *name,int directions,int dev)
{	
	unsigned int start_address = 0;
	int nums = 0;
	int i = 0;
	for(i = 0 ; i < sizeof(file_ops)/sizeof(file_ops[0]); i ++){
		if(strcmp(file_ops[i].name,name,strlen(file_ops[i].name))==0){
			break;
		}else{
			start_address += file_ops[i].size * 4;
		}	 
	}
	for(i = 0 ; i <  TYPE_MAX ; i ++){
		if(	file_ops[i].type == TYPE_GENERAL){
			start_address += get_general_address();
			break;
		}
		if(	file_ops[i].type == TYPE_OUTPUT){
			start_address += get_output_address();
			break;
		}
		if(	file_ops[i].type == TYPE_EQ){
			start_address += get_eq_address();
			break;
		}
	}

	nums = (start_address - general_start_address)/4;
	file_debug("nums %s,start:%d",file_ops[i].name,nums);
	/*write*/
	if(directions == D_WRITE){	
		for(i = 0 ; i < AUDIO_CHANS ; i ++){
			if(	dev == DEV_CFG_TO_MEMORY){
				{
					SPI_CODE.x.instru_functions= 0 ;
					SPI_CODE.x.param_type      = 3 ; 
					SPI_CODE.x.param_sub_type  = 3;
					SPI_CODE.x.param1          = i ;
					SPI_CODE.x.param2          = 2 ;
					SPI_CODE.x.param3          = SPI_DATA_t.type3_low_cut[i];
				}
				file_write_dev(dev,(unsigned int)&memory_cfg_buf[nums],(unsigned char *)&SPI_CODE.spi_instruction,4);

				file_debug("write memory:0x%0x",SPI_CODE.spi_instruction);
				file_udelay(DELAY_TIME);
			}

			if(	dev == DEV_MEMORY_TO_EEPROM){
				file_write_dev(dev, start_address ,(unsigned char *)&memory_cfg_buf[nums],4);
				file_debug("write eeprom:0x%0x",memory_cfg_buf[nums]);
				file_udelay(DELAY_TIME);
			}
			file_udelay(DELAY_TIME);
			start_address = start_address + 4; 
			nums ++;
		}
	}

	/*read*/
	if(directions == D_READ){
		/*1.read division nums*/
		for(i = 0 ; i < AUDIO_CHANS ; i ++){
			if(dev== DEV_EEPROM_TO_MEMORY){
				file_read_dev(dev,start_address, (unsigned char *)&memory_cfg_buf[nums] ,4);
				file_debug("read eeprom:0x%0x",memory_cfg_buf[nums]);
				file_udelay(DELAY_TIME);
			}
			if(dev== DEV_MEMORY_TO_CFG){
				file_read_dev(dev,(unsigned int)&memory_cfg_buf[nums], (unsigned char *)&SPI_CODE.spi_instruction ,4);
				SPI_DATA_t.type3_low_cut[i] = SPI_CODE.x.param3;
				file_debug("read memory:0x%0x",memory_cfg_buf[nums]);
			}
			start_address = start_address + 4;
			file_udelay(DELAY_TIME);
			nums ++; 
		}		
	}
	file_debug("file_type3_high_cut ");	
	return nums;
}





unsigned int file_type4_chans_geq(char *name,int directions,int dev)
{	
	unsigned int start_address = 0;
	int nums = 0;
	int i = 0,j;
	for(i = 0 ; i < sizeof(file_ops)/sizeof(file_ops[0]); i ++){
		if(strcmp(file_ops[i].name,name,strlen(file_ops[i].name))==0){
			break;
		}else{
			start_address += file_ops[i].size * 4;
		}	 
	}

	for(i = 0 ; i <  TYPE_MAX ; i ++){
		if(	file_ops[i].type == TYPE_GENERAL){
			start_address += get_general_address();
			break;
		}
		if(	file_ops[i].type == TYPE_OUTPUT){
			start_address += get_output_address();
			break;
		}
		if(	file_ops[i].type == TYPE_EQ){
			start_address += get_eq_address();
			break;
		}
	}

	nums = (start_address - general_start_address)/4;
	file_debug("nums %s,start:%d",file_ops[i].name,nums);
	/*write*/
	if(directions == D_WRITE){	
		for(i = 0 ; i < AUDIO_CHANS ; i ++){
			for(j = 0 ; j < GEQ_BANDS ; j ++){
				if(	dev == DEV_CFG_TO_MEMORY){
					{
						SPI_CODE.x.instru_functions= 0 ;
						SPI_CODE.x.param_type      = 4 ; 
						SPI_CODE.x.param_sub_type  = 1;
						SPI_CODE.x.param1          = i ;
						SPI_CODE.x.param2          = j ;
						SPI_CODE.x.param3          = (SPI_DATA_t.type4_geq_gain[i][j]&0x1fff);
					}
					file_write_dev(dev,(unsigned int)&memory_cfg_buf[nums],(unsigned char *)&SPI_CODE.spi_instruction,4);
	
					file_debug("write memory:0x%0x",SPI_CODE.spi_instruction);
					file_udelay(DELAY_TIME);
				}
	
				if(	dev == DEV_MEMORY_TO_EEPROM){
					file_write_dev(dev, start_address ,(unsigned char *)&memory_cfg_buf[nums],4);
					file_debug("write eeprom:0x%0x",memory_cfg_buf[nums]);
					file_udelay(DELAY_TIME);
				}
				file_udelay(DELAY_TIME);
				start_address = start_address + 4; 
				nums ++;
			}
		}
	}

	/*read*/
	if(directions == D_READ){
		/*1.read division nums*/
		for(i = 0 ; i < AUDIO_CHANS ; i ++){
			for(j = 0 ; j < GEQ_BANDS ; j ++){
				if(dev== DEV_EEPROM_TO_MEMORY){
					file_read_dev(dev,start_address, (unsigned char *)&memory_cfg_buf[nums] ,4);
					file_debug("read eeprom:0x%0x",memory_cfg_buf[nums]);
					file_udelay(DELAY_TIME);
				}
				if(dev== DEV_MEMORY_TO_CFG){
					file_read_dev(dev,(unsigned int)&memory_cfg_buf[nums], (unsigned char *)&SPI_CODE.spi_instruction ,4);
					SPI_DATA_t.type4_geq_gain[i][j] = (signed char)(SPI_CODE.x.param3&0x1fff);
					file_debug("read memory:0x%0x",memory_cfg_buf[nums]);
				}
				start_address = start_address + 4;
				file_udelay(DELAY_TIME);
				nums ++; 
			}
		}		
	}
	file_debug("file_type4_chans_geq ");	
	return nums;
}

unsigned int file_type4_all_chans_geq_gain(char *name,int directions,int dev)
{	
	unsigned int start_address = 0;
	int nums = 0;
	int i = 0,j;
	for(i = 0 ; i < sizeof(file_ops)/sizeof(file_ops[0]); i ++){
		if(strcmp(file_ops[i].name,name,strlen(file_ops[i].name))==0){
			break;
		}else{
			start_address += file_ops[i].size * 4;
		}	 
	}

	for(i = 0 ; i <  TYPE_MAX ; i ++){
		if(	file_ops[i].type == TYPE_GENERAL){
			start_address += get_general_address();
			break;
		}
		if(	file_ops[i].type == TYPE_OUTPUT){
			start_address += get_output_address();
			break;
		}
		if(	file_ops[i].type == TYPE_EQ){
			start_address += get_eq_address();
			break;
		}
	}

	nums = (start_address - general_start_address)/4;
	file_debug("nums %s,start:%d",file_ops[i].name,nums);
	/*write*/
	if(directions == D_WRITE){	
		 
			for(j = 0 ; j < GEQ_BANDS ; j ++){
				if(	dev == DEV_CFG_TO_MEMORY){
					{
						SPI_CODE.x.instru_functions= 0 ;
						SPI_CODE.x.param_type      = 4 ; 
						SPI_CODE.x.param_sub_type  = 13;
						SPI_CODE.x.param1          = 0 ;
						SPI_CODE.x.param2          = j ;
						SPI_CODE.x.param3          = (SPI_DATA_t.type4_all_geq_gain[j]&0x1fff);
					}
					file_write_dev(dev,(unsigned int)&memory_cfg_buf[nums],(unsigned char *)&SPI_CODE.spi_instruction,4);
	
					file_debug("write memory:0x%0x",SPI_CODE.spi_instruction);
					file_udelay(DELAY_TIME);
				}
	
				if(	dev == DEV_MEMORY_TO_EEPROM){
					file_write_dev(dev, start_address ,(unsigned char *)&memory_cfg_buf[nums],4);
					file_debug("write eeprom:0x%0x",memory_cfg_buf[nums]);
					file_udelay(DELAY_TIME);
				}
				file_udelay(DELAY_TIME);
				start_address = start_address + 4; 
				nums ++;
			}
	 
	}

	/*read*/
	if(directions == D_READ){
		/*1.read division nums*/
	 
			for(j = 0 ; j < GEQ_BANDS ; j ++){
				if(dev== DEV_EEPROM_TO_MEMORY){
					file_read_dev(dev,start_address, (unsigned char *)&memory_cfg_buf[nums] ,4);
					file_debug("read eeprom:0x%0x",memory_cfg_buf[nums]);
					file_udelay(DELAY_TIME);
				}
				if(dev== DEV_MEMORY_TO_CFG){
					file_read_dev(dev,(unsigned int)&memory_cfg_buf[nums], (unsigned char *)&SPI_CODE.spi_instruction ,4);
					SPI_DATA_t.type4_all_geq_gain[j] = (signed char)(SPI_CODE.x.param3&0x1fff);
					file_debug("read memory:0x%0x",memory_cfg_buf[nums]);
				}
				start_address = start_address + 4;
				file_udelay(DELAY_TIME);
				nums ++; 
			}
	  		
	}
	file_debug("file_type4_all_chans_geq_gain ");	
	return nums;
}





unsigned int file_type4_chans_peq_type(char *name,int directions,int dev)
{	
	unsigned int start_address = 0;
	int nums = 0;
	int i = 0,j;
	for(i = 0 ; i < sizeof(file_ops)/sizeof(file_ops[0]); i ++){
		if(strcmp(file_ops[i].name,name,strlen(file_ops[i].name))==0){
			break;
		}else{
			start_address += file_ops[i].size * 4;
		}	 
	}

	for(i = 0 ; i <  TYPE_MAX ; i ++){
		if(	file_ops[i].type == TYPE_GENERAL){
			start_address += get_general_address();
			break;
		}
		if(	file_ops[i].type == TYPE_OUTPUT){
			start_address += get_output_address();
			break;
		}
		if(	file_ops[i].type == TYPE_EQ){
			start_address += get_eq_address();
			break;
		}
	}

	nums = (start_address - general_start_address)/4;
	file_debug("nums %s,start:%d",file_ops[i].name,nums);
	/*write*/
	if(directions == D_WRITE){	
		for(i = 0 ; i < AUDIO_CHANS ; i ++){
			for(j = 0 ; j < PEQ_NUMS ; j ++){
				if(	dev == DEV_CFG_TO_MEMORY){
					{
						SPI_CODE.x.instru_functions= 0 ;
						SPI_CODE.x.param_type      = 4 ; 
						SPI_CODE.x.param_sub_type  = 4;
						SPI_CODE.x.param1          = i ;
						SPI_CODE.x.param2          = j ;
						SPI_CODE.x.param3          = SPI_DATA_t.type4_peq_type[i][j];
					}
					file_write_dev(dev,(unsigned int)&memory_cfg_buf[nums],(unsigned char *)&SPI_CODE.spi_instruction,4);
	
					file_debug("write memory:0x%0x",SPI_CODE.spi_instruction);
					file_udelay(DELAY_TIME);
				}
	
				if(	dev == DEV_MEMORY_TO_EEPROM){
					file_write_dev(dev, start_address ,(unsigned char *)&memory_cfg_buf[nums],4);
					file_debug("write eeprom:0x%0x",memory_cfg_buf[nums]);
					file_udelay(DELAY_TIME);
				}
				file_udelay(DELAY_TIME);
				start_address = start_address + 4; 
				nums ++;
			}
		}
	}

	/*read*/
	if(directions == D_READ){
		/*1.read division nums*/
		for(i = 0 ; i < AUDIO_CHANS ; i ++){
			for(j = 0 ; j < PEQ_NUMS ; j ++){
				if(dev== DEV_EEPROM_TO_MEMORY){
					file_read_dev(dev,start_address, (unsigned char *)&memory_cfg_buf[nums] ,4);
					file_debug("read eeprom:0x%0x",memory_cfg_buf[nums]);
					file_udelay(DELAY_TIME);
				}
				if(dev== DEV_MEMORY_TO_CFG){
					file_read_dev(dev,(unsigned int)&memory_cfg_buf[nums], (unsigned char *)&SPI_CODE.spi_instruction ,4);
					SPI_DATA_t.type4_peq_type[i][j] = SPI_CODE.x.param3;
					file_debug("read memory:0x%0x",memory_cfg_buf[nums]);
				}
				start_address = start_address + 4;
				file_udelay(DELAY_TIME);
				nums ++; 
			}
		}		
	}
	file_debug("file_type4_chans_peq_type ");	
	return nums;
}

unsigned int file_type4_chans_peq_freq(char *name,int directions,int dev)
{	
	unsigned int start_address = 0;
	int nums = 0;
	int i = 0,j;
	for(i = 0 ; i < sizeof(file_ops)/sizeof(file_ops[0]); i ++){
		if(strcmp(file_ops[i].name,name,strlen(file_ops[i].name))==0){
			break;
		}else{
			start_address += file_ops[i].size * 4;
		}	 
	}

	for(i = 0 ; i <  TYPE_MAX ; i ++){
		if(	file_ops[i].type == TYPE_GENERAL){
			start_address += get_general_address();
			break;
		}
		if(	file_ops[i].type == TYPE_OUTPUT){
			start_address += get_output_address();
			break;
		}
		if(	file_ops[i].type == TYPE_EQ){
			start_address += get_eq_address();
			break;
		}
	}

	nums = (start_address - general_start_address)/4;
	file_debug("nums %s,start:%d",file_ops[i].name,nums);
	/*write*/
	if(directions == D_WRITE){	
		for(i = 0 ; i < AUDIO_CHANS ; i ++){
			for(j = 0 ; j < PEQ_NUMS ; j ++){
				if(	dev == DEV_CFG_TO_MEMORY){
					{
						SPI_CODE.x.instru_functions= 0 ;
						SPI_CODE.x.param_type      = 4 ; 
						SPI_CODE.x.param_sub_type  = 6;
						SPI_CODE.x.param1          = i ;
						SPI_CODE.x.param2          = j ;
						SPI_CODE.x.param3          = SPI_DATA_t.type4_peq_freq[i][j];
					}
					file_write_dev(dev,(unsigned int)&memory_cfg_buf[nums],(unsigned char *)&SPI_CODE.spi_instruction,4);
	
					file_debug("write memory:0x%0x",SPI_CODE.spi_instruction);
					file_udelay(DELAY_TIME);
				}
	
				if(	dev == DEV_MEMORY_TO_EEPROM){
					file_write_dev(dev, start_address ,(unsigned char *)&memory_cfg_buf[nums],4);
					file_debug("write eeprom:0x%0x",memory_cfg_buf[nums]);
					file_udelay(DELAY_TIME);
				}
				file_udelay(DELAY_TIME);
				start_address = start_address + 4; 
				nums ++;
			}
		}
	}

	/*read*/
	if(directions == D_READ){
		/*1.read division nums*/
		for(i = 0 ; i < AUDIO_CHANS ; i ++){
			for(j = 0 ; j < PEQ_NUMS ; j ++){
				if(dev== DEV_EEPROM_TO_MEMORY){
					file_read_dev(dev,start_address, (unsigned char *)&memory_cfg_buf[nums] ,4);
					file_debug("read eeprom:0x%0x",memory_cfg_buf[nums]);
					file_udelay(DELAY_TIME);
				}
				if(dev== DEV_MEMORY_TO_CFG){
					file_read_dev(dev,(unsigned int)&memory_cfg_buf[nums], (unsigned char *)&SPI_CODE.spi_instruction ,4);
					SPI_DATA_t.type4_peq_freq[i][j] = SPI_CODE.x.param3;
					file_debug("read memory:0x%0x",memory_cfg_buf[nums]);
				}
				start_address = start_address + 4;
				file_udelay(DELAY_TIME);
				nums ++; 
			}
		}		
	}
	file_debug("file_type4_chans_peq_freq ");	
	return nums;
} 

unsigned int file_type4_chans_peq_q(char *name,int directions,int dev)
{	
	unsigned int start_address = 0;
	int nums = 0;
	int i = 0,j;
	for(i = 0 ; i < sizeof(file_ops)/sizeof(file_ops[0]); i ++){
		if(strcmp(file_ops[i].name,name,strlen(file_ops[i].name))==0){
			break;
		}else{
			start_address += file_ops[i].size * 4;
		}	 
	}

	for(i = 0 ; i <  TYPE_MAX ; i ++){
		if(	file_ops[i].type == TYPE_GENERAL){
			start_address += get_general_address();
			break;
		}
		if(	file_ops[i].type == TYPE_OUTPUT){
			start_address += get_output_address();
			break;
		}
		if(	file_ops[i].type == TYPE_EQ){
			start_address += get_eq_address();
			break;
		}
	}

	nums = (start_address - general_start_address)/4;
	file_debug("nums %s,start:%d",file_ops[i].name,nums);
	/*write*/
	if(directions == D_WRITE){	
		for(i = 0 ; i < AUDIO_CHANS ; i ++){
			for(j = 0 ; j < PEQ_NUMS ; j ++){
				if(	dev == DEV_CFG_TO_MEMORY){
					{
						SPI_CODE.x.instru_functions= 0 ;
						SPI_CODE.x.param_type      = 4 ; 
						SPI_CODE.x.param_sub_type  = 8;
						SPI_CODE.x.param1          = i ;
						SPI_CODE.x.param2          = j ;
						SPI_CODE.x.param3          = SPI_DATA_t.type4_peq_q[i][j];
					}
					file_write_dev(dev,(unsigned int)&memory_cfg_buf[nums],(unsigned char *)&SPI_CODE.spi_instruction,4);
	
					file_debug("write memory:0x%0x",SPI_CODE.spi_instruction);
					file_udelay(DELAY_TIME);
				}
	
				if(	dev == DEV_MEMORY_TO_EEPROM){
					file_write_dev(dev, start_address ,(unsigned char *)&memory_cfg_buf[nums],4);
					file_debug("write eeprom:0x%0x",memory_cfg_buf[nums]);
					file_udelay(DELAY_TIME);
				}
				file_udelay(DELAY_TIME);
				start_address = start_address + 4; 
				nums ++;
			}
		}
	}

	/*read*/
	if(directions == D_READ){
		/*1.read division nums*/
		for(i = 0 ; i < AUDIO_CHANS ; i ++){
			for(j = 0 ; j < PEQ_NUMS ; j ++){
				if(dev== DEV_EEPROM_TO_MEMORY){
					file_read_dev(dev,start_address, (unsigned char *)&memory_cfg_buf[nums] ,4);
					file_debug("read eeprom:0x%0x",memory_cfg_buf[nums]);
					file_udelay(DELAY_TIME);
				}
				if(dev== DEV_MEMORY_TO_CFG){
					file_read_dev(dev,(unsigned int)&memory_cfg_buf[nums], (unsigned char *)&SPI_CODE.spi_instruction ,4);
					SPI_DATA_t.type4_peq_q[i][j] = SPI_CODE.x.param3;
					file_debug("read memory:0x%0x",memory_cfg_buf[nums]);
				}
				start_address = start_address + 4;
				file_udelay(DELAY_TIME);
				nums ++; 
			}
		}		
	}
	file_debug("file_type4_chans_peq_q ");	
	return nums;
}

unsigned int file_type4_chans_peq_gain(char *name,int directions,int dev)
{	
	unsigned int start_address = 0;
	int nums = 0;
	int i = 0,j;
	for(i = 0 ; i < sizeof(file_ops)/sizeof(file_ops[0]); i ++){
		if(strcmp(file_ops[i].name,name,strlen(file_ops[i].name))==0){
			break;
		}else{
			start_address += file_ops[i].size * 4;
		}	 
	}

	for(i = 0 ; i <  TYPE_MAX ; i ++){
		if(	file_ops[i].type == TYPE_GENERAL){
			start_address += get_general_address();
			break;
		}
		if(	file_ops[i].type == TYPE_OUTPUT){
			start_address += get_output_address();
			break;
		}
		if(	file_ops[i].type == TYPE_EQ){
			start_address += get_eq_address();
			break;
		}
	}

	nums = (start_address - general_start_address)/4;
	file_debug("nums %s,start:%d",file_ops[i].name,nums);
	/*write*/
	if(directions == D_WRITE){	
		for(i = 0 ; i < AUDIO_CHANS ; i ++){
			for(j = 0 ; j < PEQ_NUMS ; j ++){
				if(	dev == DEV_CFG_TO_MEMORY){
					{
						SPI_CODE.x.instru_functions= 0 ;
						SPI_CODE.x.param_type      = 4 ; 
						SPI_CODE.x.param_sub_type  = 10;
						SPI_CODE.x.param1          = i ;
						SPI_CODE.x.param2          = j ;
						SPI_CODE.x.param3          = SPI_DATA_t.type4_peq_gain[i][j]&0x1fff;
					}
					file_write_dev(dev,(unsigned int)&memory_cfg_buf[nums],(unsigned char *)&SPI_CODE.spi_instruction,4);
	
					file_debug("write memory:0x%0x",SPI_CODE.spi_instruction);
					file_udelay(DELAY_TIME);
				}
	
				if(	dev == DEV_MEMORY_TO_EEPROM){
					file_write_dev(dev, start_address ,(unsigned char *)&memory_cfg_buf[nums],4);
					file_debug("write eeprom:0x%0x",memory_cfg_buf[nums]);
					file_udelay(DELAY_TIME);
				}
				file_udelay(DELAY_TIME);
				start_address = start_address + 4; 
				nums ++;
			}
		}
	}

	/*read*/
	if(directions == D_READ){
		/*1.read division nums*/
		for(i = 0 ; i < AUDIO_CHANS ; i ++){
			for(j = 0 ; j < PEQ_NUMS ; j ++){
				if(dev== DEV_EEPROM_TO_MEMORY){
					file_read_dev(dev,start_address, (unsigned char *)&memory_cfg_buf[nums] ,4);
					file_debug("read eeprom:0x%0x",memory_cfg_buf[nums]);
					file_udelay(DELAY_TIME);
				}
				if(dev== DEV_MEMORY_TO_CFG){
					file_read_dev(dev,(unsigned int)&memory_cfg_buf[nums], (unsigned char *)&SPI_CODE.spi_instruction ,4);
					SPI_DATA_t.type4_peq_gain[i][j] = (signed char)(SPI_CODE.x.param3&0x1fff);
					file_debug("read memory:0x%0x",memory_cfg_buf[nums]);
				}
				start_address = start_address + 4;
				file_udelay(DELAY_TIME);
				nums ++; 
			}
		}		
	}
	file_debug("file_type4_chans_peq_gain ");	
	return nums;
}


/*
*	  read cfg from eeprom or etc
*
*     note: the reading order can't be changed
*/
void file_read(void)
{
	int nums = 0;
	//nums  = file_read_data(GENERAL_P,TO_CFG);
	//file_info("cfg_nums1:%d",nums);

	nums  = file_read_data(GENERAL_P,DEV_EEPROM_TO_MEMORY);
	nums  = file_read_data(GENERAL_P,DEV_MEMORY_TO_CFG);
	file_info("file_read_general_params nums:%d",nums);
	//nums += file_read_data(OUTPUT_P,TO_CFG);
	//file_info("cfg_nums2:%d",nums);
	//nums += file_read_data(EQ_P,TO_CFG);
	//file_info("cfg_nums3:%d",nums);
}
 
/*
*		@param fdir 0:write 1: read
* 	
*
*/
int file_operate(char fdir,enum DEVICE dev)
{
	int i = 0,j=0;
	int nums = 0;
	unsigned int start_address = general_start_address;
	/*reserved for head*/	
	if(fdir==D_READ){
		/*general params*/	
		nums = file_type3_chan_gain("chan_gain",D_READ,dev);	
		file_info("file_type3_chan_gain:%d",nums);

		nums = file_type3_all_chan_gain("all_gain",D_READ,dev);	
		file_info("file_type3_all_chan_gain:%d",nums);

		nums = file_type3_chan_delay("chan_delay",D_READ,dev);	
		file_info("file_type3_chan_delay:%d",nums);

		nums = file_type3_all_chan_delay("all_delay",D_READ,dev);	
		file_info("file_type3_all_chan_delay:%d",nums);

		nums = file_type3_gradual_max("gradual_max",D_READ,dev);	
		file_info("file_type3_gradual_max:%d",nums);

		nums = file_type3_gradual_min("gradual_min",D_READ,dev);	
		file_info("file_type3_gradual_min:%d",nums);

		nums = file_type3_high_cut("high_cut_freq",D_READ,dev);
		file_info("file_type3_high_cut:%d",nums);

		nums = file_type3_low_cut("low_cut_freq",D_READ,dev);
		file_info("file_type3_low_cut:%d",nums);

		nums = file_type3_division_nums("division nums",D_READ,dev);	
		file_info("file_type3_division_nums:%d",nums);

 		nums = file_type3_divison_chans("division chans",D_READ,dev);
		file_info("file_type3_divison_chans:%d",nums);

		nums = file_type3_divison_point("frequency point",D_READ,dev);
		file_info("file_type3_divison_point:%d",nums);

		nums = file_type4_chans_geq("chans geq",D_READ,dev);
		file_info("file_type4_chans_geq:%d",nums);

		nums = file_type4_chans_peq_type("chans peq type",D_READ,dev);
		file_info("file_type4_chans_peq_type:%d",nums);

		nums = file_type4_chans_peq_freq("chans peq freq",D_READ,dev);
		file_info("file_type4_chans_peq_freq:%d",nums);
		
		nums = file_type4_chans_peq_q("chans peq q",D_READ,dev);
		file_info("file_type4_chans_peq_q:%d",nums);	
		
		nums = file_type4_chans_peq_gain("chans peq gain",D_READ,dev);
		file_info("file_type4_chans_peq_gain:%d",nums);
		
		nums = file_type4_all_chans_geq_gain("chans all gain",D_READ,dev);
		file_info("file_type4_all_chans_geq_gain:%d",nums);	
	}
	
	if(fdir==D_WRITE){
		file_type3_chan_gain("chan_gain",D_WRITE,dev);	
		file_info("file_type3_chan_gain");

		file_type3_all_chan_gain("all_gain",D_WRITE,dev);	
		file_info("file_type3_all_chan_gain");

		file_type3_chan_delay("chan_delay",D_WRITE,dev);	
		file_info("file_type3_chan_delay");

		file_type3_all_chan_delay("all_delay",D_WRITE,dev);	
		file_info("file_type3_all_chan_delay");

		file_type3_gradual_max("gradual_max",D_WRITE,dev);	
		file_info("file_type3_gradual_max");

		file_type3_gradual_min("gradual_min",D_WRITE,dev);	
		file_info("file_type3_gradual_min");

		file_type3_high_cut("high_cut_freq",D_WRITE,dev);
		file_info("file_type3_high_cut");

		file_type3_low_cut("low_cut_freq",D_WRITE,dev);
		file_info("file_type3_low_cut");

		file_type3_division_nums("division nums",D_WRITE,dev);	
		file_info("file_type3_division_nums");

 		file_type3_divison_chans("division chans",D_WRITE,dev);
		file_info("file_type3_divison_chans");

		file_type3_divison_point("frequency point",D_WRITE,dev);
		file_info("file_type3_divison_point");

		file_type4_chans_geq("chans geq",D_WRITE,dev);
		file_info("file_type4_chans_geq");	

		file_type4_chans_peq_type("chans peq type",D_WRITE,dev);
		file_info("file_type4_chans_peq_type");	

		file_type4_chans_peq_freq("chans peq freq",D_WRITE,dev);
		file_info("file_type4_chans_peq_freq");
		
		file_type4_chans_peq_q("chans peq q",D_WRITE,dev);
		file_info("file_type4_chans_peq_q");		
		
		file_type4_chans_peq_gain("chans peq gain",D_WRITE,dev);
		file_info("file_type4_chans_peq_gain");	
		
		file_type4_all_chans_geq_gain("chans all gain",D_WRITE,dev);
		file_info("file_type4_all_chans_geq_gain");						
	}	
	
	return nums;	
}

void file_store_data(enum SAVE_PARAMS para)
{
#if 0
	switch(para){
		case GENERAL_P:/*general params*/
			file_store_general_params();	
		break;
		case OUTPUT_P:/*output params*/
			file_store_output_params();	
		break;
		case EQ_P:/*eq params*/
			file_store_eq_params();	
		break;
	}
#else

	file_operate(D_WRITE,DEV_CFG_TO_MEMORY);
	file_operate(D_WRITE,DEV_MEMORY_TO_EEPROM);

	return;
	switch(para){
		case GENERAL_P:/*general params*/
			file_operate(D_WRITE,DEV_CFG_TO_MEMORY);
			file_operate(D_WRITE,DEV_MEMORY_TO_EEPROM);
		break;
		case OUTPUT_P:/*output params*/
			file_operate(D_WRITE,DEV_CFG_TO_MEMORY);
			file_operate(D_WRITE,DEV_MEMORY_TO_EEPROM);
		break;
		case EQ_P:/*output params*/
			file_operate(D_WRITE,DEV_CFG_TO_MEMORY);
			file_operate(D_WRITE,DEV_MEMORY_TO_EEPROM);
		break;
	}
#endif
}

/*read data from eeprom...*/
int _file_read_data(enum SAVE_PARAMS para,enum READ_DIR dir)
{
	int ret = 0;
#if 0
	switch(para){
		case GENERAL_P:/*general params*/
			ret = file_read_general_params(dir);	
		break;
		case OUTPUT_P:/*output params*/
			ret = file_read_output_params(dir);	
		break;
		case EQ_P:/*eq params*/
			ret = file_read_eq_params(dir);	
		break;
	}
#else
	switch(para){
		case GENERAL_P:/*general params*/
			//ret = file_read_general_params(dir);
			file_operate(D_READ,DEV_CFG_TO_MEMORY);	
		break;
		case OUTPUT_P:/*general params*/
			//ret = file_read_general_params(dir);
			file_operate(D_READ,DEV_CFG_TO_MEMORY);	
		break;
	}
#endif
	return ret;
}

/*read data from eeprom...*/
int file_read_data(enum SAVE_PARAMS para,enum DEVICE dev)
{
	int ret = 0;
 
	switch(para){
		case GENERAL_P:/*general params*/
			ret = file_operate(D_READ,dev);	
		break;
		case OUTPUT_P:/*general params*/
			ret = file_operate(D_READ,dev);	
		break;
		case EQ_P:/*eq params*/
			ret = file_operate(D_READ,dev);	
		break;
	}
 
	return ret;
}

/**
* 	@brief: restore  file to factory mode
*
*	@retval: none
*
*   2018-08-14: just clear the eeprom data.
*/
void file_factory(void)
{
	int i = 0;
	int file_total_size = 0;
	unsigned char buf[256];
	unsigned int start_address = general_start_address;

	memset(buf,0,sizeof(buf));
	file_info("array len : %d\r\n",sizeof(file_ops)/sizeof(file_ops[0]));
	for(i = 0 ; i < sizeof(file_ops)/sizeof(file_ops[0]); i ++){
		file_total_size += file_ops[i].size * 4;
	}
	file_info("file total size is:%d B",file_total_size);

	for(i = 0 ; i < file_total_size / 256 ; i ++){
	   iic_eeprom_at24cm02_write(start_address+i*256,buf,256);
	   file_udelay(DELAY_TIME*4);
	}

	if(file_total_size%256!=0){
		iic_eeprom_at24cm02_write(start_address+i*256,buf,256);	
		file_udelay(DELAY_TIME*4);
	}
}




