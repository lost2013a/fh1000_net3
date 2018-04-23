//////////////////////////////////////////////////////////////////////////////////	 
//��������:2016/5/26
//�汾��V1.0
//ZX						  
//�޸ļ�¼��1��2016.06.21�޸Ľ����������ΪCAN1_RX1_STA&0xffff0000	
//�޸ļ�¼��2��2016.11.09�ӳ��ϵ�󣬶��忨��ַ��ʱ�䣬�����޸�CAN���˲�������
////////////////////////////////////////////////////////////////////////////////// 
#include "can.h"
#include "main.h"
#include "lcdmessage.h"
//bit15��	������ɱ�־
//
//bit13~0��	���յ�����Ч�ֽ���Ŀ	

//uint16_t BoardADDR;


extern RunTimeOpts rtOpts;			/* statically allocated run-time configuration data */

uint16_t ReadBoardADDR__(void)
{
	uint16_t read_addr;
	read_addr  =	GPIO_ReadInputData(GPIOE);	
	read_addr &= 	0x7F; //������8λ
	return read_addr;
}

uint32_t CAN1_SEND_ID;

u8 CAN1_RX0_BUF[CAN1_RX0_LEN]; 	//CAN����BUF,���CAN1_RX0_LEN���ֽ�.
u16 CAN1_RX0_STA=0;       			//����״̬���	


 /**
  * @brief  CAN1��ʼ��
  * @param  ��
  * @retval ��ʼ���ɹ�->0
  */
u8 CAN1_Mode_Init(void)
{
		u8 READ_SLOT;
		u8 READ_PORT;
		uint32_t MASK_ID_H,MASK_ID_L,i;
		uint16_t BoardADDR =0xffff;
		uint16_t temp			 =0xffff;
		uint8_t key=1;

	
  	GPIO_InitTypeDef 				GPIO_InitStructure; 
	  CAN_InitTypeDef       	CAN_InitStructure;
  	CAN_FilterInitTypeDef  	CAN_FilterInitStructure;
	
		
		for (i = 0; i < 100000000; i++);//�ӳ�������6ns*100 000 000=600ms֮���ٶ��忨��ַ
		while(key)
			{
				BoardADDR=ReadBoardADDR__();//��������ڵ�ַ
				printf("read boardaddr 1st %x\n",BoardADDR);
				for (i = 0; i < 100000; i++);
				temp=ReadBoardADDR__();			//��������ڵ�ַ
				printf("read boardaddr 2nd %x\n",temp);
				if((BoardADDR==temp)&&(BoardADDR!=0xffff))
					key=0;
			}
		READ_SLOT=(BoardADDR>>3)+2;
		READ_PORT=BoardADDR&0x7;
		
		if((READ_SLOT!=0)||(READ_PORT>2))
			{
				myprintf("PTP Master Only!");
				GlobalConfig.SlaveorMaster=(bool)0;
			}
		MASK_ID_H=((6<<8)|(READ_SLOT<<4)|READ_PORT)<<3;
		MASK_ID_L=(0x0904<<3)	;
		CAN1_SEND_ID=((8<<24)|(READ_SLOT<<20)|(READ_PORT<<16)|0x0940);
    //ʹ�����ʱ��
	  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//ʹ��PORTBʱ��	    
  	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);//ʹ��CAN1ʱ��	
    //��ʼ��GPIO
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11| GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	  //���Ÿ���ӳ������
	  GPIO_PinAFConfig(GPIOA,GPIO_PinSource11,GPIO_AF_CAN1); 
	  GPIO_PinAFConfig(GPIOA,GPIO_PinSource12,GPIO_AF_CAN1); 

		CAN_InitStructure.CAN_TTCM=DISABLE;			   //MCR-TTCM  �ر�ʱ�䴥��ͨ��ģʽʹ��
		CAN_InitStructure.CAN_ABOM=DISABLE;			   //MCR-ABOM  �ر��Զ����߹��� 
		CAN_InitStructure.CAN_AWUM=DISABLE;			   //MCR-AWUM  �ر��Զ�����ģʽ
		CAN_InitStructure.CAN_NART=ENABLE;			   //MCR-NART  ���������Զ��ش�	  
		CAN_InitStructure.CAN_RFLM=DISABLE;			   //MCR-RFLM  ����FIFO ������ ���ʱ�±��ĻḲ��ԭ�б���  
		CAN_InitStructure.CAN_TXFP=DISABLE;			   //MCR-TXFP  ����FIFO���ȼ� ȡ���ڱ��ı�ʾ�� 
		CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;  //��������ģʽ
		CAN_InitStructure.CAN_SJW=CAN_SJW_1tq;		  //BTR-SJW ����ͬ����Ծ��� 1��ʱ�䵥Ԫ
		CAN_InitStructure.CAN_BS1=CAN_BS1_4tq;		  //BTR-TS1 ʱ���1 ռ����4��ʱ�䵥Ԫ
		CAN_InitStructure.CAN_BS2=CAN_BS1_2tq;		  //BTR-TS1 ʱ���2 ռ����2��ʱ�䵥Ԫ	   
		CAN_InitStructure.CAN_Prescaler =6;		  		//BTR-BRP �����ʷ�Ƶ��  ������ʱ�䵥Ԫ��ʱ�䳤�� (CAN ʱ��Ƶ��Ϊ APB 1 = 42 MHz) 42/(1+4+2)/6=1 Mbps
		
  	CAN_Init(CAN1, &CAN_InitStructure); // ��ʼ��CAN1 
		//���ù�����0
 	  CAN_FilterInitStructure.CAN_FilterNumber=0;	  //������0
  	CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask; 	//����ģʽ
  	CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit; 	//32λ 																				
  	CAN_FilterInitStructure.CAN_FilterIdHigh=MASK_ID_H;      				//ֻɸѡCAN1_MASK_ID�ı���
  	CAN_FilterInitStructure.CAN_FilterIdLow=MASK_ID_L;																				
  	CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0xFFFF;     				//32λMASK������Ϊ1��λ������ʾ��FilterId��ͬ��ɸѡλ�ɹ���
  	CAN_FilterInitStructure.CAN_FilterMaskIdLow=0xF800;							//0xffff������3λ
   	CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_Filter_FIFO0;//������0������FIFO0
  	CAN_FilterInitStructure.CAN_FilterActivation=ENABLE; //���������0
  	CAN_FilterInit(&CAN_FilterInitStructure);//�˲�����ʼ��		
	  CAN_ITConfig(CAN1,CAN_IT_FMP0,ENABLE);//FIFO0��Ϣ�Һ��ж�����.		    
  
		return 0;
}   
 /**
  * @brief  CAN1_RX0��Ϣ�Һ��жϷ�����������POWERPCÿ�붯̬��Ϣ
  * @param  ��
  * @retval ��
  */		 

void CAN1_RX0_IRQHandler(void)
{
	CanRxMsg RxMessage;
	u32 i, FRAME_LEN,FRAME_NUMS,bufoff,FRAME_REV; 
	//u8 sendmsg[8]={0};
	if((CAN1_RX0_STA&0xff00)!=0)		//������ɱ�־	2016.06.21�޸Ľ����������ΪCAN1_RX1_STA&0xffff0000
		CAN1->RF0R |= CAN_RF0R_RFOM0; 		//��FIFO_0�Һ���Ϣ
	else
	{
		CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
		if(RxMessage.RTR)
			{
				CAN1_RX0_STA|=0x200;
			}
		else
			{
				FRAME_NUMS=RxMessage.ExtId&0x0f;     	//֡���
				bufoff=FRAME_NUMS<<3;			//����BUF��ŵĵ�ַƫ����
				FRAME_LEN=(RxMessage.ExtId&0xf0)>>4; 	//���İ�֡����
				FRAME_REV=(CAN1_RX0_STA&0xff)>>3;		//�յ�֡��=(�ֽ���/8)
				if(4==FRAME_LEN)							//
					{
						if(FRAME_REV==FRAME_LEN)		//2016.08.15�޸ģ�1.���İ�֡����=֡���=�յ�֡��2.���İ�֡����=֡���=0�������	
							CAN1_RX0_STA|=0x100;
					}	
				else if(FRAME_NUMS==0)							//����յ����İ�ͷ�����¿�ʼ����
					CAN1_RX0_STA=0;
				for(i=0;i<RxMessage.DLC;i++)				
					{
						CAN1_RX0_BUF[i+(bufoff)]=RxMessage.Data[i];
						CAN1_RX0_STA++;
						if((CAN1_RX0_STA&0xff)>(CAN1_RX0_LEN-1))
							CAN1_RX0_STA=0;//�������ݴ���,���¿�ʼ����	
					}	
			}
	
	}		
}


 /**
  * @brief  CAN��������״̬��ѯ
	* @param  CANx:CAN��x
	* @retval 0,�п�����; 1:�޿�����;
  */	
uint8_t CAN_GETMAILFLAG(CAN_TypeDef* CANx)
{
  uint8_t transmit_mailbox = 0;
  /* Check the parameters */
  assert_param(IS_CAN_ALL_PERIPH(CANx));
  /* Select one empty transmit mailbox */
  if ((CANx->TSR&CAN_TSR_TME0) == CAN_TSR_TME0)
  {
    transmit_mailbox = 0;
  }
  else if ((CANx->TSR&CAN_TSR_TME1) == CAN_TSR_TME1)
  {
    transmit_mailbox = 1;
  }
  else if ((CANx->TSR&CAN_TSR_TME2) == CAN_TSR_TME2)
  {
    transmit_mailbox = 2;
  }
  else
  {
    transmit_mailbox = CAN_TxStatus_NoMailBox;
  }
  if (transmit_mailbox != CAN_TxStatus_NoMailBox)
		return 0;
	else
		return 1;
}


 /**
  * @brief  CAN1�������ݺ���

  * @param  msg:����ָ��,���Ϊ8���ֽ�
	* @param  ext_id:���͵�CAN��չID
	* @param	len:���ݳ���(���Ϊ8)
	* @retval 0,�ɹ�; 1:����ʧ�ܣ�2:���߷�æ/����;
  */	
u8 CAN1_Send_Msg(u8* msg,u32 ext_id,u8 len)
{	
  u8 mbox;
  u32 i=0;
  CanTxMsg TxMessage;                   //����ṹ��TxMessage 
	TxMessage.ExtId=ext_id;	 							//������չ��ʾ����29λ��      	0 to 0x1FFFFFFF.  
  TxMessage.IDE=CAN_Id_Extended;		  	//ʹ����չ��ʽ
  TxMessage.RTR=CAN_RTR_Data;		  			//��Ϣ����Ϊ����֡
  TxMessage.DLC=len;							 			//���ݳ���(��Լ��Լ��Ϊ�̶�8)
  for(i=0;i<len;i++)
		{
			TxMessage.Data[i]=msg[i];	
		}
	while(CAN_GETMAILFLAG(CAN1))
		{
			i++;	//�ȴ�����
			if(i>=0XFFFF)
				{
					printf("can send no mailbox!\n");
					return 2;
				}						
		}
	mbox= CAN_Transmit(CAN1, &TxMessage); 
  i=0;
  while(CAN_TransmitStatus(CAN1, mbox)==CAN_TxStatus_Failed)
		{
			i++;	//�ȴ����ͽ���
			if(i>=0XFFFF)
				{
					printf("can send time put!\n");
					return 1;
				}
		}
	return 0;		
}



void CAN1_Send_u32msg(u32* u32msg,u32 ext_id)
{	
	u8 msg[8];
	msg[0] = ((u32msg[0])&0xff000000)>>24;
	msg[1] = ((u32msg[0])&0xff0000)>>16;
	msg[2] = ((u32msg[0])&0xff00)>>8;
	msg[3] = ((u32msg[0])&0xff);
	msg[4] = ((u32msg[1])&0xff000000)>>24;
	msg[5] = ((u32msg[1])&0xff0000)>>16;
	msg[6] = ((u32msg[1])&0xff00)>>8;
	msg[7] = ((u32msg[1])&0xff);
  CAN1_Send_Msg(msg,ext_id,8);
}



