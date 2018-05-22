#include "protocol.h"
#include "inc.h"
#include "file.h"

#ifdef PROJECT_DEBUG
#if 1
#define PROT_DEBUG
#endif
#endif /* PROJECT_DEBUG */

#define PROT_ERROR

#ifdef PROT_DEBUG
#define prot_debug(fmt, ...) rt_kprintf("[%s][protocol.c][Line:%d]"fmt"\n",get_sys_time(), __LINE__, ##__VA_ARGS__)
#else
#define prot_debug(fmt, ...)
#endif

#ifdef PROT_ERROR
#define prot_error(fmt, ...) rt_kprintf("[protocol.c][PROT][Line:%d]"fmt"\n", __LINE__, ##__VA_ARGS__)
#else
#define prot_error(fmt, ...)
#endif

#define REVERSE_U32(x)  (((x&0xff)<<24) | ((x&0xff00)<<8) | ((x&0xff0000)>>8) | ((x&0xff000000)>>24))


union SPI_CODE
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
};

extern unsigned int memory_cfg_buf[MEMORY_CFG_BUF];
//extern unsigned int reversed_memory_cfg_buf[MEMORY_CFG_BUF];

SPI_HANDLE_T SPI_HANDLE_t[]={
		{ID_TYPE1_READ_CFG_FROM_DEVICE,set_type1_read_cfg_from_device},
		{ID_TYPE1_SAVE_CFG,set_type1_save_cfg},


		{ID_TYPE2_SOUNDFILED_LWH,set_type2_soundfiled_lwh},
		{ID_TYPE2_FACTORY_MODE,set_type2_factory_mode},

		{ID_TYPE3_GAIN,set_type3_gain},
		{ID_TYPE3_DELAY,set_type3_delay},
		{ID_TYPE3_HIGH_LOWCUT_POINT,set_type3_high_low_cut_point},
		{ID_TYPE3_GRADUAL,set_type3_gradual},
		{ID_TYPE3_VOLUME,set_type3_volume},
		{ID_TYPE3_FREQ_DIVNUMS,set_type3_freq_divnums},
		{ID_TYPE3_FREQ_POINT,set_type3_freq_point},
		{ID_TYPE3_OUTPUT_CHANS,set_type3_output_chans},
		/*type4*/
		{ID_TYPE4_GEQ_GAIN,set_type4_geq_gain},
		{ID_TYPE4_RESET_GEQ_GAIN,set_type4_reset_geq_gain},
		{ID_TYPE4_PEQ_TYPE,set_type4_peq_type},
		{ID_TYPE4_PEQ_FREQ,set_type4_peq_freq},
		{ID_TYPE4_PEQ_Q,set_type4_peq_q},
		{ID_TYPE4_PEQ_GAIN,set_type4_peq_gain},
		{ID_TYPE4_ALL_GEQ_GAIN,set_type4_all_geq_gain},

		/*type6*/
		//{ID_TYPE6_ALLIGNMENT,set_type6_allignment},
		//{ID_TYPE6_ALLIGNMENT,set_type6_allignment},
};

SPI_DATA_T SPI_DATA_t;

void reset_struct_value()
{
	int i = 0;
	/*default division params*/
	for(i = 0 ; i < 8 ; i ++){
		SPI_DATA_t.type3_freq_divnums[i] = 1;
	}
}



void test_big_little_edian()
{
	union SPI_CODE SPI_CODE_var;
	SPI_CODE_var.spi_instruction = 0x9190a006;
	rt_kprintf("x1:%d,x2:%d,x3:%d,x4:%d,x5:%d,x6:%d \n", SPI_CODE_var.x.instru_functions,SPI_CODE_var.x.param_type,
		SPI_CODE_var.x.param_sub_type,SPI_CODE_var.x.param1,SPI_CODE_var.x.param2,SPI_CODE_var.x.param3);	
}



/**
*	 read cfg data from stm32
*	 @param1 the instruction param1
*	 @param2 the instruction param2
*	 @param3 the instruction param3
*    note:
*        the memory_cfg_buf actually need to reverse the value.but for convinent of the pc,we don't do it.
*/
unsigned int set_type1_read_cfg_from_device(unsigned int param1,unsigned int param2,unsigned int param3)
{
	int i ;
	int nums = 0;
	int pos = 0;
	if((param1 !=0 ) && (param2 !=0) && (param3!=0)){
		prot_error("set_type1_read_cfg_from_device error\r\n"); 
		return 0;
	}

	//nums += file_read_data(OUTPUT_P,TO_MEMORY);
	//nums += file_read_data(EQ_P,TO_MEMORY);

	//if(nums < 3){
	//	prot_error("set_type1_read_cfg_from_device error\r\n"); 
	// 	return 0;
	//}

	//for(i = 0 ; i < nums ; i ++ ){
	//	reversed_memory_cfg_buf[i] = memory_cfg_buf[i];//REVERSE_U32(memory_cfg_buf[i]);	
	//}
	//tcp_send_data((unsigned char *)reversed_memory_cfg_buf,nums*4);

	//nums = file_read_data(GENERAL_P,TO_MEMORY);
	nums = file_read_data(GENERAL_P,DEV_EEPROM_TO_MEMORY);
	tcp_send_data((unsigned char *)&memory_cfg_buf[pos],nums*4);
	pos = pos + nums;

	nums = file_read_data(OUTPUT_P,DEV_EEPROM_TO_MEMORY);
	tcp_send_data((unsigned char *)&memory_cfg_buf[pos],nums*4);
	pos = pos + nums;

	nums = file_read_data(EQ_P,DEV_EEPROM_TO_MEMORY);
	tcp_send_data((unsigned char *)&memory_cfg_buf[pos],nums*4);
	return 0;
}

/*save file info eeprom*/
unsigned int set_type1_save_cfg(unsigned int param1,unsigned int param2,unsigned int param3)
{
	if((param1 > 3 ) ){
		prot_error("set_type1_save_cfg error"); 
		return 0;
	}
	/*1.save general configuration*/
	if(param1 == 1){
		file_store_data(GENERAL_P);					
	}
	/*2.save output configuration*/
	if(param1 == 2){
		file_store_data(OUTPUT_P);					
	}
	/*3.save eq configuration*/
	if(param1 == 3){
		file_store_data(EQ_P);					
	}
	return 0;
}




unsigned int set_type2_soundfiled_lwh(unsigned int param1,unsigned int param2,unsigned int param3)
{
	if((param2>3) || (param2<1)){
		prot_error("param2:%d\r\n",param2); 
		return 0;
	}
	if(param2==1){
		prot_debug("val:%f\r\n",param3/100.0);	 
		 
	}
	if(param2==2){
		prot_debug("val:%f\r\n",param3/100.0);	
		 
	}
	if(param2==3){
		prot_debug("val:%f\r\n",param3/100.0);		 	 
	}
	return 0;
}

unsigned int set_type2_factory_mode(unsigned int param1,unsigned int param2,unsigned int param3)
{
 
	if((param1!=0) || (param2!=0)){
		prot_error("param2:%d\r\n",param2); 
		return 0;
	}	
 	file_factory();
	prot_debug("reset value to default..."); 
	return 0;
}


/*set gain*/
unsigned int set_type3_gain(unsigned int param1,unsigned int param2,unsigned int param3)
{
	if((param2>2)){
		prot_error("param2:%d\r\n",param2); 
		return 0;
	}

	if(param2==0){
   		SPI_DATA_t.type3_gain[param1] = ((char)(param3&0x1fff));
	}
	if(param2==2){
   		SPI_DATA_t.type3_all_gain = ((char)(param3&0x1fff));
	}	
	prot_debug("set_type3_gain:%d\r\n",SPI_DATA_t.type3_gain[param1]);		 	 
	return 0;
}

/*set delay*/
unsigned int set_type3_delay(unsigned int param1,unsigned int param2,unsigned int param3)
{
	if((param2>2)){
		prot_error("param2:%d\r\n",param2); 
		return 0;
	}
	if(param2==0){
   		SPI_DATA_t.type3_delay[param1] = param3;
	}
	if(param2==2){
   		SPI_DATA_t.type3_all_delay = param3;
	}
	prot_debug("set_type3_delay:%d\r\n",SPI_DATA_t.type3_delay[param1]);		 	 
	return 0;
}

/*set high low cut point*/
unsigned int set_type3_high_low_cut_point(unsigned int param1,unsigned int param2,unsigned int param3)
{
	if((param2>3)){
		prot_error("param2:%d\r\n",param2); 
		return 0;
	}
	if(param2==0){
		SPI_DATA_t.type3_high_cut[param1]= param3;
		prot_debug("set_type3_high_low_cut_point:%d\r\n",SPI_DATA_t.type3_high_cut[param1]);
	}
	if(param2==2){
		SPI_DATA_t.type3_low_cut[param1]= param3;
		prot_debug("set_type3_high_low_cut_point:%d\r\n",SPI_DATA_t.type3_low_cut[param1]);
	}
			 	 
	return 0;
}

/*set gradual*/
unsigned int set_type3_gradual(unsigned int param1,unsigned int param2,unsigned int param3)
{
	if((param2!=0)){
		prot_error("param2:%d\r\n",param2); 
		return 0;
	}
	if(param1==0){
		SPI_DATA_t.type3_gradual_max = param3;
		prot_debug("set_type3_gradual:%d\r\n",SPI_DATA_t.type3_gradual_max);	
	}
	if(param1==1){
		SPI_DATA_t.type3_gradual_min = param3;
		prot_debug("set_type3_gradual:%d\r\n",SPI_DATA_t.type3_gradual_min);	
	}		 	 
	return 0;
}

/*set volume*/
unsigned int set_type3_volume(unsigned int param1,unsigned int param2,unsigned int param3)
{
	if((param2!=0)){
		prot_error("param2:%d\r\n",param2); 
		return 0;
	}
	SPI_DATA_t.type3_volume = param3;
	prot_debug("set_type3_volume:%d\r\n",SPI_DATA_t.type3_volume);		 	 
	return 0;
}

/*set freq division*/
unsigned int set_type3_freq_divnums(unsigned int param1,unsigned int param2,unsigned int param3)
{
	if((param1>8) && (param2!=0)){
		prot_error("param1:%d\r\n",param1); 
		return 0;
	}
	SPI_DATA_t.type3_freq_divnums[param1]= param3;
	prot_debug("set_type3_freq_divnums:%d\r\n",SPI_DATA_t.type3_freq_divnums[param1]);		 	 
	return 0;
}


/*set freq point*/
unsigned int set_type3_freq_point(unsigned int param1,unsigned int param2,unsigned int param3)
{
	if( (param2>3)){
		prot_error("param2:%d\r\n",param2); 
		return 0;
	}
	SPI_DATA_t.type3_freq_point[param1][param2]= param3;
	prot_debug("set_type3_freq_point:%d\r\n",SPI_DATA_t.type3_freq_point[param1][param2]);		 	 
	return 0;
}

/*set output channels*/
unsigned int set_type3_output_chans(unsigned int param1,unsigned int param2,unsigned int param3)
{
	if((param2>5)){
		prot_error("param2:%d\r\n",param2); 
		return 0;
	}
	SPI_DATA_t.type3_output_chans[param1][param2]= param3;
	prot_debug("set_type3_freq_point:%d\r\n",SPI_DATA_t.type3_output_chans[param1][param2]);		 	 
	return 0;
}


/*set geq gain*/
unsigned int set_type4_geq_gain(unsigned int param1,unsigned int param2,unsigned int param3)
{
	if((param2>GEQ_BANDS)){
		prot_error("param2:%d\r\n",param2); 
		return 0;
	}
	SPI_DATA_t.type4_geq_gain[param1][param2]= (signed char)(param3&0x1fff);
	prot_debug("set_type4_geq_gaint:%d\r\n",SPI_DATA_t.type4_geq_gain[param1][param2]);		 	 
	return 0;
}


/*reset geq gain*/
unsigned int set_type4_reset_geq_gain(unsigned int param1,unsigned int param2,unsigned int param3)
{
	if((param1>7)){
		prot_error("param2:%d\r\n",param1); 
		return 0;
	}
	if(param2==0){
		/*重置当前声道的GEQ*/

	}
	if(param2==2){
		/*重置所有声道的GEQ*/

	}		 	 
	return 0;
}

/*set peq type*/
unsigned int set_type4_peq_type(unsigned int param1,unsigned int param2,unsigned int param3)
{
	if((param2>GEQ_BANDS)){
		prot_error("param2:%d\r\n",param2); 
		return 0;
	}
	SPI_DATA_t.type4_peq_type[param1][param2]= param3;
	prot_debug("set_type4_peq_type:%d\r\n",SPI_DATA_t.type4_peq_type[param1][param2]);		 	 
	return 0;
}

/*set peq freq*/
unsigned int set_type4_peq_freq(unsigned int param1,unsigned int param2,unsigned int param3)
{
	if((param2>PEQ_NUMS)){
		prot_error("param2:%d\r\n",param2); 
		return 0;
	}
	SPI_DATA_t.type4_peq_freq[param1][param2]= param3;
	prot_debug("set_type3_freq_point:%d\r\n",SPI_DATA_t.type4_peq_freq[param1][param2]);		 	 
	return 0;
}

/*set peq q*/
unsigned int set_type4_peq_q(unsigned int param1,unsigned int param2,unsigned int param3)
{
	if((param2>PEQ_NUMS)){
		prot_error("param2:%d\r\n",param2); 
		return 0;
	}
	SPI_DATA_t.type4_peq_q[param1][param2]= param3;
	prot_debug("set_type4_peq_q:%d\r\n",SPI_DATA_t.type4_peq_q[param1][param2]);		 	 
	return 0;
}

/*set peq gain*/
unsigned int set_type4_peq_gain(unsigned int param1,unsigned int param2,unsigned int param3)
{
	if((param2>PEQ_NUMS)){
		prot_error("param2:%d\r\n",param2); 
		return 0;
	}
	SPI_DATA_t.type4_peq_gain[param1][param2]= (signed char)(param3&0x1fff);
	prot_debug("set_type4_peq_gain:%d\r\n",SPI_DATA_t.type4_peq_gain[param1][param2]);		 	 
	return 0;
}

/*set all geq gain*/
unsigned int set_type4_all_geq_gain(unsigned int param1,unsigned int param2,unsigned int param3)
{
	if((param1!=0)||(param2>GEQ_BANDS)){
		prot_error("param1:%d\r\n",param1); 
		return 0;
	}
	SPI_DATA_t.type4_all_geq_gain[param2]= (signed char)(param3&0x1fff);
	prot_debug("set_type4_all_geq_gain:%d",SPI_DATA_t.type4_all_geq_gain[param2]);		 	 
	return 0;
}


/*we need to let the spi thread to handle it...*/
unsigned int spi_data_handle_functions(unsigned int id,unsigned int param1,unsigned int param2,unsigned int param3)
{
	int i = 0;
	for(i = 0 ; i < sizeof(SPI_HANDLE_t)/sizeof(SPI_HANDLE_t[0]); i ++ ){
		if(id == SPI_HANDLE_t[i].id){
			return SPI_HANDLE_t[i].fun(param1,param2,param3);
		}
	}
	return 0;
}




void tcp_recv_data(char *buf,int len)
{
	int i = 0;
	union SPI_CODE SPI_CODE_var;
	for(i = 0 ; i < len / 4 ; i ++)
	{
		SPI_CODE_var.spi_instruction = REVERSE_U32(*(unsigned int *)&buf[i*4]);
		prot_debug("net:0x%0x",SPI_CODE_var.spi_instruction);

		write_spi_buf(SPI_CODE_var.spi_instruction);

	}				
}

void spi_to_cfg(unsigned int spi_app_recv)
{
	union SPI_CODE SPI_CODE_var;
	SPI_CODE_var.spi_instruction = spi_app_recv;
	spi_data_handle_functions(GET_ID(SPI_CODE_var.x.instru_functions,SPI_CODE_var.x.param_type,SPI_CODE_var.x.param_sub_type,0,0,0),
				SPI_CODE_var.x.param1,SPI_CODE_var.x.param2,SPI_CODE_var.x.param3);	
}


void socket_send_show(unsigned int *buf,int len)
{
	int i = 0;
	union SPI_CODE SPI_CODE_var;
	prot_debug("socket send:%d",len);
	for(i = 0 ; i < len ; i ++){
		SPI_CODE_var.spi_instruction = *buf;
		prot_debug("0x%0x ",SPI_CODE_var.spi_instruction);
		buf++;
	}
	prot_debug("socket send end");
}


void spi_send_to_tcp(unsigned int data,unsigned int *buf, int len)
{	
	int i = 0 ; 
	union SPI_CODE SPI_CODE_var;
	union SPI_CODE SPI_CODE_check;
	unsigned int crc = 0;
	int protocol_len = 0;
	SPI_CODE_var.spi_instruction = data;

	SPI_CODE_check.spi_instruction = buf[0];
	//校验，并且去头部，去尾部
	if(len>1){
		protocol_len = SPI_CODE_check.x.param3;	
		if(len == protocol_len+3){
			SPI_CODE_check.spi_instruction = buf[len-1];
			if(	SPI_CODE_check.x.param3 + 3 == len){
				for(i = 0 ; i < len - 2 ; i ++){
					crc += buf[i];
				}
				if(crc != buf[len-2]){
				   	prot_debug("crc error...%d",len);
					return;	
				}
			}
		}
		buf=buf+1;
		len=len-3;		
	}

	//实际声压
	if(SPI_CODE_var.spi_instruction==ID_TYPE1_READ_REAL_SPL){
		tcp_send_data((unsigned char *)buf,len*4);
		socket_send_show(buf,len);	
	}
	//软件版本
	if(SPI_CODE_var.spi_instruction== ID_TYPE1_SOFTWARE_VERSION){
		tcp_send_data((unsigned char *)buf,len*4);
		socket_send_show(buf,len);	
	} 
	//频谱能量
	if(SPI_CODE_var.spi_instruction== ID_TYPE1_SPECTRUM_ENERGY){
		tcp_send_data((unsigned char *)buf,len*4);
		socket_send_show(buf,len);	
	} 
	
	//获取当前校准声道
	if(SPI_CODE_var.spi_instruction== ID_TYPE1_CURRENT_ALIGN_CHAN){
		tcp_send_data((unsigned char *)buf,len*4);
		socket_send_show(buf,len);	
	} 
	//获取自动校准状态	
	if(SPI_CODE_var.spi_instruction== ID_TYPE1_CURRENT_ALIGN_STATE){
		tcp_send_data((unsigned char *)buf,len*4);
		socket_send_show(buf,len);	
	}
	
	//获取自动延时 	
	if(SPI_CODE_var.spi_instruction== ID_TYPE1_AUTO_DELAY){
		tcp_send_data((unsigned char *)buf,len*4);
		socket_send_show(buf,len);	
	}	 
	
	//获取自动延时开关状态 	
	if(SPI_CODE_var.spi_instruction== ID_TYPE1_AUTO_DELAY_SWITCH){
		tcp_send_data((unsigned char *)buf,len*4);
		socket_send_show(buf,len);	
	}		 	
}

