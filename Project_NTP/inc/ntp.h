#ifndef __NTP_H__
#define __NTP_H__

#include "stm32f4x7_eth.h"
#include "ptpd.h"
#include "sys.h"
#include "lwip/memp.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include <stdio.h>

typedef struct 
{
	unsigned int seconds;
	unsigned int fraction;
}ntp_time;

#define NTP_PORT    		123 //NTP�˿ڹ涨ʹ��123 
#define NTP_PCK_LEN		  48
#define MAX_GOOD_DATA	  1

#define LI_NOWARNING		(0 << 6)	
#define LI_PLUSSEC			(1 << 6)	
#define LI_MINUSSEC			(2 << 6)
#define LI_ALARM				(3 << 6)

#define NTP_MAXSTRATUM	 15
#define MODEMASK				(7 << 0)
#define VERSIONMASK			(7 << 3)
#define LIMASK					(3 << 6)

#define JAN_1970 		0x83AA7E80 
#define JAN_2030	 	0x70DBD880 + JAN_1970

#define LOC_STRATUM			1
#define LOC_PRECISION 	0xe3//0xe9 //���ɵ�ȡ����0xe9
#define LOC_RTDELAY			0
#define LOC_DISPER			0
#define LOC_FERID 			0x00535047 //0x53504700   //0x4d4f5441//gps��ascii��//���ɵ�ȡ����0x4d4f5441

#define DEFAULT_STATUS	0xdb		//�ͻ���δͬ��
#define NOMAL_STATUS		0x1b		
#define DEFAULT_STRATUM	0
#define DEFAULT_POLL		6
#define DEFAULT_PREC		0xf0
#define DEFAULT_RTDELAY	0
#define DEFAULT_DISPER	0
#define DEFAULT_FERID		0x53504700
#define Test_LOC_FERID  0x4d535354

enum
{	ACT_MOD=1,    //1
	PAS_MOD,   		//2
	CLIENT_MOD,		//3
	SERVER_MOD,		//4
	BDC_MOD				//5
};

//typedef struct 
//{
//	unsigned int seconds;
//	unsigned int fraction;
//}ntp_time;

typedef struct cfgDataHead
{
	unsigned char  frame_head;  //*
	unsigned char	 cmd_type;
	unsigned char  temp0;// Out delay��ns��signed      bit	
	unsigned char	 temp1;// 			// �ͷ������б�־ 	   				1
	unsigned int	 Local_ip_32;		// Local ip 						4
	unsigned int	 Server_ip_32;		// Server ip 	   					4
	unsigned int	 Gate_ip_32;			// Gate ip 4
	unsigned int   Mask_ip_32;
	unsigned int	 testMark;// 		// Out delay��ns�� 			4
	unsigned int	 temp3;// 			// �ͻ�����ʱ������us�� 	   		4
	unsigned int	 temp4;// 			// �������ʱ������us��				4
	unsigned char  temp5;//			 //ʱ������
	unsigned char  temp6;// 
	unsigned char	 temp7;// 			// CHKSUM �����                                                      4
	unsigned char  frame_end;        //  #
}ConfigHead;
/*
*@ntp ���ݽṹ��
*/
typedef struct 
{
	unsigned char 	status;		//bit7-6������ʾ,bit5-3NTP�汾,bit2-0����ģʽ
	unsigned char  	stratum;	//ʱ�ӵȼ�
	unsigned char  	poll;			//��������NTP����ѯ���
	signed char  	precision;	//ϵͳʱ�Ӿ���
	unsigned int 	rootdelay;	//���ص����ο�ʱ��Դ������ʱ��
	unsigned int 	dispersion;	//ϵͳʱ����������ο�ʱ�ӵ�������
	unsigned int 	refid;			//�ο�ʱ��Դ�ı�ʶ
	ntp_time 		reftime;			//ϵͳʱ�����һ�α��趨����µ�ʱ��
	ntp_time 		orgtime;			//NTP�������뿪���Ͷ�ʱ���Ͷ˵ı���ʱ��
	ntp_time 		rxtime;				//NTP�����ĵ�����ն�ʱ���ն˵ı���ʱ��
	ntp_time 		txtime;				//Ӧ�����뿪Ӧ����ʱӦ���ߵı���ʱ��
}ntp_msg;


typedef struct timeData
{
	unsigned char    frame_head; //֡ͷ0xeb
	unsigned char    frame_head1; //֡ͷ0x90
	unsigned char    frame_head2; //֡ͷ0xeb
	unsigned char    frame_head3; //֡ͷ0x90
	unsigned int     T1secondsFrom1900; //1900��1��1��0ʱ0������������
	unsigned int     T1fraction;        //΢���4294.967296(=2^32/10^6)��
	unsigned int     T2secondsFrom1900; //1900��1��1��0ʱ0������������
	unsigned int     T2fraction;        //΢���4294.967296(=2^32/10^6)��
	unsigned int     T3secondsFrom1900;  //1900��1��1��0ʱ0������������
	unsigned int     T3fraction;        //΢���4294.967296(=2^32/10^6)��
	unsigned int     T4secondsFrom1900;   //1900��1��1��0ʱ0������������
	unsigned int     T4fraction;          //΢���4294.967296(=2^32/10^6)��

	unsigned char    data_type; //��Ӧ�������ʽ����DATA_TYPEö��
	unsigned char    updateNum; //����ѭ�����--���ڼ�������Ƿ����
	unsigned char    chkSum_1; //ʱ������
	unsigned char    CFL_leapflag;//temp8;//��������ı�־λ����
	unsigned int     fjm;
	/* 
	unsigned int  temp2; // �ͻ�����ʱ������us��      4
	unsigned int  temp3; // �ͻ�����ʱ������us��      4
	unsigned int  temp4; // �ͻ�����ʱ������us��      4
	unsigned int  temp5; // �ͻ�����ʱ������us��      4
	unsigned int  temp6; // �ͻ�����ʱ������us��      4
	*/
	unsigned char    frame_end0; //֡β=0xEF 
	unsigned char    frame_end1; //֡β=0x9F
	unsigned char    frame_end2; //֡β=0xEF 
	unsigned char    frame_end3; //֡β=0x9F
}TimeData;
/*
NTP����֡ͷ��ʽ
LI������:00(��Ԥ��),01(����61),10(����59),11(����)
VN�汾��
MODE:Э��ģʽ(0-7)����0,�Գ�1,�����Գ�2,�ͻ���3,��������4,�㲥5,NTP����ϵͳ6,7�Ա���
Stratum:ʱ�ӵĲ��ˮƽ,0δָ��,1���ο�,2-255�ڶ��ο�
Poll:��ѯ���signed integer,2�Ĵ���
Precision:����
Root Delay:���ӳ�,�����ο�Դ�����������ӳ�
Root Disersion�����:��������ο�Դ��������
Reference Identifier�ο�ʱ�ӱ��:��32Ϊ����ض��Ĳο�
			 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |LI | VN  |Mode |    Stratum    |     Poll      |   Precision   	|
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                       Root Delay(32)                      			|
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                       Root Disersion(32)                 		  	|
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                       Reference Identifier(32)            			|
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                       Reference Timestamp(64)             			|
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                          �ο�ʱ���(64) T1                    	|
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                          ԭʼʱ���(64) T2                    	|
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                          ����ʱ���(64) T3                    	|
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                          ����ʱ���(64) T4                    	|
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			|                          ������֤�ߵ���Ϣ(96)                 	|
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

static void client_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,
               struct ip_addr *addr, u16_t port);
void Ntp_server_init(void);
void ntpClientLoop(unsigned int ip);
#endif
