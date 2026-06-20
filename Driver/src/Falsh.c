
#include "Flash.h"
#include "gd32f4xx.h"
#include "stdbool.h"


uint32_t *ptrd;
uint32_t address = 0x00000000U;


//uint32_t PageNum = (FMC_WRITE_END_ADDR - FMC_WRITE_START_ADDR) / FMC_PAGE_SIZE;
//uint32_t WordNum = ((FMC_WRITE_END_ADDR - FMC_WRITE_START_ADDR) >> 2);//相当于除4

//更新标志
uint32_t UpdatePageNum = (Update_WRITE_END_ADDR - Update_WRITE_START_ADDR) / Update_PAGE_SIZE;
uint32_t UpdateWordNum = ((Update_WRITE_END_ADDR - Update_WRITE_START_ADDR) >> 2);

uint32_t SensorIDPageNum= (SensorID_WR_END_ADDR - SensorID_WR_START_ADDR) / SensorID_PAGE_SIZE;
uint32_t SensorIDWordNum=	((SensorID_WR_END_ADDR - SensorID_WR_START_ADDR) >> 2);


//---------------------------------------------通用FLASH 操作函数-------------------------------------------------------------
/*
	擦写检查函数	
	开始检查地址  WRITE_START_ADDR
	检查结束地址  WRITE_END_ADDR
*/
uint32_t  Flash_EraseCheck(uint32_t WRITE_START_ADDR,uint32_t WRITE_END_ADDR)
{
	uint32_t *CheckADD;
	bool Erase_Faile=false;
    
	CheckADD = (uint32_t *)WRITE_START_ADDR;
	
      for(;CheckADD<(uint32_t *)WRITE_END_ADDR; CheckADD++)
	 {							 
		if((*CheckADD) != 0xFFFFFFFF)
		{
				Erase_Faile=true;
				break;
		}						      
      }
			
    if(Erase_Faile) return 0;
	else return 1;		
}
/*
			Flash 						写数据
			WRITE_START_ADDR  内存起始地址
			WRITE_END_ADDR    内存结束地址
			data              要写入的数据
*/
void Flash_WriteData (uint32_t WRITE_START_ADDR,uint32_t WRITE_END_ADDR,uint32_t data)
{
    fmc_unlock();

    address = WRITE_START_ADDR;

    while(address < WRITE_END_ADDR)
	{
		fmc_word_program(address, data);
		address += 4;
		fmc_flag_clear(FMC_STAT_END);
		fmc_flag_clear(FMC_STAT_WPERR);
		fmc_flag_clear(FMC_STAT_OPERR);
	}
	address=0x0000000;
    fmc_lock();
}

/***********************************************************************
函数：扇区擦除函数
参数：扇区地址			扇区1~4

      CTL_SECTOR_NUMBER_0: sector 0    0x08000000 - 0x08003FFF  16kb
      CTL_SECTOR_NUMBER_1: sector 1    0x08004000 - 0x08007FFF  16kb
      CTL_SECTOR_NUMBER_2: sector 2    0x08008000 - 0x0800BFFF  16kb
      CTL_SECTOR_NUMBER_3: sector 3    0x0800C000 - 0x0800FFFF  16kb
      CTL_SECTOR_NUMBER_4: sector 4    0x08010000 - 0x0801FFFF  64kb
************************************************************************/
void Flash_erase_Sector(uint32_t WriteAddr)
{
//    uint32_t EraseSector=0x00;  		
    fmc_unlock();

    fmc_flag_clear(FMC_STAT_END);
    fmc_flag_clear(FMC_STAT_WPERR);
    fmc_flag_clear(FMC_STAT_OPERR);
	//扇区选择
	if     ((0x08000000<=WriteAddr)&&(WriteAddr<=0x08003FFF)){fmc_sector_erase(CTL_SECTOR_NUMBER_0);}	
	else if((0x08004000<=WriteAddr)&&(WriteAddr<=0x08007FFF)){fmc_sector_erase(CTL_SECTOR_NUMBER_1);}
	else if((0x08008000<=WriteAddr)&&(WriteAddr<=0x0800BFFF)){fmc_sector_erase(CTL_SECTOR_NUMBER_2);}
	else if((0x0800C000<=WriteAddr)&&(WriteAddr<=0x0800FFFF)){fmc_sector_erase(CTL_SECTOR_NUMBER_3);}		
	else if((0x08010000<=WriteAddr)&&(WriteAddr<=0x0801FFFF)){fmc_sector_erase(CTL_SECTOR_NUMBER_4);}			
	
	fmc_flag_clear(FMC_STAT_END);
	fmc_flag_clear(FMC_STAT_WPERR);
	fmc_flag_clear(FMC_STAT_OPERR);		
    fmc_lock();
}



/*
		读取数据
		WRITE_START_ADDR  数据起始地址
		WordNum           数据量
*/
uint32_t Flash_ReadData(uint32_t WRITE_START_ADDR,uint32_t WordNum)
{
      uint32_t i,DATA=0;
      ptrd = (uint32_t *)WRITE_START_ADDR;
      for(i = 0; i < WordNum; i++)
			{
          if((*ptrd) != 0)
          {
              DATA =*ptrd;
              break;
          }
          else   ptrd++;
         
      }
      return DATA;
}


















