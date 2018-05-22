/*
 * protocol.h
 *
 *  Created on:  
 * Corporation:  
 *      Author:  
 * Description:  
 */

#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include <stdio.h>

#include "inc.h"

#define AUDIO_CHANS    8
#define GEQ_BANDS      27
#define PEQ_NUMS       5

typedef struct{
	unsigned int id;
	unsigned int (*fun)(unsigned int param1,unsigned int param2,unsigned int param3);
}SPI_HANDLE_T;


typedef struct {
	/*type3*/
	signed char  type3_gain[AUDIO_CHANS];                        /*各声道增益值*/
	signed char  type3_all_gain;		                          /*整体增益值*/
	unsigned short  type3_delay[AUDIO_CHANS];             /*各声道延时*/
	unsigned short  type3_all_delay; 		              /*整体延时*/
	unsigned short  type3_high_cut[AUDIO_CHANS];          /*各声道高切频率*/
	unsigned short  type3_low_cut[AUDIO_CHANS];                     /*各声道低切频率*/
 	unsigned short  type3_gradual_max;                    /*渐强时间*/
	unsigned short  type3_gradual_min;                    /*渐弱时间*/
	char  type3_volume;                                   /*旋钮音量*/
 	char  type3_freq_divnums[AUDIO_CHANS];
	unsigned short type3_freq_point[AUDIO_CHANS][2];		       
	char type3_output_chans[AUDIO_CHANS][3];
	/*type4*/
	signed char type4_geq_gain[AUDIO_CHANS][GEQ_BANDS];   /*参数增益值*/
	char type4_peq_type[AUDIO_CHANS][PEQ_NUMS];           /*PEQ类型*/
	unsigned short type4_peq_freq[AUDIO_CHANS][PEQ_NUMS]; /*PEQ中心频率*/
	unsigned char type4_peq_q[AUDIO_CHANS][PEQ_NUMS];     /*PEQ的q值*/
	signed char type4_peq_gain[AUDIO_CHANS][PEQ_NUMS];    /*PEQ的增益*/
	signed char type4_all_geq_gain[GEQ_BANDS];        	  /*所有声道的GEQ*/
	/*type6*/
 	unsigned short type6_ref_SPL;                         /*自动校准参考声压*/
   	unsigned short type6_real_SPL;                        /*自动校准实际声压*/
	char type6_switch;									  /*自动校准开关*/
	char type6_current_align;							  /*当前校准声道*/
	char type6_all_align;							      /*当前校准声道,所有声道*/
	char type6_signal_type;							      /*信号发生器类型*/
	char type6_work_mode;							      /*当前工作模式*/
	unsigned char type6_spectrum_energy;				  /*频谱能量*/
	unsigned char type6_in_loudness[AUDIO_CHANS];		  /*输入响度*/
	unsigned char type6_out_loudness[AUDIO_CHANS];		  /*输出响度*/
}SPI_DATA_T;


#define GET_ID(x,y,z,a,b,c) ((x<<31) | (y<<27) | (z<<23) | (a<<18) | (b<<13) | c)

#define SET_DATA 0 
//set
/* type 1*/
#define ID_TYPE1_READ_CFG_FROM_DEVICE  	(GET_ID(SET_DATA,1,3,0,0,0))
#define ID_TYPE1_SAVE_CFG               (GET_ID(SET_DATA,1,4,0,0,0))

#define ID_TYPE1_READ_REAL_SPL          (GET_ID(SET_DATA,1,10,3,0,0))      
#define ID_TYPE1_SOFTWARE_VERSION       (GET_ID(SET_DATA,1,11,0,0,0))    
#define ID_TYPE1_SPECTRUM_ENERGY        (GET_ID(SET_DATA,1,10,4,0,0))
#define ID_TYPE1_CURRENT_ALIGN_CHAN     (GET_ID(SET_DATA,1,10,5,0,0))
#define ID_TYPE1_CURRENT_ALIGN_STATE    (GET_ID(SET_DATA,1,10,6,0,0))
#define ID_TYPE1_AUTO_DELAY             (GET_ID(SET_DATA,1,10,1,0,0))
#define ID_TYPE1_AUTO_DELAY_SWITCH      (GET_ID(SET_DATA,1,10,7,0,0))

//类型2
#define ID_TYPE2_SOUNDFILED_LWH         (GET_ID(SET_DATA,2,1,0,0,0))
#define ID_TYPE2_COORDINATE_XYZ         (GET_ID(SET_DATA,2,2,0,0,0))
#define ID_TYPE2_SOUNDBOX_XYZ           (GET_ID(SET_DATA,2,3,0,0,0))
#define ID_TYPE2_INPUT_TYPE             (GET_ID(SET_DATA,2,4,0,0,0))
#define ID_TYPE2_INPUT_CHANS            (GET_ID(SET_DATA,2,9,0,0,0))
#define ID_TYPE2_FACTORY_MODE           (GET_ID(SET_DATA,2,13,0,0,0))
 
 
 

#define ID_TYPE3_GAIN                   (GET_ID(SET_DATA,3,1,0,0,0))
#define ID_TYPE3_DELAY                  (GET_ID(SET_DATA,3,2,0,0,0))
#define ID_TYPE3_HIGH_LOWCUT_POINT      (GET_ID(SET_DATA,3,3,0,0,0))
#define ID_TYPE3_GRADUAL                (GET_ID(SET_DATA,3,4,0,0,0))/*渐强渐弱*/
#define ID_TYPE3_VOLUME                 (GET_ID(SET_DATA,3,6,0,0,0))
#define ID_TYPE3_FREQ_DIVNUMS           (GET_ID(SET_DATA,3,11,0,0,0))
#define ID_TYPE3_FREQ_POINT             (GET_ID(SET_DATA,3,12,0,0,0))
#define ID_TYPE3_OUTPUT_CHANS           (GET_ID(SET_DATA,3,13,0,0,0)) 


/*type4*/
#define ID_TYPE4_GEQ_GAIN               (GET_ID(SET_DATA,4,1,0,0,0))
#define ID_TYPE4_RESET_GEQ_GAIN         (GET_ID(SET_DATA,4,3,0,0,0))
#define ID_TYPE4_PEQ_TYPE               (GET_ID(SET_DATA,4,4,0,0,0))
#define ID_TYPE4_PEQ_FREQ               (GET_ID(SET_DATA,4,6,0,0,0))
#define ID_TYPE4_PEQ_Q                  (GET_ID(SET_DATA,4,8,0,0,0))
#define ID_TYPE4_PEQ_GAIN               (GET_ID(SET_DATA,4,10,0,0,0))
#define ID_TYPE4_ALL_GEQ_GAIN           (GET_ID(SET_DATA,4,13,0,0,0))

/*type6*/
#define ID_TYPE6_ALIGNMENT             (GET_ID(SET_DATA,6,4,0,0,0)) 
#define ID_TYPE6_ALLALIGNMENT          (GET_ID(SET_DATA,6,5,0,0,0))
#define ID_TYPE6_SIGNAL_TYPE           (GET_ID(SET_DATA,6,6,0,0,0))
#define ID_TYPE6_WORK_MODE             (GET_ID(SET_DATA,6,7,0,0,0))
    

extern unsigned int set_type1_read_cfg_from_device(unsigned int param1,unsigned int param2,unsigned int param3);
extern unsigned int set_type1_save_cfg(unsigned int param1,unsigned int param2,unsigned int param3);
extern unsigned int set_type2_soundfiled_lwh( unsigned int param1,unsigned int param2,unsigned int param3);
extern unsigned int set_type2_factory_mode( unsigned int param1,unsigned int param2,unsigned int param3);
   
/*type3*/
extern unsigned int set_type3_gain(unsigned int param1,unsigned int param2,unsigned int param3);
extern unsigned int set_type3_delay(unsigned int param1,unsigned int param2,unsigned int param3);
extern unsigned int set_type3_high_low_cut_point(unsigned int param1,unsigned int param2,unsigned int param3);
extern unsigned int set_type3_gradual(unsigned int param1,unsigned int param2,unsigned int param3);
extern unsigned int set_type3_volume(unsigned int param1,unsigned int param2,unsigned int param3);
extern unsigned int set_type3_freq_divnums(unsigned int param1,unsigned int param2,unsigned int param3);
extern unsigned int set_type3_freq_point(unsigned int param1,unsigned int param2,unsigned int param3);
extern unsigned int set_type3_output_chans(unsigned int param1,unsigned int param2,unsigned int param3);
/*type4*/
extern unsigned int set_type4_geq_gain(unsigned int param1,unsigned int param2,unsigned int param3);
extern unsigned int set_type4_reset_geq_gain(unsigned int param1,unsigned int param2,unsigned int param3);
extern unsigned int set_type4_peq_type(unsigned int param1,unsigned int param2,unsigned int param3);
extern unsigned int set_type4_peq_freq(unsigned int param1,unsigned int param2,unsigned int param3);
extern unsigned int set_type4_peq_q(unsigned int param1,unsigned int param2,unsigned int param3);
extern unsigned int set_type4_peq_gain(unsigned int param1,unsigned int param2,unsigned int param3);
extern unsigned int set_type4_all_geq_gain(unsigned int param1,unsigned int param2,unsigned int param3);
	

#endif /* __PROTOCOL_H */




