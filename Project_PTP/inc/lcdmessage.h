#ifndef __LCDMESSAGE_H_
#define __LCDMESSAGE_H_

#include "main.h"
#include "share.h"






//ʹ�ô��ڷ���
typedef struct ARMtoFPGA{   //godin ���ڷ�����Ϣ�ṹ
  unsigned char synA;				 //ͬ����  90
  unsigned char synB;				 //ͬ����  eb
  unsigned char command;				 //������
  unsigned char addencode;			 //�忨��ַ
  unsigned char messagelen;			 //���ĳ���	
  unsigned short year;		 
  unsigned char	 month;
  unsigned char	 day;
  unsigned char  hour;
  unsigned char  minute;
  unsigned char  seconds;
  char leap;				 		 //������	 0������1�����룬2�����룬3δ֪
  char sync_state;			 //ͬ��״̬	 A��Ч41H  V��Ч56H
  char sync_mode; 			 //ͬ��ģʽ	
  char transport;				 //����ģʽ
  char Running_state;		 //ptp����״̬
  char bestmac_addr[6];		
  char mac_addr[6];		
  int  path_delay;			
  int  correctionField;		
  char chksum;				 //У���	
}ARMtoFPGA_PTP;


void ARM_to_FPGA(void);
//void ReadBoardADDR(void);
void CanRxHandle(void);

#endif
