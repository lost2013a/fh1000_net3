
#ifndef __ARM2FPGA_H
#define __ARM2FPGA_H
#include "stm32f4xx.h"

//使用串口发送
typedef struct _ARMtoFPGA
{   
  uint8_t 	framhead1;				 //同步符  90
  uint8_t 	framhead2;				 //同步符  eb
  uint8_t 	command;				 	 //命令码	 11
  uint8_t 	slotadr;			   	 //地址码
  uint8_t 	messagelen;				 //报文长度	
  uint16_t 	year;		 				 	 //时间信息年
  uint8_t	 	month;						 //月
  uint8_t	 	day;							 //日	
  uint8_t  	hour;							 //时
  uint8_t  	minute;						 //分
  uint8_t  	seconds;					 //秒
  uint8_t		leapinfo;				 	 //闰秒标记	 0正常，1正闰秒，2负闰秒，3未知
  uint8_t 	sync_state;			 	 //同步状态	 A有效41H  V无效56H
//  uint8_t 	sync_mode; 			 	 //同步模式	
//  uint8_t 	transport;				 //传输模式
//  uint8_t 	Running_state;		 //ptp运行状态
//  uint8_t 	bestmac_addr[6];		
//  uint8_t 	mac_addr[6];		
//  int  			path_delay;			
//  int  			correctionField;		
//  char 			chksum;				 			//校验和	
}ARMtoFPGAstruct;


#endif
