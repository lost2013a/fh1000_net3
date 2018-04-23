/* Includes ------------------------------------------------------------------*/
#include "flash_conf.h"


//��ȡָ����ַ�İ���(16λ����) 
//faddr:����ַ 
//����ֵ:��Ӧ����.
u32 STMFLASH_ReadWord(u32 faddr)
{
	return *(vu32*)faddr; 
} 


void FLASH_Init(void)
{
	//��ȡflash����Ҫ����
	FLASH_Unlock();//�����Ӷ��ɶ�flashģ��Ĵ���д
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
								FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR |FLASH_FLAG_PGSERR);
	//��������,��������,д��������,��̶������,��̲���λ������,���˳�����
}
void EraseSector(uint32_t EraseAddr,uint32_t NumToErase)
{
	int i;
	uint32_t endaddr=0;
	uint16_t StartSector,EndSector;
	endaddr = EraseAddr+NumToErase;
	
	FLASH_Init();
	
	StartSector = GetSector(EraseAddr);//��ȡд����ʼ��ַ�����������
	EndSector = GetSector(endaddr);//��ȡ������ַ���������ı��
	
	FLASH_DataCacheCmd(DISABLE);//FLASH�����ڼ�,�����ֹ���ݻ���!!!
	for(i=StartSector;i<=EndSector;i += 8)//�������ÿ������8
	{
		//��������i��
		if(FLASH_EraseSector(i,VoltageRange_3) != FLASH_COMPLETE)
		{
			while(1);//����ʧ��
		}
	}
	FLASH_DataCacheCmd(ENABLE);//FLASH��������,��������fetch
}

void FLASH_Write(uint32_t WriteAddr,int32_t *pBuffer,uint32_t NumToWrite)
{
	FLASH_Status status = FLASH_COMPLETE;
	uint32_t addrx=0;
	uint32_t endaddr=0;	
	if(WriteAddr<0x08000000||WriteAddr%4)return;	//�Ƿ���ַ
	FLASH_Init();	//���� 
	FLASH_DataCacheCmd(DISABLE);//FLASH�����ڼ�,�����ֹ���ݻ���!!!
	addrx 	= WriteAddr;	//д�����ʼ��ַ
	endaddr = WriteAddr+(NumToWrite*4);	//д��Ľ�����ַ
	

	if(addrx<0X1FFF0000)			//ֻ�����洢��,����Ҫִ�в�������!!
		{
			while(addrx<endaddr)		
				{
					if(STMFLASH_ReadWord(addrx)!=0XFFFFFFFF)//�з�0XFFFFFFFF�ĵط�,Ҫ�����������
						{   
							status=FLASH_EraseSector(GetSector(addrx),VoltageRange_3);//VCC=2.7~3.6V֮��!!
							if(status!=FLASH_COMPLETE)break;	//����������
						}else addrx+=4;
				}	
		}
	if(status==FLASH_COMPLETE)
		{
			while(WriteAddr<endaddr)//д����
			{
				if(FLASH_ProgramWord(WriteAddr,*pBuffer)!=FLASH_COMPLETE)//д������
				{ 
					break;	//д���쳣
				}
				WriteAddr+=4;
				pBuffer++;
			} 
		}
	FLASH_DataCacheCmd(ENABLE);//FLASH��������,��������fetch
	FLASH_Lock();//����
}


void FLASH_Read(uint32_t ReadAddr,int32_t *pBuffer,uint32_t NumToRead)   	
{
	uint32_t i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]= *(__IO uint32_t*)ReadAddr;//��ȡ4���ֽ�.
		ReadAddr+=4;//ƫ��4���ֽ�.	
	}
}

//������Ҫ��ȡ��flash��ַת���������ı��
uint16_t GetSector(uint32_t Address)
{
  uint16_t sector = 0;
  
  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_Sector_0;  
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_Sector_1;  
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_Sector_2;  
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_Sector_3;  
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_Sector_4;  
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_Sector_5;  
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_Sector_6;  
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_Sector_7;  
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_Sector_8;  
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_Sector_9;  
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_Sector_10;  
  }
  else if((Address < ADDR_FLASH_SECTOR_12) && (Address >= ADDR_FLASH_SECTOR_11))
  {
    sector = FLASH_Sector_11;  
  }
	else if((Address < ADDR_FLASH_SECTOR_13) && (Address >= ADDR_FLASH_SECTOR_12))
  {
    sector = FLASH_Sector_12;  
  }
	else if((Address < ADDR_FLASH_SECTOR_14) && (Address >= ADDR_FLASH_SECTOR_13))
  {
    sector = FLASH_Sector_13;  
  }
	else if((Address < ADDR_FLASH_SECTOR_15) && (Address >= ADDR_FLASH_SECTOR_14))
  {
    sector = FLASH_Sector_14;  
  }
	else if((Address < ADDR_FLASH_SECTOR_16) && (Address >= ADDR_FLASH_SECTOR_15))
  {
    sector = FLASH_Sector_15;  
  }
	else if((Address < ADDR_FLASH_SECTOR_17) && (Address >= ADDR_FLASH_SECTOR_16))
  {
    sector = FLASH_Sector_16;  
  }
	else if((Address < ADDR_FLASH_SECTOR_18) && (Address >= ADDR_FLASH_SECTOR_17))
  {
    sector = FLASH_Sector_17;  
  }
	else if((Address < ADDR_FLASH_SECTOR_23) && (Address >= ADDR_FLASH_SECTOR_18))
  {
    sector = FLASH_Sector_18;  
  }
	else /*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_23)*/
	{
		sector = FLASH_Sector_23;
	}
  return sector;
}
