#ifndef __FLASH_IF_H
#define __FLASH_IF_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

#define FLASH_START_ADDR 0x08000000//flash��ʼ��ַ
#define FLASH_END_ADDR	 0x081E0000//flash������ַ
#define FLASH_SAVE_ADDR  0x08140000//����Ҫд�����ݵĵ�ַ����18
#define	FLASH_SAVE_ADDR1 0x081E0000
//#define FLASH_SAVE_ADDR2 0x08120000


//FLASH ��������ʼ��ַ
#define ADDR_FLASH_SECTOR_0     ((u32)0x08000000) 	//����0��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_1     ((u32)0x08004000) 	//����1��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_2     ((u32)0x08008000) 	//����2��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_3     ((u32)0x0800C000) 	//����3��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_4     ((u32)0x08010000) 	//����4��ʼ��ַ, 64 Kbytes  
#define ADDR_FLASH_SECTOR_5     ((u32)0x08020000) 	//����5��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_6     ((u32)0x08040000) 	//����6��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_7     ((u32)0x08060000) 	//����7��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_8     ((u32)0x08080000) 	//����8��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_9     ((u32)0x080A0000) 	//����9��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_10    ((u32)0x080C0000) 	//����10��ʼ��ַ,128 Kbytes  
#define ADDR_FLASH_SECTOR_11    ((u32)0x080E0000) 	//����11��ʼ��ַ,128 Kbytes  
#define ADDR_FLASH_SECTOR_12    ((u32)0x08100000) 	//����12��ʼ��ַ,16 Kbytes  
#define ADDR_FLASH_SECTOR_13    ((u32)0x08104000) 	//����13��ʼ��ַ,16 Kbytes
#define ADDR_FLASH_SECTOR_14    ((u32)0x08108000) 	//����14��ʼ��ַ,16 Kbytes 
#define ADDR_FLASH_SECTOR_15    ((u32)0x0810C000) 	//����15��ʼ��ַ,16 Kbytes 
#define ADDR_FLASH_SECTOR_16    ((u32)0x08110000) 	//����16��ʼ��ַ,128 Kbytes  
#define ADDR_FLASH_SECTOR_17    ((u32)0x08120000) 	//����17��ʼ��ַ,128 Kbytes  
#define ADDR_FLASH_SECTOR_18    ((u32)0x08140000) 	//����18��ʼ��ַ,128 Kbytes  
#define ADDR_FLASH_SECTOR_23    ((u32)0x081E0000) 	//����23��ʼ��ַ,128 Kbytes  


//#define FLASH_Sector_0     ((uint16_t)0x0000) /*!< Sector Number 0 */
//#define FLASH_Sector_1     ((uint16_t)0x0008) /*!< Sector Number 1 */
//#define FLASH_Sector_2     ((uint16_t)0x0010) /*!< Sector Number 2 */
//#define FLASH_Sector_3     ((uint16_t)0x0018) /*!< Sector Number 3 */
//#define FLASH_Sector_4     ((uint16_t)0x0020) /*!< Sector Number 4 */
//#define FLASH_Sector_5     ((uint16_t)0x0028) /*!< Sector Number 5 */
//#define FLASH_Sector_6     ((uint16_t)0x0030) /*!< Sector Number 6 */

void FLASH_Init(void);//����flash�����һЩ���
uint16_t GetSector(uint32_t Address);//ͨ����ʼ��ַ��ȡ�����������
void FLASH_Read(uint32_t ReadAddr,int32_t *pBuffer,uint32_t NumToRead);//��ָ����flash��ȡ����
void FLASH_Write(uint32_t WriteAddr,int32_t *pBuffer,uint32_t NumToWrite);//д�����ݵ�ָ���ĵط�
void EraseSector(uint32_t EraseAddr,uint32_t NumToErase);

#endif 
