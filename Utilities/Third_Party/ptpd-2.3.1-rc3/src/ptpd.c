/*-
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file   ptpd.c
 * @date   Wed Jun 23 10:13:38 2010
 *
 * @brief  The main() function for the PTP daemon
 *
 * This file contains very little code, as should be obvious,
 * and only serves to tie together the rest of the daemon.
 * All of the default options are set here, but command line
 * arguments and configuration file is processed in the 
 * ptpdStartup() routine called
 * below.
 */

#include "ptpd.h"
#include "share.h"

extern void doState(RunTimeOpts*,PtpClock*);
void ptpHandleap(RunTimeOpts *rtOpts, PtpClock *ptpClock);
RunTimeOpts rtOpts;			/* statically allocated run-time configuration data */
PtpClock G_ptpClock;

extern FigStructData GlobalConfig;
extern err_t eth_send_llc( u16_t length);


 



int PTPd_Init(void)
{
	Integer16 ret;
	memset(&rtOpts,0,sizeof(RunTimeOpts)); 		//�ṹ��������0
	memset(&G_ptpClock, 0, sizeof(PtpClock));	//�ṹ��������0
	
	rtOpts.slaveOnly = (bool)GlobalConfig.SlaveorMaster;									//0:��ʱ�� 1:�� //ֻ���ڲ��3�ĵ�һ�ڶ�������
	rtOpts.clockQuality.clockAccuracy = 0x03;					  									//ʱ�Ӿ���0xFE
	rtOpts.clockQuality.clockClass 		= GlobalConfig.clockClass;					//ʱ�ӵȼ�248			ֻ���ӣ�255������0-127�������ɴӣ�128-254
	rtOpts.clockQuality.offsetScaledLogVariance = DEFAULT_CLOCK_VARIANCE;	//0xFFFF
	rtOpts.priority1    						= GlobalConfig.priority1;							// ���ȼ�1					= 128
	rtOpts.priority2   						  = GlobalConfig.priority1;							// ���ȼ�2					= ���ȼ�1
	
	rtOpts.announceInterval 				= 1;	 		// announce ����	���		= 0^2��s��
	rtOpts.syncInterval 						= GlobalConfig.SyncInterval; 					// sync			����	���		= 0^2��s��
	rtOpts.initial_delayreq 				= GlobalConfig.DelayInterval; 				// delayreq	��ʼ��	���		=	0^2��s��������Э�̺��޸ģ�
	rtOpts.logMinPdelayReqInterval 	= GlobalConfig.PDelayInterval; 				// pdelayreq��ǰ	���		=	0^2��s��
	
	rtOpts.announceReceiptTimeout  	= 6;		// announce	����	���		= 2^6��s��
	
	rtOpts.announceTimeoutGracePeriod  	= 0;															// announce���ճ�ʱ���޴��� =	0��
	
	rtOpts.ip_mode 	 = IPMODE_MULTICAST;																	//�鲥ģʽ	,Э���ǻ����鲥(�ಥ)��ƣ�����Ҳ����֧�ֵ���
	
	switch(GlobalConfig.WorkMode)
		{
			case 0: 
				  rtOpts.transport 			= IEEE_802_3;	//0:E2E-MAC
					rtOpts.delayMechanism = E2E; 				
					break;
			case 1:
					rtOpts.transport 			= IEEE_802_3;	//1:P2P-MAC
					rtOpts.delayMechanism = P2P; 
					break;
			case 2: 
					rtOpts.transport 			= UDP_IPV4;		//2:E2E-UDP
					rtOpts.delayMechanism = E2E; 
					break;
			case 3: 
					rtOpts.transport 			= UDP_IPV4;		//4:P2P-UDP
					rtOpts.delayMechanism = P2P; 
					break;
			default: 					
					rtOpts.transport			= IEEE_802_3;	//����:E2E-MAC
					rtOpts.delayMechanism = E2E;	
					break;
		}
	rtOpts.domainNumber												  = DEFAULT_DOMAIN_NUMBER;//ʱ����				=	0
	rtOpts.timeProperties.currentUtcOffset 			= GlobalConfig.Offset_UTC;	//UtcOffsetֵ	=	0
	rtOpts.timeProperties.currentUtcOffsetValid = TRUE;											//UtcOffset		=	��Ч
	if(GlobalConfig.Offset_UTC==0)
		rtOpts.timeProperties.currentUtcOffsetValid = FALSE;									//UtcOffset		=	��Ч
	
	rtOpts.timeProperties.timeSource 						= INTERNAL_OSCILLATOR;	//ʱ��Դ				= ����
	rtOpts.timeProperties.timeTraceable 				= FALSE;								//ʱ��ͬ��			= FALSE
	rtOpts.timeProperties.frequencyTraceable 		= FALSE;								//Ƶ������			= FALSE
	rtOpts.timeProperties.ptpTimescale 					= FALSE;								//PTP״̬			= FALSE
	rtOpts.unicastAddr												  = 0;										//������ַ			= 0
	
	rtOpts.servo.sDelay	  = 5;																	//sDelay								=	4
	rtOpts.servo.sOffset  = 1;																	//sOffset								=	1
	rtOpts.servo.ap 			= 3;																	//PI�����㷨-����ϵ��	P	=	2					
	rtOpts.servo.ai 			= 32;																	//PI�����㷨����ʱ�䳣��I	=	16	
	

	rtOpts.inboundLatency.nanoseconds  = DEFAULT_INBOUND_LATENCY;				//����ϵͳ�ı߽���ʱ 0
	rtOpts.outboundLatency.nanoseconds = DEFAULT_OUTBOUND_LATENCY;			//�뿪ϵͳʱ�߽���ʱ 0
	rtOpts.max_foreign_records 				 = DEFAULT_MAX_FOREIGN_RECORDS;		//���������ʱ��		 5
	
	 
	rtOpts.alwaysRespectUtcOffset			 =	FALSE;		// Ĭ�Ϲرգ���Э��涨��
	rtOpts.preferUtcValid							 =	FALSE;		// Ĭ�Ϲرգ���Э��涨��
	rtOpts.requireUtcValid						 =	FALSE;		// Ĭ�Ϲرգ���Э��涨��

	rtOpts.noAdjust 						= NO_ADJUST;  						//�Ƿ�ͨ��PTPͬ���Լ���ʱ��
	rtOpts.noResetClock  			  = DEFAULT_NO_RESET_CLOCK; //PTPͬ���Լ���ʱ��ʱ������ʱ��
	rtOpts.maxListen 						= 5;											//5������״̬���ں�����Э��ջ
	rtOpts.syncSequenceChecking = TRUE;										//sync���к�У��

	


	
	if ((ptpdStartup(&ret,&G_ptpClock ,&rtOpts)) != 1)  //����Э��ջ
	{
		if (ret != 0)
			ERROR(USER_DESCRIPTION" startup failed\n");
			return ret;
	}
	toState(PTP_INITIALIZING, &rtOpts, &G_ptpClock);		//Э��״̬ת��->��ʼ��״̬
	adjFreq(0);		//��ʼ��Ƶ��ֵ

	printf("slaveOnly=%d\n",rtOpts.slaveOnly);
	printf("currentUtcOffsetValid=%d\n",rtOpts.timeProperties.currentUtcOffsetValid);
	printf("currentUtcOffset=%d\n",rtOpts.timeProperties.currentUtcOffset);
	return 1;
}


/* loop forever. doState() has a switch for the actions and events to be
   checked for 'port_state'. the actions and events may or may not change
   'port_state' by calling toState(), but once they are done we loop around
   again and perform the actions required for the new 'port_state'. */
void ptpd_Periodic_Handle(__IO UInteger32 localtime)
{
		/* do the protocol engine */
    static UInteger32 old_localtime = 0;		//static�����ȫ�ֱ���

    catch_alarm(localtime - old_localtime);	//ȡһ��ʱ����
    old_localtime = localtime;

    /* at least once run stack */
		/* forever loop.. */
    do
    {
			 if (G_ptpClock.portState == PTP_INITIALIZING)
			 {
					WARNING("Debug Initializing...\n");
					if (!doInit(&rtOpts,&G_ptpClock)) 
					{//��ʼ��
						ERROR("Initializing was FAILED\n");
						return;
					}
				}
				else 
				{
					doState(&rtOpts,&G_ptpClock);//����Э��
				}
        /* if there are still some packets - run stack again */
    }
    while (netSelect(NULL,&(G_ptpClock.netPath)) > 0);//��������ݰ�����������ַ>0������һֱ����Э��ջ������
}

void ptpHandleap(RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
	struct ptptime_t timeupdata;
	TimeInternal lastseconds;
	
	if(ptpClock->timePropertiesDS.leap61 || ptpClock->timePropertiesDS.leap59)
	{
		ptpClock->leapSecondPending = TRUE;
		getTime(&lastseconds);
	}
	
	if(ptpClock->timePropertiesDS.leap61)//������
	{
		if(lastseconds.seconds%60 == 1)//
		{
			timeupdata.tv_sec = -1;
			timeupdata.tv_nsec = 0;
			ptpClock->leapSecondInProgress = TRUE;
			ETH_PTPTime_UpdateOffset(&timeupdata);
			ptpClock->leapSecondPending = FALSE;		
//			printf("leap61\n");
		}
	}
	else if(ptpClock->timePropertiesDS.leap59)//������
	{
		if(lastseconds.seconds%60 == 59)
		{
			timeupdata.tv_sec = 1;//
			timeupdata.tv_nsec = 0;
			ptpClock->leapSecondInProgress = TRUE;
			ETH_PTPTime_UpdateOffset(&timeupdata);
			ptpClock->leapSecondPending = FALSE;
//			printf("leap58\n");
		}
	}
}

char *dump_TimeInternal2(const char *st1, const TimeInternal * p1, const char *st2, const TimeInternal * p2)
{
	static char buf[BUF_SIZE];

	return buf;
}



