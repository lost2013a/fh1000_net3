#ifndef __PC2CFG__
#define __PC2CFG__

#include "stm32f4x7_eth.h"
#include "ptpd.h"
#include "sys.h"
#include "lwip/memp.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include <stdio.h>

#define COMDEF_GETSETINFO      1
#define COMDEF_ACKGETSETINFO   2
#define COMDEF_SETINFO         3
#define COMDEF_ACKSETINFO      4 
#define COMDEF_GETDISPLAY      5 
#define  COMDEF_ACKDISPLAY     6
#define COMDEF_SENDIP					 7
#define COMDEF_IP							 8


#define MSG_ONESTEP  0
#define MSG_TWOSTEP  1

#define MSG_P2PMODE 0
#define MSG_E2EMODE 1

#define MSG_UDPMODE 0
#define MSG_ETHMODE 1

#define MSG_MASTERCLOCK  0
#define MSG_SLAVECLOCK   1



typedef struct tagUdpcom
{
   unsigned char comtype;
   unsigned char data[50];

}__attribute__((packed)) UDP_COM;

/*����ṹ�嶨��*/
typedef struct TAGPTPSETMSG //godin �������ýṹ��
{
	unsigned int  ip;	
	unsigned int  gw_ip;
	unsigned int  SubnetMask;
	unsigned char ETHERNET_MODE;
	unsigned char MULTICAST_MODE;
	unsigned char E2E_mode;
	unsigned char ANNOUNCE_INTERVAL;
	unsigned char ANNOUNCE_RECEIPT_TIMEOUT;
	unsigned char SYNC_INTERVAL;
	unsigned char PDELAYREQ_INTERVAL;
  unsigned char DELAYREQ_INTERVAL;
	unsigned char SLAVE_PRIORITY1 ;//���ȼ�1
  unsigned char SLAVE_PRIORITY2 ;//���ȼ�2
  unsigned char DOMAINNUMBER ;//ʱ����
  unsigned char TWOSTEPFLAG ;//������
	
}__attribute__((packed)) PTP_SETMSG;

typedef struct  TAGPTPDISPLAY
{
  unsigned char    clock_type;
	unsigned char    twostepflag;
	unsigned char    ethernet_mode;
	unsigned char    e2e_mode;
	unsigned char    portState;
	unsigned char    timeSource;
  unsigned char    bestmasterclock_mac[6];	
	char     				 local_mac[6];
	unsigned char    warning_signal;
	int                 offsetFromMaster_seconds;
	int                 offsetFromMaster_nanoseconds;
	int                meanPathDelay_seconds;
	int                meanPathDelay_nanoseconds;
}__attribute__((packed))  PTP_DISPLAY;//ʵ��ռ���ֽ�ģʽ(ȡ���Զ�����)

void Multicastserverinit(void);	//������������

#endif
