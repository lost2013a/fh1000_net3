/**
 * @file   protocol.c
 * @date   Wed Jun 23 09:40:39 2010
 * 
 * @brief  The code that handles the IEEE-1588 protocol and state machine
 * 
 * 
 */

//�޸ļ�¼ ��20161107  1.�޸�����ʱFollowUp���͵�ʱ���ΪЭ���׼(sys����ʱ)���������յ��Լ�SYS���ĵ�ʱ��


#include "ptpd.h"
#include "main.h"

Boolean doInit(RunTimeOpts*,PtpClock*);
void doState(RunTimeOpts*,PtpClock*);

void handle(RunTimeOpts*,PtpClock*);
static void handleAnnounce(MsgHeader*, ssize_t,Boolean, RunTimeOpts*,PtpClock*);
static void handleSync(const MsgHeader*, ssize_t,TimeInternal*,Boolean,RunTimeOpts*,PtpClock*);
static void handleFollowUp(const MsgHeader*, ssize_t,Boolean,RunTimeOpts*,PtpClock*);
static void handlePDelayReq(MsgHeader*, ssize_t,const TimeInternal*,Boolean,RunTimeOpts*,PtpClock*);
static void handleDelayReq(const MsgHeader*, ssize_t, const TimeInternal*,Boolean,RunTimeOpts*,PtpClock*);
static void handlePDelayResp(const MsgHeader*, TimeInternal* ,ssize_t,Boolean,RunTimeOpts*,PtpClock*);
static void handleDelayResp(const MsgHeader*, ssize_t, RunTimeOpts*,PtpClock*);
static void handlePDelayRespFollowUp(const MsgHeader*, ssize_t, Boolean, RunTimeOpts*,PtpClock*);
static void handleManagement(MsgHeader*, Boolean,RunTimeOpts*,PtpClock*);
static void handleSignaling(PtpClock*);
//static void updateDatasets(PtpClock* ptpClock, RunTimeOpts* rtOpts);

static void issueAnnounce(RunTimeOpts*,PtpClock*);
static void issueSync(RunTimeOpts*,PtpClock*);
static void issueFollowup(const TimeInternal*,RunTimeOpts*,PtpClock*, const UInteger16);
static void issuePDelayReq(RunTimeOpts*,PtpClock*);
static void issueDelayReq(RunTimeOpts*,PtpClock*);
static void issuePDelayResp(const TimeInternal*,MsgHeader*,RunTimeOpts*,PtpClock*);
static void issueDelayResp(const TimeInternal*,MsgHeader*,RunTimeOpts*,PtpClock*);
static void issuePDelayRespFollowUp(const TimeInternal*,MsgHeader*,RunTimeOpts*,PtpClock*, const UInteger16);
#if 0
static void issueManagement(MsgHeader*,MsgManagement*,RunTimeOpts*,PtpClock*);
#endif
static void issueManagementRespOrAck(MsgManagement*,RunTimeOpts*,PtpClock*);
static void issueManagementErrorStatus(MsgManagement*,RunTimeOpts*,PtpClock*);
static void processMessage(RunTimeOpts* rtOpts, PtpClock* ptpClock, TimeInternal* timeStamp, ssize_t length);
//static void processSyncFromSelf(const TimeInternal * tint, RunTimeOpts * rtOpts, PtpClock * ptpClock, const UInteger16 sequenceId);
static void processDelayReqFromSelf(const TimeInternal * tint, RunTimeOpts * rtOpts, PtpClock * ptpClock);
static void processPDelayReqFromSelf(const TimeInternal * tint, RunTimeOpts * rtOpts, PtpClock * ptpClock);
static void processPDelayRespFromSelf(const TimeInternal * tint, RunTimeOpts * rtOpts, PtpClock * ptpClock, const UInteger16 sequenceId);

void addForeign(Octet*,MsgHeader*,PtpClock*);
static int handle_Repeat(PtpClock *ptpClock);
extern err_t eth_send_ptp(char  *buf, u16_t length, Boolean P2P, TimeInternal *time);
int id[8] = {-1,-1,-1,-1,-1,-1,-1,-1};

/* loop forever. doState() has a switch for the actions and events to be
   checked for 'port_state'. the actions and events may or may not change
   'port_state' by calling toState(), but once they are done we loop around
   again and perform the actions required for the new 'port_state'. */
//void protocol(RunTimeOpts *rtOpts, PtpClock *ptpClock)
//{
//	DBG("event POWERUP\n");

//	//�״ν���Э����Э������ʼ��״̬
//	toState(PTP_INITIALIZING, rtOpts, ptpClock);

////	timerStart(STATUSFILE_UPDATE_TIMER,rtOpts->statusFileUpdateInterval,ptpClock->itimer);

//	DBG("Debug Initializing...\n"); //����ptpdЭ��ĳ�ʼ��

//	for (;;)
//	{
//		/* 20110701: this main loop was rewritten to be more clear */

//		if (ptpClock->portState == PTP_INITIALIZING) { //ptpd���ڳ�ʼ��״̬
//			if (!doInit(rtOpts, ptpClock)) {//ͨ��doInit��������һЩ�еĳ�ʼ��
//				return;
//			}
//		} else {
//			doState(rtOpts, ptpClock);
//		}

//		if (ptpClock->message_activity)
//			DBGV("activity\n");

//		/* Configuration has changed */
//		//ptpd���÷����ı�
//		//������������Э��
//	}
//}


/* perform actions required when leaving 'port_state' and entering 'state' */
void 
toState(UInteger8 state, RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
	ptpClock->message_activity = TRUE;		//msg�����Ч
	
	/* leaving state tasks */
	switch (ptpClock->portState)															//�˳�ԭ����ʱ��״̬	
	{
	case PTP_MASTER:
			initClock(rtOpts,ptpClock);															//��ʱ�ӣ�ʱ����صĲ�������0
			timerStop(SYNC_INTERVAL_TIMER, ptpClock->itimer);  			//�ص���ʱ�����3����ʹ�߼�ʱ����
			timerStop(ANNOUNCE_INTERVAL_TIMER, ptpClock->itimer);
			timerStop(PDELAYREQ_INTERVAL_TIMER, ptpClock->itimer); 
			break;
	case PTP_SLAVE:																						
			timerStop(ANNOUNCE_RECEIPT_TIMER, ptpClock->itimer);		//��ʱ�ӣ��ر�ANNOUNCE���ܳ�ʱ��ʱ��

			if (ptpClock->delayMechanism == E2E)
				timerStop(DELAYREQ_INTERVAL_TIMER, ptpClock->itimer);	//E2E���ر�DELAYREQ������ʱ��
			else if (ptpClock->delayMechanism == P2P)
				timerStop(PDELAYREQ_INTERVAL_TIMER, ptpClock->itimer);//P2P���ر�PDELAYREQ������ʱ��

			//���ptpdЭ����Ҫ�뿪��ģʽ		
			/* If statistics are enabled, drift should have been saved already - otherwise save it*/
			#ifdef HAVE_SYS_TIMEX_H
					/* save observed drift value, don't inform user ֻ�ܱ����ģʽ�µ�ƫ��*/
					saveDrift(ptpClock, rtOpts, TRUE); //��֪ͨ�û�����Ư��ֵ(δʹ��)
			#endif /* HAVE_SYS_TIMEX_H */
			initClock(rtOpts, ptpClock); 														//ʱ����صĲ�������0
			break;
	case PTP_PASSIVE:
			initClock(rtOpts, ptpClock);														//ʱ����صĲ�������0
			timerStop(PDELAYREQ_INTERVAL_TIMER, ptpClock->itimer);
			timerStop(ANNOUNCE_RECEIPT_TIMER, ptpClock->itimer);
			break;
	case PTP_LISTENING:
			timerStop(ANNOUNCE_RECEIPT_TIMER, ptpClock->itimer);
			if(state != PTP_LISTENING)
				{
					ptpClock->listenCount = 0;		/* we're leaving LISTENING - reset counter */
				}
			break;
	default:
			break;
	}
	
	/* entering state tasks */

	ptpClock->counters.stateTransitions++;								//״̬�л������ļ���		
	DBG("state %s\n",portState_getName(state));

	/*
	 * No need of PRE_MASTER state because of only ordinary clockimplementation.
	 *	����Ҫʹ��	PRE_MASTER ״̬
	 */
	
	switch (state)
	{
	case PTP_INITIALIZING:
		DBG("state PTP_INITIALIZING\n");
		ptpClock->portState = PTP_INITIALIZING;
		break;
		
	case PTP_FAULTY:
		 DBG("state PTP_FAULTY\n");
		ptpClock->portState = PTP_FAULTY;
		break;
		
	case PTP_DISABLED:
		 DBG("state PTP_DISABLED\n");
		ptpClock->portState = PTP_DISABLED;
		break;
		
	case PTP_LISTENING:
		DBG("state PTP_LISTENING\n");
		/* in Listening mode, make sure we don't send anything. Instead we just expect/wait for announces (started below) */
		timerStop(SYNC_INTERVAL_TIMER,      ptpClock->itimer);
		timerStop(ANNOUNCE_INTERVAL_TIMER,  ptpClock->itimer);
		timerStop(PDELAYREQ_INTERVAL_TIMER, ptpClock->itimer);
		timerStop(DELAYREQ_INTERVAL_TIMER,  ptpClock->itimer);

		/*
		 *  Count how many _unique_ timeouts happen to us.
		 *  If we were already in Listen mode, then do not count this as a seperate reset, but stil do a new IGMP refresh
		 */
		if (ptpClock->portState != PTP_LISTENING)
			{
				ptpClock->resetCount++;			//������״̬���������������reset
			} 
		else
			{
				ptpClock->listenCount++;
				if( ptpClock->listenCount >= rtOpts->maxListen ) 
					{
						WARNING("Still in LISTENING after %d restarts - will do a full network reset\n",rtOpts->maxListen);
						toState(PTP_FAULTY, rtOpts, ptpClock);		//����������һ���̶ȣ��������к���������PTP_FAULTY��������
						ptpClock->listenCount = 0;
						break;
					}
			}

		ptpClock->logMinDelayReqInterval = rtOpts->initial_delayreq;	//DelayReq���	=	��ʼֵ
		timerStart(ANNOUNCE_RECEIPT_TIMER,(ptpClock->announceReceiptTimeout)*(pow2ms(ptpClock->logAnnounceInterval)),ptpClock->itimer);//����announce���ռ����ʱ
		ptpClock->announceTimeouts = 0;

		/* 
		 * Log status update only once - condition must be checked before we write the new state,
		 * but the new state must be eritten before the log message...
		 */
		if (ptpClock->portState != state) 
			{
				ptpClock->portState = PTP_LISTENING;				//����Ǵ�����״̬ת������
				displayStatus(ptpClock, "Now in state: ");	//��ӡ������Ϣ
			} 
		else
			{
				ptpClock->portState = PTP_LISTENING;
			}
		break;

	case PTP_MASTER:
		DBG("state PTP_MASTER\n");
		timerStart(SYNC_INTERVAL_TIMER,pow2ms(ptpClock->logSyncInterval), ptpClock->itimer);				//SyncInterval����������
		DBG("SYNC INTERVAL TIMER : %d \n",pow2ms(ptpClock->logSyncInterval));
		timerStart(ANNOUNCE_INTERVAL_TIMER, pow2ms(ptpClock->logAnnounceInterval),ptpClock->itimer);//AnnounceInterval����������	
		if(ptpClock->delayMechanism == P2P)
			{
				timerStart(PDELAYREQ_INTERVAL_TIMER,pow2ms(ptpClock->logMinPdelayReqInterval),ptpClock->itimer);//��P2P��PdelayReqInterval����
			}
		ptpClock->portState = PTP_MASTER;
		displayStatus(ptpClock, "Now in state: ");
		break;

	case PTP_PASSIVE:
		timerStart(PDELAYREQ_INTERVAL_TIMER,pow2ms(ptpClock->logMinPdelayReqInterval),ptpClock->itimer);//PdelayReqInterval����
		timerStart(ANNOUNCE_RECEIPT_TIMER, (ptpClock->announceReceiptTimeout)*(pow2ms(ptpClock->logAnnounceInterval)),ptpClock->itimer);//AnnounceInterval����
		ptpClock->portState = PTP_PASSIVE;
		p1(ptpClock, rtOpts);//PASSIVEģʽ��Э������	����ʼֵ
		displayStatus(ptpClock, "Now in state: ");
		break;

	case PTP_UNCALIBRATED:
		ptpClock->portState = PTP_UNCALIBRATED;
		break;

	case PTP_SLAVE:
		initClock(rtOpts, ptpClock);							//���ʱ������

		ptpClock->waitingForFollow = FALSE;				//״̬���̳�ʼ��_��δ׼������Follow����
		ptpClock->waitingForDelayResp = FALSE;		//״̬���̳�ʼ��_��δ׼������DelayResp����

		// FIXME: clear these vars inside initclock
		clearTime(&ptpClock->delay_req_send_time);
		clearTime(&ptpClock->delay_req_receive_time);
		clearTime(&ptpClock->pdelay_req_send_time);
		clearTime(&ptpClock->pdelay_req_receive_time);
		clearTime(&ptpClock->pdelay_resp_send_time);
		clearTime(&ptpClock->pdelay_resp_receive_time);
		
		timerStart(ANNOUNCE_RECEIPT_TIMER,(ptpClock->announceReceiptTimeout) * 
			   (pow2ms(ptpClock->logAnnounceInterval)),ptpClock->itimer);
		
		/*
		 * Previously, this state transition would start the
		 * delayreq timer immediately.  However, if this was
		 * faster than the first received sync, then the servo
		 * would drop the delayResp Now, we only start the
		 * timer after we receive the first sync (in
		 * handle_sync())
		 */
		ptpClock->waiting_for_first_sync = TRUE;
		ptpClock->waiting_for_first_delayresp = TRUE;
		ptpClock->announceTimeouts = 0;
		ptpClock->portState = PTP_SLAVE;
		displayStatus(ptpClock, "Now in state: ");
		ptpClock->followUpGap = 0;

#ifdef HAVE_SYS_TIMEX_H

		/* leap second pending in kernel but no leap second 
		 * info from GM - withdraw kernel leap second
		 * if the flags have disappeared but we're past 
		 * leap second event, do nothing - kernel flags 
		 * will be unset in handleAnnounce()
		 */
		if((!ptpClock->timePropertiesDS.leap59 && !ptpClock->timePropertiesDS.leap61) &&
		    !ptpClock->leapSecondInProgress &&
		   (checkTimexFlags(STA_INS) || checkTimexFlags(STA_DEL))) {
			WARNING("Leap second pending in kernel but not on "
				"GM: aborting kernel leap second\n");
			unsetTimexFlags(STA_INS | STA_DEL, TRUE);
		}
#endif /* HAVE_SYS_TIMEX_H */
		break;
	default:
		DBG("to unrecognized state\n");
		break;
	}
}


Boolean doInit(RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
	DBG("manufacturerIdentity: %s\n", MANUFACTURER_ID);

	/* initialize networking */
	/*��ʼ������*/
	netShutdown(&ptpClock->netPath);//�ر���������

	//�������� ���׽��ֲ����г�ʼ��
	if (!netInit(&ptpClock->netPath, rtOpts, ptpClock)) {
		ERROR("Failed to initialize network\n");
		toState(PTP_FAULTY, rtOpts, ptpClock);
		return FALSE;
	}

	/* initialize other stuff */
	initData(rtOpts, ptpClock);
	initTimer();
	initClock(rtOpts, ptpClock);
		
	m1(rtOpts, ptpClock );
	msgPackHeader(ptpClock->msgObuf, ptpClock);
	
	toState(PTP_LISTENING, rtOpts, ptpClock);
	return TRUE;
}

/* handle actions and events for 'port_state' */
void doState(RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
	UInteger8 state;
	
	ptpClock->message_activity = FALSE;										//msg�����Ч
	
	/* Process record_update (BMC algorithm) before everything else */
	/* ��������BMC�㷨 */
	switch (ptpClock->portState)
	{
	case PTP_LISTENING:
	case PTP_PASSIVE:
	case PTP_SLAVE:
	case PTP_MASTER:
		/*State decision Event*/

		/* If we received a valid Announce message
 		 * and can use it (record_update),
		 * or we received a SET management message that
		 * changed an attribute in ptpClock,
		 * then run the BMC algorithm
		 */ 
		if(ptpClock->record_update)						//���յ�announce�����߹����ģ�������BMC�㷨
		{
			DBG2("event STATE_DECISION_EVENT\n");
			ptpClock->record_update = FALSE;
			state = bmc(ptpClock->foreign, rtOpts, ptpClock);	//�����㷨������л�״̬
			if(state != ptpClock->portState)									//��Ҫ�л�
				toState(state, rtOpts, ptpClock);							 	//�л�Э��	
		}
		break;
		
	default:
		break;
	}
	
	
	switch (ptpClock->portState)
	{
	case PTP_FAULTY:
			/* imaginary troubleshooting */
			DBG("event FAULT_CLEARED\n");
			toState(PTP_INITIALIZING, rtOpts, ptpClock);				//����״̬�����³�ʼ��PTPDЭ��ջ
			return;
		
	case PTP_LISTENING:
	case PTP_UNCALIBRATED:
	case PTP_SLAVE:
	// passive mode behaves like the SLAVE state, in order to wait for the announce timeout of the current active master
	case PTP_PASSIVE:																				//�������ӣ�����״̬ʱ
		
			handle(rtOpts, ptpClock);														//�����ģ���ʱ�����޴����ձ��ģ��ٴ������ģ�
	
								/*
								 * handle SLAVE timers:
								 *   - No Announce message was received
								 *   - Time to send new delayReq  (miss of delayResp is not monitored explicitelly)
								 */
			if (timerExpired(ANNOUNCE_RECEIPT_TIMER, ptpClock->itimer)) //announce���ճ�ʱ����
				{
					DBG("event ANNOUNCE_RECEIPT_TIMEOUT_EXPIRES\n");
					
					if(!ptpClock->slaveOnly && ptpClock->clockQuality.clockClass != SLAVE_ONLY_CLOCK_CLASS)	 //Э�������ʱ��
						{
							ptpClock->number_foreign_records = 0;							//ANNOUNCE�ı���������0	
							ptpClock->foreign_record_i = 0;
							m1(rtOpts,ptpClock);															//������ʱ����Ϊ����ʱ��
							toState(PTP_MASTER, rtOpts, ptpClock); 						//ptpd������ʱ��
						}
					else if(ptpClock->portState != PTP_LISTENING) 		  	//ֻ���ӣ�������״̬����PTP_LISTENING(�ڴ�ʱ��״̬)
						{	
							if(ptpClock->announceTimeouts < rtOpts->announceTimeoutGracePeriod) //��ʱ����С��announce��ʱ�޿����ִ�ʱ��״̬��
								{	
									if(ptpClock->grandmasterClockQuality.clockClass != 255 &&ptpClock->grandmasterPriority1 != 255 && ptpClock->grandmasterPriority2 != 255) 
										{
											ptpClock->grandmasterClockQuality.clockClass = 255;																	//����ʱ��δ����͵ȼ�ʱ�����ó����ʱ�ӵȼ�
											ptpClock->grandmasterPriority1 = 255;
											ptpClock->grandmasterPriority2 = 255;
											ptpClock->foreign[ptpClock->foreign_record_best].announce.grandmasterPriority1=255;	//ȡ����ǰ��GM��ʱ��
											ptpClock->foreign[ptpClock->foreign_record_best].announce.grandmasterPriority2=255;
											ptpClock->foreign[ptpClock->foreign_record_best].announce.grandmasterClockQuality.clockClass=255;
											WARNING("GM announce timeout, disqualified current best GM\n");
											// ����Ҫ����ptpd ,ֻ��Ҫȡ��(disqualify)��ǰ��GM��ʱ�� �������������ʱ�Ӵ���,����ѡ����������ʱ��,����(otherwise) ��ʱ����ѭ���������ǽ�������	
											//ptpClock->counters.announceTimeouts++;
										}
									 if (rtOpts->announceTimeoutGracePeriod > 0) 
										 {
												ptpClock->announceTimeouts++;															//announce��ʱ����
										 }
									 INFO("Waiting for new master, %d of %d attempts\n",ptpClock->announceTimeouts,rtOpts->announceTimeoutGracePeriod);
								} 
							else 
								{
									WARNING("No active masters present. Resetting port.\n");				//�����涨ʱ�ޣ��������״̬
									ptpClock->number_foreign_records = 0;
									ptpClock->foreign_record_i = 0;
									toState(PTP_LISTENING, rtOpts, ptpClock);
								}
						} 
					else
						{
							toState(PTP_LISTENING, rtOpts, ptpClock);	 //��������������״̬
						}
				}
			if (ptpClock->delayMechanism == E2E) 
				{ 
					if(timerExpired(DELAYREQ_INTERVAL_TIMER,ptpClock->itimer)) 	//E2Eģʽ��DELAYREQ����(���ڼ����������ǻظ�����)
					{
						DBG2("event DELAYREQ_INTERVAL_TIMEOUT_EXPIRES\n");
						issueDelayReq(rtOpts,ptpClock);
					}
				} 
			else if (ptpClock->delayMechanism == P2P) 											//P2Pģʽ��PDELAYREQ����
				{
					if (timerExpired(PDELAYREQ_INTERVAL_TIMER,ptpClock->itimer))
						{
							//DBGV("event PDELAYREQ_INTERVAL_TIMEOUT_EXPIRES\n");
							DBG("event PDELAYREQ_INTERVAL_TIMEOUT_EXPIRES\n");
							issuePDelayReq(rtOpts,ptpClock);
						}
						/* FIXME: Path delay should also rearm its timer with the value received from the Master */
				}

				//if (ptpClock->timePropertiesDS.leap59 || ptpClock->timePropertiesDS.leap61) 
				//			DBGV("seconds to midnight: %.3f\n",secondsToMidnight());
				//		/* leap second period is over */
				//		if(timerExpired(LEAP_SECOND_PAUSE_TIMER,ptpClock->itimer) && ptpClock->leapSecondInProgress) {
				//				/* 
				//				 * do not unpause offset calculation just
				//				 * yet, just indicate and it will be
				//				 * unpaused in handleAnnounce()
				//				*/
				//				ptpClock->leapSecondPending = FALSE;
				//				timerStop(LEAP_SECOND_PAUSE_TIMER,ptpClock->itimer);
				//		} 
				/* check if leap second is near and if we should pause updates */
			if( ptpClock->leapSecondPending && !ptpClock->leapSecondInProgress &&
					(secondsToMidnight() <= getPauseAfterMidnight(ptpClock->logAnnounceInterval)))
				{
					WARNING("Leap second event imminent - pausing ""clock and offset updates\n");
					ptpClock->leapSecondInProgress = TRUE;
					#ifdef HAVE_SYS_TIMEX_H
						if(!checkTimexFlags(ptpClock->timePropertiesDS.leap61 ? STA_INS : STA_DEL))
						{
							WARNING("Kernel leap second flags have been unset - attempting to set again");
							setTimexFlags(ptpClock->timePropertiesDS.leap61 ? STA_INS : STA_DEL, FALSE);
						}
					#endif /* HAVE_SYS_TIMEX_H */
										/*
										 * start pause timer from now until [pause] after
										 * midnight, plus an extra second if inserting
										 * a leap second
										 */
				}
			break;
				
				

		/*
		 * handle SLAVE timers:
		 *   - Time to send new Announce
		 *   - Time to send new PathDelay
		 *   - Time to send new Sync (last order - so that follow-up always follows sync
		 *     in two-step mode: improves interoperability
		 *      (DelayResp has no timer - as these are sent and retransmitted by the slaves)
		 */
		
	case PTP_MASTER:	
			if (timerExpired(ANNOUNCE_INTERVAL_TIMER, ptpClock->itimer)) 
				{
					DBGV("event ANNOUNCE_INTERVAL_TIMEOUT_EXPIRES\n");
					issueAnnounce(rtOpts, ptpClock);
				}	
			if (timerExpired(SYNC_INTERVAL_TIMER, ptpClock->itimer))
				{
					DBGV("event SYNC_INTERVAL_TIMEOUT_EXPIRES\n");
					issueSync(rtOpts, ptpClock);
				}
				//		if (ptpClock->delayMechanism == P2P) {
				//			if (timerExpired(PDELAYREQ_INTERVAL_TIMER,ptpClock->itimer)) {
				//				DBGV("event PDELAYREQ_INTERVAL_TIMEOUT_EXPIRES\n");
				//				issuePDelayReq(rtOpts,ptpClock);
				//			}
				//		}
				
			handle(rtOpts, ptpClock);	//�����յ�����Ϣ����ʱ�����ȴ������ģ��ٴ����գ�
				
			if (ptpClock->slaveOnly || ptpClock->clockQuality.clockClass == SLAVE_ONLY_CLOCK_CLASS)		
				{
					toState(PTP_LISTENING, rtOpts, ptpClock);	//���ʱ��״̬�Ǵ�ģʽ��ô�������Ͻ��м���
				}
			break;
	case PTP_DISABLED:
			handle(rtOpts, ptpClock);
			break;
		
	default:
			DBG("(doState) do unrecognized state\n");
			break;
	}
}

static Boolean
isFromCurrentParent(const PtpClock *ptpClock, const MsgHeader* header)
{
	return (Boolean)(!memcmp(ptpClock->parentPortIdentity.clockIdentity,
		header->sourcePortIdentity.clockIdentity,CLOCK_IDENTITY_LENGTH)	&& 
		(ptpClock->parentPortIdentity.portNumber == header->sourcePortIdentity.portNumber));
}

void
processMessage(RunTimeOpts* rtOpts, PtpClock* ptpClock, TimeInternal* timeStamp, ssize_t length)
{
#ifdef PTPD_DBG //godin
		char *st;
#endif //godin
    Boolean isFromSelf;
    /*
     * make sure we use the TAI to UTC offset specified, if the
     * master is sending the UTC_VALID bit
     *
     * On the slave, all timestamps that we handle here have been
     * collected by our local clock (loopback+kernel-level
     * timestamp) This includes delayReq just send, and delayResp,
     * when it arrives.
     *
     * these are then adjusted to the same timebase of the Master
     * (+35 leap seconds, as of July 2012)
     *
     * wowczarek: added compatibility flag to always respect the
     * announced UTC offset, preventing clock jumps with some GMs
     */
    //DBGV("__UTC_offset: %d %d \n", ptpClock->timePropertiesDS.currentUtcOffsetValid, ptpClock->timePropertiesDS.currentUtcOffset);
    if (respectUtcOffset(rtOpts, ptpClock) == TRUE) //����UtcOffset������У�
			{			
					timeStamp->seconds += ptpClock->timePropertiesDS.currentUtcOffset;
			}

    ptpClock->message_activity = TRUE;		//msg�����Ч
    if (length < HEADER_LENGTH) 
			{
				//DBG("Error: message shorter than header length\n");
				//ptpClock->counters.messageFormatErrors++;
				return;
			}
			
    msgUnpackHeader(ptpClock->msgIbuf, &ptpClock->msgTmpHeader);							//�������ͷ��ȡ��ptpClock�б���ͷ���ֵ�����

    if (ptpClock->msgTmpHeader.versionPTP != ptpClock->versionNumber) 
			{
				//DBG("ignore version %d message\n", ptpClock->msgTmpHeader.versionPTP);
				//ptpClock->counters.discardedMessages++;
				//ptpClock->counters.versionMismatchErrors++;
				return;
			}

    if(ptpClock->msgTmpHeader.domainNumber != ptpClock->domainNumber)
			{
				//DBG("ignore message from domainNumber %d\n", ptpClock->msgTmpHeader.domainNumber);
				//ptpClock->counters.discardedMessages++;
				//ptpClock->counters.domainMismatchErrors++;
				return;
			}

    /*Spec 9.5.2.2*/
		/*�Ƚ�msg��portNumber�ǲ����Լ��ģ��Ƚ�clockIdentity�Ƿ�һ�£��Ӷ��ж��Ƿ����Լ�������*/
    isFromSelf = (Boolean)(ptpClock->portIdentity.portNumber == ptpClock->msgTmpHeader.sourcePortIdentity.portNumber \
	      && !memcmp(ptpClock->msgTmpHeader.sourcePortIdentity.clockIdentity, ptpClock->portIdentity.clockIdentity, CLOCK_IDENTITY_LENGTH));
		
    /*
     * subtract the inbound latency adjustment if it is not a loop
     *  back and the time stamp seems reasonable 
     */
    if (!isFromSelf && timeStamp->seconds > 0)
				subTime(timeStamp, timeStamp, &rtOpts->inboundLatency);	//��Чmsg��timeStamp��ȥ��ϵͳ���ӳٲ���
		
#ifdef PTPD_DBG //godin
    /* easy display of received messages */
    switch(ptpClock->msgTmpHeader.messageType)
    {
    case ANNOUNCE:
	st = "Announce";
	break;
    case SYNC:
	st = "Sync";
	break;
    case FOLLOW_UP:
	st = "FollowUp";
	break;
    case DELAY_REQ:
	st = "DelayReq";
	break;
    case DELAY_RESP:
	st = "DelayResp";
	break;
    case PDELAY_REQ:
	st = "PDelayReq";
	break;
    case PDELAY_RESP:
	st = "PDelayResp";
	break;
    case PDELAY_RESP_FOLLOW_UP:
	st = "PDelayRespFollowUp";
	break;	
    case MANAGEMENT:
	st = "Management";
	break;
    default:
	st = "Unk";
	break;
    }
    DBG("      ==> %s received, sequence %d\n", st, ptpClock->msgTmpHeader.sequenceId);
		
#endif  //godin

   // printf("      ==> %s received, sequence %d\n", st, ptpClock->msgTmpHeader.sequenceId);//godin
  //  printf("offset nanoseconds=\n");//godin
    /*
     *  on the table below, note that only the event messsages are passed the local time,
     *  (collected by us by loopback+kernel TS, and adjusted with UTC seconds
     *
     *  (SYNC / DELAY_REQ / PDELAY_REQ / PDELAY_RESP)
     */
    switch(ptpClock->msgTmpHeader.messageType)			//������ͷ���ͷ��ദ��msg
    {
    case ANNOUNCE:
	handleAnnounce(&ptpClock->msgTmpHeader,length,isFromSelf,rtOpts,ptpClock);
	break;
    case SYNC:
	handleSync(&ptpClock->msgTmpHeader,length, timeStamp, isFromSelf, rtOpts, ptpClock);
	break;
    case FOLLOW_UP:
	handleFollowUp(&ptpClock->msgTmpHeader,length,isFromSelf,rtOpts,ptpClock);
	break;
    case DELAY_REQ:
	handleDelayReq(&ptpClock->msgTmpHeader,length,timeStamp,isFromSelf,rtOpts,ptpClock);
	break;
    case PDELAY_REQ:
	handlePDelayReq(&ptpClock->msgTmpHeader,length,timeStamp,isFromSelf,rtOpts,ptpClock);
	break;  
    case DELAY_RESP:
	handleDelayResp(&ptpClock->msgTmpHeader,length,rtOpts,ptpClock);
	break;
    case PDELAY_RESP:
	handlePDelayResp(&ptpClock->msgTmpHeader,timeStamp,length,isFromSelf,rtOpts,ptpClock);
	break;
    case PDELAY_RESP_FOLLOW_UP:
	handlePDelayRespFollowUp(&ptpClock->msgTmpHeader,length,isFromSelf,rtOpts,ptpClock);
	break;
    case MANAGEMENT:
	handleManagement(&ptpClock->msgTmpHeader,isFromSelf, rtOpts, ptpClock);
	break;
    case SIGNALING:
	handleSignaling(ptpClock);
	break;
    default:
	DBG("handle: unrecognized message\n");
//	ptpClock->counters.discardedMessages++;
//	ptpClock->counters.unknownMessages++;
	break;
    }

  if (rtOpts->displayPackets)
		msgDump(ptpClock);
}


/* check and handle received messages */
void
handle(RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
  int ret;
	int length;
  TimeInternal timeStamp = { 0, 0 };

	
 //printf("      ==>portState is  %d received godin\n", ptpClock->portState);//godin
 //printf("      ==>message_activity is  %d received godin\n", ptpClock->message_activity);//godin
	if (FALSE == ptpClock->message_activity) 				//msg�����Ч�����������·����������
	{ 
		ret = netSelect(NULL, &ptpClock->netPath);		//���������Ϣ�յ���netQCheck��netQCheck����һ���β�������
		if (ret < 0) {
				PERROR("failed to poll sockets");
				//ptpClock->counters.messageRecvErrors++;
				toState(PTP_FAULTY, rtOpts, ptpClock);
				return;
		} else if (!ret) {													//ret==0
				/* DBGV("handle: nothing\n"); */
				return;
		}
		/* else length > 0 */
  }
  DBGV("handle: something\n");
 //printf("      ==>length is  %d received godin\n", length);//godin
	length = netRecvEvent(&ptpClock->netPath,ptpClock->msgIbuf, &timeStamp);			//����ȡ�¼����ĵ�msgIbuf������ʱMAC������ʱ���������time��
	if (length < 0) {
			PERROR("failed to receive on the event socket");
			toState(PTP_FAULTY, rtOpts, ptpClock);
			return;
	}
	else if(length == 0){
		length = netRecvGeneral(&ptpClock->netPath,ptpClock->msgIbuf, &timeStamp);	//�ٳ���ȡͨ�ñ��ĵ�msgIbuf������ʱMAC������ʱ���������time��
		if(length < 0){
				ERROR("handle: failed to receive on the general socket\n");
				toState(PTP_FAULTY, rtOpts, ptpClock);
				return;
		}else if(length == 0) return;
	}
	//printf("      ==>length1 is  %d received godin\n", length);//godin
	ptpClock->message_activity = TRUE;																					//msg�����Ч
	processMessage(rtOpts, ptpClock, &timeStamp, length);												//����msg
}

/*spec 9.5.3*/
static void 
handleAnnounce(MsgHeader *header, ssize_t length, 
	       Boolean isFromSelf, RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
	DBGV("HandleAnnounce : Announce message received : \n");
  
	if(length < ANNOUNCE_LENGTH) {
		DBG("Error: Announce message too short\n");
		ptpClock->counters.messageFormatErrors++;
		return;
	}

	DBGV("  >> HandleAnnounce : %d  \n", ptpClock->portState);

	switch (ptpClock->portState) {
	case PTP_INITIALIZING:
	case PTP_FAULTY:
	case PTP_DISABLED:
		DBG("Handleannounce : disregard\n");
		ptpClock->counters.discardedMessages++;
		return;
		
	case PTP_UNCALIBRATED:	
	case PTP_SLAVE:
		if (isFromSelf) {
			DBGV("HandleAnnounce : Ignore message from self \n");
			return;
		}
		if(rtOpts->requireUtcValid && !IS_SET(header->flagField1, UTCV)) {
			ptpClock->counters.ignoredAnnounce++;
			return;
		}
		
		/*
		 * Valid announce message is received : BMC algorithm
		 * will be executed 
		 */
		ptpClock->record_update = TRUE;

		switch (isFromCurrentParent(ptpClock, header)) {//Announce���Լ���ʱ�ӷ�����(������ʱ�����ݼ�)
		case TRUE: 
	   	msgUnpackAnnounce(ptpClock->msgIbuf,&ptpClock->msgTmp.announce);
			/* update datasets (file bmc.c) */
	    s1(header,&ptpClock->msgTmp.announce,ptpClock, rtOpts);
			/* update current master in the fmr as well */
			memcpy(&ptpClock->foreign[ptpClock->foreign_record_best].header,header,sizeof(MsgHeader));
			memcpy(&ptpClock->foreign[ptpClock->foreign_record_best].announce,&ptpClock->msgTmp.announce,sizeof(MsgAnnounce));
			
			if(ptpClock->leapSecondInProgress == TRUE) { //���ڴ�������
				if (!ptpClock->leapSecondPending) {
					WARNING("Leap second event over - resuming clock and offset updates\n");
					ptpClock->leapSecondInProgress=FALSE;
					ptpClock->timePropertiesDS.leap59 = FALSE;
					ptpClock->timePropertiesDS.leap61 = FALSE;
				}
			}
			
			DBG2("___ Announce: received Announce from current Master, so reset the Announce timer\n");
	   	/*Reset Timer handling Announce receipt timeout*/
	    timerStart(ANNOUNCE_RECEIPT_TIMER,(ptpClock->announceReceiptTimeout)*(pow2ms(ptpClock->logAnnounceInterval)),ptpClock->itimer);

			if (rtOpts->announceTimeoutGracePeriod && ptpClock->announceTimeouts > 0) {
					NOTICE("Received Announce message from master - cancelling timeout\n");
					ptpClock->announceTimeouts = 0;
			}

			// remember IP address of our master for hybrid mode
			// todo: add this to bmc(), to cover the very first packet
//			ptpClock->masterAddr = ptpClock->netPath.lastRecvAddr;
			break;

		case FALSE: //Announce���������Լ�����ʱ��˵����������ʱ�Ӵ���
			/* addForeign takes care of AnnounceUnpacking */

			/* the actual decision to change masters is
			 * only done in doState() / record_update ==
			 * TRUE / bmc()
			 */

			/*
			 * wowczarek: do not restart timer here:
			 * the slave will  sit idle if current parent
			 * is not announcing, but another GM is
			 */
			addForeign(ptpClock->msgIbuf,header,ptpClock); //����������ʱ�����ݼ�
			break;

		default:
			DBG("HandleAnnounce : (isFromCurrentParent) strange value ! \n");
	   		return;

		} /* switch on (isFromCurrentParrent) */
		break;

	/*
	 * Passive case: previously, this was handled in the default, just like the master case.
	 * This the announce would call addForeign(), but NOT reset the timer, so after 12s it would expire and we would come alive periodically 
	 *
	 * This code is now merged with the slave case to reset the timer, and call addForeign() if it's a third master
	 *
	 */
	case PTP_PASSIVE:
		if (isFromSelf) {
			DBGV("HandleAnnounce : Ignore message from self \n");
			return;
		}
		if(rtOpts->requireUtcValid && !IS_SET(header->flagField1, UTCV)) {
			ptpClock->counters.ignoredAnnounce++;
			return;
		}
		/*
		 * Valid announce message is received : BMC algorithm
		 * will be executed
		 */
		ptpClock->counters.announceMessagesReceived++;
		ptpClock->record_update = TRUE;

		if (isFromCurrentParent(ptpClock, header)) {
			msgUnpackAnnounce(ptpClock->msgIbuf,&ptpClock->msgTmp.announce);

			/* TODO: not in spec
			 * datasets should not be updated by another master
			 * this is the reason why we are PASSIVE and not SLAVE
			 * this should be p1(ptpClock, rtOpts);
			 */
			/* update datasets (file bmc.c) */
			s1(header,&ptpClock->msgTmp.announce,ptpClock, rtOpts);

			DBG("___ Announce: received Announce from current Master, so reset the Announce timer\n\n");
			/*Reset Timer handling Announce receipt timeout*/
			timerStart(ANNOUNCE_RECEIPT_TIMER,(ptpClock->announceReceiptTimeout)*(pow2ms(ptpClock->logAnnounceInterval)),ptpClock->itimer);
		} else {
			/*addForeign takes care of AnnounceUnpacking*/
			/* the actual decision to change masters is only done in  doState() / record_update == TRUE / bmc() */
			/* the original code always called: addforeign(new master) + timerstart(announce) */

			DBG("___ Announce: received Announce from another master, will add to the list, as it might be better\n\n");
			DBGV("this is to be decided immediatly by bmc())\n\n");
			addForeign(ptpClock->msgIbuf,header,ptpClock);
		}
		break;

		
	case PTP_MASTER:
	case PTP_LISTENING:  /* listening mode still causes timeouts in order to send IGMP refreshes */
	default :
		if (isFromSelf) {
			DBGV("HandleAnnounce : Ignore message from self \n");
			return;
		}
		if(rtOpts->requireUtcValid && !IS_SET(header->flagField1, UTCV)) {
			ptpClock->counters.ignoredAnnounce++;
			return;
		}
		ptpClock->counters.announceMessagesReceived++;
		DBGV("Announce message from another foreign master\n");
		addForeign(ptpClock->msgIbuf,header,ptpClock);
		ptpClock->record_update = TRUE;    /* run BMC() as soon as possible */
		break;

	} /* switch on (port_state) */
}


static void 
handleSync(const MsgHeader *header, ssize_t length, 
	   TimeInternal *tint, Boolean isFromSelf,RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
	TimeInternal OriginTimestamp;
	TimeInternal correctionField;

	DBGV("Sync message received : \n");

	if (length < SYNC_LENGTH) {
		DBG("Error: Sync message too short\n");
		ptpClock->counters.messageFormatErrors++;
		return;
	}

	switch (ptpClock->portState) {
	case PTP_INITIALIZING:
	case PTP_FAULTY:
	case PTP_DISABLED:
			DBGV("HandleSync : disregard\n");
			ptpClock->counters.discardedMessages++;
			return;
	
	case PTP_UNCALIBRATED:
	case PTP_SLAVE:
		if (isFromSelf) 
			{
				DBGV("HandleSync: Ignore message from self \n");
				return;
			}
		if (isFromCurrentParent(ptpClock, header)) 	//�жϱ���ͷ���������ʱ��
			{
				if (ptpClock->waiting_for_first_sync) 		//��һ���յ�ͬ������
					{
						ptpClock->waiting_for_first_sync = FALSE;
						//NOTICE("Received first Sync from Master\n");
						if (ptpClock->delayMechanism == E2E)
							{
								timerStart(DELAYREQ_INTERVAL_TIMER,pow2ms(ptpClock->logMinDelayReqInterval),ptpClock->itimer);	//��DelayReq��ʱ��
							}
						else if (ptpClock->delayMechanism == P2P)
							{
								timerStart(PDELAYREQ_INTERVAL_TIMER,pow2ms(ptpClock->logMinPdelayReqInterval), ptpClock->itimer);
							}
					} 
				
				else if ( rtOpts->syncSequenceChecking && 
							( (((UInteger16)(ptpClock->recvSyncSequenceId + 32768)) > (header->sequenceId + 32767)) ||
							(((UInteger16)(ptpClock->recvSyncSequenceId + 1)) > (header->sequenceId)) )  )  //��鷢�͵����
					{
							DBG("HandleSync : sequence mismatch - received: %d, expected %d or greater\n",
									header->sequenceId,(UInteger16)(ptpClock->recvSyncSequenceId + 1));
							ptpClock->counters.discardedMessages++;
							ptpClock->counters.sequenceMismatchErrors++;
							break;
					}
				//������Ϣ���(����Э��master��slave�Լ��ķ��ͼ��)
				ptpClock->logSyncInterval = header->logMessageInterval; 
				ptpClock->sync_receive_time.seconds = tint->seconds;
				ptpClock->sync_receive_time.nanoseconds = tint->nanoseconds;
				
	//			printf("Sync recvTime %ds.%dns\n",tint->seconds,tint->nanoseconds);
				
				if ((header->flagField0 & PTP_TWO_STEP) == PTP_TWO_STEP)
					{
						//DBG2("HandleSync: waiting for follow-up \n");
						ptpClock->twoStepFlag=TRUE;
						if(ptpClock->waitingForFollow) //����Ѿ���ǵȴ�followUp
						{
							ptpClock->followUpGap++;		//����followUpGap
							if(ptpClock->followUpGap < MAX_FOLLOWUP_GAP) //<3������
							{ 
								//DBG("Received Sync while waiting for follow-up - will wait for another %d messages\n",
								//MAX_FOLLOWUP_GAP - ptpClock->followUpGap);
								break;		    
							}
							//DBG("No follow-up for %d sync - waiting for new follow-up\n",ptpClock->followUpGap);
						}
						
						ptpClock->waitingForFollow = TRUE;//Э������_��ǵȴ�followUp
						/*Save correctionField of Sync message*/
						scaledNanosecondsToInternalTime(&header->correctionField,&correctionField);	//��¼��ǰsync�еĽ�����
						ptpClock->lastSyncCorrectionField.seconds = correctionField.seconds;	
						ptpClock->lastSyncCorrectionField.nanoseconds = correctionField.nanoseconds;
						
						ptpClock->recvSyncSequenceId = header->sequenceId;
						//break;
					} 
				else //һ����
					{
						ptpClock->recvSyncSequenceId = header->sequenceId;
						msgUnpackSync(ptpClock->msgIbuf,&ptpClock->msgTmp.sync);
						scaledNanosecondsToInternalTime(&ptpClock->msgTmpHeader.correctionField,&correctionField);
						timeInternal_display(&correctionField);
						ptpClock->waitingForFollow = FALSE;
						toInternalTime(&OriginTimestamp,&ptpClock->msgTmp.sync.originTimestamp);
						updateOffset(&OriginTimestamp,&ptpClock->sync_receive_time,&ptpClock->ofm_filt,rtOpts,ptpClock,&correctionField);
						#ifndef FH1100_Slave
							updateClock(rtOpts,ptpClock);	//��������ʱ�䣨/*�����ǲ���ʱ*/��
						#endif
						ptpClock->twoStepFlag=FALSE;
						//break;
					}
			} 	
		else	//����������ʱ�ӣ�����
			{
				DBG("HandleSync: Sync message received from another Master not our own \n");
				ptpClock->counters.discardedMessages++;
			}
		break;
	case PTP_MASTER:
	default :
		if (!isFromSelf) 
			{
				DBGV("HandleSync: Sync message received from another Master  \n");
				/* we are the master, but another is sending */
				ptpClock->counters.discardedMessages++;
				break;
			} 
		//		if (ptpClock->twoStepFlag) 
		//			{ //�Լ�������sync���ܱ��Լ��յ�
		//				DBGV("HandleSync: going to send followup message\n");
		//				processSyncFromSelf(tint, rtOpts, ptpClock, header->sequenceId);
		//				break;
		//			}
		//		else 
		//			{
		//				DBGV("HandleSync: Sync message received from self\n");
		//			}
	}
}

//static void processSyncFromSelf(const TimeInternal * tint, RunTimeOpts * rtOpts, PtpClock * ptpClock, const UInteger16 sequenceId)
//{
//	TimeInternal timestamp;
//	/*Add latency*/
//	addTime(&timestamp, tint, &rtOpts->outboundLatency); //���ϱ߽�ʱ�䣨��ǰ�߽�ʱ����0ֵ��
//	/* Issue follow-up CORRESPONDING TO THIS SYNC */
//	issueFollowup(&timestamp, rtOpts, ptpClock, sequenceId);
//}


static void 
handleFollowUp(const MsgHeader *header, ssize_t length, 
	       Boolean isFromSelf, RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
	TimeInternal preciseOriginTimestamp;
	TimeInternal correctionField;
	//DBGV("Handlefollowup : Follow up message received \n");

	if (length < FOLLOW_UP_LENGTH)	//̫�̣�����
	{
		DBG("Error: Follow up message too short\n");
		ptpClock->counters.messageFormatErrors++;
		return;
	}

	if (isFromSelf)					//�Լ����ģ�����
	{
		DBGV("Handlefollowup : Ignore message from self \n");
		return;
	}

	switch (ptpClock->portState)
	{
		case PTP_INITIALIZING:
		case PTP_FAULTY:
		case PTP_DISABLED:
		case PTP_LISTENING:
				DBGV("Handfollowup : disregard\n");
				ptpClock->counters.discardedMessages++;
				return;		
		case PTP_UNCALIBRATED:	
		case PTP_SLAVE:
			if (isFromCurrentParent(ptpClock, header))		//���������ʱ�ӵı���
				{
					ptpClock->counters.followUpMessagesReceived++;
					ptpClock->logSyncInterval = header->logMessageInterval;
					if (ptpClock->waitingForFollow)	//Э������_׼������Follow����
						{
							if (ptpClock->recvSyncSequenceId == header->sequenceId) 	//sequence���
								{
									ptpClock->followUpGap = 0;																				//followUp�ļ��=0
									msgUnpackFollowUp(ptpClock->msgIbuf,&ptpClock->msgTmp.follow);		//���FollowUp
									ptpClock->waitingForFollow = FALSE;																//Э������_ȡ����ǵȴ�Follow����
									toInternalTime(&preciseOriginTimestamp,&ptpClock->msgTmp.follow.preciseOriginTimestamp);		//����ʱ���	ת����		��׼ʱ��
									scaledNanosecondsToInternalTime(&ptpClock->msgTmpHeader.correctionField,&correctionField);	//������ʱ��	ת����		��׼ʱ��
									
									addTime(&correctionField,&correctionField,&ptpClock->lastSyncCorrectionField);							//������ʱ��	+=	�ϴε�Sync������ʱ��
									//printf("Sync sendTime %ds.%dns\n",preciseOriginTimestamp.seconds,preciseOriginTimestamp.nanoseconds);
									/*
									send_time = preciseOriginTimestamp (received inside followup)
									recv_time = sync_receive_time (received as CMSG in handleEvent)
									*/
									updateOffset(&preciseOriginTimestamp,&ptpClock->sync_receive_time,&ptpClock->ofm_filt,rtOpts,ptpClock,&correctionField);	//����ʱ��ƫ������
									
									#ifndef FH1100_Slave
										updateClock(rtOpts,ptpClock);	//��������ʱ�䣨/*�����ǲ���ʱ*/��
									#endif
									//break;���breakӦ����
								} 
							else 
								{	
									DBG("HandleFollowUp : sequence mismatch - last Sync: %d, this FollowUp: %d\n",	//sequence�����������
									ptpClock->recvSyncSequenceId,header->sequenceId);
									ptpClock->counters.sequenceMismatchErrors++;
									ptpClock->counters.discardedMessages++;
								}
						} 
					else 	//Э������_������Follow����
						{
							DBG2("Ignored followup, Slave was not waiting a follow up message \n");
							ptpClock->counters.discardedMessages++;
						}
				}	
			else 		//����������ʱ�ӵı���
				{
					DBG2("Ignored, Follow up message is not from current parent \n");
					ptpClock->counters.discardedMessages++;
				}
			break;//�޸ļ�¼;20160929���δ�����һ��	break;
	case PTP_MASTER:
	case PTP_PASSIVE:
			if(isFromCurrentParent(ptpClock, header))
			{
				DBGV("Ignored, Follow up message received from current master \n");
			}
			else {
			/* follow-ups and syncs are expected to be seen from parent, but not from others */
				DBGV("Follow up message received from foreign master!\n");
				//ptpClock->counters.discardedMessages++;
			}
			break;
	default:
    		DBG("do unrecognized state1\n");
    		//ptpClock->counters.discardedMessages++;
    		break;
	} /* Switch on (port_state) */
}

void
handleDelayReq(const MsgHeader *header, ssize_t length, 
	       const TimeInternal *tint, Boolean isFromSelf,RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
	if (ptpClock->delayMechanism == E2E) 
		{
			DBGV("delayReq message received : \n");
			if (length < DELAY_REQ_LENGTH) 
				{
					DBG("Error: DelayReq message too short\n");
					//ptpClock->counters.messageFormatErrors++;
					return;
				}
			switch (ptpClock->portState)
			{
				case PTP_INITIALIZING:
				case PTP_FAULTY:
				case PTP_DISABLED:
				case PTP_UNCALIBRATED:
				case PTP_LISTENING:
				case PTP_PASSIVE:
						DBGV("HandledelayReq : disregard\n");
						ptpClock->counters.discardedMessages++;
						return;

				case PTP_SLAVE:
						if (isFromSelf)	
							{
								DBG("==> Handle DelayReq (%d)\n",header->sequenceId);
								if ( ((UInteger16)(header->sequenceId + 1)) != ptpClock->sentDelayReqSequenceId) 
									{
										DBG("HandledelayReq : sequence mismatch - last DelayReq sent: %d, received: %d\n",ptpClock->sentDelayReqSequenceId,header->sequenceId);
										ptpClock->counters.discardedMessages++;
										ptpClock->counters.sequenceMismatchErrors++;
										break;
									}

								/*
								 * Get sending timestamp from IP stack
								 * with SO_TIMESTAMP
								 */

								/*
								 *  Make sure we process the REQ
								 *  _before_ the RESP. While we could
								 *  do this by any order, (because
								 *  it's implicitly indexed by
								 *  (ptpClock->sentDelayReqSequenceId
								 *  - 1), this is now made explicit
								 */

								processDelayReqFromSelf(tint, rtOpts, ptpClock);

								break;
							}
						else
							{
								DBG2("HandledelayReq : disregard delayreq from other client\n");
								//ptpClock->counters.discardedMessages++;
							}
						break;

				case PTP_MASTER:
						msgUnpackHeader(ptpClock->msgIbuf,&ptpClock->delayReqHeader);
						ptpClock->counters.delayReqMessagesReceived++;

						// remember IP address of this client for hybrid mode
						//ptpClock->LastSlaveAddr = ptpClock->netPath.lastRecvAddr;

						issueDelayResp(tint,&ptpClock->delayReqHeader,rtOpts,ptpClock);	//����DelayResp����
						break;

				default:
						DBG("do unrecognized state2\n");
						//ptpClock->counters.discardedMessages++;
						break;
			}
		}
		else if (ptpClock->delayMechanism == P2P)
			{	
				/* (Peer to Peer mode) */
				DBG("Delay messages are ignored in Peer to Peer mode\n");
				//ptpClock->counters.discardedMessages++;
				//ptpClock->counters.delayModeMismatchErrors++;
				/* no delay mechanism */
			}
		else 
			{
				DBG("DelayReq ignored - we are in DELAY_DISABLED mode");
				//ptpClock->counters.discardedMessages++;
			}
}


static void
processDelayReqFromSelf(const TimeInternal * tint, RunTimeOpts * rtOpts, PtpClock * ptpClock) {
	ptpClock->waitingForDelayResp = TRUE;		//��״̬����_�ɴ���DelayResp

	ptpClock->delay_req_send_time.seconds = tint->seconds;
	ptpClock->delay_req_send_time.nanoseconds = tint->nanoseconds;

	/*Add latency*/
	addTime(&ptpClock->delay_req_send_time,&ptpClock->delay_req_send_time,&rtOpts->outboundLatency);
//	printf("delay_Req sendTime %ds.%dns\n",tint->seconds,tint->nanoseconds);
//	DBGV("processDelayReqFromSelf: %s %d\n",dump_TimeInternal(&ptpClock->delay_req_send_time),rtOpts->outboundLatency);
}

static void
handleDelayResp(const MsgHeader *header, ssize_t length,RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
	if (ptpClock->delayMechanism == E2E)
		{
			TimeInternal requestReceiptTimestamp;
			TimeInternal correctionField;				
			DBGV("delayResp message received : \n");
			if(length < DELAY_RESP_LENGTH) 	//�������ϸ�ı���										
				{
					DBG("Error: DelayResp message too short\n");
					//ptpClock->counters.messageFormatErrors++;
					return;
				}
			switch(ptpClock->portState) 
			{
				case PTP_INITIALIZING:
				case PTP_FAULTY:
				case PTP_DISABLED:
				case PTP_UNCALIBRATED:
				case PTP_LISTENING:
						DBGV("HandledelayResp : disregard\n");
						//ptpClock->counters.discardedMessages++;
						return;
				case PTP_SLAVE:				//ֻ��״̬�Ŵ���
						msgUnpackDelayResp(ptpClock->msgIbuf,&ptpClock->msgTmp.resp);	//��������

						if ((memcmp(ptpClock->portIdentity.clockIdentity , ptpClock->msgTmp.resp.requestingPortIdentity.clockIdentity,
									CLOCK_IDENTITY_LENGTH) == 0) && (ptpClock->portIdentity.portNumber ==  ptpClock->msgTmp.resp.requestingPortIdentity.portNumber) 
									&& isFromCurrentParent(ptpClock, header)) 	//�ж��Ƿ�����ѡ�е�ʱ��
						
							{
								//DBG("==> Handle DelayResp (%d)\n",header->sequenceId);

								if (!ptpClock->waitingForDelayResp) 
									{
										DBGV("Ignored DelayResp - wasn't waiting for one\n");	//״̬����_������DelayResp����ʱ����������
										//ptpClock->counters.discardedMessages++;
										break;
									}

								if (ptpClock->sentDelayReqSequenceId != ((UInteger16)(header->sequenceId + 1))) 	//sequenceId�����������
									{
										DBG("HandledelayResp : sequence mismatch - last DelayReq sent: %d, delayResp received: %d\n",
												ptpClock->sentDelayReqSequenceId,header->sequenceId);
										//ptpClock->counters.discardedMessages++;
										//ptpClock->counters.sequenceMismatchErrors++;
										break;
									}

								ptpClock->counters.delayRespMessagesReceived++;	//����
								ptpClock->waitingForDelayResp = FALSE;					//״̬���̱仯_ȡ����ǵȴ�DelayResp����

								toInternalTime(&requestReceiptTimestamp,&ptpClock->msgTmp.resp.receiveTimestamp);	//�ѱ��Ľ���������ʱ���ת���ɱ�׼ʱ��
								ptpClock->delay_req_receive_time.seconds = requestReceiptTimestamp.seconds;				//ʱ�����Э��ջ
								ptpClock->delay_req_receive_time.nanoseconds = requestReceiptTimestamp.nanoseconds;
								
								scaledNanosecondsToInternalTime(&header->correctionField,&correctionField);	//������ʱ��ת���ɱ�׼ʱ��
								/*
									send_time = delay_req_send_time (received as CMSG in handleEvent)
									recv_time = requestReceiptTimestamp (received inside delayResp)
								*/

								updateDelay(&ptpClock->owd_filt,rtOpts,ptpClock, &correctionField);//�����ӳ�����
								
								//if (ptpClock->waiting_for_first_delayresp) 
								//	{
								//		NOTICE("Received first Delay Response from Master\n");
								//	}
								
								if (rtOpts->ignore_delayreq_interval_master == FALSE)  //��������ʱ�ӵļ��Э��
									{
										DBGV("current delay_req: %d  new delay req: %d \n",ptpClock->logMinDelayReqInterval,header->logMessageInterval);
										if (ptpClock->logMinDelayReqInterval != header->logMessageInterval)//�Լ���ǰ���ͼ������ʱ�ӵļ����ͬ
										{
											if((header->logMessageInterval == UNICAST_MESSAGEINTERVAL) && rtOpts->autoDelayReqInterval) //�Զ�Э��
												{ 
													if(ptpClock->waiting_for_first_delayresp) 
														{
															NOTICE("Received %d Delay Interval from master (unicast) - overriding with %d\n",header->logMessageInterval, rtOpts->subsequent_delayreq);
														}
													ptpClock->logMinDelayReqInterval = rtOpts->subsequent_delayreq;	//���ü��	=	�Զ�Э��
												} 
											else 
												{
													/* Accept new DelayReq value from the Master */
													NOTICE("Received new Delay Request interval %d from Master (was: %d)\n",header->logMessageInterval, ptpClock->logMinDelayReqInterval );
													// collect new value indicated from the Master
													ptpClock->logMinDelayReqInterval = header->logMessageInterval;	//���ü��	= ����
												}
												/* FIXME: the actual rearming of this timer with the new value only happens later in doState()/issueDelayReq() */
										}
									} 
								else //�̶����������Э��
									{
										if (ptpClock->logMinDelayReqInterval != rtOpts->subsequent_delayreq) 
											{
												INFO("New Delay Request interval applied: %d (was: %d)\n",rtOpts->subsequent_delayreq, ptpClock->logMinDelayReqInterval);
											}
										ptpClock->logMinDelayReqInterval = rtOpts->subsequent_delayreq;
									}
							} 
						else 	//���ϸ�ģ�����
							{
								DBG("HandledelayResp : delayResp doesn't match with the delayReq. \n");
								ptpClock->counters.discardedMessages++;
								break;
							}
						ptpClock->waiting_for_first_delayresp = FALSE;		//������յ���һ��delayresp
							
				default:
						break;			
			}			//end switch
	} 			//end if(E2E)
		
	else if (ptpClock->delayMechanism == P2P) //P2P��������
	{ /* (Peer to Peer mode) */
		DBG("Delay messages are disregarded in Peer to Peer mode \n");
		//ptpClock->counters.discardedMessages++;
		//ptpClock->counters.delayModeMismatchErrors++;
		/* no delay mechanism */
	}
	else
	{
		DBG("DelayResp ignored - we are in DELAY_DISABLED mode");//��BUG��
		//ptpClock->counters.discardedMessages++;
	}

}


static void
handlePDelayReq(MsgHeader *header, ssize_t length, 
		const TimeInternal *tint, Boolean isFromSelf, RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
	if (ptpClock->delayMechanism == P2P) {
		DBGV("PdelayReq message received : \n");

		if(length < PDELAY_REQ_LENGTH) {
			DBG("Error: PDelayReq message too short\n");
			ptpClock->counters.messageFormatErrors++;
			return;
		}

		switch (ptpClock->portState ) {
		case PTP_INITIALIZING:
		case PTP_FAULTY:
		case PTP_DISABLED:
		case PTP_UNCALIBRATED:
		case PTP_LISTENING:
			DBGV("HandlePdelayReq : disregard\n");
			ptpClock->counters.discardedMessages++;
			return;

		case PTP_SLAVE:
		case PTP_MASTER:
		case PTP_PASSIVE:

			if (isFromSelf) { //�Լ����������ݰ����Լ��յ�(һ�㲻����)
//				processPDelayReqFromSelf(tint, rtOpts, ptpClock);
				break;
			} else {
				ptpClock->counters.pdelayReqMessagesReceived++;
				msgUnpackHeader(ptpClock->msgIbuf,&ptpClock->PdelayReqHeader);
				issuePDelayResp(tint,header,rtOpts,ptpClock);	
				break;
			}
		default:
			DBG("do unrecognized state3\n");
			ptpClock->counters.discardedMessages++;
			break;
		}
	} else if (ptpClock->delayMechanism == E2E){/* (End to End mode..) */
		DBG("Peer Delay messages are disregarded in End to End mode \n");
		ptpClock->counters.discardedMessages++;
		ptpClock->counters.delayModeMismatchErrors++;
	/* no delay mechanism */
	} else {
		DBG("PDelayReq ignored - we are in DELAY_DISABLED mode");
		ptpClock->counters.discardedMessages++;
	}
}

static void
processPDelayReqFromSelf(const TimeInternal * tint, RunTimeOpts * rtOpts, PtpClock * ptpClock) {
	/*
	 * Get sending timestamp from IP stack
	 * with SO_TIMESTAMP
	 */
	ptpClock->pdelay_req_send_time.seconds = tint->seconds;
	ptpClock->pdelay_req_send_time.nanoseconds = tint->nanoseconds;
	/*Add latency*/
	addTime(&ptpClock->pdelay_req_send_time,&ptpClock->pdelay_req_send_time,&rtOpts->outboundLatency);
}

static void
handlePDelayResp(const MsgHeader *header, TimeInternal *tint,
		 ssize_t length, Boolean isFromSelf, 
		 RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
	if (ptpClock->delayMechanism == P2P) {
		/* Boolean isFromCurrentParent = FALSE; NOTE: This is never used in this function */
		TimeInternal requestReceiptTimestamp;
		TimeInternal correctionField;
	
		DBG("PdelayResp message received : \n");

		if (length < PDELAY_RESP_LENGTH) {
			DBG("Error: PDelayResp message too short\n");
			//ptpClock->counters.messageFormatErrors++;
			return;
		}

		switch (ptpClock->portState ) {
		case PTP_INITIALIZING:
		case PTP_FAULTY:
		case PTP_DISABLED:
		case PTP_UNCALIBRATED:
		case PTP_LISTENING:
			DBGV("HandlePdelayResp : disregard\n");
			//ptpClock->counters.discardedMessages++;
			break;
		case PTP_SLAVE:
		case PTP_MASTER:
			
			if (ptpClock->twoStepFlag && isFromSelf) 
			{
				processPDelayRespFromSelf(tint, rtOpts, ptpClock, header->sequenceId);
				break;
			}
			msgUnpackPDelayResp(ptpClock->msgIbuf,&ptpClock->msgTmp.presp);
		
			if (ptpClock->sentPDelayReqSequenceId != ((UInteger16)(header->sequenceId + 1))) 
			{
				    DBG("PDelayResp: sequence mismatch - sent: %d, received: %d\n",
					    ptpClock->sentPDelayReqSequenceId,header->sequenceId);
				    //ptpClock->counters.discardedMessages++;
				    //ptpClock->counters.sequenceMismatchErrors++;
				    break;
			}
			if ((!memcmp(ptpClock->portIdentity.clockIdentity,ptpClock->msgTmp.presp.requestingPortIdentity.clockIdentity,CLOCK_IDENTITY_LENGTH))
				 && ( ptpClock->portIdentity.portNumber == ptpClock->msgTmp.presp.requestingPortIdentity.portNumber))	
			{
				ptpClock->counters.pdelayRespMessagesReceived++;
        /* Two Step Clock */
				if ((header->flagField0 & PTP_TWO_STEP) == PTP_TWO_STEP) {
					/*Store t4 (Fig 35)*/
					ptpClock->pdelay_resp_receive_time.seconds = tint->seconds;
					ptpClock->pdelay_resp_receive_time.nanoseconds = tint->nanoseconds;
					/*store t2 (Fig 35)*/
					toInternalTime(&requestReceiptTimestamp,&ptpClock->msgTmp.presp.requestReceiptTimestamp);
					ptpClock->pdelay_req_receive_time.seconds = requestReceiptTimestamp.seconds;
					ptpClock->pdelay_req_receive_time.nanoseconds = requestReceiptTimestamp.nanoseconds;
					
					scaledNanosecondsToInternalTime(&header->correctionField,&correctionField);
					ptpClock->lastPdelayRespCorrectionField.seconds = correctionField.seconds;
					ptpClock->lastPdelayRespCorrectionField.nanoseconds = correctionField.nanoseconds;
				} else {
				/* One step Clock */
					ptpClock->pdelay_resp_receive_time.seconds = tint->seconds;
					ptpClock->pdelay_resp_receive_time.nanoseconds = tint->nanoseconds;
					
					scaledNanosecondsToInternalTime(&header->correctionField,&correctionField);
					updatePeerDelay(rtOpts,ptpClock,&correctionField,FALSE);
				}
				ptpClock->recvPDelayRespSequenceId = header->sequenceId;
				break;
			} else {
				DBG("HandlePdelayResp : Pdelayresp doesn't match with the PdelayReq. \n");
				//ptpClock->counters.discardedMessages++;
			}
			break; /* XXX added by gnn for safety */	
		default:
			DBG("do unrecognized state4\n");
			//ptpClock->counters.discardedMessages++;
			break;
		}
	} else if (ptpClock->delayMechanism == E2E) { /* (End to End mode..) */
		DBG("Peer Delay messages are disregarded in End to End mode \n");
		//ptpClock->counters.discardedMessages++;
		//ptpClock->counters.delayModeMismatchErrors++;
	/* no delay mechanism */
	} else {
		DBG("PDelayResp ignored - we are in DELAY_DISABLED mode");
		//ptpClock->counters.discardedMessages++;
	}
}

static void
processPDelayRespFromSelf(const TimeInternal * tint, RunTimeOpts * rtOpts, PtpClock * ptpClock, UInteger16 sequenceId) {
	TimeInternal timestamp;
	
	addTime(&timestamp, tint, &rtOpts->outboundLatency);

	issuePDelayRespFollowUp(&timestamp, &ptpClock->PdelayReqHeader,rtOpts, ptpClock, sequenceId);
}

static void 
handlePDelayRespFollowUp(const MsgHeader *header, ssize_t length, 
			 Boolean isFromSelf, RunTimeOpts *rtOpts, PtpClock *ptpClock)
{

	if (ptpClock->delayMechanism == P2P) {
		TimeInternal responseOriginTimestamp;
		TimeInternal correctionField;
	
		DBG("PdelayRespfollowup message received : \n");
	
		if(length < PDELAY_RESP_FOLLOW_UP_LENGTH) {
			DBG("Error: PDelayRespfollowup message too short\n");
			ptpClock->counters.messageFormatErrors++;
			return;
		}	
	
		switch(ptpClock->portState) {
		case PTP_INITIALIZING:
		case PTP_FAULTY:
		case PTP_DISABLED:
		case PTP_UNCALIBRATED:
			DBGV("HandlePdelayResp : disregard\n");
			//ptpClock->counters.discardedMessages++;
			return;
		
		case PTP_SLAVE:
		case PTP_MASTER:
			if (isFromSelf) {
				DBGV("HandlePdelayRespFollowUp : Ignore message from self \n");
				return;
			}
			if (( ((UInteger16)(header->sequenceId + 1)) == ptpClock->sentPDelayReqSequenceId) && (header->sequenceId == ptpClock->recvPDelayRespSequenceId)) {
				msgUnpackPDelayRespFollowUp(ptpClock->msgIbuf,&ptpClock->msgTmp.prespfollow);
				ptpClock->counters.pdelayRespFollowUpMessagesReceived++;
				toInternalTime(&responseOriginTimestamp,&ptpClock->msgTmp.prespfollow.responseOriginTimestamp);
				ptpClock->pdelay_resp_send_time.seconds = responseOriginTimestamp.seconds;
				ptpClock->pdelay_resp_send_time.nanoseconds = responseOriginTimestamp.nanoseconds;
				scaledNanosecondsToInternalTime(&ptpClock->msgTmpHeader.correctionField,&correctionField);
				addTime(&correctionField,&correctionField,&ptpClock->lastPdelayRespCorrectionField);
				updatePeerDelay (rtOpts, ptpClock,&correctionField,TRUE);
				break;
			} else {
				DBG("PdelayRespFollowup: sequence mismatch - Received: %d PdelayReq sent: %d, PdelayResp received: %d\n",
				header->sequenceId, ptpClock->sentPDelayReqSequenceId,ptpClock->recvPDelayRespSequenceId);
				//ptpClock->counters.discardedMessages++;
				//ptpClock->counters.sequenceMismatchErrors++;
				break;
			}
		default:
			DBGV("Disregard PdelayRespFollowUp message  \n");
			//ptpClock->counters.discardedMessages++;
		}
	} else if (ptpClock->delayMechanism == E2E) { /* (End to End mode..) */
		DBG("Peer Delay messages are disregarded in End to End mode \n");
		//ptpClock->counters.discardedMessages++;
		//ptpClock->counters.delayModeMismatchErrors++;
	/* no delay mechanism */
	} else {
		DBG("PDelayRespFollowUp ignored - we are in DELAY_DISABLED mode");
		//ptpClock->counters.discardedMessages++;
	}
}

/* Only accept the management message if it satisfies 15.3.1 Table 36 */ 
int
acceptManagementMessage(PortIdentity thisPort, PortIdentity targetPort)
{
        ClockIdentity allOnesClkIdentity;
        UInteger16    allOnesPortNumber = 0xFFFF;
        memset(allOnesClkIdentity, 0xFF, sizeof(allOnesClkIdentity));

        return ((memcmp(targetPort.clockIdentity, thisPort.clockIdentity, CLOCK_IDENTITY_LENGTH) == 0) ||
                (memcmp(targetPort.clockIdentity, allOnesClkIdentity, CLOCK_IDENTITY_LENGTH) == 0))
                &&
                ((targetPort.portNumber == thisPort.portNumber) ||
                (targetPort.portNumber == allOnesPortNumber));
}


static void
handleManagement(MsgHeader *header,
		 Boolean isFromSelf, RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
	DBGV("Management message received : \n");

	if(!rtOpts->managementEnabled) {
		DBGV("Dropping management message - management message support disabled");
		ptpClock->counters.discardedMessages++;
		return;
	}

	if (isFromSelf) {
		DBGV("handleManagement: Ignore message from self \n");
		return;
	}

	msgUnpackManagement(ptpClock->msgIbuf,&ptpClock->msgTmp.manage, header, ptpClock);

	if(ptpClock->msgTmp.manage.tlv == NULL) {
		DBGV("handleManagement: TLV is empty\n");
		ptpClock->counters.messageFormatErrors++;
		return;
	}

        if(!acceptManagementMessage(ptpClock->portIdentity, ptpClock->msgTmp.manage.targetPortIdentity))
        {
                DBGV("handleManagement: The management message was not accepted");
		ptpClock->counters.discardedMessages++;
                /* cleanup msgTmp managementTLV */
                freeManagementTLV(&ptpClock->msgTmp.manage);
                return;
        }

	if(!rtOpts->managementSetEnable &&
	    (ptpClock->msgTmp.manage.actionField == SET ||
	    ptpClock->msgTmp.manage.actionField == COMMAND)) {
		DBGV("Dropping SET/COMMAND management message - read-only mode enabled");
		ptpClock->counters.discardedMessages++;
                /* cleanup msgTmp managementTLV */
                freeManagementTLV(&ptpClock->msgTmp.manage);
		return;
	}

	/* is this an error status management TLV? */
	if(ptpClock->msgTmp.manage.tlv->tlvType == TLV_MANAGEMENT_ERROR_STATUS) {
		DBGV("handleManagement: Error Status TLV\n");
		unpackMMErrorStatus(ptpClock->msgIbuf, &ptpClock->msgTmp.manage, ptpClock);
		handleMMErrorStatus(&ptpClock->msgTmp.manage);
		ptpClock->counters.managementMessagesReceived++;
		/* cleanup msgTmp managementTLV */
		if(ptpClock->msgTmp.manage.tlv) {
			DBGV("cleanup ptpClock->msgTmp.manage message \n");
			if(ptpClock->msgTmp.manage.tlv->dataField) {
				freeMMErrorStatusTLV(ptpClock->msgTmp.manage.tlv);
				free(ptpClock->msgTmp.manage.tlv->dataField);
			}
			free(ptpClock->msgTmp.manage.tlv);
		}
		return;
	} else if (ptpClock->msgTmp.manage.tlv->tlvType != TLV_MANAGEMENT) {
		/* do nothing, implemention specific handling */
		DBGV("handleManagement: Currently unsupported management TLV type\n");
		ptpClock->counters.discardedMessages++;
                /* cleanup msgTmp managementTLV */
                freeManagementTLV(&ptpClock->msgTmp.manage);
		return;
	}

	switch(ptpClock->msgTmp.manage.tlv->managementId)
	{
	case MM_NULL_MANAGEMENT:
		DBGV("handleManagement: Null Management\n");
		handleMMNullManagement(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp, ptpClock);
		ptpClock->counters.managementMessagesReceived++;
		break;
	case MM_CLOCK_DESCRIPTION:
		DBGV("handleManagement: Clock Description\n");
		unpackMMClockDescription(ptpClock->msgIbuf, &ptpClock->msgTmp.manage, ptpClock);
		handleMMClockDescription(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp, ptpClock);
		ptpClock->counters.managementMessagesReceived++;
		break;
	case MM_USER_DESCRIPTION:
		DBGV("handleManagement: User Description\n");
		unpackMMUserDescription(ptpClock->msgIbuf, &ptpClock->msgTmp.manage, ptpClock);
		handleMMUserDescription(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp, ptpClock);
		ptpClock->counters.managementMessagesReceived++;
		break;
	case MM_SAVE_IN_NON_VOLATILE_STORAGE:
		DBGV("handleManagement: Save In Non-Volatile Storage\n");
		handleMMSaveInNonVolatileStorage(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp, ptpClock);
		ptpClock->counters.managementMessagesReceived++;
		break;
	case MM_RESET_NON_VOLATILE_STORAGE:
		DBGV("handleManagement: Reset Non-Volatile Storage\n");
		handleMMResetNonVolatileStorage(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp, ptpClock);
		ptpClock->counters.managementMessagesReceived++;
		break;
	case MM_INITIALIZE:
		DBGV("handleManagement: Initialize\n");
		unpackMMInitialize(ptpClock->msgIbuf, &ptpClock->msgTmp.manage, ptpClock);
		handleMMInitialize(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp, ptpClock);
		ptpClock->counters.managementMessagesReceived++;
		break;
	case MM_DEFAULT_DATA_SET:
		DBGV("handleManagement: Default Data Set\n");
		unpackMMDefaultDataSet(ptpClock->msgIbuf, &ptpClock->msgTmp.manage, ptpClock);
		handleMMDefaultDataSet(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp, ptpClock);
		ptpClock->counters.managementMessagesReceived++;
		break;
	case MM_CURRENT_DATA_SET:
		DBGV("handleManagement: Current Data Set\n");
		unpackMMCurrentDataSet(ptpClock->msgIbuf, &ptpClock->msgTmp.manage, ptpClock);
		handleMMCurrentDataSet(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp, ptpClock);
		ptpClock->counters.managementMessagesReceived++;
		break;
        case MM_PARENT_DATA_SET:
                DBGV("handleManagement: Parent Data Set\n");
                unpackMMParentDataSet(ptpClock->msgIbuf, &ptpClock->msgTmp.manage, ptpClock);
                handleMMParentDataSet(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp, ptpClock);
		ptpClock->counters.managementMessagesReceived++;
                break;
        case MM_TIME_PROPERTIES_DATA_SET:
                DBGV("handleManagement: TimeProperties Data Set\n");
                unpackMMTimePropertiesDataSet(ptpClock->msgIbuf, &ptpClock->msgTmp.manage, ptpClock);
                handleMMTimePropertiesDataSet(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp, ptpClock);
		ptpClock->counters.managementMessagesReceived++;
                break;
        case MM_PORT_DATA_SET:
                DBGV("handleManagement: Port Data Set\n");
                unpackMMPortDataSet(ptpClock->msgIbuf, &ptpClock->msgTmp.manage, ptpClock);
                handleMMPortDataSet(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp, ptpClock);
		ptpClock->counters.managementMessagesReceived++;
                break;
        case MM_PRIORITY1:
                DBGV("handleManagement: Priority1\n");
                unpackMMPriority1(ptpClock->msgIbuf, &ptpClock->msgTmp.manage, ptpClock);
                handleMMPriority1(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp, ptpClock);
		ptpClock->counters.managementMessagesReceived++;
                break;
        case MM_PRIORITY2:
                DBGV("handleManagement: Priority2\n");
                unpackMMPriority2(ptpClock->msgIbuf, &ptpClock->msgTmp.manage, ptpClock);
                handleMMPriority2(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp, ptpClock);
		ptpClock->counters.managementMessagesReceived++;
                break;
        case MM_DOMAIN:
                DBGV("handleManagement: Domain\n");
                unpackMMDomain(ptpClock->msgIbuf, &ptpClock->msgTmp.manage, ptpClock);
                handleMMDomain(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp, ptpClock);
		ptpClock->counters.managementMessagesReceived++;
                break;
	case MM_SLAVE_ONLY:
		DBGV("handleManagement: Slave Only\n");
		unpackMMSlaveOnly(ptpClock->msgIbuf, &ptpClock->msgTmp.manage, ptpClock);
		handleMMSlaveOnly(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp, ptpClock);
		ptpClock->counters.managementMessagesReceived++;
		break;
        case MM_LOG_ANNOUNCE_INTERVAL:
                DBGV("handleManagement: Log Announce Interval\n");
                unpackMMLogAnnounceInterval(ptpClock->msgIbuf, &ptpClock->msgTmp.manage, ptpClock);
                handleMMLogAnnounceInterval(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp, ptpClock);
		ptpClock->counters.managementMessagesReceived++;
                break;
        case MM_ANNOUNCE_RECEIPT_TIMEOUT:
                DBGV("handleManagement: Announce Receipt Timeout\n");
                unpackMMAnnounceReceiptTimeout(ptpClock->msgIbuf, &ptpClock->msgTmp.manage, ptpClock);
                handleMMAnnounceReceiptTimeout(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp, ptpClock);
		ptpClock->counters.managementMessagesReceived++;
                break;
        case MM_LOG_SYNC_INTERVAL:
                DBGV("handleManagement: Log Sync Interval\n");
                unpackMMLogSyncInterval(ptpClock->msgIbuf, &ptpClock->msgTmp.manage, ptpClock);
                handleMMLogSyncInterval(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp, ptpClock);
		ptpClock->counters.managementMessagesReceived++;
                break;
        case MM_VERSION_NUMBER:
                DBGV("handleManagement: Version Number\n");
                unpackMMVersionNumber(ptpClock->msgIbuf, &ptpClock->msgTmp.manage, ptpClock);
                handleMMVersionNumber(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp, ptpClock);
		ptpClock->counters.managementMessagesReceived++;
                break;
        case MM_ENABLE_PORT:
                DBGV("handleManagement: Enable Port\n");
                handleMMEnablePort(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp, ptpClock);
		ptpClock->counters.managementMessagesReceived++;
                break;
        case MM_DISABLE_PORT:
                DBGV("handleManagement: Disable Port\n");
                handleMMDisablePort(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp, ptpClock);
		ptpClock->counters.managementMessagesReceived++;
                break;
        case MM_TIME:
                DBGV("handleManagement: Time\n");
                unpackMMTime(ptpClock->msgIbuf, &ptpClock->msgTmp.manage, ptpClock);
                handleMMTime(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp, ptpClock, rtOpts);
		ptpClock->counters.managementMessagesReceived++;
                break;
        case MM_CLOCK_ACCURACY:
                DBGV("handleManagement: Clock Accuracy\n");
                unpackMMClockAccuracy(ptpClock->msgIbuf, &ptpClock->msgTmp.manage, ptpClock);
                handleMMClockAccuracy(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp, ptpClock);
		ptpClock->counters.managementMessagesReceived++;
                break;
        case MM_UTC_PROPERTIES:
                DBGV("handleManagement: Utc Properties\n");
                unpackMMUtcProperties(ptpClock->msgIbuf, &ptpClock->msgTmp.manage, ptpClock);
                handleMMUtcProperties(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp, ptpClock);
		ptpClock->counters.managementMessagesReceived++;
                break;
        case MM_TRACEABILITY_PROPERTIES:
                DBGV("handleManagement: Traceability Properties\n");
                unpackMMTraceabilityProperties(ptpClock->msgIbuf, &ptpClock->msgTmp.manage, ptpClock);
                handleMMTraceabilityProperties(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp, ptpClock);
		ptpClock->counters.managementMessagesReceived++;
                break;
        case MM_DELAY_MECHANISM:
                DBGV("handleManagement: Delay Mechanism\n");
                unpackMMDelayMechanism(ptpClock->msgIbuf, &ptpClock->msgTmp.manage, ptpClock);
                handleMMDelayMechanism(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp, ptpClock);
		ptpClock->counters.managementMessagesReceived++;
                break;
        case MM_LOG_MIN_PDELAY_REQ_INTERVAL:
                DBGV("handleManagement: Log Min Pdelay Req Interval\n");
                unpackMMLogMinPdelayReqInterval(ptpClock->msgIbuf, &ptpClock->msgTmp.manage, ptpClock);
                handleMMLogMinPdelayReqInterval(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp, ptpClock);
		ptpClock->counters.managementMessagesReceived++;
                break;
	case MM_FAULT_LOG:
	case MM_FAULT_LOG_RESET:
	case MM_TIMESCALE_PROPERTIES:
	case MM_UNICAST_NEGOTIATION_ENABLE:
	case MM_PATH_TRACE_LIST:
	case MM_PATH_TRACE_ENABLE:
	case MM_GRANDMASTER_CLUSTER_TABLE:
	case MM_UNICAST_MASTER_TABLE:
	case MM_UNICAST_MASTER_MAX_TABLE_SIZE:
	case MM_ACCEPTABLE_MASTER_TABLE:
	case MM_ACCEPTABLE_MASTER_TABLE_ENABLED:
	case MM_ACCEPTABLE_MASTER_MAX_TABLE_SIZE:
	case MM_ALTERNATE_MASTER:
	case MM_ALTERNATE_TIME_OFFSET_ENABLE:
	case MM_ALTERNATE_TIME_OFFSET_NAME:
	case MM_ALTERNATE_TIME_OFFSET_MAX_KEY:
	case MM_ALTERNATE_TIME_OFFSET_PROPERTIES:
	case MM_TRANSPARENT_CLOCK_DEFAULT_DATA_SET:
	case MM_TRANSPARENT_CLOCK_PORT_DATA_SET:
	case MM_PRIMARY_DOMAIN:
		DBGV("handleManagement: Currently unsupported managementTLV %d\n",
				ptpClock->msgTmp.manage.tlv->managementId);
		handleErrorManagementMessage(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp,
			ptpClock, ptpClock->msgTmp.manage.tlv->managementId,
			NOT_SUPPORTED);
		ptpClock->counters.discardedMessages++;
		break;
	default:
		DBGV("handleManagement: Unknown managementTLV %d\n",
				ptpClock->msgTmp.manage.tlv->managementId);
		handleErrorManagementMessage(&ptpClock->msgTmp.manage, &ptpClock->outgoingManageTmp,
			ptpClock, ptpClock->msgTmp.manage.tlv->managementId,
			NO_SUCH_ID);
		ptpClock->counters.discardedMessages++;
	}

        /* If the management message we received was unicast, we also reply with unicast */
//        if((header->flagField0 & PTP_UNICAST) == PTP_UNICAST)
//                ptpClock->LastSlaveAddr = ptpClock->netPath.lastRecvAddr;
//        else ptpClock->LastSlaveAddr = 0;

	/* send management message response or acknowledge */
	if(ptpClock->outgoingManageTmp.tlv->tlvType == TLV_MANAGEMENT) {
		if(ptpClock->outgoingManageTmp.actionField == RESPONSE ||
				ptpClock->outgoingManageTmp.actionField == ACKNOWLEDGE) {
			issueManagementRespOrAck(&ptpClock->outgoingManageTmp, rtOpts, ptpClock);
		}
	} else if(ptpClock->outgoingManageTmp.tlv->tlvType == TLV_MANAGEMENT_ERROR_STATUS) {
		issueManagementErrorStatus(&ptpClock->outgoingManageTmp, rtOpts, ptpClock);
	}

	/* cleanup msgTmp managementTLV */
	freeManagementTLV(&ptpClock->msgTmp.manage);
	/* cleanup outgoing managementTLV */
	freeManagementTLV(&ptpClock->outgoingManageTmp);

}

static void 
handleSignaling(PtpClock *ptpClock)
{
	//ptpClock->counters.signalingMessagesReceived++;
}

/*Pack and send on general multicast ip adress an Announce message*/
static void issueAnnounce(RunTimeOpts *rtOpts,PtpClock *ptpClock)
{
	msgPackAnnounce(ptpClock->msgObuf,ptpClock);
	
	if(rtOpts->transport == IEEE_802_3)
	{	//0��ʾʹ��ͨ�õ�ַ��NULL��ʾ��Ҫ����ʱ��
		if(eth_send_ptp(ptpClock->msgObuf, ANNOUNCE_LENGTH, FALSE, NULL) != ERR_OK){
			ERROR("IEEE_802_3 issueAnnounce:can't sent\n");
			ptpClock->counters.messageSendErrors++;
			toState(PTP_FAULTY,rtOpts,ptpClock);
		}else{
			DBGV("IEEE_802_3 Announce MSG sent! \n");
			ptpClock->sentAnnounceSequenceId++;
			ptpClock->counters.announceMessagesSent++;
		}
	}
	else if(rtOpts->transport == UDP_IPV4)
	{ //�鲥��ַ
		if (!netSendGeneral(&ptpClock->netPath,ptpClock->msgObuf,ANNOUNCE_LENGTH)) {
			toState(PTP_FAULTY,rtOpts,ptpClock);
			ptpClock->counters.messageSendErrors++;
			ERROR("Announce message can't be sent -> FAULTY state \n");
		} else {
			DBGV("Announce MSG sent ! \n");
			ptpClock->sentAnnounceSequenceId++;
			ptpClock->counters.announceMessagesSent++;
		}
	}
}



/*Pack and send on event multicast ip adress a Sync message*/
static void issueSync(RunTimeOpts *rtOpts,PtpClock *ptpClock)
{
	Timestamp originTimestamp;
	TimeInternal internalTime;
	
	getTime(&internalTime); //��ȡ���Է���ʱ��
	if (respectUtcOffset(rtOpts, ptpClock) == TRUE)
		{ //�Ƿ���UTCƫ��
			internalTime.seconds += ptpClock->timePropertiesDS.currentUtcOffset;
		}
	//printf("TAI-UTC=%d\n",ptpClock->timePropertiesDS.currentUtcOffset);
	fromInternalTime(&internalTime,&originTimestamp); //ת������ʱ��
	msgPackSync(ptpClock->msgObuf,&originTimestamp,ptpClock); //���
	if(rtOpts->transport == UDP_IPV4)			//UDPģʽ
		{
			if (!netSendEvent(&ptpClock->netPath,ptpClock->msgObuf,SYNC_LENGTH,&internalTime))
				{ 
					toState(PTP_FAULTY,rtOpts,ptpClock);
					ptpClock->counters.messageSendErrors++;
					ERROR("Sync message can't be sent -> FAULTY state \n");
				} 
			else 																//���ͳɹ�
				{
					if(internalTime.seconds != 0) 	//���ص�ʱ�����Ч
						{ 
							if (respectUtcOffset(rtOpts, ptpClock) == TRUE) 	//����UTCʱ��ƫ��(�����)
								{
									internalTime.seconds += ptpClock->timePropertiesDS.currentUtcOffset;
								}
							if((ptpClock->twoStepFlag == TRUE) && (internalTime.seconds != 0))	//�����������������FollowUP
								{
									issueFollowup(&internalTime, rtOpts, ptpClock, ptpClock->sentSyncSequenceId);
									//processSyncFromSelf(&internalTime, rtOpts, ptpClock, ptpClock->sentSyncSequenceId);
								}
						}
					ptpClock->sentSyncSequenceId++;
					ptpClock->counters.syncMessagesSent++;
				}
		}
	else if(rtOpts->transport == IEEE_802_3)	//MACģʽ
		{
			if (eth_send_ptp(ptpClock->msgObuf, SYNC_LENGTH, FALSE, &internalTime) != ERR_OK)
				{
					toState(PTP_FAULTY,rtOpts,ptpClock);
				}
			else																								//���ͳɹ�
				{
					if (respectUtcOffset(rtOpts, ptpClock) == TRUE) //����UTCʱ��ƫ��(�����)
						{
							internalTime.seconds += ptpClock->timePropertiesDS.currentUtcOffset;
						}
					if((ptpClock->twoStepFlag == TRUE) && (internalTime.seconds != 0))		//�����������������FollowUP
						{
							//printf("sysn timetap %d\n",internalTime.seconds);
							issueFollowup(&internalTime, rtOpts, ptpClock, ptpClock->sentSyncSequenceId);
							//processSyncFromSelf(&internalTime, rtOpts, ptpClock, ptpClock->sentSyncSequenceId);
						}
					ptpClock->sentSyncSequenceId++;
					ptpClock->counters.syncMessagesSent++;
				}
		}
}


/*Pack and send on general multicast ip adress a FollowUp message*/
static void
issueFollowup(const TimeInternal *tint,RunTimeOpts *rtOpts,PtpClock *ptpClock, UInteger16 sequenceId)
{
	Timestamp preciseOriginTimestamp;
	fromInternalTime(tint,&preciseOriginTimestamp);
	
	msgPackFollowUp(ptpClock->msgObuf,&preciseOriginTimestamp,ptpClock,sequenceId);	
	//printf("FollowUp tint = %d .%d, %d\n",(preciseOriginTimestamp.secondsField.lsb),preciseOriginTimestamp.secondsField.msb,preciseOriginTimestamp.nanosecondsField);
	if(rtOpts->transport == UDP_IPV4)
	{
		if (!netSendGeneral(&ptpClock->netPath,ptpClock->msgObuf,FOLLOW_UP_LENGTH)) {
			toState(PTP_FAULTY,rtOpts,ptpClock);
			//ptpClock->counters.messageSendErrors++;
			DBGV("FollowUp message can't be sent -> FAULTY state \n");
		} else {
			DBGV("FollowUp MSG sent ! \n");
			ptpClock->counters.followUpMessagesSent++;
		}
	}else if(rtOpts->transport == IEEE_802_3)
	{
		if (eth_send_ptp(ptpClock->msgObuf, FOLLOW_UP_LENGTH, FALSE,NULL) != ERR_OK)
		{
			ERROR("IEEE_802_3 FollowUp message can't be sent\n");
	    toState(PTP_FAULTY,rtOpts,ptpClock);
		}
		else{
			DBGV("IEEE_802_3 FollowUp sent!\n");
			ptpClock->counters.followUpMessagesSent++;
	  }
	}
}


/*Pack and send on event multicast ip adress a DelayReq message*/
static void
issueDelayReq(RunTimeOpts *rtOpts,PtpClock *ptpClock)
{
	Timestamp originTimestamp;
	TimeInternal internalTime;
//	Integer32 dst = 0;

	DBG("==> Issue DelayReq (%d)\n", ptpClock->sentDelayReqSequenceId );

	/*
	 * call GTOD. This time is later replaced in handleDelayReq,
	 * to get the actual send timestamp from the OS
	 */
	getTime(&internalTime);
	if (respectUtcOffset(rtOpts, ptpClock) == TRUE)
		{
			internalTime.seconds += ptpClock->timePropertiesDS.currentUtcOffset;	//Ӳ��ʱ��ת����UCTʱ��
		}
	fromInternalTime(&internalTime,&originTimestamp);	//Ӳ��ʱ��(UCT)ʱ��ת����ʱ���

	// uses current sentDelayReqSequenceId
	msgPackDelayReq(ptpClock->msgObuf,&originTimestamp,ptpClock);//DelayReq�����������ʱ���


	if(rtOpts->transport == UDP_IPV4)	
		{
			if (!netSendEvent(&ptpClock->netPath,ptpClock->msgObuf,DELAY_REQ_LENGTH,&internalTime)) //��UDP_IPV4ģʽ����
				{
					toState(PTP_FAULTY,rtOpts,ptpClock);							//����ʧ��
					ptpClock->counters.messageSendErrors++;
					ERROR("delayReq message can't be sent -> FAULTY state \n");
				}
			else
				{
					DBGV("DelayReq MSG sent ! \n");					//���ͳɹ�
					myprintf("UDP_IPV4 \n");	
					if(internalTime.seconds != 0) 
						{
							if (respectUtcOffset(rtOpts, ptpClock) == TRUE) 
								{
									internalTime.seconds += ptpClock->timePropertiesDS.currentUtcOffset;
								}			
							processDelayReqFromSelf(&internalTime, rtOpts, ptpClock); //���淢��ʱ���(T3)��׼������T4
						}
				}	
			ptpClock->sentDelayReqSequenceId++; 			//DelayReqID++
			ptpClock->counters.delayReqMessagesSent++;
		}
	else if(rtOpts->transport == IEEE_802_3)	
	{
		if (eth_send_ptp(ptpClock->msgObuf, DELAY_REQ_LENGTH, FALSE, &internalTime) != ERR_OK)	//IEEE_802_3ģʽ����
			{
				ERROR("IEEE_802_3 delayReq message can't be sent\n");		//����ʧ��
				toState(PTP_FAULTY,rtOpts,ptpClock);
			}
		else
			{
				DBGV("IEEE_802_3 DelayReq MSG sent !\n");			//���ͳɹ�
				if(internalTime.seconds != 0) 
					{
						if (respectUtcOffset(rtOpts, ptpClock) == TRUE)
							{
								internalTime.seconds += ptpClock->timePropertiesDS.currentUtcOffset;
							}			
						processDelayReqFromSelf(&internalTime, rtOpts, ptpClock);//���淢��ʱ���(T3)��׼������T4
					}
				ptpClock->sentDelayReqSequenceId++;//����ID++
				ptpClock->counters.delayReqMessagesSent++;
			}
	}
	/* From now on, we will only accept delayreq and
	 * delayresp of (sentDelayReqSequenceId - 1) */
	/* Explicitelly re-arm timer for sending the next delayReq */
	//��ʹ���������
//	timerStart_random(DELAYREQ_INTERVAL_TIMER,pow2ms(ptpClock->logMinDelayReqInterval),ptpClock->itimer);
}

/*Pack and send on event multicast ip adress a PDelayReq message*/
static void
issuePDelayReq(RunTimeOpts *rtOpts,PtpClock *ptpClock)
{
	Timestamp originTimestamp;
	TimeInternal internalTime;
	
	getTime(&internalTime);
	if (respectUtcOffset(rtOpts, ptpClock) == TRUE) {
		internalTime.seconds += ptpClock->timePropertiesDS.currentUtcOffset;
	}
	fromInternalTime(&internalTime,&originTimestamp);
	
	msgPackPDelayReq(ptpClock->msgObuf,&originTimestamp,ptpClock);
	
	if(rtOpts->transport == UDP_IPV4){
		if (!netSendPeerEvent(&ptpClock->netPath,ptpClock->msgObuf,PDELAY_REQ_LENGTH,&internalTime)) {
			toState(PTP_FAULTY,rtOpts,ptpClock);
			ptpClock->counters.messageSendErrors++;
			DBGV("PdelayReq message can't be sent -> FAULTY state \n");
		} else {
			DBGV("PDelayReq MSG sent ! \n");
			myprintf("UDP_IPV4 \n");
			if(internalTime.seconds != 0) {
				if (respectUtcOffset(rtOpts, ptpClock) == TRUE) {
					internalTime.seconds += ptpClock->timePropertiesDS.currentUtcOffset;
				}			
				processPDelayReqFromSelf(&internalTime, rtOpts, ptpClock);
			}
			
			ptpClock->sentPDelayReqSequenceId++;
			ptpClock->counters.pdelayReqMessagesSent++;
		}
	}else if(rtOpts->transport == IEEE_802_3){
		if (eth_send_ptp(ptpClock->msgObuf, PDELAY_REQ_LENGTH, TRUE, &internalTime) != ERR_OK)
		{
			ERROR("IEEE_802_3 DelayReq message can't be sent\n");
			toState(PTP_FAULTY,rtOpts,ptpClock);
		}
		else{
			DBGV("IEEE_802_3 DelayReq message sent !\n");
			myprintf("IEEE_802_3 \n");
			if(internalTime.seconds != 0) {
				if (respectUtcOffset(rtOpts, ptpClock) == TRUE) {
					internalTime.seconds += ptpClock->timePropertiesDS.currentUtcOffset;
				}			
				processPDelayReqFromSelf(&internalTime, rtOpts, ptpClock);
			}
			ptpClock->sentPDelayReqSequenceId++;
			ptpClock->counters.pdelayReqMessagesSent++;
		}
	}
}

/*Pack and send on event multicast ip adress a PDelayResp message*/
static void
issuePDelayResp(const TimeInternal *tint,MsgHeader *header,RunTimeOpts *rtOpts,
		PtpClock *ptpClock)
{
	Timestamp requestReceiptTimestamp;
	TimeInternal internalTime;
	
	fromInternalTime(tint,&requestReceiptTimestamp);
	msgPackPDelayResp(ptpClock->msgObuf,header,&requestReceiptTimestamp,ptpClock);

	if(rtOpts->transport == UDP_IPV4){
		if (!netSendPeerEvent(&ptpClock->netPath,ptpClock->msgObuf,PDELAY_RESP_LENGTH,&internalTime)) {
			toState(PTP_FAULTY,rtOpts,ptpClock);
			ptpClock->counters.messageSendErrors++;
			ERROR("PdelayResp message can't be sent -> FAULTY state \n");
		} else {
			DBGV("PDelayResp MSG sent ! \n");
			
			if(internalTime.seconds != 0) {
				if (respectUtcOffset(rtOpts, ptpClock) == TRUE) {
					internalTime.seconds += ptpClock->timePropertiesDS.currentUtcOffset;
				}			
				processPDelayRespFromSelf(&internalTime, rtOpts, ptpClock, header->sequenceId);
			}
			ptpClock->counters.pdelayRespMessagesSent++;
		}
	}else if(rtOpts->transport == IEEE_802_3){
		if (eth_send_ptp(ptpClock->msgObuf, PDELAY_RESP_LENGTH, TRUE, &internalTime) != ERR_OK)
		{
			ERROR("IEEE_802_3 issuePDelayReq: can't sent\n");
	    toState(PTP_FAULTY,rtOpts,ptpClock);
		}else{
			DBGV("PDelayResp MSG sent ! \n");
			
			if(internalTime.seconds != 0) {
				if (respectUtcOffset(rtOpts, ptpClock) == TRUE) {
					internalTime.seconds += ptpClock->timePropertiesDS.currentUtcOffset;
				}			
				processPDelayRespFromSelf(&internalTime, rtOpts, ptpClock, header->sequenceId);
			}
		}
	}
}


/*Pack and send on event multicast ip adress a DelayResp message*/
static void
issueDelayResp(const TimeInternal *tint,MsgHeader *header,RunTimeOpts *rtOpts, PtpClock *ptpClock)
{	
//	Integer32 dst = 0;
	Timestamp requestReceiptTimestamp;
	fromInternalTime(tint,&requestReceiptTimestamp);
	msgPackDelayResp(ptpClock->msgObuf,header,&requestReceiptTimestamp,ptpClock);

	if(rtOpts->transport == UDP_IPV4){
		if (!netSendGeneral(&ptpClock->netPath,ptpClock->msgObuf, DELAY_RESP_LENGTH)) {
			toState(PTP_FAULTY,rtOpts,ptpClock);
			//ptpClock->counters.messageSendErrors++;
			DBGV("DelayResp message can't be sent -> FAULTY state \n");
		} else {
			DBGV("DelayResp MSG sent ! \n");
			//ptpClock->counters.delayRespMessagesSent++;
		}
	}else if(rtOpts->transport == IEEE_802_3){
		if (eth_send_ptp(ptpClock->msgObuf, DELAY_RESP_LENGTH, FALSE,NULL) != ERR_OK)
		{		
			ERROR("IEEE_802_3 issueDelayResp: can't sent\n");
			toState(PTP_FAULTY,rtOpts,ptpClock);
		}else{
			DBGV("IEEE_802_3 issueDelayResp MSG sent ! \n");
		}
		
	}
}


static void
issuePDelayRespFollowUp(const TimeInternal *tint, MsgHeader *header,
			     RunTimeOpts *rtOpts, PtpClock *ptpClock, const UInteger16 sequenceId)
{
	Timestamp responseOriginTimestamp;
	fromInternalTime(tint,&responseOriginTimestamp);

	msgPackPDelayRespFollowUp(ptpClock->msgObuf,header,&responseOriginTimestamp,ptpClock, sequenceId);
	if(rtOpts->transport == UDP_IPV4){
		if (!netSendPeerGeneral(&ptpClock->netPath,ptpClock->msgObuf,PDELAY_RESP_FOLLOW_UP_LENGTH)) {
			toState(PTP_FAULTY,rtOpts,ptpClock);
			ptpClock->counters.messageSendErrors++;
			ERROR("PdelayRespFollowUp message can't be sent -> FAULTY state \n");
		} else {
			DBGV("PDelayRespFollowUp MSG sent ! \n");
			ptpClock->counters.pdelayRespFollowUpMessagesSent++;
		}
	}else if(rtOpts->transport == IEEE_802_3){
		if (eth_send_ptp(ptpClock->msgObuf, PDELAY_RESP_FOLLOW_UP_LENGTH, TRUE ,NULL) != ERR_OK)
		{		
			ERROR("IEEE_802_3 PDelayRespFollowUp: can't sent\n");
			toState(PTP_FAULTY,rtOpts,ptpClock);
		}else{
			DBGV("IEEE_802_3 PDelayRespFollowUp MSG sent ! \n");
		}
	}
}

#if 0
static void 
issueManagement(MsgHeader *header,MsgManagement *manage,RunTimeOpts *rtOpts,
		PtpClock *ptpClock)
{

	ptpClock->counters.managementMessagesSent++;

}
#endif

static void 
issueManagementRespOrAck(MsgManagement *outgoing, RunTimeOpts *rtOpts,
		PtpClock *ptpClock)
{
//	Integer32 dst = 0;

	/* pack ManagementTLV */
	msgPackManagementTLV( ptpClock->msgObuf, outgoing, ptpClock);

	/* set header messageLength, the outgoing->tlv->lengthField is now valid */
	outgoing->header.messageLength = MANAGEMENT_LENGTH +
					TL_LENGTH +
					outgoing->tlv->lengthField;

	msgPackManagement( ptpClock->msgObuf, outgoing, ptpClock);

	/* If LastSlaveAddr was populated, we reply via Unicast */
//	if (ptpClock->LastSlaveAddr > 0)
//		dst = ptpClock->LastSlaveAddr;

//	if(!netSendGeneral(ptpClock->msgObuf, outgoing->header.messageLength,
//			   &ptpClock->netPath, rtOpts, dst)) {
//		DBGV("Management response/acknowledge can't be sent -> FAULTY state \n");
//		ptpClock->counters.messageSendErrors++;
//		toState(PTP_FAULTY, rtOpts, ptpClock);
//	} else {
//		DBGV("Management response/acknowledge msg sent \n");
//		ptpClock->counters.managementMessagesSent++;
//	}
}

static void
issueManagementErrorStatus(MsgManagement *outgoing, RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
//	Integer32 dst = 0;

	/* pack ManagementErrorStatusTLV */
	msgPackManagementErrorStatusTLV( ptpClock->msgObuf, outgoing, ptpClock);

	/* set header messageLength, the outgoing->tlv->lengthField is now valid */
	outgoing->header.messageLength = MANAGEMENT_LENGTH +
					TL_LENGTH +
					outgoing->tlv->lengthField;

	msgPackManagement( ptpClock->msgObuf, outgoing, ptpClock);

//	if (ptpClock->LastSlaveAddr > 0)
//		dst = ptpClock->LastSlaveAddr;

//	if(!netSendGeneral(ptpClock->msgObuf, outgoing->header.messageLength,
//			   &ptpClock->netPath, rtOpts, dst)) {
//		DBGV("Management error status can't be sent -> FAULTY state \n");
//		ptpClock->counters.messageSendErrors++;
//		toState(PTP_FAULTY, rtOpts, ptpClock);
//	} else {
//		DBGV("Management error status msg sent \n");
//		ptpClock->counters.managementMessagesSent++;
//	}

}

void
addForeign(Octet *buf,MsgHeader *header,PtpClock *ptpClock)
{
	int i,j;
	Boolean found = FALSE;

	j = ptpClock->foreign_record_best;
	
	/*Check if Foreign master is already known*/
	for (i=0;i<ptpClock->number_foreign_records;i++) {
		if (!memcmp(header->sourcePortIdentity.clockIdentity,
			    ptpClock->foreign[j].foreignMasterPortIdentity.clockIdentity,
			    CLOCK_IDENTITY_LENGTH) && 
		    (header->sourcePortIdentity.portNumber == 
		     ptpClock->foreign[j].foreignMasterPortIdentity.portNumber))
		{
			/*Foreign Master is already in Foreignmaster data set*/
			ptpClock->foreign[j].foreignMasterAnnounceMessages++; 
			found = TRUE;
			DBGV("addForeign : AnnounceMessage incremented \n");
			msgUnpackHeader(buf,&ptpClock->foreign[j].header);
			msgUnpackAnnounce(buf,&ptpClock->foreign[j].announce);
			break;
		}
	
		j = (j+1)%ptpClock->number_foreign_records;
	}

	/*New Foreign Master*/
	if (!found) {
		if (ptpClock->number_foreign_records < 
		    ptpClock->max_foreign_records) {
			ptpClock->number_foreign_records++;
		}
		
		/* Preserve best master record from overwriting (sf FR #22) - use next slot*/
		if (ptpClock->foreign_record_i == ptpClock->foreign_record_best) {
			ptpClock->foreign_record_i++;
			ptpClock->foreign_record_i %= ptpClock->number_foreign_records;
		}
		
		j = ptpClock->foreign_record_i;
		
		/*Copy new foreign master data set from Announce message*/
		copyClockIdentity(ptpClock->foreign[j].foreignMasterPortIdentity.clockIdentity,
		       header->sourcePortIdentity.clockIdentity);
		ptpClock->foreign[j].foreignMasterPortIdentity.portNumber = 
			header->sourcePortIdentity.portNumber;
		ptpClock->foreign[j].foreignMasterAnnounceMessages = 0;
		
		/*
		 * header and announce field of each Foreign Master are
		 * usefull to run Best Master Clock Algorithm
		 */
		msgUnpackHeader(buf,&ptpClock->foreign[j].header);
		msgUnpackAnnounce(buf,&ptpClock->foreign[j].announce);
		DBGV("New foreign Master added \n");
		
		ptpClock->foreign_record_i = (ptpClock->foreign_record_i+1) % ptpClock->max_foreign_records;	
	}
}

//#if 0
///* Update dataset fields which are safe to change without going into INITIALIZING */
//static void
//updateDatasets(PtpClock* ptpClock, RunTimeOpts* rtOpts)
//{

//	switch(ptpClock->portState) {
//		/* We are master so update both the port and the parent dataset */
//		case PTP_MASTER:
//			ptpClock->numberPorts = NUMBER_PORTS;
//			ptpClock->delayMechanism = rtOpts->delayMechanism;
//			ptpClock->versionNumber = VERSION_PTP;

//			ptpClock->clockQuality.clockAccuracy = 
//				rtOpts->clockQuality.clockAccuracy;
//			ptpClock->clockQuality.clockClass = rtOpts->clockQuality.clockClass;
//			ptpClock->clockQuality.offsetScaledLogVariance = 
//				rtOpts->clockQuality.offsetScaledLogVariance;
//			ptpClock->priority1 = rtOpts->priority1;
//			ptpClock->priority2 = rtOpts->priority2;

//			ptpClock->grandmasterClockQuality.clockAccuracy = 
//				ptpClock->clockQuality.clockAccuracy;
//			ptpClock->grandmasterClockQuality.clockClass = 
//				ptpClock->clockQuality.clockClass;
//			ptpClock->grandmasterClockQuality.offsetScaledLogVariance = 
//				ptpClock->clockQuality.offsetScaledLogVariance;
//			ptpClock->clockQuality.clockAccuracy = 
//			ptpClock->grandmasterPriority1 = ptpClock->priority1;
//			ptpClock->grandmasterPriority2 = ptpClock->priority2;
//			ptpClock->timePropertiesDS.currentUtcOffsetValid = rtOpts->timeProperties.currentUtcOffsetValid;
//			ptpClock->timePropertiesDS.currentUtcOffset = rtOpts->timeProperties.currentUtcOffset;
//			ptpClock->timePropertiesDS.timeTraceable = rtOpts->timeProperties.timeTraceable;
//			ptpClock->timePropertiesDS.frequencyTraceable = rtOpts->timeProperties.frequencyTraceable;
//			ptpClock->timePropertiesDS.ptpTimescale = rtOpts->timeProperties.ptpTimescale;
//			ptpClock->timePropertiesDS.timeSource = rtOpts->timeProperties.timeSource;
//			ptpClock->logAnnounceInterval = rtOpts->announceInterval;
//			ptpClock->announceReceiptTimeout = rtOpts->announceReceiptTimeout;
//			ptpClock->logSyncInterval = rtOpts->syncInterval;
//			ptpClock->logMinPdelayReqInterval = rtOpts->logMinPdelayReqInterval;
//			ptpClock->logMinDelayReqInterval = rtOpts->initial_delayreq;
//			break;
//		/*
//		 * we are not master so update the port dataset only - parent will be updated
//		 * by m1() if we go master - basically update the fields affecting BMC only
//		 */
//		case PTP_SLAVE:
//      if(ptpClock->logMinDelayReqInterval == UNICAST_MESSAGEINTERVAL &&
//				rtOpts->autoDelayReqInterval) {
//					NOTICE("Running at %d Delay Interval (unicast) - overriding with %d\n",
//					ptpClock->logMinDelayReqInterval, rtOpts->subsequent_delayreq);
//					ptpClock->logMinDelayReqInterval = rtOpts->subsequent_delayreq;
//				}
//		case PTP_PASSIVE:
//			ptpClock->numberPorts = NUMBER_PORTS;
//			ptpClock->delayMechanism = rtOpts->delayMechanism;
//			ptpClock->versionNumber = VERSION_PTP;
//			ptpClock->clockQuality.clockAccuracy = 
//				rtOpts->clockQuality.clockAccuracy;
//			ptpClock->clockQuality.clockClass = rtOpts->clockQuality.clockClass;
//			ptpClock->clockQuality.offsetScaledLogVariance = 
//				rtOpts->clockQuality.offsetScaledLogVariance;
//			ptpClock->priority1 = rtOpts->priority1;
//			ptpClock->priority2 = rtOpts->priority2;
//			break;
//		/* In all other states the datasets will be updated when going into an operational state */
//		default:
//		    break;
//	}

//}
//#endif


/* check and handle received messages */
void handle_ptp_eth(RunTimeOpts *rtOpts,PtpClock *ptpClock,TimeInternal *timeStamp, ssize_t length)
{
	Boolean isFromSelf;
#ifdef PTPD_DBG
	char *st;
#endif
	
  DBGV("handle: PTP IEEE_802_3 something\n");
	
	if(length <= 0)
	{
		ERROR("failed to receive on the IEEE_802_3 socket");
		toState(PTP_FAULTY, rtOpts, ptpClock);
    return;
	}
	
	ptpClock->message_activity = TRUE;		//msg�����Ч
	
	if (length < HEADER_LENGTH)
	{
		ERROR("handle: message shorter than header length\n");
		toState(PTP_FAULTY, rtOpts, ptpClock);
    return;
	}
	

	DBGV("__UTC_offset: %d %d \n", ptpClock->timePropertiesDS.currentUtcOffsetValid, ptpClock->timePropertiesDS.currentUtcOffset);
	if (respectUtcOffset(rtOpts, ptpClock) == TRUE) {
			timeStamp->seconds += ptpClock->timePropertiesDS.currentUtcOffset;
	}
	
	msgUnpackHeader(ptpClock->msgIbuf, &ptpClock->msgTmpHeader);

	if (ptpClock->msgTmpHeader.versionPTP != ptpClock->versionNumber) {
		DBG("ignore version %d message\n", ptpClock->msgTmpHeader.versionPTP);
		return;
	}

	if(ptpClock->msgTmpHeader.domainNumber != ptpClock->domainNumber) {
		DBG("ignore message from domainNumber %d\n", ptpClock->msgTmpHeader.domainNumber);
		return;
	}
	
		//�����ظ���
//	printf("ptpClock->msgTmpHeader.sequenceId=%d\n",ptpClock->msgTmpHeader.sequenceId);
	if(handle_Repeat(ptpClock) == 1)
	{
		ERROR("handle: Packets to repeat\n");
		return ;
	}
	
	/*Spec 9.5.2.2*/
	isFromSelf = (Boolean)(ptpClock->portIdentity.portNumber == ptpClock->msgTmpHeader.sourcePortIdentity.portNumber \
			&& !memcmp(ptpClock->msgTmpHeader.sourcePortIdentity.clockIdentity, ptpClock->portIdentity.clockIdentity, CLOCK_IDENTITY_LENGTH));

	/*
	 * subtract the inbound latency adjustment if it is not a loop
	 *  back and the time stamp seems reasonable 
	 */
	if (!isFromSelf && timeStamp->seconds > 0)
			subTime(timeStamp, timeStamp, &rtOpts->inboundLatency);
#ifdef PTPD_DBG
    /* easy display of received messages */
    switch(ptpClock->msgTmpHeader.messageType)
    {
    case ANNOUNCE:
	st = "Announce";
	break;
    case SYNC:
	st = "Sync";
	break;
    case FOLLOW_UP:
	st = "FollowUp";
	break;
    case DELAY_REQ:
	st = "DelayReq";
	break;
    case DELAY_RESP:
	st = "DelayResp";
	break;
    case PDELAY_REQ:
	st = "PDelayReq";
	break;
    case PDELAY_RESP:
	st = "PDelayResp";
	break;
    case PDELAY_RESP_FOLLOW_UP:
	st = "PDelayRespFollowUp";
	break;	
    case MANAGEMENT:
	st = "Management";
	break;
    default:
	st = "Unknow";
	break;
    }
    DBG("      ==> %s IEEE_802_3 received, sequence %d\n", st, ptpClock->msgTmpHeader.sequenceId);
//		printf("      ==> %s IEEE_802_3 received, sequence %d\n", st, ptpClock->msgTmpHeader.sequenceId);
#endif

   
		switch(ptpClock->msgTmpHeader.messageType)
    {
    case ANNOUNCE:
	handleAnnounce(&ptpClock->msgTmpHeader,length,isFromSelf,rtOpts,ptpClock);
	break;
    case SYNC:
	handleSync(&ptpClock->msgTmpHeader,length, timeStamp, isFromSelf, rtOpts, ptpClock);
	break;
    case FOLLOW_UP:
	handleFollowUp(&ptpClock->msgTmpHeader,length,isFromSelf,rtOpts,ptpClock);
	break;
    case DELAY_REQ:
	handleDelayReq(&ptpClock->msgTmpHeader,length,timeStamp,isFromSelf,rtOpts,ptpClock);
	break;
    case PDELAY_REQ:
	handlePDelayReq(&ptpClock->msgTmpHeader,length,timeStamp,isFromSelf,rtOpts,ptpClock);
	break;  
    case DELAY_RESP:
	handleDelayResp(&ptpClock->msgTmpHeader,length,rtOpts,ptpClock);
	break;
    case PDELAY_RESP:
	handlePDelayResp(&ptpClock->msgTmpHeader,timeStamp,length,isFromSelf,rtOpts,ptpClock);
	break;
    case PDELAY_RESP_FOLLOW_UP:
	handlePDelayRespFollowUp(&ptpClock->msgTmpHeader,length,isFromSelf,rtOpts,ptpClock);
	break;
    case MANAGEMENT:
	handleManagement(&ptpClock->msgTmpHeader,isFromSelf, rtOpts, ptpClock);
	break;
    case SIGNALING:
	handleSignaling(ptpClock);
	break;
    default:
	DBG("handle: unrecognized message\n");
	break;
    }

  if (rtOpts->displayPackets)
		msgDump(ptpClock);
}

//�����ظ����ݰ�
static int handle_Repeat(PtpClock *ptpClock)
{
	switch(ptpClock->msgTmpHeader.messageType)
	{
		case SYNC: 
			if(id[0] == ptpClock->msgTmpHeader.sequenceId)
			{
				return 1;
			}
			else {id[0] = ptpClock->msgTmpHeader.sequenceId;} 
			break;
			
		case DELAY_REQ: 
			if(id[1] == ptpClock->msgTmpHeader.sequenceId)
			{
				return 1;
			}
			else {id[1] = ptpClock->msgTmpHeader.sequenceId;} 
			break;
		case PDELAY_REQ: 
			if(id[2] == ptpClock->msgTmpHeader.sequenceId)
			{
				return 1;
			}
			else {id[2] = ptpClock->msgTmpHeader.sequenceId;} 
			break;
		case PDELAY_RESP: 
			if(id[3] == ptpClock->msgTmpHeader.sequenceId)
			{
				return 1;
			}
			else {id[3] = ptpClock->msgTmpHeader.sequenceId;} 
			break;
		case FOLLOW_UP: 
			if(id[4] == ptpClock->msgTmpHeader.sequenceId)
			{

				return 1;
			}
			else {id[4] = ptpClock->msgTmpHeader.sequenceId;} 
			break;
		case DELAY_RESP: 
			if(id[5] == ptpClock->msgTmpHeader.sequenceId)
			{
				return 1;
			}
			else {id[5] = ptpClock->msgTmpHeader.sequenceId;} 
			break;
		case PDELAY_RESP_FOLLOW_UP: 
			if(id[6] == ptpClock->msgTmpHeader.sequenceId)
			{
				return 1;
			}
			else {id[6] = ptpClock->msgTmpHeader.sequenceId;} 
			break;
		case ANNOUNCE: 
			if(id[7] == ptpClock->msgTmpHeader.sequenceId)
			{
				return 1;
			}
			else {id[7] = ptpClock->msgTmpHeader.sequenceId;} 
			break;
		default: 
			break;
	}
	return 0;
}

void
clearCounters(PtpClock * ptpClock)
{
	/* TODO: print port info */
	DBG("Port counters cleared\n");
	memset(&ptpClock->counters, 0, sizeof(ptpClock->counters));
}

Boolean
respectUtcOffset(RunTimeOpts * rtOpts, PtpClock * ptpClock) {
	if (ptpClock->timePropertiesDS.currentUtcOffsetValid || rtOpts->alwaysRespectUtcOffset) {
		return TRUE;
	}
	return FALSE;
}


