#include "ptpd.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/init.h"
#include "lwip/tcp.h"
#include "lwip/tcp_impl.h"
#include "lwip/udp.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "ethernetif.h"
#include "main.h"
#include "netconf.h"
#include <stdio.h>
#include "share.h"


extern FigStructData GlobalConfig;
struct netif gnetif;
extern struct ip_addr ipaddr1;
extern struct ip_addr netmask1;
extern struct ip_addr gw1;

uint32_t TCPTimer = 0;
uint32_t ARPTimer = 0;
uint32_t IPaddress = 0;

#ifdef USE_DHCP
uint32_t DHCPfineTimer = 0;
uint32_t DHCPcoarseTimer = 0;
__IO uint8_t DHCP_state;
#endif

extern __IO uint32_t  EthStatus;		//����״̬
extern struct ADDressInfo addrInfo;
void LwIP_DHCP_Process_Handle(void);



void LwIP_Init(void)
{
  struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;
	uint8_t Raad_MACddr[6];
	
  mem_init();									//�ڴ���ʼ��
  memp_init();								//�ڴ������ʼ��
	Set_MAC_Address(GlobalConfig.MACaddr);
#ifdef USE_DHCP 							//DHCP�Զ������ַ
  ipaddr.addr = 0;
  netmask.addr = 0;
  gw.addr = 0;
#else													//���Զ��֣��Ӳ������ö�IP��Ϣ
	ipaddr.addr 	= htonl(GlobalConfig.IPaddr);
	netmask.addr 	= htonl(GlobalConfig.NETmark);
	gw.addr 			= htonl(GlobalConfig.GWaddr);
	printf("---------IP:%d.%d.%d.%d---------\r\n",(uint32_t)((ipaddr.addr&0x000000ff)>>0),(uint32_t)((ipaddr.addr&0x0000ff00)>>8),
																								(uint32_t)((ipaddr.addr&0x00ff0000)>>16),(uint32_t)((ipaddr.addr&0xff000000)>>24));
#endif  

  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);	//��ʼ����������gnetif
	//���бȽ���Ҫ������������ ethernetif_init�� ethernet_input��
	//ǰ�����û��Լ�����ĵײ�ӿڳ�ʼ�������� ethernet_input �������� IP ��ݽ����ݰ��ĺ���
  netif_set_default(&gnetif);												//ָ��gnetifΪLwip������ӿ�
  netif_set_up(&gnetif);														//ʹ������ӿ�
	ETH_GetMACAddress(ETH_MAC_Address0,Raad_MACddr);				//���Դ����ڶ���MAC��ַ
	printf("Local Mac:[ %02x:%02x:%02x:%02x:%02x:%02x ]\n",Raad_MACddr[0],Raad_MACddr[1],Raad_MACddr[2],Raad_MACddr[3],Raad_MACddr[4],Raad_MACddr[5]);
}


/**
* @brief  Called when a frame is received
* @param  None
* @retval None
*/
void LwIP_Pkt_Handle(void)	//�����жϵ��ú���
{
  ethernetif_input(&gnetif);
}

/**
* @brief  LwIP periodic tasks
* @param  localtime the current LocalTime value
* @retval None
*/
void LwIP_Periodic_Handle(__IO uint32_t localtime)	//Lwip���ڵ��õĺ���
{
#if LWIP_TCP
  if (localtime - TCPTimer >= TCP_TMR_INTERVAL)		//ÿ250ms����һ��tcp_tmr()����
		{
			TCPTimer =  localtime;
			tcp_tmr();
		}
#endif
  if ((localtime - ARPTimer) >= ARP_TMR_INTERVAL)	//ARPÿ5s�����Ե���һ��
		{
			ARPTimer =  localtime;
			etharp_tmr();
		}
#ifdef USE_DHCP							//���ʹ��DHCP�Ļ�
  if (localtime - DHCPfineTimer >= DHCP_FINE_TIMER_MSECS)	
		{
			DHCPfineTimer =  localtime;
			dhcp_fine_tmr();				//ÿ500ms����һ��dhcp_fine_tmr()
			if ((DHCP_state != DHCP_ADDRESS_ASSIGNED) && 
					(DHCP_state != DHCP_TIMEOUT) &&
						(DHCP_state != DHCP_LINK_DOWN))
				{
					LwIP_DHCP_Process_Handle();
				}
		}
  //ÿ60sִ��һ��DHCP�ֲڴ���
  if (localtime - DHCPcoarseTimer >= DHCP_COARSE_TIMER_MSECS)
		{
			DHCPcoarseTimer =  localtime;
			dhcp_coarse_tmr();
		}
#endif
}

#ifdef USE_DHCP
/**
* @brief  LwIP_DHCP_Process_Handle
* @param  None
* @retval None
*/
void LwIP_DHCP_Process_Handle()	//DHCP������
{
  struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;
	uint8_t	MAX_DHCP_TRIES	= 4;
  uint8_t iptab[4] = {0};
  uint8_t iptxt[20];
  switch (DHCP_state)
  {
		case DHCP_START:											//����DHCP״̬
				{
					DHCP_state = DHCP_WAIT_ADDRESS;	//״̬�л���DHCP_WAIT_ADDRESS
					dhcp_start(&gnetif);
					IPaddress = 0;
				}
				break;
		case DHCP_WAIT_ADDRESS:
				IPaddress = gnetif.ip_addr.addr;	//��ȡ��IP��ַ
				if (IPaddress!=0) 								//��ȷ��ȡ��IP��ַ��ʱ��
					{
						DHCP_state = DHCP_ADDRESS_ASSIGNED;		//DHCP�ɹ�
						dhcp_stop(&gnetif);
					}
				else
					{
						if (gnetif.dhcp->tries > MAX_DHCP_TRIES)	//ͨ��DHCP�����ȡIP��ַʧ��,�ҳ�������Դ���4
							{
								DHCP_state = DHCP_TIMEOUT;
								dhcp_stop(&gnetif);										//DHCP��ʱʧ��,ʹ�þ�̬IP��ַ
								IP4_ADDR(&ipaddr, IP_ADDR0 ,IP_ADDR1 , IP_ADDR2 , IP_ADDR3 );
								IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
								IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
								netif_set_addr(&gnetif, &ipaddr , &netmask, &gw);
							}
					}
				break;
		default: 
				break;
  }
}
#endif


