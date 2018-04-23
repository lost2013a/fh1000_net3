
#ifndef __ARM2FPGA_H
#define __ARM2FPGA_H
#include "stm32f4xx.h"

//ʹ�ô��ڷ���
typedef struct _ARMtoFPGA
{   
  uint8_t 	framhead1;				 //ͬ����  90
  uint8_t 	framhead2;				 //ͬ����  eb
  uint8_t 	command;				 	 //������	 11
  uint8_t 	slotadr;			   	 //��ַ��
  uint8_t 	messagelen;				 //���ĳ���	
  uint16_t 	year;		 				 	 //ʱ����Ϣ��
  uint8_t	 	month;						 //��
  uint8_t	 	day;							 //��	
  uint8_t  	hour;							 //ʱ
  uint8_t  	minute;						 //��
  uint8_t  	seconds;					 //��
  uint8_t		leapinfo;				 	 //������	 0������1�����룬2�����룬3δ֪
  uint8_t 	sync_state;			 	 //ͬ��״̬	 A��Ч41H  V��Ч56H
//  uint8_t 	sync_mode; 			 	 //ͬ��ģʽ	
//  uint8_t 	transport;				 //����ģʽ
//  uint8_t 	Running_state;		 //ptp����״̬
//  uint8_t 	bestmac_addr[6];		
//  uint8_t 	mac_addr[6];		
//  int  			path_delay;			
//  int  			correctionField;		
//  char 			chksum;				 			//У���	
}ARMtoFPGAstruct;


#endif
