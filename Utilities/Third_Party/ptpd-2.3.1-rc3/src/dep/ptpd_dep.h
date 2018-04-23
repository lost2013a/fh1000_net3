/**
 * @file   ptpd_dep.h
 * 
 * @brief  External definitions for inclusion elsewhere.
 * 
 * 
 */

#ifndef PTPD_DEP_H_
#define PTPD_DEP_H_

//#define PTPD_DBGV

#ifdef RUNTIME_DEBUG
#undef PTPD_DBGV
#define PTPD_DBGV
#endif

 /** \name System messages*/
 /**\{*/


// Syslog ordering. We define extra debug levels above LOG_DEBUG for internal use - but message() doesn't pass these to SysLog

// extended from <sys/syslog.h>
#define LOG_DEBUG1   7
#define LOG_DEBUG2   8
#define LOG_DEBUG3   9
#define LOG_DEBUGV   9


#define EMERGENCY(x, ...) //logMessage(LOG_EMERG, x, ##__VA_ARGS__)
#define ALERT(x, ...)     //logMessage(LOG_ALERT, x, ##__VA_ARGS__)
#define CRITICAL(x, ...)  //logMessage(LOG_CRIT, x, ##__VA_ARGS__)
#define ERROR(x, ...) 	 { TimeInternal tmpTime; getTime(&tmpTime); printf("(ERROR %d.%09d) ", tmpTime.seconds, tmpTime.nanoseconds); printf(x,##__VA_ARGS__); }
#define PERROR(x, ...)   { TimeInternal tmpTime; getTime(&tmpTime); printf("(PERROR %d.%09d) ", tmpTime.seconds, tmpTime.nanoseconds); printf(x,##__VA_ARGS__); }
#define WARNING(x, ...)   { TimeInternal tmpTime; getTime(&tmpTime); printf("(WARNING %d.%09d) ", tmpTime.seconds, tmpTime.nanoseconds); printf(x,##__VA_ARGS__); }
#define NOTIFY(x, ...) 	{ TimeInternal tmpTime; getTime(&tmpTime); printf("(NOTIFY %d.%09d) ", tmpTime.seconds, tmpTime.nanoseconds); printf(x,##__VA_ARGS__); }
#define NOTICE(x, ...)    { TimeInternal tmpTime; getTime(&tmpTime); printf("(NOTICE %d.%09d) ", tmpTime.seconds, tmpTime.nanoseconds); printf(x,##__VA_ARGS__); }
#define INFO(x, ...)   { TimeInternal tmpTime; getTime(&tmpTime); printf("(INFO %d.%09d) ", tmpTime.seconds, tmpTime.nanoseconds); printf(x,##__VA_ARGS__); }



#include <assert.h>


/*
  list of per-module defines:

./dep/sys.c:#define PRINT_MAC_ADDRESSES
./dep/timer.c:#define US_TIMER_INTERVAL 125000
*/
#define USE_BINDTODEVICE


//#define PTPD_DBG //godin


// enable this line to show debug numbers in nanoseconds instead of microseconds 
// #define DEBUG_IN_NS

#define DBG_UNIT_US (1000)
#define DBG_UNIT_NS (1)

#ifdef DEBUG_IN_NS
#define DBG_UNIT DBG_UNIT_NS
#else
#define DBG_UNIT DBG_UNIT_US
#endif




/** \}*/

/** \name Debug messages*/
 /**\{*/

#ifdef PTPD_DBGV
#undef PTPD_DBG
#undef PTPD_DBG2
#define PTPD_DBG
#define PTPD_DBG2

#define DBGV(x, ...) { TimeInternal tmpTime; getTime(&tmpTime); printf("(I %d.%09d) ", tmpTime.seconds, tmpTime.nanoseconds); printf(x,##__VA_ARGS__); }
#else
#define DBGV(x, ...)
#endif

/*
 * new debug level DBG2:
 * this is above DBG(), but below DBGV() (to avoid changing hundreds of lines)
 */


#ifdef PTPD_DBG2
#undef PTPD_DBG
#define PTPD_DBG
#define DBG2(x, ...) { TimeInternal tmpTime; getTime(&tmpTime); printf("(D2 %d.%09d) ", tmpTime.seconds, tmpTime.nanoseconds); printf(x,##__VA_ARGS__); }
#else
#define DBG2(x, ...)
#endif

#ifdef PTPD_DBG
#define DBG(x, ...) { TimeInternal tmpTime; getTime(&tmpTime); printf("(D %d.%09d) ", tmpTime.seconds, tmpTime.nanoseconds); printf(x,##__VA_ARGS__); }
#else
#define DBG(x, ...)
#endif

/** \}*/

/** \name Endian corrections*/
 /**\{*/

#if defined(PTPD_MSBF)
#define shift8(x,y)   ( (x) << ((3-y)<<3) )
#define shift16(x,y)  ( (x) << ((1-y)<<4) )
#elif defined(PTPD_LSBF)
#define shift8(x,y)   ( (x) << ((y)<<3) )
#define shift16(x,y)  ( (x) << ((y)<<4) )
#endif

#define flip16(x) htons(x)
#define flip32(x) htonl(x)

/* i don't know any target platforms that do not have htons and htonl,
   but here are generic funtions just in case */
/*
#if defined(PTPD_MSBF)
#define flip16(x) (x)
#define flip32(x) (x)
#elif defined(PTPD_LSBF)
static inline Integer16 flip16(Integer16 x)
{
   return (((x) >> 8) & 0x00ff) | (((x) << 8) & 0xff00);
}

static inline Integer32 flip32(x)
{
  return (((x) >> 24) & 0x000000ff) | (((x) >> 8 ) & 0x0000ff00) |
         (((x) << 8 ) & 0x00ff0000) | (((x) << 24) & 0xff000000);
}
#endif
*/

/** \}*/


/** \name Bit array manipulations*/
 /**\{*/
#define getFlag(flagField, mask) (Boolean)(((flagField) & (mask)) == (mask))
#define setFlag(flagField, mask) (flagField) |= (mask)
#define clearFlag(flagField, mask) (flagField) &= ~(mask)
//#define getFlag(x,y)  !!( *(UInteger8*)((x)+((y)<8?1:0)) &   (1<<((y)<8?(y):(y)-8)) )
//#define setFlag(x,y)    ( *(UInteger8*)((x)+((y)<8?1:0)) |=   1<<((y)<8?(y):(y)-8)  )
//#define clearFlag(x,y)  ( *(UInteger8*)((x)+((y)<8?1:0)) &= ~(1<<((y)<8?(y):(y)-8)) )
/** \}*/

/** \name msg.c
 *-Pack and unpack PTP messages */
 /**\{*/

void msgUnpackHeader(Octet * buf,MsgHeader*);
void msgUnpackAnnounce (Octet * buf,MsgAnnounce*);
void msgUnpackSync(Octet * buf,MsgSync*);
void msgUnpackFollowUp(Octet * buf,MsgFollowUp*);
void msgUnpackDelayReq(Octet * buf, MsgDelayReq * delayreq);
void msgUnpackDelayResp(Octet * buf,MsgDelayResp *);
void msgUnpackPDelayReq(Octet * buf,MsgPDelayReq*);
void msgUnpackPDelayResp(Octet * buf,MsgPDelayResp*);
void msgUnpackPDelayRespFollowUp(Octet * buf,MsgPDelayRespFollowUp*);
void msgUnpackManagement(Octet * buf,MsgManagement*, MsgHeader*, PtpClock *ptpClock);
void msgPackHeader(Octet * buf,PtpClock*);
void msgPackAnnounce(Octet * buf,PtpClock*);
void msgPackSync(Octet * buf,Timestamp*,PtpClock*);
void msgPackFollowUp(Octet * buf,Timestamp*,PtpClock*, const UInteger16);
void msgPackDelayReq(Octet * buf,Timestamp *,PtpClock *);
void msgPackDelayResp(Octet * buf,MsgHeader *,Timestamp *,PtpClock *);
void msgPackPDelayReq(Octet * buf,Timestamp*,PtpClock*);
void msgPackPDelayResp(Octet * buf,MsgHeader*,Timestamp*,PtpClock*);
void msgPackPDelayRespFollowUp(Octet * buf,MsgHeader*,Timestamp*,PtpClock*, const UInteger16);
void msgPackManagement(Octet * buf,MsgManagement*,PtpClock*);
void msgPackManagementRespAck(Octet *,MsgManagement*,PtpClock*);
void msgPackManagementTLV(Octet *,MsgManagement*, PtpClock*);
void msgPackManagementErrorStatusTLV(Octet *,MsgManagement*,PtpClock*);

void freeMMErrorStatusTLV(ManagementTLV*);
void freeMMTLV(ManagementTLV*);

void msgDump(PtpClock *ptpClock);
void msgDebugHeader(MsgHeader *header);
void msgDebugSync(MsgSync *sync);
void msgDebugAnnounce(MsgAnnounce *announce);
void msgDebugDelayReq(MsgDelayReq *req);
void msgDebugFollowUp(MsgFollowUp *follow);
void msgDebugDelayResp(MsgDelayResp *resp);
void msgDebugManagement(MsgManagement *manage);

void copyClockIdentity( ClockIdentity dest, ClockIdentity src);
void copyPortIdentity( PortIdentity * dest, PortIdentity * src);

void unpackMsgManagement(Octet *, MsgManagement*, PtpClock*);
void packMsgManagement(MsgManagement*, Octet *);
void unpackManagementTLV(Octet*, MsgManagement*, PtpClock*);
void packManagementTLV(ManagementTLV*, Octet*);
void freeManagementTLV(MsgManagement*);
void unpackMMClockDescription( Octet* buf, MsgManagement*, PtpClock* );
UInteger16 packMMClockDescription( MsgManagement*, Octet*);
void freeMMClockDescription( MMClockDescription*);
void unpackMMUserDescription( Octet* buf, MsgManagement*, PtpClock* );
UInteger16 packMMUserDescription( MsgManagement*, Octet*);
void freeMMUserDescription( MMUserDescription*);
void unpackMMErrorStatus( Octet* buf, MsgManagement*, PtpClock* );
UInteger16 packMMErrorStatus( MsgManagement*, Octet*);
void freeMMErrorStatus( MMErrorStatus*);
void unpackMMInitialize( Octet* buf, MsgManagement*, PtpClock* );
UInteger16 packMMInitialize( MsgManagement*, Octet*);
void unpackMMDefaultDataSet( Octet* buf, MsgManagement*, PtpClock* );
UInteger16 packMMDefaultDataSet( MsgManagement*, Octet*);
void unpackMMCurrentDataSet( Octet* buf, MsgManagement*, PtpClock* );
UInteger16 packMMCurrentDataSet( MsgManagement*, Octet*);
void unpackMMParentDataSet( Octet* buf, MsgManagement*, PtpClock* );
UInteger16 packMMParentDataSet( MsgManagement*, Octet*);
void unpackMMTimePropertiesDataSet( Octet* buf, MsgManagement*, PtpClock* );
UInteger16 packMMTimePropertiesDataSet( MsgManagement*, Octet*);
void unpackMMPortDataSet( Octet* buf, MsgManagement*, PtpClock* );
UInteger16 packMMPortDataSet( MsgManagement*, Octet*);
void unpackMMPriority1( Octet* buf, MsgManagement*, PtpClock* );
UInteger16 packMMPriority1( MsgManagement*, Octet*);
void unpackMMPriority2( Octet* buf, MsgManagement*, PtpClock* );
UInteger16 packMMPriority2( MsgManagement*, Octet*);
void unpackMMDomain( Octet* buf, MsgManagement*, PtpClock* );
UInteger16 packMMDomain( MsgManagement*, Octet*);
void unpackMMSlaveOnly( Octet* buf, MsgManagement*, PtpClock* );
UInteger16 packMMSlaveOnly( MsgManagement*, Octet* );
void unpackMMLogAnnounceInterval( Octet* buf, MsgManagement*, PtpClock* );
UInteger16 packMMLogAnnounceInterval( MsgManagement*, Octet*);
void unpackMMAnnounceReceiptTimeout( Octet* buf, MsgManagement*, PtpClock* );
UInteger16 packMMAnnounceReceiptTimeout( MsgManagement*, Octet*);
void unpackMMLogSyncInterval( Octet* buf, MsgManagement*, PtpClock* );
UInteger16 packMMLogSyncInterval( MsgManagement*, Octet*);
void unpackMMVersionNumber( Octet* buf, MsgManagement*, PtpClock* );
UInteger16 packMMVersionNumber( MsgManagement*, Octet*);
void unpackMMTime( Octet* buf, MsgManagement*, PtpClock * );
UInteger16 packMMTime( MsgManagement*, Octet*);
void unpackMMClockAccuracy( Octet* buf, MsgManagement*, PtpClock* );
UInteger16 packMMClockAccuracy( MsgManagement*, Octet*);
void unpackMMUtcProperties( Octet* buf, MsgManagement*, PtpClock* );
UInteger16 packMMUtcProperties( MsgManagement*, Octet*);
void unpackMMTraceabilityProperties( Octet* buf, MsgManagement*, PtpClock* );
UInteger16 packMMTraceabilityProperties( MsgManagement*, Octet*);
void unpackMMDelayMechanism( Octet* buf, MsgManagement*, PtpClock* );
UInteger16 packMMDelayMechanism( MsgManagement*, Octet*);
void unpackMMLogMinPdelayReqInterval( Octet* buf, MsgManagement*, PtpClock* );
UInteger16 packMMLogMinPdelayReqInterval( MsgManagement*, Octet*);


void unpackPortAddress( Octet* buf, PortAddress*, PtpClock*);
void packPortAddress( PortAddress*, Octet*);
void freePortAddress( PortAddress*);
void unpackPTPText( Octet* buf, PTPText*, PtpClock*);
void packPTPText( PTPText*, Octet*);
void freePTPText( PTPText*);
void unpackPhysicalAddress( Octet* buf, PhysicalAddress*, PtpClock*);
void packPhysicalAddress( PhysicalAddress*, Octet*);
void freePhysicalAddress( PhysicalAddress*);
void unpackClockIdentity( Octet* buf, ClockIdentity *c, PtpClock*);
void packClockIdentity( ClockIdentity *c, Octet* buf);
void freeClockIdentity( ClockIdentity *c);
void unpackClockQuality( Octet* buf, ClockQuality *c, PtpClock*);
void packClockQuality( ClockQuality *c, Octet* buf);
void freeClockQuality( ClockQuality *c);
void unpackTimeInterval( Octet* buf, TimeInterval *t, PtpClock*);
void packTimeInterval( TimeInterval *t, Octet* buf);
void freeTimeInterval( TimeInterval *t);
void unpackPortIdentity( Octet* buf, PortIdentity *p, PtpClock*);
void packPortIdentity( PortIdentity *p, Octet*  buf);
void freePortIdentity( PortIdentity *p);
void unpackTimestamp( Octet* buf, Timestamp *t, PtpClock*);
void packTimestamp( Timestamp *t, Octet* buf);
void freeTimestamp( Timestamp *t);
UInteger16 msgPackManagementResponse(Octet * buf,MsgHeader*,MsgManagement*,PtpClock*);
/** \}*/

/** \name net.c (Unix API dependent)
 * -Init network stuff, send and receive datas*/
 /**\{*/

Boolean testInterface(char* ifaceName, RunTimeOpts* rtOpts);
Boolean netInit(NetPath*,RunTimeOpts*,PtpClock*);
Boolean netShutdown(NetPath*);
int netSelect(TimeInternal*,NetPath*);
ssize_t netRecvEvent(NetPath *netPath, Octet *buf, TimeInternal *time);
ssize_t netRecvGeneral(NetPath *netPath, Octet *buf, TimeInternal *time);
ssize_t netSendEvent(NetPath *netPath, const Octet *buf, UInteger16 length, TimeInternal *time);
ssize_t netSendGeneral(NetPath *netPath, const Octet *buf, UInteger16 length);
ssize_t netSendPeerGeneral(NetPath *netPath, const Octet *buf, UInteger16 length);
ssize_t netSendPeerEvent(NetPath *netPath, const Octet *buf, UInteger16 length, TimeInternal* time);
Boolean netRefreshIGMP(NetPath *, RunTimeOpts *, PtpClock *);
Boolean hostLookup(const char* hostname, Integer32* addr);

/** \}*/

//#if defined PTPD_SNMP
///** \name snmp.c (SNMP subsystem)
// * -Handle SNMP subsystem*/
// /**\{*/

//void snmpInit(RunTimeOpts *, PtpClock *);
//void snmpShutdown();

///** \}*/
//#endif

/** \name servo.c
 * -Clock servo*/
 /**\{*/

void initClock(RunTimeOpts*,PtpClock*);
void updatePeerDelay (RunTimeOpts*,PtpClock*,TimeInternal*,Boolean);
void updateDelay (Filter*, RunTimeOpts*, PtpClock*,TimeInternal*);
void updateOffset(TimeInternal*,TimeInternal*,Filter*,RunTimeOpts*,PtpClock*,TimeInternal*);
void updateClock(RunTimeOpts*,PtpClock*);

void servo_perform_clock_step(RunTimeOpts * rtOpts, PtpClock * ptpClock);

/** \}*/

/** \name startup.c (Unix API dependent)
 * -Handle with runtime options*/
 /**\{*/
int logToFile(RunTimeOpts * rtOpts);
int recordToFile(RunTimeOpts * rtOpts);
int ptpdStartup(Integer16*,PtpClock*,RunTimeOpts*);
void ptpdShutdown(RunTimeOpts *rtOpts,PtpClock * ptpClock);

void checkSignals(RunTimeOpts * rtOpts, PtpClock * ptpClock);

void enable_runtime_debug(void );
void disable_runtime_debug(void );

#define D_ON      do { enable_runtime_debug();  } while (0);
#define D_OFF     do { disable_runtime_debug( ); } while (0);


//Boolean netInit(NetPath*, PtpClock*);
Boolean netShutdown(NetPath*);
//Integer32 netSelect(NetPath*, const TimeInternal*);
ssize_t netRecvEvent(NetPath*, Octet*, TimeInternal*);
ssize_t netRecvGeneral(NetPath*, Octet*, TimeInternal*);
ssize_t netSendEvent(NetPath*, const Octet*, UInteger16, TimeInternal*);
ssize_t netSendGeneral(NetPath*, const Octet*, UInteger16);
ssize_t netSendPeerGeneral(NetPath*, const Octet*, UInteger16);
ssize_t netSendPeerEvent(NetPath*, const Octet*, UInteger16, TimeInternal*);
void netEmptyEventQ(NetPath *netPath);

/** \}*/

/** \name sys.c (Unix API dependent)
 * -Manage timing system API*/
 /**\{*/

/* new debug methods to debug time variables */
char *time2st(const TimeInternal * p);
void DBG_time(const char *name, const TimeInternal  p);

/** \name sys.c (Linux API dependent)
 * -Manage timing system API*/
/**\{*/
void displayStats(const PtpClock *ptpClock);
Boolean nanoSleep(const TimeInternal*);
void getTime(TimeInternal*);
void setTime(const TimeInternal*);
void updateTime(const TimeInternal*);
Boolean adjFreq(Integer32);
UInteger32 getRand(UInteger32);
/** \}*/

void displayStatus(PtpClock *ptpClock, const char *prefixMessage);
void displayPortIdentity(PortIdentity *port, const char *prefixMessage);



void recordSync(RunTimeOpts * rtOpts, UInteger16 sequenceId, TimeInternal * time);


void informClockSource(PtpClock* ptpClock);

/* Observed drift save / recovery functions */
void restoreDrift(PtpClock * ptpClock, RunTimeOpts * rtOpts, Boolean quiet);
void saveDrift(PtpClock * ptpClock, RunTimeOpts * rtOpts, Boolean quiet);

/** \}*/

/** \name timer.c (Unix API dependent)
 * -Handle with timers*/
 /**\{*/
void catch_alarm(int sig);
void initTimer(void);
void timerUpdate(IntervalTimer*);
void timerStop(UInteger16,IntervalTimer*);

//void timerStart(UInteger16,UInteger16,IntervalTimer*);

/* R135 patch: we went back to floating point periods (for less than 1s )*/
void timerStart(UInteger16 index, UInteger32 interval, IntervalTimer * itimer);

/* Version with randomized backoff */
void timerStart_random(UInteger16 index, UInteger32 interval, IntervalTimer * itimer);

Boolean timerExpired(UInteger16,IntervalTimer*);
Boolean timerStopped(UInteger16,IntervalTimer*);
Boolean timerRunning(UInteger16,IntervalTimer*);
/** \}*/

void
reset_operator_messages(RunTimeOpts * rtOpts, PtpClock * ptpClock);


//#ifdef PTPD_STATISTICS
////void updatePtpEngineStats (PtpClock* ptpClock, RunTimeOpts* rtOpts);
//#endif /* PTPD_STATISTICS */

void writeStatusFile(PtpClock *ptpClock, RunTimeOpts *rtOpts, Boolean quiet);

#endif /*PTPD_DEP_H_*/
