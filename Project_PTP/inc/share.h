#ifndef __SHARE_H
#define __SHARE_H

#include "ntp.h"
#include "main.h"
#include "flash_conf.h"

#ifdef __cplusplus
 extern "C" {
#endif

#define NTPSET_PORT 7861


	 

struct Sync_UartData
{
	unsigned char frame_head;
	unsigned char state_flag1;
	unsigned char state_flag2;
	unsigned char state_flag3;
	unsigned char state_flag4;
	unsigned char year_4;
	unsigned char year_3;
	unsigned char year_2;
	unsigned char year_1;
	unsigned char month_2;
	unsigned char month_1;
	unsigned char day_2;
	unsigned char day_1;
	unsigned char hour_2;
	unsigned char hour_1;
	unsigned char min_2;
	unsigned char min_1;
	unsigned char sec_2;
	unsigned char sec_1;
	unsigned char chksum_2;
	unsigned char chksum_1;
	unsigned char end_flag1;
	unsigned char end_flag2;
};





//#define MSG_ONESTEP  0
//#define MSG_TWOSTEP  1

//#define MSG_P2PMODE 0
//#define MSG_E2EMODE 1

//#define MSG_UDPMODE 0
//#define MSG_ETHMODE 1

//#define MSG_MASTERCLOCK  0
//#define MSG_SLAVECLOCK   1


//#define COMDEF_GETSETINFO      1
//#define COMDEF_ACKGETSETINFO   2
//#define COMDEF_SETINFO         3
//#define COMDEF_ACKSETINFO      4 
//#define COMDEF_GETDISPLAY      5 
//#define  COMDEF_ACKDISPLAY     6
//#define COMDEF_SENDIP					 7
//#define COMDEF_IP							 8

typedef struct SendIP
{
	unsigned int  net_ip;
	unsigned int  net_gw;
	unsigned int  net_mark;
}__attribute__((packed)) Send_IP;


typedef struct SetIP
{
	unsigned int  ip;
	unsigned short port;
	unsigned char time;
}__attribute__((packed)) Set_IP;

//typedef struct tagUdpcom
//{
//   unsigned char comtype;
//   unsigned char data[50];

//}__attribute__((packed)) UDP_COM;

///*网络结构体定义*/
//typedef struct TAGPTPSETMSG //godin 网络配置结构体
//{
//	unsigned int  ip;	
//	unsigned int  gw_ip;
//	unsigned int  SubnetMask;
//	unsigned char ETHERNET_MODE;
//	unsigned char MULTICAST_MODE;
//	unsigned char E2E_mode;
//	unsigned char ANNOUNCE_INTERVAL;
//	unsigned char ANNOUNCE_RECEIPT_TIMEOUT;
//	unsigned char SYNC_INTERVAL;
//	unsigned char PDELAYREQ_INTERVAL;
//  unsigned char DELAYREQ_INTERVAL;
//	unsigned char SLAVE_PRIORITY1 ;//优先级1
//  unsigned char SLAVE_PRIORITY2 ;//优先级2
//  unsigned char DOMAINNUMBER ;//时钟域
//  unsigned char TWOSTEPFLAG ;//两步法
//	
//}__attribute__((packed)) PTP_SETMSG;

//typedef struct  TAGPTPDISPLAY
//{
//  unsigned char    clock_type;
//	unsigned char    twostepflag;
//	unsigned char    ethernet_mode;
//	unsigned char    e2e_mode;
//	unsigned char    portState;
//	unsigned char    timeSource;
//  unsigned char    bestmasterclock_mac[6];	
//	char     				 local_mac[6];
//	unsigned char    warning_signal;
//	int                 offsetFromMaster_seconds;
//	int                 offsetFromMaster_nanoseconds;
//	int                meanPathDelay_seconds;
//	int                meanPathDelay_nanoseconds;
//}__attribute__((packed))  PTP_DISPLAY;//实际占用字节模式(取消自动对齐)

typedef struct
{
	unsigned short usYear;//! The number of years since 0 AD.
	unsigned char ucMon; //! The month, where January is 0 and December is 11.
	unsigned char ucMday; //! The day of the month.
	unsigned char ucWday;//! The day of the week, where Sunday is 0 and Saturday is 6.
	unsigned char ucHour; //! The number of hours.
	unsigned char ucMin; //! The number of minutes.
	unsigned char ucSec; //! The number of seconds.
}
tTime;

typedef struct _syncdata	//35
{
	unsigned char frame_head;
	unsigned char state_flag1;
	unsigned char state_flag2;
	unsigned char state_flag3;
	unsigned char state_cma1;//逗号
	unsigned char state_flag4;
	unsigned char state_flag5;
	unsigned char state_flag6;
	unsigned char state_cma2;//逗号
	unsigned char year_4;
	unsigned char year_3;
	unsigned char year_2;
	unsigned char year_1;
	unsigned char state_cma3;//逗号
	unsigned char month_2;
	unsigned char month_1;
	unsigned char state_cma4;//逗号
	unsigned char day_2;
	unsigned char day_1;
	unsigned char state_cma5;//逗号
	unsigned char hour_2;
	unsigned char hour_1;
	unsigned char state_cma6;//逗号
	unsigned char min_2;
	unsigned char min_1;
	unsigned char state_cma7;//逗号
	unsigned char sec_2;
	unsigned char sec_1;
	unsigned char state_cma8;//逗号
	unsigned char Tzone_1;
	unsigned char Tzone_2;
	unsigned char Tzone_3;
	unsigned char state_cma9;//逗号
	unsigned char sync_state;
	unsigned char frame_end;
}SyncData;

////*IE1,CEB,xxx,xx,%d#
//typedef struct _recleapdata	//18
//{
//	unsigned char frame_head;
//	unsigned char state_flag1;	//IE1
//	unsigned char state_flag2;
//	unsigned char state_flag3;
//	unsigned char state_cma1;		//逗号
//	unsigned char state_flag4;	//CEL
//	unsigned char state_flag5;
//	unsigned char state_flag6;
//	unsigned char state_cma2;		//逗号
//	unsigned char state_flag7;	//xxx
//	unsigned char state_flag8;
//	unsigned char state_flag9;
//	unsigned char state_cma3;		//逗号
//	unsigned char state_flag10;	//xx
//	unsigned char state_flag11;
//	unsigned char state_cma4;		//逗号
//	unsigned char leap;
//	unsigned char frame_end;
//}_leapcData;



////*IE1,CEL,%bu#（0：无；1：正；2：负）
//typedef struct _vrecleapdata	//11
//{
//	unsigned char frame_head;
//	unsigned char state_flag1;	//IE1
//	unsigned char state_flag2;
//	unsigned char state_flag3;
//	unsigned char state_cma1;		//逗号
//	unsigned char state_flag4;	//CEL
//	unsigned char state_flag5;
//	unsigned char state_flag6;
//	unsigned char state_cma2;		//逗号
//	unsigned char leap;
//	unsigned char frame_end;
//}_vleapcData;







//*****************************************************************************
// This array contains the number of days in a year at the beginning of each
// month of the year, in a non-leap year.
//*****************************************************************************
static const short g_psDaysToMonth[12] =
{
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
};
static const char *ppcDay[7] =
{
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
static const char *ppcMonth[12] =
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};


void ulocaltime(unsigned long ulTime, tTime *psTime);
void NTP_Init(void);

void NTPClientLoop(void);
void offset_time(sysTime *calcTime);
void abjClock(const TimeInternal subtime);
unsigned long  Serial_Htime(tTime *sulocaltime);
unsigned long TimeToSeconds( tTime *psTime );
unsigned char getCfgDataHeadSum(unsigned char* ptr,int len);
unsigned char char_to_8421(const unsigned char num);
void Debug_Time(tTime Time,Integer32 nanoseconds);
void UARTSend(const unsigned char *pucBuffer, unsigned long ulCount);

#ifdef __cplusplus
}
#endif
#endif
