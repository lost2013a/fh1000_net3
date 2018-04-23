#include "arm2fpga.h"
#include "main.h"
#include "share.h"
#include "sys.h"

extern unsigned char leap61;
extern unsigned char leap59;
extern unsigned char leapwring;
extern unsigned char synflags;


void reversestr(char *source,char target[],unsigned int length)
{
	int i;
	for(i=0;i<length;i++)
		target[i]=source[length-1-i];
	target[i]=0;
}


void IntToHex(unsigned long num,char *hexStr)
{
	unsigned long n = num;
	char hextable[]="0123456789ABCDEF";
	char temphex[16],hex[16];
	int i=0;
	while(n){
		temphex[i++]=hextable[n%16];
		n /= 16;
	}
	temphex[i]=0;
	reversestr(temphex,hex,i);
	strcpy(hexStr,hex);
}


void ARM_to_FPGA(void)
{
	ARMtoFPGAstruct   ARM_to_FPGAM;
	TimeInternal  Localtime;
	tTime         L_sLocalTime;
	char buf[4]={0};
	
	ARM_to_FPGAM.framhead1 = 0x90;
	ARM_to_FPGAM.framhead2 = 0xeb;
	ARM_to_FPGAM.command   = 0x11;
	ARM_to_FPGAM.slotadr   = 0x51;
	ARM_to_FPGAM.messagelen = sizeof(ARM_to_FPGAM) - 5;

	getTime(&Localtime);
	ulocaltime(Localtime.seconds, &L_sLocalTime);//将秒时间转换为年月日时分秒
	
	IntToHex(L_sLocalTime.usYear,buf);
	ARM_to_FPGAM.year = *((short *)buf);
	memset(buf,0,sizeof(buf));
	
	IntToHex((L_sLocalTime.ucMon+1),buf);
	ARM_to_FPGAM.month = *((char *)buf);
	memset(buf,0,sizeof(buf));
	
	IntToHex(L_sLocalTime.ucMday,buf);
	ARM_to_FPGAM.day = *((char *)buf);
	memset(buf,0,sizeof(buf));
	
	L_sLocalTime.ucHour = (L_sLocalTime.ucHour+8)%24;//	小时加8 是将UTC时间转换成北京时间
	IntToHex(L_sLocalTime.ucHour,buf);
	ARM_to_FPGAM.hour = *((char *)buf);
	memset(buf,0,sizeof(buf));
	
	IntToHex(L_sLocalTime.ucMin,buf);
	ARM_to_FPGAM.minute = *((char *)buf);
	memset(buf,0,sizeof(buf));
	
	IntToHex(L_sLocalTime.ucSec,buf);
	ARM_to_FPGAM.seconds = *((char *)buf);
	memset(buf,0,sizeof(buf));
	
//	IntToHex(L_sLocalTime.usYear,(char *)&ARM_to_FPGAM.year);
//	IntToHex((L_sLocalTime.ucMon+1),(char*)&ARM_to_FPGAM.month); //L_sLocalTime.ucMon
//	IntToHex(L_sLocalTime.ucMday,(char *)&ARM_to_FPGAM.day); //L_sLocalTime.ucMday
//	L_sLocalTime.ucHour = (L_sLocalTime.ucHour+8)%24;//	小时加8 是将UTC时间转换成北京时间
//	IntToHex(L_sLocalTime.ucHour,(char *)&ARM_to_FPGAM.hour);
//	IntToHex(L_sLocalTime.ucMin, (char *)&ARM_to_FPGAM.minute);
//	IntToHex(L_sLocalTime.ucSec, (char *)&ARM_to_FPGAM.seconds);


//	printf("[ %s-",(char *)&ARM_to_FPGAM.year);
//	printf(" %s-", (char *)&ARM_to_FPGAM.month);
//	printf(" %s ] ",(char *)&ARM_to_FPGAM.day);
//	printf(" %s:",(char *)&ARM_to_FPGAM.hour);
//	printf(" %s:",(char *)&ARM_to_FPGAM.minute);
//	printf(" %s\n",(char *)&ARM_to_FPGAM.seconds);
	
//	if(leapwring)
//		ARM_to_FPGAM.leap = LI_ALARM; //未同步
//	if(leap61)
//		ARM_to_FPGAM.leap = LI_PLUSSEC;
//	else if(leap59)
//		ARM_to_FPGAM.leap = LI_MINUSSEC;
//	else
//		ARM_to_FPGAM.leap = LI_NOWARNING;//正常时间

//	if(synflags == 1)
//		ARM_to_FPGAM.sync_state = 0x41;
//	else
//		ARM_to_FPGAM.sync_state = 0x56;
	
//	ARM_to_FPGAM.sync_mode 			 = GlobalConfig.WorkMode;
//	ARM_to_FPGAM.Running_state 	 = G_ptpClock.portState;
//	ARM_to_FPGAM.bestmac_addr[0] = G_ptpClock.grandmasterIdentity[0];
//	ARM_to_FPGAM.bestmac_addr[1] = G_ptpClock.grandmasterIdentity[1];
//	ARM_to_FPGAM.bestmac_addr[2] = G_ptpClock.grandmasterIdentity[2];
//	ARM_to_FPGAM.bestmac_addr[3] = G_ptpClock.grandmasterIdentity[5];
//	ARM_to_FPGAM.bestmac_addr[4] = G_ptpClock.grandmasterIdentity[6];
//	ARM_to_FPGAM.bestmac_addr[5] = G_ptpClock.grandmasterIdentity[7];
//	ARM_to_FPGAM.mac_addr[0] = G_ptpClock.clockIdentity[0];
//	ARM_to_FPGAM.mac_addr[1] = G_ptpClock.clockIdentity[1];
//	ARM_to_FPGAM.mac_addr[2] = G_ptpClock.clockIdentity[2];
//	ARM_to_FPGAM.mac_addr[3] = G_ptpClock.clockIdentity[5];
//	ARM_to_FPGAM.mac_addr[4] = G_ptpClock.clockIdentity[6];
//	ARM_to_FPGAM.mac_addr[5] = G_ptpClock.clockIdentity[7];
//	if(G_ptpClock.delayMechanism == E2E)
//		ARM_to_FPGAM.path_delay =  G_ptpClock.meanPathDelay.nanoseconds;	
//	else if (G_ptpClock.delayMechanism == P2P)
//		ARM_to_FPGAM.path_delay = G_ptpClock.peerMeanPathDelay.nanoseconds;
//	
//	ARM_to_FPGAM.correctionField = G_ptpClock.lastSyncCorrectionField.nanoseconds;

	
	UARTSend((uint8_t *)&ARM_to_FPGAM,sizeof(ARM_to_FPGAM));//
}









