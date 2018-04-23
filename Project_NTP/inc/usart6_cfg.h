
#ifndef __USART6_CFG_H
#define __USART6_CFG_H

#include "ntp.h"
#include "main.h"
#include "flash_conf.h"




	 


 
struct ConfigPTPData //测试仪下的ptp配置帧
{
	unsigned char  frame_head;  	//*		0x2A
	unsigned char	 cmd_type;   		//0x01表示确认修改配置,0x02开始工作,0x03停止工作
	unsigned char  temp0;					//保留 0x00
	unsigned char	 temp1; 				//保留 0x00
	unsigned int	 Local_ip_32;		// Local ip 		4					   			
	unsigned int	 Gate_ip_32;		// Gate ip      4
	unsigned int   Mask_ip_32; 		// mask ip      4
	unsigned int	 temp2;		     	//保留 0x00000000
	unsigned int	 temp3;   			//保留 0x00000000
	unsigned int	 temp4; 			 	//保留 0x00000000   		
	unsigned int	 temp5; 				//保留 0x00000000
	unsigned char  ethernet_mode; //协议层数 MAC：0x01  UDP：0x02
	unsigned char  E2E_mode;      //链路方式 E2E: 0x01  P2P：0x02
	unsigned char	 slaveOnly;     //主从方式: 只做从 0x00
	unsigned char  twoStep;       //一步发两步法 只做两步法 0x00
	unsigned char  temp10;				//保留 0x00
	unsigned char	 temp11;   			//保留 0x00
	unsigned char	 temp12;        //保留 0x00                                   
	unsigned char  frame_end;     //#		0x23
};



struct ConfigNTPData
{
	unsigned char  frame_head;  //*
	unsigned char	 cmd_type;
	unsigned char  temp0;// Out delay（ns）signed      bit	
	unsigned char	 temp1;// 			// 客服端运行标志 	   				1
	unsigned int	 Local_ip_32;		// Local ip 						4
	unsigned int	 Server_ip_32;		// Server ip 	   					4
	unsigned int	 Gate_ip_32;			// Gate ip 4
	unsigned int   Mask_ip_32;
	unsigned int	 testMark;// 		// Out delay（ns） 			4
	unsigned int	 temp3;// 			// 客户端延时补偿（us） 	   		4
	unsigned int	 temp4;// 			// 服务端延时补偿（us）				4
	unsigned char  temp5;//			 //时区设置
	unsigned char  temp6;// 
	unsigned char	 temp7;// 			// CHKSUM 代码和                                                      4
	unsigned char  frame_end;        //  #
};



//struct ConfigNTPData
//{
//	unsigned char  frame_head;  //*
//	unsigned char	 cmd_type;
//	unsigned char  temp0;// Out delay（ns）signed      bit	
//	unsigned char	 temp1;// 			// 客服端运行标志 	   				1
//	unsigned int	 Local_ip_32;		// Local ip 						4
//	unsigned int	 Server_ip_32;		// Server ip 	   					4
//	unsigned int	 Gate_ip_32;			// Gate ip 4
//	unsigned int   Mask_ip_32;
//	unsigned int	 testMark;// 		// Out delay（ns） 			4
//	unsigned int	 temp3;// 			// 客户端延时补偿（us） 	   		4
//	unsigned int	 temp4;// 			// 服务端延时补偿（us）				4
//	unsigned char  temp5;//			 //时区设置
//	unsigned char  temp6;// 
//	unsigned char	 temp7;// 			// CHKSUM 代码和                                                      4
//	unsigned char  frame_end;        //  #
//};







void Gx_HandlePTPFIG(void);
void Gx_HandleNTPFIGC(void);
void Gx_HandleNTPFIGS(void);

#endif
