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
/************************************调试段码输出printf*******************************************/
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
__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms *///本地时间
uint32_t LocalTime1 = 0;
uint32_t timingdelay;
volatile sysTime sTime;
volatile Servo Mservo; //用于本地时钟和GPS对时的调整分量指数
Filter	 MofM_filt;			//调整时间数据集的数字滤波器
FigStructData GlobalConfig;
unsigned int test_fjm;
uint32_t mcuID[3];
extern uint8_t CAN1_Mode_Init(void);
extern void ARM_to_FPGA(void);
extern PtpClock G_ptpClock;
extern RunTimeOpts rtOpts;
extern uint8_t synflags;



//LED1-运行
//LED2-同步
//LED3-PPS
//LED4-晶振驯服
		
int main(void)
{
	Mservo.ai	  = 16;	//积分时间常数I	 = 16	
	Mservo.ap		= 2;	//比例系数P		   = 2
	MofM_filt.n = 0;				
	MofM_filt.s = 1;		//alpha 0.1-0.4 
	sTime.noAdjust 		 = FALSE;			//允许调整系统时钟
	sTime.noResetClock = FALSE;			//允许重置系统时钟
	RCC_Configuration();
	Read_ConfigData(&GlobalConfig); //注意：需要先读写FLASH，在开硬件中断前
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
//	CAN1_Mode_Init();								//读板卡地址，初始化CAN	
	NVIC_Configuration();									
	ETH_GPIO_Config();
	ETH_MACDMA_Config();
 	LwIP_Init();
	PTPd_Init();		
  // NTP_Init();		
	Multicastserverinit();  //godn 组播网络配置（用的是之前的配置结构，现在用需改动成can的结构）
	STM_LEDon(mLED1); 				 			//初始化完成，点亮LED1
	printf("waiting synflags...\n");
  while (1)
		{ 
			handleap(); 				
			Hand_serialSync(); 
//			CanRxHandle();
			LwIP_Periodic_Handle(LocalTime);
			if(synflags)				//同步之后才开始发PTP报文
				{
					ptpd_Periodic_Handle(LocalTime);
				}
		}
}

void Usart6_SEND(void)
{
	static int 	oldseconds = 0;
	TimeInternal Itime;
	if(rtOpts.slaveOnly == TRUE )//godin ptp做从时在100ms和350ms之间发送串口时间
		{
			if(LocalTime%200 == 0 )	 //步进10ms
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


void Read_ConfigData(FigStructData *FigData)//godin flash配置信息读取
{
	int32_t readflash[10] = {0},i;
	FLASH_Read(FLASH_SAVE_ADDR1,readflash,10);//读出信息到readflash中
	if(readflash[0] == 0xffffffff)//默认配置
		{
			FigData->IPaddr		 = 0xc0a80100|13;	//192.168.1.13 默认IP
			FigData->NETmark	 = 0xffffff00;	
			FigData->GWaddr		 = 0xc0a80101;	
			FigData->DstIpaddr = 0xc0a80100|11;	//192.168.1.11
			FigData->MACaddr[0] = 0x06;
			FigData->MACaddr[1] = 0x04;
			FigData->MACaddr[2] = 0x00;
			FigData->MACaddr[3] = 0xFF;
			cpuidGetId(); 										//读取芯片ID唯一编号，用于取随机MAC地址
			FigData->MACaddr[4] = rand()%255; //通过随机数获取mac地址
			FigData->MACaddr[5] = rand()%255; //通过随机数获取mac地址
			FigData->LocalPort = 8000;					//godin 频率信息端口号，组播端口号
			FigData->DstPort 	 = 7777;					//godin 频率信息目的地址端口号
			
			FigData->MSorStep           = 0; 												//两步发
			FigData->ClockDomain 				= DEFAULT_DOMAIN_NUMBER;		//默认时钟域0
			FigData->WorkMode		 				=	0;												//默认E2E-MAC; 
			FigData->ip_mode	   				= IPMODE_MULTICAST;					//默认组播
			FigData->BroadcastInterval 	= 0;												//广播间隔
			FigData->SlaveorMaster 			= 0;												//0主
			FigData->Offset_UTC					= 0; 												//0
			FigData->SyncInterval				=	DEFAULT_SYNC_INTERVAL; 						//0
			FigData->PDelayInterval			= DEFAULT_PDELAYREQ_INTERVAL; 			//1
			FigData->DelayInterval			= DEFAULT_DELAYREQ_INTERVAL; 				//0
			FigData->clockClass 				= 248; 															//时钟等级，用于填写闰秒总数
			FigData->priority1 					= DEFAULT_PRIORITY1; 								//128
			memcpy((int8_t *)readflash,(int8_t *)FigData,16);
			readflash[4]	=	((FigData->LocalPort)<<16)|FigData->DstPort;
			readflash[5]	=	((FigData->WorkMode)<<24)|((FigData->ip_mode)<<16)|((FigData->BroadcastInterval)<<8)|(FigData->tmp1);
			readflash[6]	=	((FigData->MACaddr[0])<<24)|((FigData->MACaddr[1])<<16)|((FigData->MACaddr[2])<<8)|(FigData->MACaddr[3]);
			readflash[7]	=	((FigData->MACaddr[4])<<24)|((FigData->MACaddr[5])<<16)|((FigData->MSorStep)<<8)|(FigData->ClockDomain);
			readflash[8]	=	((FigData->SlaveorMaster)<<24)|((FigData->Offset_UTC)<<16)|((FigData->SyncInterval)<<8)|(FigData->PDelayInterval);
			readflash[9]	=	((FigData->DelayInterval)<<24)|((FigData->clockClass)<<16)|((FigData->priority1)<<8)|(FigData->tmp2);
			FLASH_Write(FLASH_SAVE_ADDR1,readflash,10);
			for(i=0;i<1000;i++){}//等待烧写结束	
		}
	else 
		{ //读取到新的配置
			FigData->IPaddr 	= readflash[0];
			FigData->NETmark 	= readflash[1];
			FigData->GWaddr 	= readflash[2];
			FigData->DstIpaddr= readflash[3];
			FigData->LocalPort	= (uint16_t)((readflash[4]&0xffff0000)>>16); 
			FigData->DstPort	  = (uint16_t)((readflash[4]&0x0000ffff)>>0);
			FigData->WorkMode		= (uint8_t)((readflash[5]&0xff000000)>>24);			//ptp工作模式
			FigData->ip_mode		= (uint8_t)((readflash[5]&0xff0000)>>16);			//NTP网络模式(单播组播广播)
			FigData->BroadcastInterval 	= (uint8_t)((readflash[5]&0xff00)>>8);//广播间隔
			FigData->MACaddr[0] = (uint8_t)((readflash[6]&0xff000000)>>24);
			FigData->MACaddr[1] = (uint8_t)((readflash[6]&0xff0000)>>16);
			FigData->MACaddr[2] = (uint8_t)((readflash[6]&0xff00)>>8);	
			FigData->MACaddr[3] = (uint8_t)(readflash[6]&0xff);	
			FigData->MACaddr[4] = (uint8_t)((readflash[7]&0xff000000)>>24);
			FigData->MACaddr[5] = (uint8_t)((readflash[7]&0xff0000)>>16);
			FigData->MSorStep		= (uint8_t)((readflash[7]&0xff00)>>8);			//PTP一步法两步法
			FigData->ClockDomain= (uint8_t)(readflash[7]&0xff);				//时钟域
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
	srand((mcuID[0]+mcuID[1])+mcuID[2]); //用作随机数生成的种子
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


