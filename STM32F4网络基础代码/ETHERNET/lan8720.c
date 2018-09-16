#include "lan8720.h"
#include "stm32f4x7_eth.h"
#include "usart.h"  
#include "delay.h" 
#include "pcf8574.h"
#include "stdio.h"
//////////////////////////////////////////////////////////////////////////////////	   
////////////////////////////////////////////////////////////////////////////////// 	
#define CHECKSUM_BY_HARDWARE


ETH_DMADESCTypeDef *DMARxDscrTab;	//以太网DMA接收描述符数据结构体指针
ETH_DMADESCTypeDef *DMATxDscrTab;	//以太网DMA发送描述符数据结构体指针 
uint8_t *Rx_Buff; 					//以太网底层驱动接收buffers指针 
uint8_t *Tx_Buff; 					//以太网底层驱动发送buffers指针
  
//LAN8720初始化
//返回值:0,成功;
//    其他,失败
u8 LAN8720_Init(void)
{
	u8 rval=0;
	//ETH IO接口初始化
 	RCC->AHB1ENR|=1<<0;     //使能PORTA时钟 
 	RCC->AHB1ENR|=1<<1;     //使能PORTB时钟 
 	RCC->AHB1ENR|=1<<2;     //使能PORTC时钟  
	RCC->AHB1ENR|=1<<6;     //使能PORTG时钟 
 	RCC->APB2ENR|=1<<14;   	//使能SYSCFG时钟
	SYSCFG->PMC|=1<<23;		  //使用RMII PHY接口.
	 	
	GPIO_Set(GPIOA,PIN1|PIN2|PIN7,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);		//PA1,2,7复用输出
	GPIO_Set(GPIOC,PIN1|PIN4|PIN5,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);		//PC1,4,5复用输出
	GPIO_Set(GPIOG,PIN13|PIN14,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);		//PG13,14复用输出
	GPIO_Set(GPIOB,PIN11,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);				//PB11复用输出
 
	GPIO_AF_Set(GPIOA,1,11);	//PA1,AF11
	GPIO_AF_Set(GPIOA,2,11);	//PA2,AF11
	GPIO_AF_Set(GPIOA,7,11);	//PA7,AF11

	GPIO_AF_Set(GPIOB,11,11);	//PB11,AF11
 
	GPIO_AF_Set(GPIOC,1,11);	//PC1,AF11
	GPIO_AF_Set(GPIOC,4,11);	//PC4,AF11
	GPIO_AF_Set(GPIOC,5,11);	//PC5,AF11

	GPIO_AF_Set(GPIOG,13,11);	//PG13,AF11
	GPIO_AF_Set(GPIOG,14,11);	//PG14,AF11  
	
	PCF8574_WriteBit(ETH_RESET_IO,1);//硬件复位LAN8720
 	delay_ms(50);	
	PCF8574_WriteBit(ETH_RESET_IO,0);//结束复位

 	MY_NVIC_Init(0,0,ETH_IRQn,2);//配置ETH中的分组
	rval=ETH_MACDMA_Config();
	return !rval;				//ETH的规则为:0,失败;1,成功;所以要取反一下 
}
//得到8720的速度模式
//返回值:
//001:10M半双工
//101:10M全双工
//010:100M半双工
//110:100M全双工
//其他:错误.
u8 LAN8720_Get_Speed(void)
{
	u8 speed;
	speed=((ETH_ReadPHYRegister(0x00,31)&0x1C)>>2); //从LAN8720的31号寄存器中读取网络速度和双工模式
	return speed;
}
/////////////////////////////////////////////////////////////////////////////////////////////////
//以下部分为STM32网卡配置/接口函数.

//初始化ETH MAC层及DMA配置
//返回值:ETH_ERROR,发送失败(0)
//		ETH_SUCCESS,发送成功(1)
u8 ETH_MACDMA_Config(void)
{
	u8 rval;
	ETH_InitTypeDef ETH_InitStructure; 
	
	//使能以太网时钟
	RCC->AHB1ENR|=7<<25;//使能ETH MAC/MAC_Tx/MAC_Rx时钟
	
	ETH_DeInit();  								//AHB总线重启以太网
	ETH_SoftwareReset();  						//软件重启网络
	while (ETH_GetSoftwareResetStatus() == SET);//等待软件重启网络完成 
	ETH_StructInit(&ETH_InitStructure); 	 	//初始化网络为默认值  

	///网络MAC参数设置 
	ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;   			                  //开启网络自适应功能
	ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;					                      //关闭反馈
	ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable; 		                  //关闭重传功能kp
	ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable; 	              //关闭自动去除PDA/CRC功能 
	ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;						                        //关闭接收所有的帧
	ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;         //允许接收所有广播帧
	ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;			                    //关闭混合模式的地址过滤  
	ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;              //对于组播地址使用完美地址过滤   
	ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;	                //对单播地址使用完美地址过滤 
#ifdef CHECKSUM_BY_HARDWARE
	ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable; 			                    //开启ipv4和TCP/UDP/ICMP的帧校验和卸载   
#endif
	//当我们使用帧校验和卸载功能的时候，一定要使能存储转发模式,存储转发模式中要保证整个帧存储在FIFO中,
	//这样MAC能插入/识别出帧校验值,当真校验正确的时候DMA就可以处理帧,否则就丢弃掉该帧
	ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable;   //开启丢弃TCP/IP错误帧
	ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;                   //开启接收数据的存储转发模式    
	ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;                 //开启发送数据的存储转发模式  

	ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;                  	//禁止转发错误帧  
	ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;	//不转发过小的好帧 
	ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;  		                //打开处理第二帧功能
	ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;  	                //开启DMA传输的地址对齐功能
	ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;            			                    //开启固定突发功能    
	ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;     		                //DMA发送的最大突发长度为32个节拍   
	ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;			                    //DMA接收的最大突发长度为32个节拍
	ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;
	ETH_InitStructure.Sys_Clock_Freq=192;//系统时钟频率为192Mhz
	rval=ETH_Init(&ETH_InitStructure,LAN8720_PHY_ADDRESS);//配置ETH
	if(rval==ETH_SUCCESS)//配置成功
	{
		printf("enable network interrupt\r\n");
		ETH_DMAITConfig(ETH_DMA_IT_NIS|ETH_DMA_IT_R,ENABLE);//使能以太网接收中断	
	}
	return rval;
}

void mac_init(void)
{
	int i = 0;
	unsigned char hwaddr[6];
	hwaddr[0] = 2;
	hwaddr[1] = 0;	
	hwaddr[2] = 0;
	hwaddr[3] = 1;
	hwaddr[4] = 2;
	hwaddr[5] = 3;
	
	ETH_MACAddressConfig(ETH_MAC_Address0, hwaddr); //向STM32F4的MAC地址寄存器中写入MAC地址
	ETH_DMATxDescChainInit(DMATxDscrTab, Tx_Buff, ETH_TXBUFNB);
	ETH_DMARxDescChainInit(DMARxDscrTab, Rx_Buff, ETH_RXBUFNB);	
	
	for(i=0;i<ETH_TXBUFNB;i++)	//使能TCP,UDP和ICMP的发送帧校验,TCP,UDP和ICMP的接收帧校验在DMA中配置了
	{
		ETH_DMATxDescChecksumInsertionConfig(&DMATxDscrTab[i], ETH_DMATxDesc_ChecksumTCPUDPICMPFull);
	}	
	
	ETH_Start();
}

int eth_interrupt_flag = 0;
int eth_len = 0;

ETH_DMADESCTypeDef DMARxDscrTab_array[ETH_RXBUFNB*sizeof(ETH_DMADESCTypeDef)];
ETH_DMADESCTypeDef DMATxDscrTab_array[ETH_TXBUFNB*sizeof(ETH_DMADESCTypeDef)];
uint8_t Rx_Buff_array[ETH_RX_BUF_SIZE*ETH_RXBUFNB];					  //以太网底层驱动接收buffers指针 
uint8_t Tx_Buff_array[ETH_TX_BUF_SIZE*ETH_TXBUFNB]; 					//以太网底层驱动发送buffers指针	



unsigned char test_buf1[]={
	0x00, 0xe0, 0x4c, 0x08, 0xcd, 0x77, 0x02, 0x00,     0x00, 0x01, 0x02, 0x03, 0x08, 0x06, 0x00, 0x01,     
  0x08, 0x00, 0x06, 0x04, 0x00, 0x02, 0x02, 0x00,     0x00, 0x01, 0x02, 0x03, 0xc0, 0xa8, 0x01, 0x03,     
  0x00, 0xe0, 0x4c, 0x08, 0xcd, 0x77, 0xc0, 0xa8,     0x01, 0x02
};

unsigned char icmp_reply[]={
	0x00, 0xe0, 0x4c, 0x08, 0xcd, 0x77, 0x02, 0x00,     0x00, 0x01, 0x02, 0x03, 0x08, 0x00, 0x45, 0x00,     
  0x00, 0x3C, 0xa0, 0xc4, 0x00, 0x00, 0x40, 0x01,     0x00, 0x00, 0xc0, 0xa8, 0x01, 0x03, 0xc0, 0xa8,     
	0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,     0x00, 0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
	0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e,     0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 
	0x77, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,     0x68, 0x69
};


unsigned char tcp_reply[58]={
	0x00, 0xe0, 0x4c, 0x08, 0xcd, 0x77, 0x02, 0x00,            0x00, 0x01, 0x02, 0x03, 0x08, 0x00, 0x45, 0x00,     
  0x00, 0x2C,/*len*/ 0xa0, 0xc4, 0x00, 0x00, 0x40, 0x06/*tcp*/,     0x00, 0x00, 0xc0, 0xa8, 0x01, 0x03, 0xc0, 0xa8,     
	0x01, 0x02,/*head*/0x12, 0x34, 0xc3, 0x50, /*port*/0x00, 0x00,     0x00, 0x00,/*seq*/ 0x00, 0x00, 0x00, 0x01,/*ack*/ 0x50, 0x10,
  0x00, 0x01,	0x00, 0x00,/*crc*/ 0x00, 0x00, 
};

unsigned short checksum(unsigned short *buf,int nword)
{
	unsigned long sum;
	for(sum=0;nword>0;nword--)
	{
		sum += *buf++;
		sum = (sum>>16) + (sum&0xffff);
	}
	return ~sum;
}

static unsigned short net_crc[100];

unsigned short net_checksum(unsigned char *buf,int len)
{
	unsigned short sum;
	unsigned short val;
	int i = 0 ;

	
	//memset(net_crc,0,sizeof(net_crc));
	
	for(i = 0; i < len ; i = i + 2){
		net_crc[i/2] = ((buf[i] << 8) + buf[i+1]);
	}
	
	//for(i = 0; i < len /2; i = i + 1){
	//	printf("0x%0x " ,net_crc[i]);
	//}
	//printf("\r\n");
	
	sum = checksum(net_crc,len/2);
	//printf("0x%0x \r\n",sum);
	
	return sum;
}

char ip_address[]={192,168,1,3};

void anal_buff( void )
{
	unsigned char *buffer;
	unsigned short crc = 0;
	unsigned short value = 0;
	unsigned short *p = (unsigned short *)&icmp_reply[14];
	int i ;
	int ret;
	if(*((unsigned short *)&Rx_Buff[12]) == 0x0608){
	/*arp protocol*/
		if(*((unsigned int *)&Rx_Buff[38]) == 0x0301a8c0){
			buffer=(u8 *)ETH_GetCurrentTxBuffer(); 
			for(i = 0; i < 42 ; i ++){
					buffer[i] = test_buf1[i];
			}
			ret = ETH_Tx_Packet(42);
			if(ret==ETH_ERROR){
					printf("error ...\r\n");
			}			
		}
	}
	/*icmp protocol*/
	if(*((unsigned short *)&Rx_Buff[12]) == 0x0008){
			if(Rx_Buff[23] == 0x01){
			
				p = (unsigned short *)&icmp_reply[14];
				crc = checksum(p,10);
				icmp_reply[24] = ((crc >> 8)&0xff);
				icmp_reply[25] = ((crc)&0xff);
				
				icmp_reply[40] = Rx_Buff[40];
				icmp_reply[41] = Rx_Buff[41];
				
				//crc = net_checksum(&icmp_reply[34],40);
				icmp_reply[36] = 0;//((crc >> 8)&0xff);
				icmp_reply[37] = 0;//((crc)&0xff);		

				//printf("icmp_reply[36]:0x%0x \r\n",icmp_reply[36]);
				//printf("icmp_reply[37]:0x%0x \r\n",icmp_reply[37]);
				//printf("0x%0x, 0x%0x\r\n",icmp_reply[36],icmp_reply[37]);
				//for(i = 0 ; i < 40 ; i ++){
				//	printf("%0x ",icmp_reply[34+i]);
				//}
				//printf("\r\n");
				
				buffer=(u8 *)ETH_GetCurrentTxBuffer(); 
				for(i = 0; i < 74 ; i ++){
						buffer[i] = icmp_reply[i];
				}
				ret = ETH_Tx_Packet(74);
				if(ret==ETH_ERROR){
						printf("error ...\r\n");
				}	

				icmp_reply[24] = 0;
				icmp_reply[25] = 0;
				
				icmp_reply[34] = 0;
				icmp_reply[35] = 0;
				icmp_reply[36] = 0;
				icmp_reply[37] = 0;
				icmp_reply[40] = 0;
				icmp_reply[41] = 0;		
			}
			
			if(Rx_Buff[23] == 0x06){
					value = (Rx_Buff[38]<<24) + (Rx_Buff[39]<<16) + (Rx_Buff[40]<<8)+(Rx_Buff[41])+1;
					tcp_reply[38] = Rx_Buff[42];
					tcp_reply[39] = Rx_Buff[43];
					tcp_reply[40] = Rx_Buff[44];
					tcp_reply[41] = Rx_Buff[45];
				
					tcp_reply[42] = (value>>24);
					tcp_reply[43] = (value>>16)&0xff;
					tcp_reply[44] = (value>>8)&0xff;
					tcp_reply[45] = (value)&0xff;	
				
					tcp_reply[48] = Rx_Buff[48];
					tcp_reply[49] = Rx_Buff[49];
				
					tcp_reply[54] = Rx_Buff[54];
					tcp_reply[55] = Rx_Buff[55];
					tcp_reply[56] = Rx_Buff[56];
					tcp_reply[57] = Rx_Buff[57];			
				
					buffer=(u8 *)ETH_GetCurrentTxBuffer(); 
					for(i = 0; i < 58 ; i ++){
							buffer[i] = tcp_reply[i];
					}		
					ret = ETH_Tx_Packet(58);
					if(ret==ETH_ERROR){
							printf("error ...\r\n");
					}			
			}			
			
	}
	

	
	
}

//以太网中断服务函数
void ETH_IRQHandler(void)
{ 
	unsigned short len;
	int i = 0;
	
	__IO ETH_DMADESCTypeDef *DMARxNextDesc;
	
#if 0	
	while(ETH_GetRxPktSize(DMARxDescToGet)!=0) 	//检测是否收到数据包
	{ 		  
		
		//lwip_packet_handler();
		FrameTypeDef frame;
		frame=ETH_Rx_Packet();	
		len=frame.length;//得到包大小
		
		eth_interrupt_flag = 1;
		eth_len= len;
		
		//printf("interrupt 1:%d....\r\n",len);

		if((ETH->DMASR&ETH_DMASR_RBUS)!=(u32)RESET)//当Rx Buffer不可用位(RBUS)被设置的时候,重置它.恢复传输
		{ 
			ETH->DMASR=ETH_DMASR_RBUS;//重置ETH DMA RBUS位 
			ETH->DMARPDR=0;//恢复DMA接收
		}		
	} 
#else
	while(ETH_CheckFrameReceived()){
		FrameTypeDef frame;
		frame = ETH_Get_Received_Frame();
		eth_len = frame.length;
		Rx_Buff = (uint8_t *)frame.buffer;
		
		anal_buff();
		
    /* Check if frame with multiple DMA buffer segments */
    if (DMA_RX_FRAME_infos->Seg_Count > 1) {
        DMARxNextDesc = DMA_RX_FRAME_infos->FS_Rx_Desc;
    } else {
        DMARxNextDesc = frame.descriptor;
    }

    /* Set Own bit in Rx descriptors: gives the buffers back to DMA */
    for (i = 0; i < DMA_RX_FRAME_infos->Seg_Count; i++) {
        DMARxNextDesc->Status = ETH_DMARxDesc_OWN;
        DMARxNextDesc = (ETH_DMADESCTypeDef *)(DMARxNextDesc->Buffer2NextDescAddr);
    }

    /* Clear Segment_Count */
    DMA_RX_FRAME_infos->Seg_Count = 0;
		
    if ((ETH->DMASR & ETH_DMASR_RBUS) != (u32)RESET) {
        /* Clear RBUS ETHERNET DMA flag */
        ETH->DMASR = ETH_DMASR_RBUS;
        /* Resume DMA reception */
        ETH->DMARPDR = 0;
    }
	}

#endif	
	
	ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
	ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS); 
}  
//接收一个网卡数据包
//返回值:网络数据包帧结构体
FrameTypeDef ETH_Rx_Packet(void)
{ 
	u32 framelength=0;
	FrameTypeDef frame={0,0};   
	//检查当前描述符,是否属于ETHERNET DMA(设置的时候)/CPU(复位的时候)
	if((DMARxDescToGet->Status&ETH_DMARxDesc_OWN)!=(u32)RESET)
	{	
		frame.length=ETH_ERROR; 
		if ((ETH->DMASR&ETH_DMASR_RBUS)!=(u32)RESET)  
		{ 
			ETH->DMASR = ETH_DMASR_RBUS;//清除ETH DMA的RBUS位 
			ETH->DMARPDR=0;//恢复DMA接收
		}
		return frame;//错误,OWN位被设置了
	}  
	if(((DMARxDescToGet->Status&ETH_DMARxDesc_ES)==(u32)RESET)&& 
	((DMARxDescToGet->Status & ETH_DMARxDesc_LS)!=(u32)RESET)&&  
	((DMARxDescToGet->Status & ETH_DMARxDesc_FS)!=(u32)RESET))  
	{       
		framelength=((DMARxDescToGet->Status&ETH_DMARxDesc_FL)>>ETH_DMARxDesc_FrameLengthShift)-4;//得到接收包帧长度(不包含4字节CRC)
 		frame.buffer = DMARxDescToGet->Buffer1Addr;//得到包数据所在的位置
	}else 
		framelength=ETH_ERROR;//错误  
	frame.length=framelength; 
	frame.descriptor=DMARxDescToGet;  
	//更新ETH DMA全局Rx描述符为下一个Rx描述符
	//为下一次buffer读取设置下一个DMA Rx描述符
	DMARxDescToGet=(ETH_DMADESCTypeDef*)(DMARxDescToGet->Buffer2NextDescAddr);   
	return frame;  
}
//发送一个网卡数据包
//FrameLength:数据包长度
//返回值:ETH_ERROR,发送失败(0)
//		ETH_SUCCESS,发送成功(1)
u8 ETH_Tx_Packet(u16 FrameLength)
{   
	//检查当前描述符,是否属于ETHERNET DMA(设置的时候)/CPU(复位的时候)
	if((DMATxDescToSet->Status&ETH_DMATxDesc_OWN)!=(u32)RESET)return ETH_ERROR;//错误,OWN位被设置了 
 	DMATxDescToSet->ControlBufferSize=(FrameLength&ETH_DMATxDesc_TBS1);//设置帧长度,bits[12:0]
	DMATxDescToSet->Status|=ETH_DMATxDesc_LS|ETH_DMATxDesc_FS;//设置最后一个和第一个位段置位(1个描述符传输一帧)
  DMATxDescToSet->Status|=ETH_DMATxDesc_OWN;//设置Tx描述符的OWN位,buffer重归ETH DMA
	if((ETH->DMASR&ETH_DMASR_TBUS)!=(u32)RESET)//当Tx Buffer不可用位(TBUS)被设置的时候,重置它.恢复传输
	{ 
		ETH->DMASR=ETH_DMASR_TBUS;//重置ETH DMA TBUS位 
		ETH->DMATPDR=0;//恢复DMA发送
	} 
	//更新ETH DMA全局Tx描述符为下一个Tx描述符
	//为下一次buffer发送设置下一个DMA Tx描述符 
	DMATxDescToSet=(ETH_DMADESCTypeDef*)(DMATxDescToSet->Buffer2NextDescAddr);    
	return ETH_SUCCESS;   
}
//得到当前描述符的Tx buffer地址
//返回值:Tx buffer地址
u32 ETH_GetCurrentTxBuffer(void)
{  
  return DMATxDescToSet->Buffer1Addr;//返回Tx buffer地址  
}




//为ETH底层驱动申请内存
//返回值:0,正常
//    其他,失败
u8 ETH_Mem_Malloc(void)
{ 
	
#if 0
	DMARxDscrTab = mymalloc(SRAMIN,ETH_RXBUFNB*sizeof(ETH_DMADESCTypeDef));//申请内存
	DMATxDscrTab = mymalloc(SRAMIN,ETH_TXBUFNB*sizeof(ETH_DMADESCTypeDef));//申请内存  
	Rx_Buff=mymalloc(SRAMIN,ETH_RX_BUF_SIZE*ETH_RXBUFNB);	//申请内存
	Tx_Buff=mymalloc(SRAMIN,ETH_TX_BUF_SIZE*ETH_TXBUFNB);	//申请内存
	if(!DMARxDscrTab||!DMATxDscrTab||!Rx_Buff||!Tx_Buff)
	{
		ETH_Mem_Free();
		return 1;	//申请失败
	}	
	return 0;		//申请成功
#else
	DMARxDscrTab = DMARxDscrTab_array;
	DMATxDscrTab = DMATxDscrTab_array;
	Rx_Buff = Rx_Buff_array;
	Tx_Buff = Tx_Buff_array;
	
	return 0;
#endif	
}
//释放ETH 底层驱动申请的内存
void ETH_Mem_Free(void)
{ 
#if 0
	myfree(SRAMIN,DMARxDscrTab);//释放内存
	myfree(SRAMIN,DMATxDscrTab);//释放内存
	myfree(SRAMIN,Rx_Buff);		//释放内存
	myfree(SRAMIN,Tx_Buff);		//释放内存 
#endif	
}


