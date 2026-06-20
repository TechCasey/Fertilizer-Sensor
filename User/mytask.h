#ifndef __MYTASK_H
#define __MYTASK_H

#include "gd32f4xx.h"
#include "stdbool.h"
#include "FreeRTOS.h"
#include "task.h"
#include "task_func.h"
#include "systick.h"
#include <stdio.h>
#include <stdint.h>
#include "string.h"
#include "stdlib.h"
#include "gd32f4xx_it.h"   //中断服务函数
#include "ADC.h"           // ADC
#include "timer.h"         //定时器
#include "CAN.h"           //CAN
#include "EXTI.h"
#include "limits.h"
#include "timers.h"

////->工作模式选择////
#define test_mode   	//测试模式
//#define All_Relase    //放大倍数全开（一般不需要开）


#define Contorller  		 //带控制器（关闭不带控制器,可单机调试）

#define Auto_ID   			 //自动识别ID
 
#define CAN_SeedType     //控制器下发种子类型（关闭根据株距判断）

#define ROW_SET          //控制器发送行数设置（未被设置的行不发CAN报文）



////->相关参数设置////
/***********************玉米**********************/
/*AD1 DA2*/
#define Corn_AD1    1950
#define Corn_DA2    1850
/*脉宽*/
#define Corn_LTim   1000  
#define Corn_HTim	  15000
/*灰尘补偿调节范围*/
#define Corn_DustLowAD1  1930
#define	Corn_DustHigAD1	 1980

/***********************大豆**********************/
//(暂时用和玉米一样的参数)
/*AD1 DA2*/
#define Soybean_AD1	   1950
#define Soybean_DA2    1850
/*脉宽*/
#define Soybean_LTim   1000
#define Soybean_HTim   15000
/*灰尘补偿调节范围*/
#define Soybean_DustLowAD1 		1930
#define	Soybean_DustHigAD1		1980

/***********************高粱**********************/
/*AD1 DA2*/

#define Sorghum_AD1  	2480

#if defined All_Relase

#define Sorghum_DA2  	2400

#else

#define Sorghum_DA2  	2360

#endif

/*脉宽*/
#define Sorghum_LTim  	250
#define Sorghum_HTim	  15000
/*灰尘补偿调节范围*/
#define Sorghum_DustLowAD1 2450
#define	Sorghum_DustHigAD1 2480



/*********************************************************************************************************************/
/**********************************************************************************************************************/

#define   Dust_compensation_EventB0   (1<<0)  //灰尘补偿事件
//#define   SeedCnt_EventB1           	(1<<1)  //种子计数事件

/*****************
*开始任务
*****************/
//任务优先级
#define START_TASK_PRIO		1
//任务堆栈大小	
#define START_STK_SIZE 		1024
//任务句柄
extern TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void *pvParameters);

#if defined test_mode

/*********
*测试任务
**********/
//任务优先级
#define TEST_TASK_PRIO		2
//任务堆栈大小	
#define TEST_STK_SIZE 		256
//任务句柄
extern TaskHandle_t TestTask_Handler;
//任务函数
void Test_task(void *pvParameters);


/***************
*PID灰尘补偿任务
***************/
//任务优先级
#define TASK2_TASK_PRIO		3
//任务堆栈大小	
#define TASK2_STK_SIZE 	    256
//任务句柄
extern TaskHandle_t Dust_compensation_Task_Handler;
//任务函数
void Dust_compensation_Task(void *pvParameters);

#else
/***************
*状态监测任务
****************/
//任务优先级
#define TASK1_TASK_PRIO		2
//任务堆栈大小	
#define TASK1_STK_SIZE 		128
//任务句柄
extern TaskHandle_t Condition_monitorHandler;
//任务函数
void Condition_monitor(void *pvParameters);



/***************
*AD值处理任务
****************/
//任务优先级
#define TASK3_TASK_PRIO		2
//任务堆栈大小	
#define TASK3_STK_SIZE 		256  
//任务句柄
extern TaskHandle_t AD_Voltage_Task_Handler;
//任务函数
void AD_Voltage_Task(void *pvParameters);


#define TASK4_TASK_PRIO		4
//任务堆栈大小	
#define TASK4_STK_SIZE 		128  
//任务句柄
extern TaskHandle_t SmallSeed_Task_Handler;
//任务函数
void SmallSeed_Task(void *pvParameters);






#endif



typedef struct
{
	uint8_t seedQualified;
	uint8_t gearOffset;
	uint16_t seedCnt;
	uint16_t currGear;
	uint16_t lastGear;
}decSeedAbnormal_t;

extern decSeedAbnormal_t sDecSeedAbnormal;



extern __IO bool     Work_State; //工作状态标志位
extern __IO uint8_t  Seed_Type;
extern __IO uint8_t  NodeID;  //节点ID
extern __IO uint16_t last_value;
void Hardware_Init(void); //硬件初始化

#endif























