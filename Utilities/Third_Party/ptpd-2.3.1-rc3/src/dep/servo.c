/*-
 * Copyright (c) 2012-2013 Wojciech Owczarek,
 * Copyright (c) 2011-2012 George V. Neville-Neil,
 *                         Steven Kreuzer, 
 *                         Martin Burnicki, 
 *                         Jan Breuer,
 *                         Gael Mace, 
 *                         Alexandre Van Kempen,
 *                         Inaqui Delgado,
 *                         Rick Ratzel,
 *                         National Instruments.
 * Copyright (c) 2009-2010 George V. Neville-Neil, 
 *                         Steven Kreuzer, 
 *                         Martin Burnicki, 
 *                         Jan Breuer,
 *                         Gael Mace, 
 *                         Alexandre Van Kempen
 *
 * Copyright (c) 2005-2008 Kendall Correll, Aidan Williams
 *
 * All Rights Reserved
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
 * @file   servo.c
 * @date   Tue Jul 20 16:19:19 2010
 * 
 * @brief  Code which implements the clock servo in software.
 * 
 * 
 */

#include "../ptpd.h"
#include "share.h"



extern unsigned char synflags;



static Integer32 order(Integer32 n)
{
    if (n < 0) {
        n = -n;
    }
    if (n == 0) {
        return 0;
    }
    return floorLog2(n);
}

/* exponencial smoothing */			//ָ�������˲��㷨
static void filter(Integer32 * nsec_current, Filter * filt)
{
    Integer32 s, s2;
    /*
        using floatingpoint math
        alpha = 1/2^s
        y[1] = x[0]
        y[n] = alpha * x[n-1] + (1-alpha) * y[n-1]
        
        or equivalent with integer math
        y[1] = x[0]
        y_sum[1] = y[1] * 2^s
        y_sum[n] = y_sum[n-1] + x[n-1] - y[n-1]
        y[n] = y_sum[n] / 2^s
    */
    filt->n++; /* increment number of samples */		//������+1 
    /* if it is first time, we are running filter, initialize it*/
    if (filt->n == 1)
			{
				filt->y_prev = *nsec_current;	//��1��������y_prev����	=	��ǰֵ
				filt->y_sum = *nsec_current;	//��1��������y_sum	����	=	��ǰֵ
				filt->s_prev = 0;							//��1��������s_prev����	=	0
			}
    s = filt->s;
    if ((1<<s) > filt->n) 						//speedup filter, if not 2^s > n 
			{
				 s = order(filt->n);					//lower the filter order
			} 
		else 
			{		
				 filt->n = 1<<s;							//avoid overflowing of n 
			}
    /* avoid overflowing of filter. 30 is because using signed 32bit integers */
    s2 = 30 - order(max(filt->y_prev, *nsec_current));

    /* use the lower filter order, higher will overflow */
    s = min(s, s2);

    /* if the order of the filter changed, change also y_sum value */
    if(filt->s_prev > s) 
			{
				filt->y_sum >>= (filt->s_prev - s);
			} 
		else if (filt->s_prev < s) 
			{
				filt->y_sum <<= (s - filt->s_prev);
			}

    /* compute the filter itself */
    filt->y_sum += *nsec_current - filt->y_prev;
    filt->y_prev = filt->y_sum >> s;

    /* save previous order of the filter */
    filt->s_prev = s;

    DBGV("filter: %d -> %d (%d)\n", *nsec_current, filt->y_prev, s);

    /* actualize target value */
    *nsec_current = filt->y_prev;		//�����㷨�Ľ��
}

void
reset_operator_messages(RunTimeOpts * rtOpts, PtpClock * ptpClock)
{
//	ptpClock->warned_operator_slow_slewing = 0;
//	ptpClock->warned_operator_fast_slewing = 0;

	//ptpClock->seen_servo_stable_first_time = FALSE;
}

void
initClock(RunTimeOpts * rtOpts, PtpClock * ptpClock)
{
	DBG("initClock\n");

	/* do not reset frequency here - restoreDrift will do it if necessary */
	//ptpClock->observedDrift = 0; //�������Ҫ����������ָ�������ۼ�ƫ��
	
	/* clear vars */
	/* clean more original filter variables */
	clearTime(&ptpClock->offsetFromMaster);	//offset				=	0
	clearTime(&ptpClock->meanPathDelay);		//meanPathDelay	=	0
	clearTime(&ptpClock->delaySM);					//Tsm						=	0
	clearTime(&ptpClock->delayMS);					//Tms						=	0

	/* one way delay */
	ptpClock->owd_filt.n = 0;												//owd_filt������		=	0
	ptpClock->owd_filt.s = ptpClock->servo.sDelay; 	//owd_filt	ƽ����	=	servo.sDelay

	/* offset from master */
	ptpClock->ofm_filt.n = 0;												//ofm_filt������		=	0
	ptpClock->ofm_filt.s = ptpClock->servo.sOffset;	//ofm_filt ƽ����	=	servo.sOffset
	
		/* scaled log variance */
	if (DEFAULT_PARENTS_STATS)
	{
		ptpClock->slv_filt.n = 0;											//slv_filt������		=	0
		ptpClock->slv_filt.s = 6;
		ptpClock->offsetHistory[0] = 0;
		ptpClock->offsetHistory[1] = 0;
	}
	
	/* reset parent statistics */
	ptpClock->parentStats = FALSE;
	ptpClock->observedParentClockPhaseChangeRate = 0;
	ptpClock->observedParentOffsetScaledLogVariance = 0;
	
//	if (!rtOpts->noAdjust)
//        adjFreq(0);
	
	netEmptyEventQ(&ptpClock->netPath);
}

void
updateDelay(Filter * owd_filt, RunTimeOpts * rtOpts, PtpClock * ptpClock, TimeInternal * correctionField)
{
/* updates paused, leap second pending - do nothing */
	if(ptpClock->leapSecondInProgress)	//������̣���������
		return;
	
	DBGV("updateDelay\n");
	//�������ǵ�ʱ�����ж��������
	if (0 == ptpClock->ofm_filt.n)			//ofm�˲�����������==0��������
	{
		//printf("updateDelay: Tms is not valid");
		return;
	}
	DBG("==>UpdateDelay(): ""Req_RECV:"" %ds.%dns\n",ptpClock->delay_req_receive_time.seconds, ptpClock->delay_req_receive_time.nanoseconds);
	DBG("==>UpdateDelay(): ""Req_SENT:"" %ds.%dns\n",ptpClock->delay_req_send_time.seconds, ptpClock->delay_req_send_time.nanoseconds);
	
	subTime(&ptpClock->delaySM, &ptpClock->delay_req_receive_time,&ptpClock->delay_req_send_time);	//Tsm	=	T4-T3
		
	addTime(&ptpClock->meanPathDelay, &ptpClock->delaySM,&ptpClock->delayMS);												//Tsm+Tms	

	subTime(&ptpClock->meanPathDelay,&ptpClock->meanPathDelay,correctionField);											//+������ʱ��

	/* Compute one-way delay */
	div2Time(&ptpClock->meanPathDelay);																															//�ٳ���2���õ�meanPathDelay
	
	//printf(" PathDelay=%d ",ptpClock->meanPathDelay.nanoseconds);
	if (ptpClock->meanPathDelay.seconds)																														//�����뼶���������˲���
		{
			INFO("Servo: Ignoring delayResp because of large OFM\n");
			return;
		}
	else if(ptpClock->meanPathDelay.nanoseconds < 0)																								//���������ӳ٣�Ҳ�������˲���
		{
			DBG("update delay: found negative value for OWD,so ignoring this value: %d\n",ptpClock->meanPathDelay.nanoseconds);
			return;
		}
	else
		{
			//printf("Path_Delay1=%d",ptpClock->meanPathDelay.nanoseconds);
			filter(&ptpClock->meanPathDelay.nanoseconds, &ptpClock->owd_filt);
			//printf("Path_Delay2=%d",ptpClock->meanPathDelay.nanoseconds);
		}
/* Update relevant statistics containers, feed outlier filter thresholds etc. */
}

void
updatePeerDelay(RunTimeOpts * rtOpts, PtpClock * ptpClock, TimeInternal * correctionField, Boolean twoStep)
{
	/* updates paused, leap second pending - do nothing */
	if(ptpClock->leapSecondInProgress)
		return;

	DBGV("updatePeerDelay\n");

	if (twoStep) {
		/* calc 'slave_to_master_delay' */
		subTime(&ptpClock->pdelayMS,&ptpClock->pdelay_resp_receive_time,&ptpClock->pdelay_resp_send_time);
		subTime(&ptpClock->pdelaySM,&ptpClock->pdelay_req_receive_time,&ptpClock->pdelay_req_send_time);
		/* update 'one_way_delay' */
		addTime(&ptpClock->peerMeanPathDelay,&ptpClock->pdelayMS,&ptpClock->pdelaySM);
		/* Substract correctionField */
		subTime(&ptpClock->peerMeanPathDelay,&ptpClock->peerMeanPathDelay, correctionField);
	} else {
		/* One step clock */
		subTime(&ptpClock->peerMeanPathDelay,&ptpClock->pdelay_resp_receive_time,&ptpClock->pdelay_req_send_time);
		/* Substract correctionField */
		subTime(&ptpClock->peerMeanPathDelay,&ptpClock->peerMeanPathDelay, correctionField);
	}
	/* Compute one-way delay */
	div2Time(&ptpClock->peerMeanPathDelay);
	
	if(ptpClock->peerMeanPathDelay.seconds != 0)
	{
		
		DBGV("updatePeerDelay: cannot filter with seconds");
        return;
	}
	
	filter(&ptpClock->peerMeanPathDelay.nanoseconds, &ptpClock->owd_filt);
}

void
updateOffset(TimeInternal * send_time, TimeInternal * recv_time,
    Filter * ofm_filt, RunTimeOpts * rtOpts, PtpClock * ptpClock, TimeInternal * correctionField)
{	
	static  uint8_t b;			
	static  char i = 0;	
	uint32_t offnansec;	
	
	DBGV("==> updateOffset\n");
	subTime(&ptpClock->delayMS, recv_time, send_time);								//Tms=T2-T1			/* Used just for End to End mode. ֻ��E2E����*/
	subTime(&ptpClock->delayMS,&ptpClock->delayMS, correctionField);	//�ڼ�ȥ������		/* Take care of correctionField */
	if (ptpClock->delayMechanism == P2P) 															
		{
			subTime(&ptpClock->offsetFromMaster,&ptpClock->delayMS,&ptpClock->peerMeanPathDelay);		//P2Pģʽ	�� - peerMeanPathDelay
		} 
	else if (ptpClock->delayMechanism == E2E || ptpClock->delayMechanism == DELAY_DISABLED ) 		
		{
			subTime(&ptpClock->offsetFromMaster,&ptpClock->delayMS,&ptpClock->meanPathDelay);				//E2Eģʽ	�� - meanPathDelay
		}
	//printf("Off1=%d,n=%d\n",ptpClock->offsetFromMaster.nanoseconds,ptpClock->owd_filt.n);	
				
	offnansec	= abs(ptpClock->offsetFromMaster.nanoseconds);
	if(b <= 7) 					
		{
			if(i <= 10) 		//��̬��δ����
				{
					if(offnansec < 300)
						i++;	
					else if(i>0)
						i--;
				}
			else 						//��̬�Ѿ�����
				{	
					if(ptpClock->offsetFromMaster.seconds)	
						{
							b+=2;		//�����뼶�����Ĵ��������ȶ�����+2
							ptpClock->offsetFromMaster.seconds = 0;		 //���Դ˴��벻����
							ptpClock->offsetFromMaster.nanoseconds = 0; 
						}
					else if((offnansec<500)&&(b>0))
						{
							b--;
						}
					else if(offnansec > 5000	)
						{
							b++; 		//���ִ󲨶������ȶ�����+1
							ptpClock->offsetFromMaster.nanoseconds = 0;  //���Դ˴�żȻ�Ĳ���
						}	
				}
		}
	else  							//���ȶ�����>7,��̬�Ѿ��ƻ�
		{
			i = 0;
			b = 0;
		}		
	if(ptpClock->leapSecondInProgress)			//�����룬�����˲�
		return;
	if (ptpClock->offsetFromMaster.seconds)	//����s�������˲�
		return;
	//printf("Off1=%d,",ptpClock->offsetFromMaster.nanoseconds);
	filter(&ptpClock->offsetFromMaster.nanoseconds, &ptpClock->ofm_filt);	//�˲�
	//printf("Off2=%d,",ptpClock->offsetFromMaster.nanoseconds);
}

/*��ʱ�ӵ���ʱ��*/
void updateClock(RunTimeOpts * rtOpts,PtpClock *ptpClock)
{
    Integer32 adj;
    TimeInternal timeTmp;
    Integer32 offsetNorm;

    DBGV("updateClock\n");
	  //printf("offset=%ds.%dns",ptpClock->offsetFromMaster.seconds,ptpClock->offsetFromMaster.nanoseconds);
	
    if (0 != ptpClock->offsetFromMaster.seconds || abs(ptpClock->offsetFromMaster.nanoseconds)>MAX_ADJ_OFFSET_NS )//ʱ��ƫ����s������ns����MAX_ADJ_OFFSET_NS(10ms)
			{
				if (!rtOpts->noAdjust)				//�����˵���ʱ������ã�(�����ǣ�noAdjust=1��������)
				{
					if (!rtOpts->noResetClock)	//����һ��������λ������
					{
						getTime(&timeTmp);																					//ȡӲ����׼ʱ��
						subTime(&timeTmp, &timeTmp, &ptpClock->offsetFromMaster);		//תUTCʱ��
						setTime(&timeTmp);																					//���ó�ϵͳʱ��
						initClock(rtOpts,ptpClock);																	//��ձ����ʱ��ƫ�����ݼ�
					}
					else												//�𲽣�����:ADJ_FREQ_MAX������ʱ��
					{
						adj = ptpClock->offsetFromMaster.nanoseconds > 0 ? ADJ_FREQ_MAX : -ADJ_FREQ_MAX;
						adjFreq(-adj);						//����PTPʱ��������Ĵ�����ע�⺯�������ظ���2���ж���ADJ_FREQ_MAX�Ĵ�С��ϵ����֤����Ծ���
					}
				}
			}
    else	//��ȷ������<MAX_ADJ_OFFSET_NS��
			{
					/* the PI controller */
					/* normalize offset to 1s sync interval -> response of the servo will
					 * be same for all sync interval values, but faster/slower 
					 * (possible lost of precision/overflow but much more stable) */
					offsetNorm = ptpClock->offsetFromMaster.nanoseconds;
					if(abs(offsetNorm) <= 200)
						STM_LEDon(mLED2);		//ƫ��С��200ns��LED2
					else 
						STM_LEDoff(mLED2);	//����200ns��LED2
					if (ptpClock->logSyncInterval > 0) 
						{
							offsetNorm >>= ptpClock->logSyncInterval;		//����ƫ��	=	ÿ��ƫ��*ͬ�����
						} 
					else if (ptpClock->logSyncInterval < 0)
						{
							offsetNorm <<= -ptpClock->logSyncInterval;	//����ƫ��	=	ÿ��ƫ��*ͬ�����
						}
					
					/* the accumulator for the I component */
					ptpClock->observedDrift += offsetNorm / ptpClock->servo.ai;	//�ۼ�ƫ��ֵ	=	ƫ��ֵ/����ϵ����Ĭ��16��

					/* clamp the accumulator to ADJ_FREQ_MAX for sanity */
					if (ptpClock->observedDrift > ADJ_FREQ_MAX)
							ptpClock->observedDrift = ADJ_FREQ_MAX;					//�ۼ�ƫ��ֵ����������ƣ��𲽣�����������	
					else if (ptpClock->observedDrift < -ADJ_FREQ_MAX)
							ptpClock->observedDrift = -ADJ_FREQ_MAX;				//�ۼ�ƫ��ֵ����������ƣ������򣩣��𲽣�����������	
					
					//printf("ptpClock->observedDrift %9d\n",ptpClock->observedDrift);
					/* apply controller output as a clock tick rate adjustment */
					if ( !rtOpts->noAdjust ) //�����˵���ʱ������ã�(�����ǣ�noAdjust=1��������)
						{
							adj = offsetNorm / ptpClock->servo.ap + ptpClock->observedDrift;	//����ֵ	=	����ƫ��/����ϵ����Ĭ��2��+	�ۼ�ƫ��ֵ
							adjFreq(-adj);			//����PTPʱ��������Ĵ���
						}
			}
			//    SWITCH (PTPCLOCK->DELAYMECHANISM)
			//			{
			//				CASE E2E:
			//						DBG("UPDATECLOCK: ONE-WAY DELAY AVERAGED (E2E):  %10DS %11DNS\N",PTPCLOCK->MEANPATHDELAY.SECONDS, PTPCLOCK->MEANPATHDELAY.NANOSECONDS);
			//						BREAK;
			//				CASE P2P:
			//						DBG("UPDATECLOCK: ONE-WAY DELAY AVERAGED (P2P):  %10DS %11DNS\N",	PTPCLOCK->PEERMEANPATHDELAY.SECONDS, PTPCLOCK->PEERMEANPATHDELAY.NANOSECONDS);
			//						BREAK;
			//				DEFAULT:
			//						DBG("UPDATECLOCK: ONE-WAY DELAY NOT COMPUTED\N");
			//						BREAK;
			//			}
			//   DBG("UPDATECLOCK: OFFSET FROM MASTER:      %10DS %11DNS\N",PTPCLOCK->OFFSETFROMMASTER.SECONDS, PTPCLOCK->OFFSETFROMMASTER.NANOSECONDS);
			//   DBG("UPDATECLOCK: OBSERVED DRIFT:          %10D\N", PTPCLOCK->OBSERVEDDRIFT);
}



