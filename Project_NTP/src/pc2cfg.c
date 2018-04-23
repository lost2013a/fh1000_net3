#include "main.h"
#include "serial_hand.h"
#include "pc2cfg.h"
#include "flash_conf.h"
extern RunTimeOpts rtOpts;
extern PtpClock G_ptpClock;
extern FigStructData GlobalConfig;

static void eventRecv(void *arg, struct udp_pcb *pcb, struct pbuf *p,
	struct ip_addr *addr, u16_t port)	//网络配置获取
{
	UDP_COM query;
	PTP_SETMSG smsg;
	PTP_SETMSG *pmsg;
	PTP_DISPLAY dmsg;
	UDP_COM *pCom = (UDP_COM*)p->payload;
	myprintf("CFG rec\n");
	switch(pCom->comtype)
		{
//			case COMDEF_GETSETINFO: //获取ptp设置信息
//					pbuf_free(p);
//					query.comtype = COMDEF_ACKGETSETINFO;
//					MEMCPY(query.data, &smsg, sizeof(smsg));
//					p=pbuf_alloc(PBUF_TRANSPORT,sizeof(query),PBUF_RAM);
//					if(p==NULL)	return;
//					MEMCPY(p->payload, &query, sizeof(query));
//					udp_sendto(pcb,p,addr,port);    
//					break;
			case COMDEF_GETDISPLAY:
					pbuf_free(p);
					query.comtype = COMDEF_ACKDISPLAY;
					dmsg.clock_type = MSG_MASTERCLOCK;
					dmsg.twostepflag = MSG_TWOSTEP;
					switch(GlobalConfig.WorkMode)
					{
						case 0: 
								dmsg.ethernet_mode 			= 1;	//0:E2E-MAC
								dmsg.e2e_mode 					= 1; 				
								break;
						case 1:
								dmsg.ethernet_mode 			= 1;	//0:E2E-MAC
								dmsg.e2e_mode 					= 0; 	
								break;
						case 2: 
								dmsg.ethernet_mode 			= 0;	//0:E2E-MAC
								dmsg.e2e_mode 					= 1; 
								break;
						case 3: 
								dmsg.ethernet_mode 			= 0;	//0:E2E-MAC
								dmsg.e2e_mode 					= 0; 
								break;
						default: 					
								dmsg.ethernet_mode 			= 1;	//0:E2E-MAC
								dmsg.e2e_mode 					= 1;			
								break;
					}
			
//					dmsg.e2e_mode =  rtOpts.delayMechanism;
//					dmsg.ethernet_mode = rtOpts.transport;
					dmsg.portState = G_ptpClock.portState;
					dmsg.timeSource = G_ptpClock.timePropertiesDS.timeSource;
					dmsg.offsetFromMaster_nanoseconds =0;// HWREG(ETH_BASE+ MAC_O_RCTL);// g_sPTPClock.monitor_1;
					dmsg.offsetFromMaster_seconds = 0;//HWREG(ETH_BASE+ MAC_O_TCTL);;
					dmsg.meanPathDelay_nanoseconds= 0;//HWREG(ETH_BASE+ MAC_O_TS);;
					dmsg.meanPathDelay_seconds = 0;// HWREG(SYSCTL_BASE+ 0x0000001C);//g_sPTPClock.monitor_delayreqlenth;;
					MEMCPY(&dmsg.local_mac ,&G_ptpClock.interfaceID,6);
					dmsg.bestmasterclock_mac[0] = G_ptpClock.grandmasterIdentity[0];
					dmsg.bestmasterclock_mac[1] = G_ptpClock.grandmasterIdentity[1];
					dmsg.bestmasterclock_mac[2] = G_ptpClock.grandmasterIdentity[2];
					dmsg.bestmasterclock_mac[3] = G_ptpClock.grandmasterIdentity[5];
					dmsg.bestmasterclock_mac[4] = G_ptpClock.grandmasterIdentity[6];
					dmsg.bestmasterclock_mac[5] = G_ptpClock.grandmasterIdentity[7];
					dmsg.local_mac[0]						=	GlobalConfig.MACaddr[0];
					dmsg.local_mac[1]						=	GlobalConfig.MACaddr[1];
					dmsg.local_mac[2]						=	GlobalConfig.MACaddr[2];
					dmsg.local_mac[3]						=	GlobalConfig.MACaddr[3];
					dmsg.local_mac[4]						=	GlobalConfig.MACaddr[4];
					dmsg.local_mac[5]						=	GlobalConfig.MACaddr[5];
					
					MEMCPY(query.data, &dmsg, sizeof(dmsg));
					p=pbuf_alloc(PBUF_TRANSPORT,sizeof(query),PBUF_RAM);
					if(p==NULL)	return;
					MEMCPY(p->payload, &query, sizeof(query));
					udp_sendto(pcb,p,addr,port);   
					break;
			case COMDEF_SETINFO:
				{
					int32_t config_tmp[10];	
					int32_t	i;
					pmsg= (PTP_SETMSG*)pCom->data;
		
					GlobalConfig.IPaddr   = pmsg->ip;
					GlobalConfig.GWaddr   = pmsg->gw_ip;
					GlobalConfig.NETmark  = pmsg->SubnetMask;
					//GlobalConfig.DstIpaddr= Gx_NTPFIGC.Server_ip_32; 
					if(pmsg->ETHERNET_MODE==1)
						{
							if(pmsg->E2E_mode)
								GlobalConfig.WorkMode=0;
							else
								GlobalConfig.WorkMode=1;
						}
					else
						{
							if(pmsg->E2E_mode)
								GlobalConfig.WorkMode=2;
							else
								GlobalConfig.WorkMode=3;
						}
					GlobalConfig.SyncInterval   	=	pmsg->SYNC_INTERVAL;
					GlobalConfig.DelayInterval		=	pmsg->DELAYREQ_INTERVAL;
					GlobalConfig.PDelayInterval		= pmsg->PDELAYREQ_INTERVAL;
					GlobalConfig.priority1				=	pmsg->SLAVE_PRIORITY1;
					memcpy((int8_t *)config_tmp,(int8_t *)&GlobalConfig,16);
					config_tmp[4]	=	((GlobalConfig.LocalPort)<<16)|GlobalConfig.DstPort;	
					config_tmp[5]	=	((GlobalConfig.WorkMode)<<24)|((GlobalConfig.ip_mode)<<16)|((GlobalConfig.BroadcastInterval)<<8)|(GlobalConfig.tmp1);
					config_tmp[6]	=	((GlobalConfig.MACaddr[0])<<24)|((GlobalConfig.MACaddr[1])<<16)|((GlobalConfig.MACaddr[2])<<8)|(GlobalConfig.MACaddr[3]);
					config_tmp[7]	=	((GlobalConfig.MACaddr[4])<<24)|((GlobalConfig.MACaddr[5])<<16)|((GlobalConfig.MSorStep)<<8)|(GlobalConfig.ClockDomain);
					config_tmp[8]	=	((GlobalConfig.SlaveorMaster)<<24)|((GlobalConfig.Offset_UTC)<<16)|((GlobalConfig.SyncInterval)<<8)|(GlobalConfig.PDelayInterval);
					config_tmp[9]	=	((GlobalConfig.DelayInterval)<<24)|((GlobalConfig.clockClass)<<16)|((GlobalConfig.priority1)<<8)|(GlobalConfig.tmp2);
						
						
					query.comtype = COMDEF_ACKSETINFO;
					MEMCPY(query.data, &smsg, sizeof(smsg));
					p=pbuf_alloc(PBUF_TRANSPORT,sizeof(query),PBUF_RAM);
					if(p==NULL)	return;
					MEMCPY(p->payload, &query, sizeof(query));
					udp_sendto(pcb,p,addr,port);
						
						
						
					pbuf_free(p); 	
					NVIC_DisableIRQ(ETH_IRQn);		//写FLASH前关闭中断
					NVIC_DisableIRQ(USART1_IRQn);	
					NVIC_DisableIRQ(EXTI1_IRQn);
					NVIC_DisableIRQ(USART6_IRQn);
					FLASH_Write(FLASH_SAVE_ADDR1,config_tmp,10);
					for(i=0;i<10000;i++){}//等待	
					NVIC_SystemReset();
				}
					break;
			default:
					pbuf_free(p);
					break;
		}	  
	pbuf_free(p);	
}


void Multicastserverinit(void)	//组网网络配置
{
	void *pcb;
	struct ip_addr ip_data;
	int ipMultiCast=inet_addr("228.18.18.18");
	ip_data.addr = htonl(ipMultiCast);	
#if LWIP_IGMP
	igmp_joingroup(IP_ADDR_ANY,&ip_data);
#endif
	pcb = udp_new();
	udp_bind(pcb, IP_ADDR_ANY, 7861);
	udp_recv(pcb, eventRecv, NULL);
}
