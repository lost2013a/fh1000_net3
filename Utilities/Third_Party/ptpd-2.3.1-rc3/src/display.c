/*-
 * Copyright (c) 2012-2013 George V. Neville-Neil,
 *                         Wojciech Owczarek
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
 * @file   display.c
 * @date   Thu Aug 12 09:06:21 2010
 * 
 * @brief  General routines for displaying internal data.
 * 
 * 
 */

#include "ptpd.h"

/**\brief Display an Integer64 type*/
void
integer64_display(const Integer64 * bigint)
{
	DBGV("Integer 64 : \n");
//	DBGV("LSB : %u\n", bigint->lsb);
//	DBGV("MSB : %d\n", bigint->msb);
}

/**\brief Display an UInteger48 type*/
void
uInteger48_display(const UInteger48 * bigint)
{
	DBGV("Integer 48 : \n");
	DBGV("LSB : %u\n", bigint->lsb);
	DBGV("MSB : %u\n", bigint->msb);
}

/** \brief Display a TimeInternal Structure*/
void
timeInternal_display(const TimeInternal * timeInternal)
{
	DBGV("seconds : %d \n", timeInternal->seconds);
	DBGV("nanoseconds %d \n", timeInternal->nanoseconds);
}

/** \brief Display a Timestamp Structure*/
void
timestamp_display(const Timestamp * timestamp)
{
	uInteger48_display(&timestamp->secondsField);
	DBGV("nanoseconds %u \n", timestamp->nanosecondsField);
}

/**\brief Display a Clockidentity Structure*/
void
clockIdentity_display(const ClockIdentity clockIdentity)
{
	DBGV(
	    "ClockIdentity : %02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx\n",
	    clockIdentity[0], clockIdentity[1], clockIdentity[2],
	    clockIdentity[3], clockIdentity[4], clockIdentity[5],
	    clockIdentity[6], clockIdentity[7]
	);
}

/**\brief Display MAC address*/
void
clockUUID_display(const Octet * sourceUuid)
{
	DBGV(
	    "sourceUuid %02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx\n",
	    sourceUuid[0], sourceUuid[1], sourceUuid[2], sourceUuid[3],
	    sourceUuid[4], sourceUuid[5]
	);
}

/**\brief Display Network info*/
void
netPath_display(const NetPath * net)
{
//	struct in_addr addr;

//	DBGV("eventSock : %d \n", net->eventSock);
//	DBGV("generalSock : %d \n", net->generalSock);
//	addr.s_addr = net->multicastAddr;
//	DBGV("multicastAdress : %s \n", inet_ntoa(addr));
//	addr.s_addr = net->peerMulticastAddr;
//	DBGV("peerMulticastAddress : %s \n", inet_ntoa(addr));
//	addr.s_addr = net->unicastAddr;
//	DBGV("unicastAddress : %s \n", inet_ntoa(addr));
}

/**\brief Display a IntervalTimer Structure*/
void
intervalTimer_display(const IntervalTimer * ptimer)
{
	DBGV("interval : %d \n", ptimer->interval);
	DBGV("left : %d \n", ptimer->left);
	DBGV("expire : %d \n", ptimer->expire);
}

/**\brief Display a TimeInterval Structure*/
void
timeInterval_display(const TimeInterval * timeInterval)
{
	integer64_display(&timeInterval->scaledNanoseconds);
}

/**\brief Display a Portidentity Structure*/
void
portIdentity_display(const PortIdentity * portIdentity)
{
	clockIdentity_display(portIdentity->clockIdentity);
	DBGV("port number : %d \n", portIdentity->portNumber);
}

/**\brief Display a Clockquality Structure*/
void
clockQuality_display(const ClockQuality * clockQuality)
{
	DBGV("clockClass : %d \n", clockQuality->clockClass);
	DBGV("clockAccuracy : %d \n", clockQuality->clockAccuracy);
	DBGV("offsetScaledLogVariance : %d \n", clockQuality->offsetScaledLogVariance);
}

/**\brief Display PTPText Structure*/
void
PTPText_display(const PTPText *p, const PtpClock *ptpClock)
{
	DBGV("    lengthField : %d \n", p->lengthField);
	DBGV("    textField : %.*s \n", (int)p->lengthField,  p->textField);
}

/**\brief Display the Network Interface Name*/
void
iFaceName_display(const Octet * iFaceName)
{

	int i;

	DBGV("iFaceName : ");

	for (i = 0; i < IFACE_NAME_LENGTH; i++) {
		DBGV("%c", iFaceName[i]);
	}
	DBGV("\n");

}

/**\brief Display an Unicast Adress*/
void
unicast_display(const Integer32 * unicast)
{

	int i;

	DBGV("Unicast adress : ");

	for (i = 0; i < NET_ADDRESS_LENGTH; i++) {
		DBGV("%c", unicast[i]);
	}
	DBGV("\n");

}


/**\brief Display Sync message*/
void
msgSync_display(const MsgSync * sync)
{
	DBGV("Message Sync : \n");
	timestamp_display(&sync->originTimestamp);
	DBGV("\n");
}

/**\brief Display Header message*/
void
msgHeader_display(const MsgHeader * header)
{
	DBGV("Message header : \n");
	DBGV("\n");
	DBGV("transportSpecific : %d\n", header->transportSpecific);
	DBGV("messageType : %d\n", header->messageType);
	DBGV("versionPTP : %d\n", header->versionPTP);
	DBGV("messageLength : %d\n", header->messageLength);
	DBGV("domainNumber : %d\n", header->domainNumber);
	DBGV("FlagField %02hhx:%02hhx\n", header->flagField0, header->flagField1);
	DBGV("CorrectionField : \n");
	integer64_display(&header->correctionField);
	DBGV("SourcePortIdentity : \n");
	portIdentity_display(&header->sourcePortIdentity);
	DBGV("sequenceId : %d\n", header->sequenceId);
	DBGV("controlField : %d\n", header->controlField);
	DBGV("logMessageInterval : %d\n", header->logMessageInterval);
	DBGV("\n");
}

/**\brief Display Announce message*/
void
msgAnnounce_display(const MsgAnnounce * announce)
{
	DBGV("Announce Message : \n");
	DBGV("\n");
	DBGV("originTimestamp : \n");
	DBGV("secondField  : \n");
	timestamp_display(&announce->originTimestamp);
	DBGV("currentUtcOffset : %d \n", announce->currentUtcOffset);
	DBGV("grandMasterPriority1 : %d \n", announce->grandmasterPriority1);
	DBGV("grandMasterClockQuality : \n");
	clockQuality_display(&announce->grandmasterClockQuality);
	DBGV("grandMasterPriority2 : %d \n", announce->grandmasterPriority2);
	DBGV("grandMasterIdentity : \n");
	clockIdentity_display(announce->grandmasterIdentity);
	DBGV("stepsRemoved : %d \n", announce->stepsRemoved);
	DBGV("timeSource : %d \n", announce->timeSource);
	DBGV("\n");
}

/**\brief Display Follow_UP message*/
void
msgFollowUp_display(const MsgFollowUp * follow)
{
	timestamp_display(&follow->preciseOriginTimestamp);
}

/**\brief Display DelayReq message*/
void
msgDelayReq_display(const MsgDelayReq * req)
{
	timestamp_display(&req->originTimestamp);
}

/**\brief Display DelayResp message*/
void
msgDelayResp_display(const MsgDelayResp * resp)
{
	timestamp_display(&resp->receiveTimestamp);
	portIdentity_display(&resp->requestingPortIdentity);
}

/**\brief Display Pdelay_Req message*/
void
msgPDelayReq_display(const MsgPDelayReq * preq)
{
	timestamp_display(&preq->originTimestamp);
}

/**\brief Display Pdelay_Resp message*/
void
msgPDelayResp_display(const MsgPDelayResp * presp)
{

	timestamp_display(&presp->requestReceiptTimestamp);
	portIdentity_display(&presp->requestingPortIdentity);
}

/**\brief Display Pdelay_Resp Follow Up message*/
void
msgPDelayRespFollowUp_display(const MsgPDelayRespFollowUp * prespfollow)
{

	timestamp_display(&prespfollow->responseOriginTimestamp);
	portIdentity_display(&prespfollow->requestingPortIdentity);
}

/**\brief Display Management message*/
void
msgManagement_display(const MsgManagement * manage)
{
	DBGV("Management Message : \n");
	DBGV("\n");
	DBGV("targetPortIdentity : \n");
	portIdentity_display(&manage->targetPortIdentity);
	DBGV("startingBoundaryHops : %d \n", manage->startingBoundaryHops);
	DBGV("boundaryHops : %d \n", manage->boundaryHops);
	DBGV("actionField : %d\n", manage->actionField);
}

/**\brief Display ManagementTLV Slave Only message*/
void
mMSlaveOnly_display(const MMSlaveOnly *slaveOnly, const PtpClock *ptpClock)
{
	DBGV("Slave Only ManagementTLV message \n");
	DBGV("SO : %d \n", slaveOnly->so);
}

/**\brief Display ManagementTLV Clock Description message*/
void
mMClockDescription_display(const MMClockDescription *clockDescription, const PtpClock *ptpClock)
{
	DBGV("Clock Description ManagementTLV message \n");
	DBGV("clockType0 : %d \n", clockDescription->clockType0);
	DBGV("clockType1 : %d \n", clockDescription->clockType1);
	DBGV("physicalLayerProtocol : \n");
	PTPText_display(&clockDescription->physicalLayerProtocol, ptpClock);
	DBGV("physicalAddressLength : %d \n", clockDescription->physicalAddress.addressLength);
	if(clockDescription->physicalAddress.addressField) {
		DBGV("physicalAddressField : \n");
		clockUUID_display(clockDescription->physicalAddress.addressField);
	}
//	DBGV("protocolAddressNetworkProtocol : %d \n", clockDescription->protocolAddress.networkProtocol);
//	DBGV("protocolAddressLength : %d \n", clockDescription->protocolAddress.addressLength);
//	if(clockDescription->addressField) {
//		DBGV("protocolAddressField : %d.%d.%d.%d \n",
//			(UInteger8)clockDescription->protocolAddress.addressField[0],
//			(UInteger8)clockDescription->protocolAddress.addressField[1],
//			(UInteger8)clockDescription->protocolAddress.addressField[2],
//			(UInteger8)clockDescription->protocolAddress.addressField[3]);
//	}
	DBGV("manufacturerIdentity0 : %d \n", clockDescription->manufacturerIdentity0);
	DBGV("manufacturerIdentity1 : %d \n", clockDescription->manufacturerIdentity1);
	DBGV("manufacturerIdentity2 : %d \n", clockDescription->manufacturerIdentity2);
	DBGV("productDescription : \n");
	PTPText_display(&clockDescription->productDescription, ptpClock);
	DBGV("revisionData : \n");
	PTPText_display(&clockDescription->revisionData, ptpClock);
	DBGV("userDescription : \n");
	PTPText_display(&clockDescription->userDescription, ptpClock);
	DBGV("profileIdentity0 : %d \n", clockDescription->profileIdentity0);
	DBGV("profileIdentity1 : %d \n", clockDescription->profileIdentity1);
	DBGV("profileIdentity2 : %d \n", clockDescription->profileIdentity2);
	DBGV("profileIdentity3 : %d \n", clockDescription->profileIdentity3);
	DBGV("profileIdentity4 : %d \n", clockDescription->profileIdentity4);
	DBGV("profileIdentity5 : %d \n", clockDescription->profileIdentity5);
}

void
mMUserDescription_display(const MMUserDescription* userDescription, const PtpClock *ptpClock)
{
	/* TODO: implement me */
}

void
mMInitialize_display(const MMInitialize* initialize, const PtpClock *ptpClock)
{
	/* TODO: implement me */
}

void
mMDefaultDataSet_display(const MMDefaultDataSet* defaultDataSet, const PtpClock *ptpClock)
{
	/* TODO: implement me */
}

void
mMCurrentDataSet_display(const MMCurrentDataSet* currentDataSet, const PtpClock *ptpClock)
{
	/* TODO: implement me */
}

void
mMParentDataSet_display(const MMParentDataSet* parentDataSet, const PtpClock *ptpClock)
{
	/* TODO: implement me */
}

void
mMTimePropertiesDataSet_display(const MMTimePropertiesDataSet* timePropertiesDataSet, const PtpClock *ptpClock)
{
	/* TODO: implement me */
}

void
mMPortDataSet_display(const MMPortDataSet* portDataSet, const PtpClock *ptpClock)
{
	/* TODO: implement me */
}

void
mMPriority1_display(const MMPriority1* priority1, const PtpClock *ptpClock)
{
	/* TODO: implement me */
}

void
mMPriority2_display(const MMPriority2* priority2, const PtpClock *ptpClock)
{
	/* TODO: implement me */
}

void
mMDomain_display(const MMDomain* domain, const PtpClock *ptpClock)
{
	/* TODO: implement me */
}

void
mMLogAnnounceInterval_display(const MMLogAnnounceInterval* logAnnounceInterval, const PtpClock *ptpClock)
{
	/* TODO: implement me */
}

void
mMAnnounceReceiptTimeout_display(const MMAnnounceReceiptTimeout* announceReceiptTimeout, const PtpClock *ptpClock)
{
	/* TODO: implement me */
}

void
mMLogSyncInterval_display(const MMLogSyncInterval* logSyncInterval, const PtpClock *ptpClock)
{
	/* TODO: implement me */
}

void
mMVersionNumber_display(const MMVersionNumber* versionNumber, const PtpClock *ptpClock)
{
	/* TODO: implement me */
}

void
mMTime_display(const MMTime* time, const PtpClock *ptpClock)
{
	/* TODO: implement me */
}

void
mMClockAccuracy_display(const MMClockAccuracy* clockAccuracy, const PtpClock *ptpClock)
{
	/* TODO: implement me */
}

void
mMUtcProperties_display(const MMUtcProperties* utcProperties, const PtpClock *ptpClock)
{
	/* TODO: implement me */
}

void
mMTraceabilityProperties_display(const MMTraceabilityProperties* traceabilityProperties, const PtpClock *ptpClock)
{
	/* TODO: implement me */
}

void
mMDelayMechanism_display(const MMDelayMechanism* delayMechanism, const PtpClock *ptpClock)
{
	/* TODO: implement me */
}

void
mMLogMinPdelayReqInterval_display(const MMLogMinPdelayReqInterval* logMinPdelayReqInterval, const PtpClock *ptpClock)
{
	/* TODO: implement me */
}

void
mMErrorStatus_display(const MMErrorStatus* errorStatus, const PtpClock *ptpClock)
{
	/* TODO: implement me */
}

#define FORMAT_SERVO	"%f"

/**\brief Display runTimeOptions structure*/
void
displayRunTimeOpts(const RunTimeOpts * rtOpts)
{

	DBGV("---Run time Options Display-- \n");
	DBGV("\n");
	DBGV("announceInterval : %d \n", rtOpts->announceInterval);
	DBGV("syncInterval : %d \n", rtOpts->syncInterval);
	clockQuality_display(&(rtOpts->clockQuality));
	DBGV("priority1 : %d \n", rtOpts->priority1);
	DBGV("priority2 : %d \n", rtOpts->priority2);
	DBGV("domainNumber : %d \n", rtOpts->domainNumber);
	DBGV("slaveOnly : %d \n", rtOpts->slaveOnly);
	DBGV("currentUtcOffset : %d \n", rtOpts->timeProperties.currentUtcOffset);
	unicast_display(&rtOpts->unicastAddr);
	DBGV("noAdjust : %d \n", rtOpts->noAdjust);
//	DBGV("logStatistics : %d \n", rtOpts->logStatistics);
	iFaceName_display(rtOpts->ifaceName);
//	DBGV("kP : %d \n", rtOpts->servoKP);
//	DBGV("kI : %d \n", rtOpts->servoKI);
	DBGV("s : %d \n", rtOpts->s);
	DBGV("inbound latency : \n");
	timeInternal_display(&(rtOpts->inboundLatency));
	DBGV("outbound latency : \n");
	timeInternal_display(&(rtOpts->outboundLatency));
	DBGV("max_foreign_records : %d \n", rtOpts->max_foreign_records);
	DBGV("transport : %d \n", rtOpts->transport);
	DBGV("\n");
}


/**\brief Display Default data set of a PtpClock*/
void
displayDefault(const PtpClock * ptpClock)
{
	DBGV("---Ptp Clock Default Data Set-- \n");
	DBGV("\n");
	DBGV("twoStepFlag : %d \n", ptpClock->twoStepFlag);
	clockIdentity_display(ptpClock->clockIdentity);
	DBGV("numberPorts : %d \n", ptpClock->numberPorts);
	clockQuality_display(&(ptpClock->clockQuality));
	DBGV("priority1 : %d \n", ptpClock->priority1);
	DBGV("priority2 : %d \n", ptpClock->priority2);
	DBGV("domainNumber : %d \n", ptpClock->domainNumber);
	DBGV("slaveOnly : %d \n", ptpClock->slaveOnly);
	DBGV("\n");
}


/**\brief Display Current data set of a PtpClock*/
void
displayCurrent(const PtpClock * ptpClock)
{
	DBGV("---Ptp Clock Current Data Set-- \n");
	DBGV("\n");

	DBGV("stepsremoved : %d \n", ptpClock->stepsRemoved);
	DBGV("Offset from master : \n");
	timeInternal_display(&ptpClock->offsetFromMaster);
	DBGV("Mean path delay : \n");
	timeInternal_display(&ptpClock->meanPathDelay);
	DBGV("\n");
}



/**\brief Display Parent data set of a PtpClock*/
void
displayParent(const PtpClock * ptpClock)
{
	DBGV("---Ptp Clock Parent Data Set-- \n");
	DBGV("\n");
	portIdentity_display(&(ptpClock->parentPortIdentity));
	DBGV("parentStats : %d \n", ptpClock->parentStats);
	DBGV("observedParentOffsetScaledLogVariance : %d \n", ptpClock->observedParentOffsetScaledLogVariance);
	DBGV("observedParentClockPhaseChangeRate : %d \n", ptpClock->observedParentClockPhaseChangeRate);
	DBGV("--GrandMaster--\n");
	clockIdentity_display(ptpClock->grandmasterIdentity);
	clockQuality_display(&ptpClock->grandmasterClockQuality);
	DBGV("grandmasterpriority1 : %d \n", ptpClock->grandmasterPriority1);
	DBGV("grandmasterpriority2 : %d \n", ptpClock->grandmasterPriority2);
	DBGV("\n");
}

/**\brief Display Global data set of a PtpClock*/
void
displayGlobal(const PtpClock * ptpClock)
{
	DBGV("---Ptp Clock Global Time Data Set-- \n");
	DBGV("\n");

	DBGV("currentUtcOffset : %d \n", ptpClock->timePropertiesDS.currentUtcOffset);
	DBGV("currentUtcOffsetValid : %d \n", ptpClock->timePropertiesDS.currentUtcOffsetValid);
	DBGV("leap59 : %d \n", ptpClock->timePropertiesDS.leap59);
	DBGV("leap61 : %d \n", ptpClock->timePropertiesDS.leap61);
	DBGV("timeTraceable : %d \n", ptpClock->timePropertiesDS.timeTraceable);
	DBGV("frequencyTraceable : %d \n", ptpClock->timePropertiesDS.frequencyTraceable);
	DBGV("ptpTimescale : %d \n", ptpClock->timePropertiesDS.ptpTimescale);
	DBGV("timeSource : %d \n", ptpClock->timePropertiesDS.timeSource);
	DBGV("\n");
}

/**\brief Display Port data set of a PtpClock*/
void
displayPort(const PtpClock * ptpClock)
{
	DBGV("---Ptp Clock Port Data Set-- \n");
	DBGV("\n");

	portIdentity_display(&ptpClock->portIdentity);
	DBGV("port state : %d \n", ptpClock->portState);
	DBGV("logMinDelayReqInterval : %d \n", ptpClock->logMinDelayReqInterval);
	DBGV("peerMeanPathDelay : \n");
	timeInternal_display(&ptpClock->peerMeanPathDelay);
	DBGV("logAnnounceInterval : %d \n", ptpClock->logAnnounceInterval);
	DBGV("announceReceiptTimeout : %d \n", ptpClock->announceReceiptTimeout);
	DBGV("logSyncInterval : %d \n", ptpClock->logSyncInterval);
	DBGV("delayMechanism : %d \n", ptpClock->delayMechanism);
	DBGV("logMinPdelayReqInterval : %d \n", ptpClock->logMinPdelayReqInterval);
	DBGV("versionNumber : %d \n", ptpClock->versionNumber);
	DBGV("\n");
}

/**\brief Display ForeignMaster data set of a PtpClock*/
void
displayForeignMaster(const PtpClock * ptpClock)
{

	ForeignMasterRecord *foreign;
	int i;

	if (ptpClock->number_foreign_records > 0) {

		DBGV("---Ptp Clock Foreign Data Set-- \n");
		DBGV("\n");
		DBGV("There is %d Foreign master Recorded \n", ptpClock->number_foreign_records);
		foreign = ptpClock->foreign;

		for (i = 0; i < ptpClock->number_foreign_records; i++) {

			portIdentity_display(&foreign->foreignMasterPortIdentity);
			DBGV("number of Announce message received : %d \n", foreign->foreignMasterAnnounceMessages);
			msgHeader_display(&foreign->header);
			msgAnnounce_display(&foreign->announce);

			foreign++;
		}

	} else {
		DBGV("No Foreign masters recorded \n");
	}

	DBGV("\n");


}

/**\brief Display other data set of a PtpClock*/

void
displayOthers(const PtpClock * ptpClock)
{

	int i;

	/* Usefull to display name of timers */
#ifdef PTPD_DBGV
	    char timer[][26] = {
		"PDELAYREQ_INTERVAL_TIMER",
		"SYNC_INTERVAL_TIMER",
		"ANNOUNCE_RECEIPT_TIMER",
		"ANNOUNCE_INTERVAL_TIMER"
	};
#endif
	DBGV("---Ptp Others Data Set-- \n");
	DBGV("\n");

	/*DBGV("master_to_slave_delay : \n");
	timeInternal_display(&ptpClock->master_to_slave_delay);
	DBGV("\n");
	DBGV("slave_to_master_delay : \n");
	timeInternal_display(&ptpClock->slave_to_master_delay);
	*/
	
	DBGV("\n");
	DBGV("delay_req_receive_time : \n");
	timeInternal_display(&ptpClock->pdelay_req_receive_time);
	DBGV("\n");
	DBGV("delay_req_send_time : \n");
	timeInternal_display(&ptpClock->pdelay_req_send_time);
	DBGV("\n");
	DBGV("delay_resp_receive_time : \n");
	timeInternal_display(&ptpClock->pdelay_resp_receive_time);
	DBGV("\n");
	DBGV("delay_resp_send_time : \n");
	timeInternal_display(&ptpClock->pdelay_resp_send_time);
	DBGV("\n");
	DBGV("sync_receive_time : \n");
	timeInternal_display(&ptpClock->sync_receive_time);
	DBGV("\n");
	//DBGV("R : %f \n", ptpClock->R);
	DBGV("sentPdelayReq : %d \n", ptpClock->sentPDelayReq);
	DBGV("sentPDelayReqSequenceId : %d \n", ptpClock->sentPDelayReqSequenceId);
	DBGV("waitingForFollow : %d \n", ptpClock->waitingForFollow);
	DBGV("\n");
	DBGV("Offset from master filter : \n");
//	DBGV("nsec_prev : %d \n", ptpClock->ofm_filt.nsec_prev);
//	DBGV("y : %d \n", ptpClock->ofm_filt.y);
	DBGV("\n");
	DBGV("One way delay filter : \n");
//	DBGV("nsec_prev : %d \n", ptpClock->owd_filt.nsec_prev);
//	DBGV("y : %d \n", ptpClock->owd_filt.y);
//	DBGV("s_exp : %d \n", ptpClock->owd_filt.s_exp);
	DBGV("\n");
//	DBGV("observed drift : "FORMAT_SERVO" \n", ptpClock->observedDrift);
	DBGV("message activity %d \n", ptpClock->message_activity);
	DBGV("\n");

	for (i = 0; i < TIMER_ARRAY_SIZE; i++) {
		DBGV("%s : \n", timer[i]);
		intervalTimer_display(&ptpClock->itimer[i]);
		DBGV("\n");
	}

	netPath_display(&ptpClock->netPath);
//	DBGV("mCommunication technology %d \n", ptpClock->port_communication_technology);
//	clockUUID_display(ptpClock->netPath.interfaceID);
	DBGV("\n");
}


/**\brief Display Buffer in & out of a PtpClock*/
void
displayBuffer(const PtpClock * ptpClock)
{

	int i;
	int j;

	j = 0;

	DBGV("PtpClock Buffer Out  \n");
	DBGV("\n");

	for (i = 0; i < PACKET_SIZE; i++) {
		DBGV(":%02hhx", ptpClock->msgObuf[i]);
		j++;

		if (j == 8) {
			DBGV(" ");

		}
		if (j == 16) {
			DBGV("\n");
			j = 0;
		}
	}
	DBGV("\n");
	j = 0;
	DBGV("\n");

	DBGV("PtpClock Buffer In  \n");
	DBGV("\n");
	for (i = 0; i < PACKET_SIZE; i++) {
		DBGV(":%02hhx", ptpClock->msgIbuf[i]);
		j++;

		if (j == 8) {
			DBGV(" ");

		}
		if (j == 16) {
			DBGV("\n");
			j = 0;
		}
	}
	DBGV("\n");
	DBGV("\n");
}

/**\convert port state to string*/
const char
*portState_getName(Enumeration8 portState)
{
		static const int max = PTP_SLAVE;
    int intstate = portState;
		static const char *ptpStates[10] = {NULL};
	
		ptpStates[PTP_INITIALIZING] = "PTP_INITIALIZING";
		ptpStates[PTP_FAULTY] = "PTP_FAULTY";
		ptpStates[PTP_DISABLED] = "PTP_DISABLED";
		ptpStates[PTP_LISTENING] = "PTP_LISTENING";
		ptpStates[PTP_PRE_MASTER] = "PTP_PRE_MASTER";
		ptpStates[PTP_MASTER] = "PTP_MASTER";
		ptpStates[PTP_PASSIVE] = "PTP_PASSIVE";
		ptpStates[PTP_UNCALIBRATED] = "PTP_UNCALIBRATED";
		ptpStates[PTP_SLAVE] = "PTP_SLAVE";
    /* converting to int to avoid compiler warnings when comparing enum*/


    if( intstate < 0 || intstate > max ) {
        return("PTP_UNKNOWN");
    }

    return(ptpStates[portState]);
}

/**\brief Display all PTP clock (port) counters*/
void
displayCounters(const PtpClock * ptpClock)
{

	/* TODO: print port identity */
	INFO("\n============= PTP port counters =============\n");

	INFO("Message counters:\n");
	INFO("              announceMessagesSent : %d\n",
		ptpClock->counters.announceMessagesSent);
	INFO("          announceMessagesReceived : %d\n",
		ptpClock->counters.announceMessagesReceived);
	INFO("                  syncMessagesSent : %d\n",
		ptpClock->counters.syncMessagesSent);
	INFO("              syncMessagesReceived : %d\n",
		ptpClock->counters.syncMessagesReceived);
	INFO("              followUpMessagesSent : %d\n",
		ptpClock->counters.followUpMessagesSent);
	INFO("          followUpMessagesReceived : %d\n",
		ptpClock->counters.followUpMessagesReceived);
	INFO("              delayReqMessagesSent : %d\n",
		 ptpClock->counters.delayReqMessagesSent);
	INFO("          delayReqMessagesReceived : %d\n",
		ptpClock->counters.delayReqMessagesReceived);
	INFO("             delayRespMessagesSent : %d\n",
		ptpClock->counters.delayRespMessagesSent);
	INFO("         delayRespMessagesReceived : %d\n",
		ptpClock->counters.delayRespMessagesReceived);
	INFO("             pdelayReqMessagesSent : %d\n",
		ptpClock->counters.pdelayReqMessagesSent);
	INFO("         pdelayReqMessagesReceived : %d\n",
		ptpClock->counters.pdelayReqMessagesReceived);
	INFO("            pdelayRespMessagesSent : %d\n",
		ptpClock->counters.pdelayRespMessagesSent);
	INFO("        pdelayRespMessagesReceived : %d\n",
		ptpClock->counters.pdelayRespMessagesReceived);
	INFO("    pdelayRespFollowUpMessagesSent : %d\n",
		ptpClock->counters.pdelayRespFollowUpMessagesSent);
	INFO("pdelayRespFollowUpMessagesReceived : %d\n",
		ptpClock->counters.pdelayRespFollowUpMessagesReceived);
	INFO("             signalingMessagesSent : %d\n",
		ptpClock->counters.signalingMessagesSent);
	INFO("         signalingMessagesReceived : %d\n",
		ptpClock->counters.signalingMessagesReceived);
	INFO("            managementMessagesSent : %d\n",
		ptpClock->counters.managementMessagesSent);
	INFO("        managementMessagesReceived : %d\n",
		ptpClock->counters.managementMessagesReceived);

/* not implemented yet */
#if 0
	INFO("FMR counters:\n");
	INFO("                      foreignAdded : %d\n",
		ptpClock->counters.foreignAdded);
	INFO("                        foreignMax : %d\n",
		ptpClock->counters.foreignMax);
	INFO("                    foreignRemoved : %d\n",
		ptpClock->counters.foreignRemoved);
	INFO("                   foreignOverflow : %d\n",
		ptpClock->counters.foreignOverflow);
#endif /* 0 */

	INFO("Protocol engine counters:\n");
	INFO("                  stateTransitions : %d\n",
		ptpClock->counters.stateTransitions);
	INFO("                     masterChanges : %d\n",
		ptpClock->counters.masterChanges);
	INFO("                  announceTimeouts : %d\n",
		ptpClock->counters.announceTimeouts);

	INFO("Discarded / unknown message counters:\n");
	INFO("                 discardedMessages : %d\n",
		ptpClock->counters.discardedMessages);
	INFO("                   unknownMessages : %d\n",
		ptpClock->counters.unknownMessages);
	INFO("                   ignoredAnnounce : %d\n",
		ptpClock->counters.ignoredAnnounce);
	INFO("    aclManagementDiscardedMessages : %d\n",
		ptpClock->counters.aclManagementDiscardedMessages);
	INFO("        aclTimingDiscardedMessages : %d\n",
		ptpClock->counters.aclTimingDiscardedMessages);

	INFO("Error counters:\n");
	INFO("                 messageSendErrors : %d\n",
		ptpClock->counters.messageSendErrors);
	INFO("                 messageRecvErrors : %d\n",
		ptpClock->counters.messageRecvErrors);
	INFO("               messageFormatErrors : %d\n",
		ptpClock->counters.messageFormatErrors);
	INFO("                    protocolErrors : %d\n",
		ptpClock->counters.protocolErrors);
	INFO("             versionMismatchErrors : %d\n",
		ptpClock->counters.versionMismatchErrors);
	INFO("              domainMismatchErrors : %d\n",
		ptpClock->counters.domainMismatchErrors);
	INFO("            sequenceMismatchErrors : %d\n",
		ptpClock->counters.sequenceMismatchErrors);
	INFO("           delayModeMismatchErrors : %d\n",
		ptpClock->counters.delayModeMismatchErrors);
	INFO("           maxDelayDrops : %d\n",
		ptpClock->counters.maxDelayDrops);


#ifdef PTPD_STATISTICS
	INFO("Outlier filter hits:\n");
	INFO("              delayMSOutliersFound : %d\n",
		ptpClock->counters.delayMSOutliersFound);
	INFO("              delaySMOutliersFound : %d\n",
		ptpClock->counters.delaySMOutliersFound);
#endif /* PTPD_STATISTICS */

}

/**\brief Display all PTP clock (port) statistics*/
void
displayStatistics(const PtpClock* ptpClock)
{

/*
	INFO("Clock stats: ofm mean: %d, ofm median: %d,"
	     "ofm std dev: %d, observed drift std dev: %d\n",
	     ptpClock->stats.ofmMean, ptpClock->stats.ofmMedian,
	     ptpClock->stats.ofmStdDev, ptpClock->stats.driftStdDev);
*/

}

void 
displayPortIdentity(PortIdentity *port, const char *prefixMessage) 
{
//	static char sbuf[SCREEN_BUFSZ];
//	int len = 0;

//	memset(sbuf, ' ', sizeof(sbuf));
//	len += snprintf(sbuf + len, sizeof(sbuf) - len, "%s ", prefixMessage);
//	len += snprint_PortIdentity(sbuf + len, sizeof(sbuf) - len, port);
//        len += snprintf(sbuf + len, sizeof(sbuf) - len, "\n");
//        INFO("%s",sbuf);
}

/* Show the hostname configured in /etc/ethers */
int
snprint_ClockIdentity_ntohost(char *s, int max_len, const ClockIdentity id)
{
	int len = 0;
	int i,j;
	char  buf[100];
//	struct ether_addr e;

	/* extract mac address */
	for (i = 0, j = 0; i< CLOCK_IDENTITY_LENGTH ; i++ ){
		/* skip bytes 3 and 4 */
		if(!((i==3) || (i==4))){
#ifdef HAVE_STRUCT_ETHER_ADDR_OCTET
			e.octet[j] = (uint8_t) id[i];
#else
//			e.ether_addr_octet[j] = (uint8_t) id[i];
#endif
			j++;
		}
	}

	/* convert and print hostname */
//	ether_ntohost_cache(buf, &e);
	len += snprintf(&s[len], max_len - len, "(%s)", buf);

	return len;
}

int 
snprint_ClockIdentity(char *s, int max_len, const ClockIdentity id)
{
	int len = 0;
	int i;

	for (i = 0; ;) {
		len += snprintf(&s[len], max_len - len, "%02x", (unsigned char) id[i]);

		if (++i >= CLOCK_IDENTITY_LENGTH)
			break;
	}

	return len;
}

int 
snprint_PortIdentity(char *s, int max_len, const PortIdentity *id)
{
	int len = 0;

#ifdef PRINT_MAC_ADDRESSES
	len += snprint_ClockIdentity_mac(&s[len], max_len - len, id->clockIdentity);
#else	
	len += snprint_ClockIdentity(&s[len], max_len - len, id->clockIdentity);
#endif

	len += snprint_ClockIdentity_ntohost(&s[len], max_len - len, id->clockIdentity);

	len += snprintf(&s[len], max_len - len, "/%02x", (unsigned) id->portNumber);
	return len;
}

int
snprint_ClockIdentity_mac(char *s, int max_len, const ClockIdentity id)
{
	int len = 0;
	int i;
	for (i = 0; ;) {
		/* skip bytes 3 and 4 */
		if(!((i==3) || (i==4))){
			len += snprintf(&s[len], max_len - len, "%02x", (unsigned char) id[i]);

			if (++i >= CLOCK_IDENTITY_LENGTH)
				break;

			/* print a separator after each byte except the last one */
			len += snprintf(&s[len], max_len - len, "%s", ":");
		} else {

			i++;
		}
	}
	return len;
}

void 
displayStatus(PtpClock *ptpClock, const char *prefixMessage)
{

	static char sbuf[SCREEN_BUFSZ];
	int len = 0;

	memset(sbuf, ' ', sizeof(sbuf));
//	strcpy(sbuf,prefixMessage);
//	len = strlen(sbuf);
	len += snprintf(sbuf + len, sizeof(sbuf) - len, "%s", prefixMessage);
	len += snprintf(sbuf + len, sizeof(sbuf) - len, "%s", 
			portState_getName(ptpClock->portState));

//	if (ptpClock->portState >= PTP_MASTER) {
//		len += snprintf(sbuf + len, sizeof(sbuf) - len, ", Best master: ");
//		len += snprint_PortIdentity(sbuf + len, sizeof(sbuf) - len,
//			&ptpClock->parentPortIdentity);
//  }
//	if(ptpClock->portState == PTP_MASTER){
//		len += snprintf(sbuf + len, sizeof(sbuf) - len, " (self)");
//	}
		len += snprintf(sbuf + len, sizeof(sbuf) - len, "\n");
		NOTICE("%s",sbuf);
}

/**\brief Display All data sets and counters of a PtpClock*/
void
displayPtpClock(const PtpClock * ptpClock)
{

	displayDefault(ptpClock);
	displayCurrent(ptpClock);
	displayParent(ptpClock);
	displayGlobal(ptpClock);
	displayPort(ptpClock);
	displayForeignMaster(ptpClock);
	displayBuffer(ptpClock);
	displayOthers(ptpClock);
	displayCounters(ptpClock);
	displayStatistics(ptpClock);

}
