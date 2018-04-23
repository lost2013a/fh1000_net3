#include "lcdmessage.h"
#include "ptpd.h" 
#include "can.h"
#include "netconf.h"
#include "main.h"

//�汾���� 20161108 �忨��ַ����8λ
extern PtpClock G_ptpClock;
extern RunTimeOpts rtOpts;
extern FigStructData GlobalConfig;
extern uint32_t CAN1_SEND_ID;


/***����PTP������Ϣ�ṹ��***/
typedef struct 
{		
	u8 SLAVEFLAG;								//11.��������	
	u8 TWOSTEPFLAG ;						//12.��������־
	u8 DOMAINNUMBER ;						//13.ʱ����
	u8 ANNOUNCE_INTERVAL;				//14.Announce��������
	u8 ANNOUNCE_RECEIPT_TIMEOUT;//15.Announce���ճ�ʱʱ��
	u8 SYNC_INTERVAL;						//16.Sync��������
	u8 PDELAYREQ_INTERVAL;			//17.PDelay��������
  u8 DELAYREQ_INTERVAL;				//18.Delay��������
	u8 SLAVE_PRIORITY1 ;				//19.���ȼ�1
  u8 SLAVE_PRIORITY2 ;				//20.���ȼ�2
} PTPD_Type;

typedef struct 
{
	u8 	slot_num;				//0.���λ/���ں�		
	u8 	ip[4];    			//1.IP��ַ					
	u8 	mask[4]; 				//2.��������						
	u8 	gw[4]; 					//3.����
	u8 	remoteip[4];		//7.�Զ�IP��ַ 
	u16 localport;			//4.�˿ں�	
	u16 remoteport;			//6.�Զ˶˿ں�
	u8 	ethernet_mode;	//10.����ģʽ 0����ETH/ETE��1����ETH-PTP;2����UDP-ETE;3����UDP-PTP��4��NTP��
	u8 	spreadmode;			//8.���䷽ʽ		
	u8 	spreadgaps;			//9.�㲥���	
	u8 	mac[6];      		//5.MAC��ַ		
	PTPD_Type ptpd;			//PTP�ṹ��
} NetDev_Type;


void u8_to_u32(u8* u8buff,u32 *u32buff)
{
	*u32buff = (u8buff[0]<<24)|(u8buff[1]<<16)|(u8buff[2]<<8)|(u8buff[3]);
}

void u32_to_u8(u32 *u32buff,u8* u8buff)
{
	u8buff[0] = ((*u32buff)&0xff000000)>>24;
	u8buff[1] = ((*u32buff)&0xff0000)>>16;
	u8buff[2] = ((*u32buff)&0xff00)>>8;
	u8buff[3] = ((*u32buff)&0xff);
}



void CanRxHandle()
{
	if(((CAN1_RX0_STA&0xff00)>>8)>0)
		{
			int32_t readflash[10] = {0};
			u32 i;
			u8  msg[40]  ={0};
			u32 can_sendid = CAN1_SEND_ID;
			uint16_t CAN_RECIVE_FLAG;
			CAN_RECIVE_FLAG=(CAN1_RX0_STA&0xf00)>>8;
			if(CAN_RECIVE_FLAG==1)
				{
					if((CAN1_RX0_BUF[39] == ((can_sendid&0xff0000)>>16))&&(CAN1_RX0_BUF[0]!=0xff))
						{
							memcpy((int32_t *)readflash,(int32_t *)&CAN1_RX0_BUF,40);
							for(i=0;i<10;i++)
								{
									u8_to_u32(&CAN1_RX0_BUF[i*4],(u32 *)(&readflash[i]));
								}
							readflash[9] &=0x00ffffff; //дFLASHǰ�ر��ж�
							NVIC_DisableIRQ(CAN1_RX0_IRQn);
							NVIC_DisableIRQ(ETH_IRQn);	
							NVIC_DisableIRQ(USART1_IRQn);	
							NVIC_DisableIRQ(EXTI1_IRQn);		
							for(i=0;i<10000;i++){}//�ȴ��жϹر�
							FLASH_Write(FLASH_SAVE_ADDR1,readflash,10);
							for(i=0;i<10000000;i++){}//�ȴ���д����	
							NVIC_SystemReset();
						}
				}	
			else if(CAN_RECIVE_FLAG==2)
				{				
					memcpy((int8_t *)msg,(u32 *)&GlobalConfig,40);
					u32_to_u8(&GlobalConfig.IPaddr,&msg[0]);
					u32_to_u8(&GlobalConfig.NETmark,&msg[4]);
					u32_to_u8(&GlobalConfig.GWaddr,&msg[8]);
					u32_to_u8(&GlobalConfig.DstIpaddr,&msg[12]);
					msg[16] = (GlobalConfig.LocalPort&0xff00)>>8;
					msg[17] = (GlobalConfig.LocalPort&0xff);
					msg[18] = (GlobalConfig.DstPort&0xff00)>>8;
					msg[19] = (GlobalConfig.DstPort&0xff);
					for(i=0;i<5;i++)
						{
							CAN1_Send_Msg(&msg[i*8],can_sendid+i,8);
						}
				}	
			CAN1_RX0_STA=0;		
		}
}



