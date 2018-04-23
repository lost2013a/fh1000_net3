#include "stm32f4x7_eth.h"
#include "netconf.h"
#include "main.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "string.h"
#include "serial_debug.h"
#include "ptpd.h"
#include "share.h"
#include "sys.h"
#include "can.h"
#include "lcdmessage.h"
#include "pc2cfg.h"
/************************************���Զ������printf*******************************************/
#define SYSTEMTICK_PERIOD_MS  10
#define ITM_Port8(n)    (*((volatile unsigned char *)(0xE0000000+4*n)))
#define ITM_Port16(n)   (*((volatile unsigned short*)(0xE0000000+4*n)))
#define ITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))
#define DEMCR           (*((volatile unsigned long *)(0xE000EDFC)))
#define TRCENA          0x01000000
struct __FILE 
{ 
	int handle1; /* Add whatever you need here */ 
};
FILE __stdout;
FILE __stdin;
int fputc(int ch, FILE *f)
{
	return ITM_SendChar(ch);
}

void cpuidGetId(void);
void Read_ConfigData(FigStructData *FigData);
__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms *///����ʱ��
uint32_t LocalTime1 = 0;
uint32_t timingdelay;
volatile sysTime sTime;
volatile Servo Mservo; //���ڱ���ʱ�Ӻ�GPS��ʱ�ĵ�������ָ��
Filter	 MofM_filt;			//����ʱ�����ݼ��������˲���
FigStructData GlobalConfig;
unsigned int test_fjm;
uint32_t mcuID[3];
extern uint8_t CAN1_Mode_Init(void);
extern void ARM_to_FPGA(void);
extern PtpClock G_ptpClock;
extern RunTimeOpts rtOpts;
extern uint8_t synflags;



//LED1-����
//LED2-ͬ��
//LED3-PPS
//LED4-����ѱ��
		
int main(void)
{
	Mservo.ai	  = 16;	//����ʱ�䳣��I	 = 16	
	Mservo.ap		= 2;	//����ϵ��P		   = 2
	MofM_filt.n = 0;				
	MofM_filt.s = 1;		//alpha 0.1-0.4 
	sTime.noAdjust 		 = FALSE;			//�������ϵͳʱ��
	sTime.noResetClock = FALSE;			//��������ϵͳʱ��
	RCC_Configuration();
	Read_ConfigData(&GlobalConfig); //ע�⣺��Ҫ�ȶ�дFLASH���ڿ�Ӳ���ж�ǰ
	GPIO_Configuration();
//	if(GlobalConfig.SlaveorMaster==0)
//		{
			UART1_Configuration();
			EXTI_Configuration();
//		}
//	else
//		{
//			UART6_Configuration();
//		}
//	CAN1_Mode_Init();								//���忨��ַ����ʼ��CAN	
	NVIC_Configuration();									
	ETH_GPIO_Config();
	ETH_MACDMA_Config();
 	LwIP_Init();
//	PTPd_Init();		
  NTP_Init();		
	Multicastserverinit();  //godn �鲥�������ã��õ���֮ǰ�����ýṹ����������Ķ���can�Ľṹ��
	STM_LEDon(mLED1); 				 			//��ʼ����ɣ�����LED1
	printf("waiting synflags...\n");
  while (1)
		{ 
			handleap(); 				
			Hand_serialSync(); 
//			CanRxHandle();
			LwIP_Periodic_Handle(LocalTime);
//			if(synflags)				//ͬ��֮��ſ�ʼ��PTP����
//				{
//					ptpd_Periodic_Handle(LocalTime);
//				}
		}
}

void Usart6_SEND(void)
{
	static int 	oldseconds = 0;
	TimeInternal Itime;
	if(rtOpts.slaveOnly == TRUE )//godin ptp����ʱ��100ms��350ms֮�䷢�ʹ���ʱ��
		{
			if(LocalTime%200 == 0 )	 //����10ms
				{
					getTime(&Itime);	
					 if(Itime.nanoseconds>100000000  && Itime.nanoseconds<350000000 )
						 { 
								if( Itime.seconds != oldseconds )
									{				
										 ARM_to_FPGA();
										 oldseconds = Itime.seconds;
											myprintf("ARM_to_FPGA\n");
									}	
						 }
				}		
		}
}





void Delay(uint32_t nCount)
{
  timingdelay = LocalTime + nCount;  
  while(timingdelay > LocalTime)
		{     
		}
}

void Time_Update(void)
{
  LocalTime += SYSTEMTICK_PERIOD_MS;
}


void Read_ConfigData(FigStructData *FigData)//godin flash������Ϣ��ȡ
{
	int32_t readflash[10] = {0},i;
	FLASH_Read(FLASH_SAVE_ADDR1,readflash,10);//������Ϣ��readflash��
	if(readflash[0] == 0xffffffff)//Ĭ������
		{
			FigData->IPaddr		 = 0xc0a80100|13;	//192.168.1.13 Ĭ��IP
			FigData->NETmark	 = 0xffffff00;	
			FigData->GWaddr		 = 0xc0a80101;	
			FigData->DstIpaddr = 0xc0a80100|11;	//192.168.1.11
			FigData->MACaddr[0] = 0x06;
			FigData->MACaddr[1] = 0x04;
			FigData->MACaddr[2] = 0x00;
			FigData->MACaddr[3] = 0xFF;
			cpuidGetId(); 										//��ȡоƬIDΨһ��ţ�����ȡ���MAC��ַ
			FigData->MACaddr[4] = rand()%255; //ͨ���������ȡmac��ַ
			FigData->MACaddr[5] = rand()%255; //ͨ���������ȡmac��ַ
			FigData->LocalPort = 7862;					//godin Ƶ����Ϣ�˿ںţ��鲥�˿ں�
			FigData->DstPort 	 = 7777;					//godin Ƶ����ϢĿ�ĵ�ַ�˿ں�
			
			FigData->MSorStep           = 0; 												//������
			FigData->ClockDomain 				= DEFAULT_DOMAIN_NUMBER;		//Ĭ��ʱ����0
			FigData->WorkMode		 				=	0;												//Ĭ��E2E-MAC; 
			FigData->ip_mode	   				= IPMODE_MULTICAST;					//Ĭ���鲥
			FigData->BroadcastInterval 	= 0;												//�㲥���
			FigData->SlaveorMaster 			= 0;												//0��
			FigData->Offset_UTC					= 0; 												//0
			FigData->SyncInterval				=	DEFAULT_SYNC_INTERVAL; 						//0
			FigData->PDelayInterval			= DEFAULT_PDELAYREQ_INTERVAL; 			//1
			FigData->DelayInterval			= DEFAULT_DELAYREQ_INTERVAL; 				//0
			FigData->clockClass 				= 248; 															//ʱ�ӵȼ���������д��������
			FigData->priority1 					= DEFAULT_PRIORITY1; 								//128
			memcpy((int8_t *)readflash,(int8_t *)FigData,16);
			readflash[4]	=	((FigData->LocalPort)<<16)|FigData->DstPort;
			readflash[5]	=	((FigData->WorkMode)<<24)|((FigData->ip_mode)<<16)|((FigData->BroadcastInterval)<<8)|(FigData->tmp1);
			readflash[6]	=	((FigData->MACaddr[0])<<24)|((FigData->MACaddr[1])<<16)|((FigData->MACaddr[2])<<8)|(FigData->MACaddr[3]);
			readflash[7]	=	((FigData->MACaddr[4])<<24)|((FigData->MACaddr[5])<<16)|((FigData->MSorStep)<<8)|(FigData->ClockDomain);
			readflash[8]	=	((FigData->SlaveorMaster)<<24)|((FigData->Offset_UTC)<<16)|((FigData->SyncInterval)<<8)|(FigData->PDelayInterval);
			readflash[9]	=	((FigData->DelayInterval)<<24)|((FigData->clockClass)<<16)|((FigData->priority1)<<8)|(FigData->tmp2);
			FLASH_Write(FLASH_SAVE_ADDR1,readflash,10);
			for(i=0;i<1000;i++){}//�ȴ���д����	
		}
	else 
		{ //��ȡ���µ�����
			FigData->IPaddr 	= readflash[0];
			FigData->NETmark 	= readflash[1];
			FigData->GWaddr 	= readflash[2];
			FigData->DstIpaddr= readflash[3];
			FigData->LocalPort	= (uint16_t)((readflash[4]&0xffff0000)>>16); 
			FigData->DstPort	  = (uint16_t)((readflash[4]&0x0000ffff)>>0);
			FigData->WorkMode		= (uint8_t)((readflash[5]&0xff000000)>>24);			//ptp����ģʽ
			FigData->ip_mode		= (uint8_t)((readflash[5]&0xff0000)>>16);			//NTP����ģʽ(�����鲥�㲥)
			FigData->BroadcastInterval 	= (uint8_t)((readflash[5]&0xff00)>>8);//�㲥���
			FigData->MACaddr[0] = (uint8_t)((readflash[6]&0xff000000)>>24);
			FigData->MACaddr[1] = (uint8_t)((readflash[6]&0xff0000)>>16);
			FigData->MACaddr[2] = (uint8_t)((readflash[6]&0xff00)>>8);	
			FigData->MACaddr[3] = (uint8_t)(readflash[6]&0xff);	
			FigData->MACaddr[4] = (uint8_t)((readflash[7]&0xff000000)>>24);
			FigData->MACaddr[5] = (uint8_t)((readflash[7]&0xff0000)>>16);
			FigData->MSorStep		= (uint8_t)((readflash[7]&0xff00)>>8);			//PTPһ����������
			FigData->ClockDomain= (uint8_t)(readflash[7]&0xff);				//ʱ����
			FigData->SlaveorMaster			= (uint8_t)((readflash[8]&0xff000000)>>24);
			FigData->Offset_UTC 				= (uint8_t)((readflash[8]&0xff0000)>>16);
			FigData->SyncInterval 			= (uint8_t)((readflash[8]&0xff00)>>8);
			FigData->PDelayInterval 		= (uint8_t)(readflash[8] &0xff);	
			FigData->DelayInterval  		= (uint8_t)((readflash[9] &0xff000000)>>24);
			FigData->clockClass	 				= (uint8_t)((readflash[9]&0xff0000)>>16);
			FigData->priority1					= (uint8_t)((readflash[9]&0xff00)>>8);		
		}
	printf("FigData->IPaddr=0x%04X\n",FigData->IPaddr);
	printf("FigData->GWaddr=0x%04X\n",FigData->GWaddr);
	printf("FigData->NETmark=0x%04X\n",FigData->NETmark);
	printf("FigData->ntpservaddr=0x%04X\n",FigData->DstIpaddr);
	printf("FigData->LocalPort=%d\n",FigData->LocalPort);
	printf("FigData->DstPort=%d\n",FigData->DstPort);
	printf("FigData->MSorOT=%d\n",FigData->MSorStep);
	printf("FigData->WorkMode=%d\n",FigData->WorkMode);
	printf("FigData->ClockDomain=%d\n",FigData->ClockDomain);
	printf("FigData->ip_mode=%d\n",FigData->ip_mode);
	printf("FigData->BroadcastInterval=%d\n",FigData->BroadcastInterval);
	printf("FigData->SyncInterval=%d\n",FigData->SyncInterval);
	printf("FigData->PDelayInterval=%d\n",FigData->PDelayInterval);
	printf("FigData->DelayInterval=%d\n",FigData->DelayInterval);
	printf("FigData->clockClass=%d\n",FigData->clockClass);
	printf("FigData->priority1=%d\n",FigData->priority1);
	printf("FigData->SlaveorMaster=%d\n",FigData->SlaveorMaster);
	printf("FigData->Offset_UTC=%d\n",FigData->Offset_UTC);	
			
}


void cpuidGetId(void)
{
	mcuID[0] = *(__IO u32*)(0x1FFF7A10);
	mcuID[1] = *(__IO u32*)(0x1FFF7A14);
	mcuID[2] = *(__IO u32*)(0x1FFF7A18);
	srand((mcuID[0]+mcuID[1])+mcuID[2]); //������������ɵ�����
}

void STM_LEDon(uint16_t LEDx)
{
	GPIO_SetBits(GPIOD,LEDx);
}

void STM_LEDoff(uint16_t LEDx)
{
	GPIO_ResetBits(GPIOD,LEDx);
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{
  while (1)
  {}
}
#endif


