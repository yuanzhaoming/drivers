#include "lan8720.h"
#include "stm32f4x7_eth.h"
#include "usart.h"  
#include "delay.h" 
#include "pcf8574.h"
#include "stdio.h"
//////////////////////////////////////////////////////////////////////////////////	   
////////////////////////////////////////////////////////////////////////////////// 	
#define CHECKSUM_BY_HARDWARE


ETH_DMADESCTypeDef *DMARxDscrTab;	//��̫��DMA�������������ݽṹ��ָ��
ETH_DMADESCTypeDef *DMATxDscrTab;	//��̫��DMA�������������ݽṹ��ָ�� 
uint8_t *Rx_Buff; 					//��̫���ײ���������buffersָ�� 
uint8_t *Tx_Buff; 					//��̫���ײ���������buffersָ��
  
//LAN8720��ʼ��
//����ֵ:0,�ɹ�;
//    ����,ʧ��
u8 LAN8720_Init(void)
{
	u8 rval=0;
	//ETH IO�ӿڳ�ʼ��
 	RCC->AHB1ENR|=1<<0;     //ʹ��PORTAʱ�� 
 	RCC->AHB1ENR|=1<<1;     //ʹ��PORTBʱ�� 
 	RCC->AHB1ENR|=1<<2;     //ʹ��PORTCʱ��  
	RCC->AHB1ENR|=1<<6;     //ʹ��PORTGʱ�� 
 	RCC->APB2ENR|=1<<14;   	//ʹ��SYSCFGʱ��
	SYSCFG->PMC|=1<<23;		  //ʹ��RMII PHY�ӿ�.
	 	
	GPIO_Set(GPIOA,PIN1|PIN2|PIN7,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);		//PA1,2,7�������
	GPIO_Set(GPIOC,PIN1|PIN4|PIN5,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);		//PC1,4,5�������
	GPIO_Set(GPIOG,PIN13|PIN14,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);		//PG13,14�������
	GPIO_Set(GPIOB,PIN11,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);				//PB11�������
 
	GPIO_AF_Set(GPIOA,1,11);	//PA1,AF11
	GPIO_AF_Set(GPIOA,2,11);	//PA2,AF11
	GPIO_AF_Set(GPIOA,7,11);	//PA7,AF11

	GPIO_AF_Set(GPIOB,11,11);	//PB11,AF11
 
	GPIO_AF_Set(GPIOC,1,11);	//PC1,AF11
	GPIO_AF_Set(GPIOC,4,11);	//PC4,AF11
	GPIO_AF_Set(GPIOC,5,11);	//PC5,AF11

	GPIO_AF_Set(GPIOG,13,11);	//PG13,AF11
	GPIO_AF_Set(GPIOG,14,11);	//PG14,AF11  
	
	PCF8574_WriteBit(ETH_RESET_IO,1);//Ӳ����λLAN8720
 	delay_ms(50);	
	PCF8574_WriteBit(ETH_RESET_IO,0);//������λ

 	MY_NVIC_Init(0,0,ETH_IRQn,2);//����ETH�еķ���
	rval=ETH_MACDMA_Config();
	return !rval;				//ETH�Ĺ���Ϊ:0,ʧ��;1,�ɹ�;����Ҫȡ��һ�� 
}
//�õ�8720���ٶ�ģʽ
//����ֵ:
//001:10M��˫��
//101:10Mȫ˫��
//010:100M��˫��
//110:100Mȫ˫��
//����:����.
u8 LAN8720_Get_Speed(void)
{
	u8 speed;
	speed=((ETH_ReadPHYRegister(0x00,31)&0x1C)>>2); //��LAN8720��31�żĴ����ж�ȡ�����ٶȺ�˫��ģʽ
	return speed;
}
/////////////////////////////////////////////////////////////////////////////////////////////////
//���²���ΪSTM32��������/�ӿں���.

//��ʼ��ETH MAC�㼰DMA����
//����ֵ:ETH_ERROR,����ʧ��(0)
//		ETH_SUCCESS,���ͳɹ�(1)
u8 ETH_MACDMA_Config(void)
{
	u8 rval;
	ETH_InitTypeDef ETH_InitStructure; 
	
	//ʹ����̫��ʱ��
	RCC->AHB1ENR|=7<<25;//ʹ��ETH MAC/MAC_Tx/MAC_Rxʱ��
	
	ETH_DeInit();  								//AHB����������̫��
	ETH_SoftwareReset();  						//�����������
	while (ETH_GetSoftwareResetStatus() == SET);//�ȴ��������������� 
	ETH_StructInit(&ETH_InitStructure); 	 	//��ʼ������ΪĬ��ֵ  

	///����MAC�������� 
	ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;   			                  //������������Ӧ����
	ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;					                      //�رշ���
	ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable; 		                  //�ر��ش�����kp
	ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable; 	              //�ر��Զ�ȥ��PDA/CRC���� 
	ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;						                        //�رս������е�֡
	ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;         //����������й㲥֡
	ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;			                    //�رջ��ģʽ�ĵ�ַ����  
	ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;              //�����鲥��ַʹ��������ַ����   
	ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;	                //�Ե�����ַʹ��������ַ���� 
#ifdef CHECKSUM_BY_HARDWARE
	ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable; 			                    //����ipv4��TCP/UDP/ICMP��֡У���ж��   
#endif
	//������ʹ��֡У���ж�ع��ܵ�ʱ��һ��Ҫʹ�ܴ洢ת��ģʽ,�洢ת��ģʽ��Ҫ��֤����֡�洢��FIFO��,
	//����MAC�ܲ���/ʶ���֡У��ֵ,����У����ȷ��ʱ��DMA�Ϳ��Դ���֡,����Ͷ�������֡
	ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable;   //��������TCP/IP����֡
	ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;                   //�����������ݵĴ洢ת��ģʽ    
	ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;                 //�����������ݵĴ洢ת��ģʽ  

	ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;                  	//��ֹת������֡  
	ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;	//��ת����С�ĺ�֡ 
	ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;  		                //�򿪴���ڶ�֡����
	ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;  	                //����DMA����ĵ�ַ���빦��
	ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;            			                    //�����̶�ͻ������    
	ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;     		                //DMA���͵����ͻ������Ϊ32������   
	ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;			                    //DMA���յ����ͻ������Ϊ32������
	ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;
	ETH_InitStructure.Sys_Clock_Freq=192;//ϵͳʱ��Ƶ��Ϊ192Mhz
	rval=ETH_Init(&ETH_InitStructure,LAN8720_PHY_ADDRESS);//����ETH
	if(rval==ETH_SUCCESS)//���óɹ�
	{
		printf("enable network interrupt\r\n");
		ETH_DMAITConfig(ETH_DMA_IT_NIS|ETH_DMA_IT_R,ENABLE);//ʹ����̫�������ж�	
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
	
	ETH_MACAddressConfig(ETH_MAC_Address0, hwaddr); //��STM32F4��MAC��ַ�Ĵ�����д��MAC��ַ
	ETH_DMATxDescChainInit(DMATxDscrTab, Tx_Buff, ETH_TXBUFNB);
	ETH_DMARxDescChainInit(DMARxDscrTab, Rx_Buff, ETH_RXBUFNB);	
	
	for(i=0;i<ETH_TXBUFNB;i++)	//ʹ��TCP,UDP��ICMP�ķ���֡У��,TCP,UDP��ICMP�Ľ���֡У����DMA��������
	{
		ETH_DMATxDescChecksumInsertionConfig(&DMATxDscrTab[i], ETH_DMATxDesc_ChecksumTCPUDPICMPFull);
	}	
	
	ETH_Start();
}

int eth_interrupt_flag = 0;
int eth_len = 0;

ETH_DMADESCTypeDef DMARxDscrTab_array[ETH_RXBUFNB*sizeof(ETH_DMADESCTypeDef)];
ETH_DMADESCTypeDef DMATxDscrTab_array[ETH_TXBUFNB*sizeof(ETH_DMADESCTypeDef)];
uint8_t Rx_Buff_array[ETH_RX_BUF_SIZE*ETH_RXBUFNB];					  //��̫���ײ���������buffersָ�� 
uint8_t Tx_Buff_array[ETH_TX_BUF_SIZE*ETH_TXBUFNB]; 					//��̫���ײ���������buffersָ��	



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

//��̫���жϷ�����
void ETH_IRQHandler(void)
{ 
	unsigned short len;
	int i = 0;
	
	__IO ETH_DMADESCTypeDef *DMARxNextDesc;
	
#if 0	
	while(ETH_GetRxPktSize(DMARxDescToGet)!=0) 	//����Ƿ��յ����ݰ�
	{ 		  
		
		//lwip_packet_handler();
		FrameTypeDef frame;
		frame=ETH_Rx_Packet();	
		len=frame.length;//�õ�����С
		
		eth_interrupt_flag = 1;
		eth_len= len;
		
		//printf("interrupt 1:%d....\r\n",len);

		if((ETH->DMASR&ETH_DMASR_RBUS)!=(u32)RESET)//��Rx Buffer������λ(RBUS)�����õ�ʱ��,������.�ָ�����
		{ 
			ETH->DMASR=ETH_DMASR_RBUS;//����ETH DMA RBUSλ 
			ETH->DMARPDR=0;//�ָ�DMA����
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
//����һ���������ݰ�
//����ֵ:�������ݰ�֡�ṹ��
FrameTypeDef ETH_Rx_Packet(void)
{ 
	u32 framelength=0;
	FrameTypeDef frame={0,0};   
	//��鵱ǰ������,�Ƿ�����ETHERNET DMA(���õ�ʱ��)/CPU(��λ��ʱ��)
	if((DMARxDescToGet->Status&ETH_DMARxDesc_OWN)!=(u32)RESET)
	{	
		frame.length=ETH_ERROR; 
		if ((ETH->DMASR&ETH_DMASR_RBUS)!=(u32)RESET)  
		{ 
			ETH->DMASR = ETH_DMASR_RBUS;//���ETH DMA��RBUSλ 
			ETH->DMARPDR=0;//�ָ�DMA����
		}
		return frame;//����,OWNλ��������
	}  
	if(((DMARxDescToGet->Status&ETH_DMARxDesc_ES)==(u32)RESET)&& 
	((DMARxDescToGet->Status & ETH_DMARxDesc_LS)!=(u32)RESET)&&  
	((DMARxDescToGet->Status & ETH_DMARxDesc_FS)!=(u32)RESET))  
	{       
		framelength=((DMARxDescToGet->Status&ETH_DMARxDesc_FL)>>ETH_DMARxDesc_FrameLengthShift)-4;//�õ����հ�֡����(������4�ֽ�CRC)
 		frame.buffer = DMARxDescToGet->Buffer1Addr;//�õ����������ڵ�λ��
	}else 
		framelength=ETH_ERROR;//����  
	frame.length=framelength; 
	frame.descriptor=DMARxDescToGet;  
	//����ETH DMAȫ��Rx������Ϊ��һ��Rx������
	//Ϊ��һ��buffer��ȡ������һ��DMA Rx������
	DMARxDescToGet=(ETH_DMADESCTypeDef*)(DMARxDescToGet->Buffer2NextDescAddr);   
	return frame;  
}
//����һ���������ݰ�
//FrameLength:���ݰ�����
//����ֵ:ETH_ERROR,����ʧ��(0)
//		ETH_SUCCESS,���ͳɹ�(1)
u8 ETH_Tx_Packet(u16 FrameLength)
{   
	//��鵱ǰ������,�Ƿ�����ETHERNET DMA(���õ�ʱ��)/CPU(��λ��ʱ��)
	if((DMATxDescToSet->Status&ETH_DMATxDesc_OWN)!=(u32)RESET)return ETH_ERROR;//����,OWNλ�������� 
 	DMATxDescToSet->ControlBufferSize=(FrameLength&ETH_DMATxDesc_TBS1);//����֡����,bits[12:0]
	DMATxDescToSet->Status|=ETH_DMATxDesc_LS|ETH_DMATxDesc_FS;//�������һ���͵�һ��λ����λ(1������������һ֡)
  DMATxDescToSet->Status|=ETH_DMATxDesc_OWN;//����Tx��������OWNλ,buffer�ع�ETH DMA
	if((ETH->DMASR&ETH_DMASR_TBUS)!=(u32)RESET)//��Tx Buffer������λ(TBUS)�����õ�ʱ��,������.�ָ�����
	{ 
		ETH->DMASR=ETH_DMASR_TBUS;//����ETH DMA TBUSλ 
		ETH->DMATPDR=0;//�ָ�DMA����
	} 
	//����ETH DMAȫ��Tx������Ϊ��һ��Tx������
	//Ϊ��һ��buffer����������һ��DMA Tx������ 
	DMATxDescToSet=(ETH_DMADESCTypeDef*)(DMATxDescToSet->Buffer2NextDescAddr);    
	return ETH_SUCCESS;   
}
//�õ���ǰ��������Tx buffer��ַ
//����ֵ:Tx buffer��ַ
u32 ETH_GetCurrentTxBuffer(void)
{  
  return DMATxDescToSet->Buffer1Addr;//����Tx buffer��ַ  
}




//ΪETH�ײ����������ڴ�
//����ֵ:0,����
//    ����,ʧ��
u8 ETH_Mem_Malloc(void)
{ 
	
#if 0
	DMARxDscrTab = mymalloc(SRAMIN,ETH_RXBUFNB*sizeof(ETH_DMADESCTypeDef));//�����ڴ�
	DMATxDscrTab = mymalloc(SRAMIN,ETH_TXBUFNB*sizeof(ETH_DMADESCTypeDef));//�����ڴ�  
	Rx_Buff=mymalloc(SRAMIN,ETH_RX_BUF_SIZE*ETH_RXBUFNB);	//�����ڴ�
	Tx_Buff=mymalloc(SRAMIN,ETH_TX_BUF_SIZE*ETH_TXBUFNB);	//�����ڴ�
	if(!DMARxDscrTab||!DMATxDscrTab||!Rx_Buff||!Tx_Buff)
	{
		ETH_Mem_Free();
		return 1;	//����ʧ��
	}	
	return 0;		//����ɹ�
#else
	DMARxDscrTab = DMARxDscrTab_array;
	DMATxDscrTab = DMATxDscrTab_array;
	Rx_Buff = Rx_Buff_array;
	Tx_Buff = Tx_Buff_array;
	
	return 0;
#endif	
}
//�ͷ�ETH �ײ�����������ڴ�
void ETH_Mem_Free(void)
{ 
#if 0
	myfree(SRAMIN,DMARxDscrTab);//�ͷ��ڴ�
	myfree(SRAMIN,DMATxDscrTab);//�ͷ��ڴ�
	myfree(SRAMIN,Rx_Buff);		//�ͷ��ڴ�
	myfree(SRAMIN,Tx_Buff);		//�ͷ��ڴ� 
#endif	
}


