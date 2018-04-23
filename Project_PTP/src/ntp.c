#include "ntp.h"
#include "main.h"
#include "share.h"
extern FigStructData GlobalConfig;

extern unsigned char leap59; 
extern unsigned char leap61;
extern unsigned char leapwarning;
extern unsigned char leapflag;
extern unsigned char synflags;//�Ѿ�ͬ�����
char ntpsendf = 0;

unsigned int NTPFRAC(unsigned int x)  
{	
	return (4294 * (x) +  ((1981 * (x)) >> 11) + ((2911*(x))>>28));	//����ȷ
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

//NTP����������Ntp���ݰ�������
void server_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,
    ip_addr_t *addr, u16_t port)
{
		struct pbuf *reply_pbuf = NULL;
		ntp_time rx_time, tx_time;
		ntp_msg query, reply;
		ip_addr_t Dstaddr;
		 
		//���������յ���Ϣʱ��ȡһ��ʱ��
		getNtpTime(&rx_time);
		if(p->len != NTP_PCK_LEN)//�ж��յ������ݳ����Ƿ����
		{
			pbuf_free(p);
			return;
		}
		memset(&reply,0,NTP_PCK_LEN);//��ջظ�����
		MEMCPY(&query,p->payload,NTP_PCK_LEN);//
		
		pbuf_free(p);
		
		if(leapwring)
		{
			reply.status  = LI_ALARM; //δͬ��
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
			reply.status = LI_NOWARNING;//����ʱ��
		
		reply.status |= (query.status & VERSIONMASK);//���յ���query.statut�ֶθ���reply //�汾��
	
		//send_pack.status = DEFAULT_STATUS;�ͷ��˷��͵���0xdbδͬ��
		switch(query.status & MODEMASK)//�жϷ���query����������״̬
		{	
			case ACT_MOD:
				reply.status |= PAS_MOD; 
				break;
			case CLIENT_MOD:
			case BDC_MOD: 
				if(GlobalConfig.ip_mode == 2) //0(����) 1(�鲥) 2(�㲥)//godin ֧�ֹ㲥ģʽ
					reply.status |=	BDC_MOD;//�㲥ģʽ
				else
					reply.status |= SERVER_MOD;//�ͷ���Ϊ3,�����������ֶ�Ϊ4
				break;
			default:
				return;
		}

		//�������
		reply.stratum 	 = LOC_STRATUM;//ʱ�Ӳ�ͱ���ʱ�Ӿ���
		reply.poll 			 = query.poll; //������Ϣ��������
		reply.precision  = LOC_PRECISION;//����ʱ�Ӿ���
		reply.rootdelay  = LOC_RTDELAY;
		reply.dispersion = LOC_DISPER;
		reply.refid 		 = LOC_FERID;		 //

		if(query.refid == Test_LOC_FERID)//����������refid��Test_LOC_FERID������ͬ��
		{
			reply.refid = Test_LOC_FERID;
		}
		//T1
		reply.orgtime.seconds		= query.txtime.seconds; //NTP����ʱ��T1
		reply.orgtime.fraction	= query.txtime.fraction;//
		//T2
		reply.rxtime.seconds 		= htonl(rx_time.seconds) ;//query�Ľ���ʱ��T2
		reply.rxtime.fraction 	= htonl(rx_time.fraction);//
		
		reply_pbuf = pbuf_alloc(PBUF_TRANSPORT,NTP_PCK_LEN,PBUF_RAM);//����һ��reply_pbuf���ڷ���reply
		if(reply_pbuf == NULL) 
		{
			return;	
		}
		
		getNtpTime(&tx_time);//�ظ�ʱ��
		//T3
		reply.reftime.seconds		= htonl(tx_time.seconds);//�������ݰ�ʱ��
    reply.reftime.fraction	= htonl(tx_time.fraction);
		//T4
		reply.txtime.seconds		= htonl(tx_time.seconds);//��ʱ�û������ݰ�ʱ�����
		reply.txtime.fraction		= htonl(tx_time.fraction);//
		
		MEMCPY(reply_pbuf->payload, &reply, NTP_PCK_LEN);
		
		if(GlobalConfig.ip_mode == 2)
		{
			Dstaddr.addr = htonl(0xffffffff); //�㲥
			udp_sendto(pcb, reply_pbuf, &Dstaddr, port);
		}
		else
			udp_sendto(pcb, reply_pbuf, addr, port);//˭�����һ��͸�˭
		
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


//��ʼ��NTP������
void NTP_Init(void)
{
#ifdef NTP_Server   //godin ntp����������
	NTP_ServerInit();
#endif
}

