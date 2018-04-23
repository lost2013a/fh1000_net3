#include "usart6_cfg.h"
extern FigStructData GlobalConfig;
extern RunTimeOpts rtOpts;
extern PtpClock  G_ptpClock;

//extern unsigned int test_fjm;
//extern sysTime sTime;
////extern struct ADDressInfo addrInfo;
//extern Servo Mservo;
//extern Filter MofM_filt;
//extern TimeInternal NtplocalTime;
//static void Mfilter(Integer32 * nsec_current, Filter * filt);
//unsigned char synflags;
////Integer32 abjbuf[5];




//#ifdef Gx_NTP_Server
//struct ConfigNTPData Gx_NTPFIGS;
//struct Gx_Flags Gx_flags;
//unsigned char Gx_NTPIndex;
//unsigned char Gx_NTPFIG_OK;

//void USART6_IRQHandler(void) //接收配置信息
//{
//	if(USART_GetITStatus(USART6,USART_IT_RXNE) != RESET)//检查是否是指定的中断发生
//	{//接收到数据产生中断 
//		if(Gx_NTPIndex==0)//一个指向数据移动到的位置a[i] i++;
//		{
//			memset(&Gx_NTPFIGS,0,sizeof(Gx_NTPFIGS));
//		}
//		
//		//这里收到的第一个数据还是0x2a
//		*((unsigned char*)(&Gx_NTPFIGS)+Gx_NTPIndex) = USART_ReceiveData(USART6);
////		printf("%d.",*((unsigned char*)(&Gx_NTPFIGS)+Gx_NTPIndex));
//		Gx_NTPIndex++;
//		
//		if(Gx_NTPIndex == 1)
//		{
//			if(Gx_NTPFIGS.frame_head != 0x2a) //"*"
//				Gx_NTPIndex=0;
//		}
//		else if(Gx_NTPIndex == sizeof(Gx_NTPFIGS))
//		{
//			if(Gx_NTPFIGS.frame_end == 0x23)//"#"
//					Gx_NTPFIG_OK = 1;
////			printf("\n");
//		}
//		USART_ClearITPendingBit(USART6,USART_IT_RXNE);//清除接收中断标记	
//	}
//}

//void Gx_HandleNTPFIGS(void)
//{
//	if(Gx_NTPFIG_OK == 1) //NTP配置帧接收完成
//	{
//		int i;
//		int32_t config_tmp[5] ={0};
//		switch(Gx_NTPFIGS.cmd_type)
//		{
//			case 0x04:
//				config_tmp[0] = Gx_NTPFIGS.Local_ip_32;
//				config_tmp[1] = Gx_NTPFIGS.Gate_ip_32;
//				config_tmp[2] = Gx_NTPFIGS.Mask_ip_32;
//				
//				FLASH_Write(FLASH_SAVE_ADDR1,config_tmp,3);//将ntp server的配置写入flash中
//				
//				addrInfo.ptpmode = 0;
//				addrInfo.ipaddr  = Gx_NTPFIGS.Local_ip_32;
//				addrInfo.gwaddr  = Gx_NTPFIGS.Gate_ip_32;
//				addrInfo.netmark = Gx_NTPFIGS.Mask_ip_32;
//				addrInfo.ntpservaddr = Gx_NTPFIGS.Mask_ip_32;
//				addrInfo.flags = 1;//标记重启网络和ptpd ntp(不重启系统)

//				for(i=0;i<10000;i++){}//等待
//					printf("ntp server SystemReset\n");
//				NVIC_SystemReset();//软件重启系统;
//				break;
//					
//			default:
//				break;
//		}	
//		Gx_NTPIndex  = 0;
//		Gx_NTPFIG_OK = 0;
//	}
//}
//#endif









