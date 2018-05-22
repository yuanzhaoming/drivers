#ifndef _file_h_
#define _file_h_

/*tftp data and file size address reserved 200KB for it*/
#define TFTP_DATA_ADDR     			256
#define TFTP_FILE_SIZE_ADDR         0


#define GENERAL_PARAM               0x32000 /*reserved for 200KB*/
#define MEMORY_CFG_BUF              1024



enum SAVE_PARAMS{
 	GENERAL_P,
	OUTPUT_P,
	EQ_P,
	ALL_P,
}; 

enum READ_DIR{
 	TO_CFG,
	TO_MEMORY,
};

/*read write directions*/
enum DIRECTIONS{
	D_WRITE,
	D_READ,	
};
/*device like memory,cfg or eeprom*/
enum DEVICE{
  DEV_APP_TO_CFG,
  DEV_CFG_TO_MEMORY,
  DEV_MEMORY_TO_EEPROM,
  /*read*/
  DEV_MEMORY_TO_CFG,
  DEV_EEPROM_TO_MEMORY,
};

enum INSTRUCTIONS_TYPE{
  TYPE_GENERAL,
  TYPE_OUTPUT,
  TYPE_EQ,
  TYPE_MAX,
};


#endif




