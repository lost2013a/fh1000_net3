//////////////////////////////////////////////////////////////////////////////////	 
//��������:2016/5/26
//�汾��V1.0
//ZX						  
////////////////////////////////////////////////////////////////////////////////// 
#ifndef __CAN_H
#define __CAN_H	 

#include "stm32f4xx.h"    

/***CAN1_BUF�����С***/
#define CAN1_RX0_LEN				100	//
#define DEBUG_CANRECIVE_ 0

extern u8  CAN1_RX0_BUF[CAN1_RX0_LEN]; 	//���ջ��� 
extern u16 CAN1_RX0_STA;         				//����״̬���	


u8 CAN1_Mode_Init(void);
u8 CAN1_Send_Msg(u8* msg,u32 ext_id,u8 len);
void CAN1_Send_u32msg(u32* u32msg,u32 ext_id);
//u8 CAN1_Send_REQ(u32 ext_id);
					
#endif


















