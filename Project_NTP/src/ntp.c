#include "ntp.h"
#include "main.h"
#include "share.h"
extern FigStructData GlobalConfig;

extern unsigned char leap59; 
extern unsigned char leap61;
extern unsigned char leapwarning;
extern unsigned char leapflag;
extern unsigned char synflags;//已经同步标记
char ntpsendf = 0;

unsigned int NTPFRAC(unsigned int x)  
{	
	return (4294 * (x) +  ((1981 * (x)) >> 11) + ((2911*(x))>>28));	//更精确
}

void getNtpTime( ntp_time* t_ntpTime)
{	
	TimeInternal t_psRxTime;
	getTime(&t_psRxTime);
	t_ntpTime->seconds = t_psRxTime.seconds + JAN_1970;
	t_ntpTime->fraction = NTPFRAC( t_psRxTime.nanoseconds /1000);
}





#ifdef NTP_Server
struct udp_pcb 	*ntps_pcb;
extern unsigned char leap61;
extern unsigned char leap59;
extern unsigned char leapwring;

//NTP服务器接收Ntp数据包处理函数
void server_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,
    ip_addr_t *addr, u16_t port)
{
		struct pbuf *reply_pbuf = NULL;
		ntp_time rx_time, tx_time;
		ntp_msg query, reply;
		ip_addr_t Dstaddr;
		 
		//当服务器收到消息时获取一次时间
		getNtpTime(&rx_time);
		if(p->len != NTP_PCK_LEN)//判断收到的数据长度是否符合
		{
			pbuf_free(p);
			return;
		}
		memset(&reply,0,NTP_PCK_LEN);//清空回复数据
		MEMCPY(&query,p->payload,NTP_PCK_LEN);//
		
		pbuf_free(p);
		
		if(leapwring)
		{
			reply.status  = LI_ALARM; //未同步
		}
		if(leap61)
		{
			reply.status = LI_PLUSSEC;
		}
		else if(leap59)
		{
			reply.status = LI_MINUSSEC;
		}
		else
			reply.status = LI_NOWARNING;//正常时间
		
		reply.status |= (query.status & VERSIONMASK);//将收到的query.statut字段赋给reply //版本号
	
		//send_pack.status = DEFAULT_STATUS;客服端发送的是0xdb未同步
		switch(query.status & MODEMASK)//判断发送query的主机所在状态
		{	
			case ACT_MOD:
				reply.status |= PAS_MOD; 
				break;
			case CLIENT_MOD:
			case BDC_MOD: 
				if(GlobalConfig.ip_mode == 2) //0(单播) 1(组播) 2(广播)//godin 支持广播模式
					reply.status |=	BDC_MOD;//广播模式
				else
					reply.status |= SERVER_MOD;//客服端为3,服务器回送字段为4
				break;
			default:
				return;
		}

		//填充数据
		reply.stratum 	 = LOC_STRATUM;//时钟层和本地时钟精度
		reply.poll 			 = query.poll; //连续消息间的最大间隔
		reply.precision  = LOC_PRECISION;//本地时钟精度
		reply.rootdelay  = LOC_RTDELAY;
		reply.dispersion = LOC_DISPER;
		reply.refid 		 = LOC_FERID;		 //

		if(query.refid == Test_LOC_FERID)//如果请求包的refid是Test_LOC_FERID，回相同的
		{
			reply.refid = Test_LOC_FERID;
		}
		//T1
		reply.orgtime.seconds		= query.txtime.seconds; //NTP发送时间T1
		reply.orgtime.fraction	= query.txtime.fraction;//
		//T2
		reply.rxtime.seconds 		= htonl(rx_time.seconds) ;//query的接收时间T2
		reply.rxtime.fraction 	= htonl(rx_time.fraction);//
		
		reply_pbuf = pbuf_alloc(PBUF_TRANSPORT,NTP_PCK_LEN,PBUF_RAM);//申请一个reply_pbuf用于发送reply
		if(reply_pbuf == NULL) 
		{
			return;	
		}
		
		getNtpTime(&tx_time);//回复时间
		//T3
		reply.reftime.seconds		= htonl(tx_time.seconds);//回送数据包时间
    reply.reftime.fraction	= htonl(tx_time.fraction);
		//T4
		reply.txtime.seconds		= htonl(tx_time.seconds);//暂时用回送数据包时间代替
		reply.txtime.fraction		= htonl(tx_time.fraction);//
		
		MEMCPY(reply_pbuf->payload, &reply, NTP_PCK_LEN);
		
		if(GlobalConfig.ip_mode == 2)
		{
			Dstaddr.addr = htonl(0xffffffff); //广播
			udp_sendto(pcb, reply_pbuf, &Dstaddr, port);
		}
		else
			udp_sendto(pcb, reply_pbuf, addr, port);//谁发给我回送给谁
		
		pbuf_free(reply_pbuf);
		//printf("ntp server send time %d\n",tx_time.seconds%60);
}

void NTP_ServerInit(void)
{
	ntps_pcb = udp_new();
	udp_bind(ntps_pcb,IP_ADDR_ANY,NTP_PORT);
	udp_recv(ntps_pcb, server_recv, NULL);
	printf("ntp Server init\n");
}	
#endif	//end NTP_Server


//初始化NTP服务器
void NTP_Init(void)
{
#ifdef NTP_Server   //godin ntp服务器开启
	NTP_ServerInit();
#endif
}

