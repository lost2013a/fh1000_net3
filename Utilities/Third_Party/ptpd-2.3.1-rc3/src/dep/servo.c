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

/* exponencial smoothing */			//指数趋近滤波算法
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
    filt->n++; /* increment number of samples */		//样本数+1 
    /* if it is first time, we are running filter, initialize it*/
    if (filt->n == 1)
			{
				filt->y_prev = *nsec_current;	//第1个样本，y_prev参数	=	当前值
				filt->y_sum = *nsec_current;	//第1个样本，y_sum	参数	=	当前值
				filt->s_prev = 0;							//第1个样本，s_prev参数	=	0
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
    *nsec_current = filt->y_prev;		//返回算法的结果
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
	//ptpClock->observedDrift = 0; //如果有需要可以在这里恢复保存的累计偏移
	
	/* clear vars */
	/* clean more original filter variables */
	clearTime(&ptpClock->offsetFromMaster);	//offset				=	0
	clearTime(&ptpClock->meanPathDelay);		//meanPathDelay	=	0
	clearTime(&ptpClock->delaySM);					//Tsm						=	0
	clearTime(&ptpClock->delayMS);					//Tms						=	0

	/* one way delay */
	ptpClock->owd_filt.n = 0;												//owd_filt样本数		=	0
	ptpClock->owd_filt.s = ptpClock->servo.sDelay; 	//owd_filt	平均数	=	servo.sDelay

	/* offset from master */
	ptpClock->ofm_filt.n = 0;												//ofm_filt样本数		=	0
	ptpClock->ofm_filt.s = ptpClock->servo.sOffset;	//ofm_filt 平均数	=	servo.sOffset
	
		/* scaled log variance */
	if (DEFAULT_PARENTS_STATS)
	{
		ptpClock->slv_filt.n = 0;											//slv_filt样本数		=	0
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
	if(ptpClock->leapSecondInProgress)	//闰秒过程，不做调整
		return;
	
	DBGV("updateDelay\n");
	//做测试仪的时候不能判断这个条件
	if (0 == ptpClock->ofm_filt.n)			//ofm滤波器中样本数==0，不处理
	{
		//printf("updateDelay: Tms is not valid");
		return;
	}
	DBG("==>UpdateDelay(): ""Req_RECV:"" %ds.%dns\n",ptpClock->delay_req_receive_time.seconds, ptpClock->delay_req_receive_time.nanoseconds);
	DBG("==>UpdateDelay(): ""Req_SENT:"" %ds.%dns\n",ptpClock->delay_req_send_time.seconds, ptpClock->delay_req_send_time.nanoseconds);
	
	subTime(&ptpClock->delaySM, &ptpClock->delay_req_receive_time,&ptpClock->delay_req_send_time);	//Tsm	=	T4-T3
		
	addTime(&ptpClock->meanPathDelay, &ptpClock->delaySM,&ptpClock->delayMS);												//Tsm+Tms	

	subTime(&ptpClock->meanPathDelay,&ptpClock->meanPathDelay,correctionField);											//+修正域时间

	/* Compute one-way delay */
	div2Time(&ptpClock->meanPathDelay);																															//再除以2，得到meanPathDelay
	
	//printf(" PathDelay=%d ",ptpClock->meanPathDelay.nanoseconds);
	if (ptpClock->meanPathDelay.seconds)																														//大于秒级，不带入滤波器
		{
			INFO("Servo: Ignoring delayResp because of large OFM\n");
			return;
		}
	else if(ptpClock->meanPathDelay.nanoseconds < 0)																								//负的网络延迟，也不计入滤波器
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
	subTime(&ptpClock->delayMS, recv_time, send_time);								//Tms=T2-T1			/* Used just for End to End mode. 只在E2E中用*/
	subTime(&ptpClock->delayMS,&ptpClock->delayMS, correctionField);	//在减去修正域		/* Take care of correctionField */
	if (ptpClock->delayMechanism == P2P) 															
		{
			subTime(&ptpClock->offsetFromMaster,&ptpClock->delayMS,&ptpClock->peerMeanPathDelay);		//P2P模式	再 - peerMeanPathDelay
		} 
	else if (ptpClock->delayMechanism == E2E || ptpClock->delayMechanism == DELAY_DISABLED ) 		
		{
			subTime(&ptpClock->offsetFromMaster,&ptpClock->delayMS,&ptpClock->meanPathDelay);				//E2E模式	再 - meanPathDelay
		}
	//printf("Off1=%d,n=%d\n",ptpClock->offsetFromMaster.nanoseconds,ptpClock->owd_filt.n);	
				
	offnansec	= abs(ptpClock->offsetFromMaster.nanoseconds);
	if(b <= 7) 					
		{
			if(i <= 10) 		//稳态还未建立
				{
					if(offnansec < 300)
						i++;	
					else if(i>0)
						i--;
				}
			else 						//稳态已经建立
				{	
					if(ptpClock->offsetFromMaster.seconds)	
						{
							b+=2;		//出现秒级波动的次数，不稳定计数+2
							ptpClock->offsetFromMaster.seconds = 0;		 //忽略此次秒不连续
							ptpClock->offsetFromMaster.nanoseconds = 0; 
						}
					else if((offnansec<500)&&(b>0))
						{
							b--;
						}
					else if(offnansec > 5000	)
						{
							b++; 		//出现大波动，不稳定计数+1
							ptpClock->offsetFromMaster.nanoseconds = 0;  //忽略此次偶然的波动
						}	
				}
		}
	else  							//不稳定计数>7,稳态已经破坏
		{
			i = 0;
			b = 0;
		}		
	if(ptpClock->leapSecondInProgress)			//有闰秒，不做滤波
		return;
	if (ptpClock->offsetFromMaster.seconds)	//大于s，不做滤波
		return;
	//printf("Off1=%d,",ptpClock->offsetFromMaster.nanoseconds);
	filter(&ptpClock->offsetFromMaster.nanoseconds, &ptpClock->ofm_filt);	//滤波
	//printf("Off2=%d,",ptpClock->offsetFromMaster.nanoseconds);
}

/*从时钟调整时间*/
void updateClock(RunTimeOpts * rtOpts,PtpClock *ptpClock)
{
    Integer32 adj;
    TimeInternal timeTmp;
    Integer32 offsetNorm;

    DBGV("updateClock\n");
	  //printf("offset=%ds.%dns",ptpClock->offsetFromMaster.seconds,ptpClock->offsetFromMaster.nanoseconds);
	
    if (0 != ptpClock->offsetFromMaster.seconds || abs(ptpClock->offsetFromMaster.nanoseconds)>MAX_ADJ_OFFSET_NS )//时间偏差是s级或者ns级＞MAX_ADJ_OFFSET_NS(10ms)
			{
				if (!rtOpts->noAdjust)				//开启了调整时间的设置，(测试仪（noAdjust=1）不调整)
				{
					if (!rtOpts->noResetClock)	//启用一步调整到位的设置
					{
						getTime(&timeTmp);																					//取硬件标准时间
						subTime(&timeTmp, &timeTmp, &ptpClock->offsetFromMaster);		//转UTC时间
						setTime(&timeTmp);																					//设置成系统时间
						initClock(rtOpts,ptpClock);																	//清空保存的时间偏差数据集
					}
					else												//逐步（步进:ADJ_FREQ_MAX）调整时间
					{
						adj = ptpClock->offsetFromMaster.nanoseconds > 0 ? ADJ_FREQ_MAX : -ADJ_FREQ_MAX;
						adjFreq(-adj);						//调整PTP时间戳加数寄存器（注意函数内又重复了2次判断与ADJ_FREQ_MAX的大小关系，考证后可以精简）
					}
				}
			}
    else	//精确调整（<MAX_ADJ_OFFSET_NS）
			{
					/* the PI controller */
					/* normalize offset to 1s sync interval -> response of the servo will
					 * be same for all sync interval values, but faster/slower 
					 * (possible lost of precision/overflow but much more stable) */
					offsetNorm = ptpClock->offsetFromMaster.nanoseconds;
					if(abs(offsetNorm) <= 200)
						STM_LEDon(mLED2);		//偏差小于200ns亮LED2
					else 
						STM_LEDoff(mLED2);	//大于200ns灭LED2
					if (ptpClock->logSyncInterval > 0) 
						{
							offsetNorm >>= ptpClock->logSyncInterval;		//本次偏差	=	每秒偏差*同步间隔
						} 
					else if (ptpClock->logSyncInterval < 0)
						{
							offsetNorm <<= -ptpClock->logSyncInterval;	//本次偏差	=	每秒偏差*同步间隔
						}
					
					/* the accumulator for the I component */
					ptpClock->observedDrift += offsetNorm / ptpClock->servo.ai;	//累计偏移值	=	偏差值/调整系数（默认16）

					/* clamp the accumulator to ADJ_FREQ_MAX for sanity */
					if (ptpClock->observedDrift > ADJ_FREQ_MAX)
							ptpClock->observedDrift = ADJ_FREQ_MAX;					//累计偏移值超过最大限制，逐步（缓慢）调整	
					else if (ptpClock->observedDrift < -ADJ_FREQ_MAX)
							ptpClock->observedDrift = -ADJ_FREQ_MAX;				//累计偏移值超过最大限制（负方向），逐步（缓慢）调整	
					
					//printf("ptpClock->observedDrift %9d\n",ptpClock->observedDrift);
					/* apply controller output as a clock tick rate adjustment */
					if ( !rtOpts->noAdjust ) //开启了调整时间的设置，(测试仪（noAdjust=1）不调整)
						{
							adj = offsetNorm / ptpClock->servo.ap + ptpClock->observedDrift;	//调整值	=	本次偏差/调整系数（默认2）+	累计偏移值
							adjFreq(-adj);			//调整PTP时间戳加数寄存器
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



