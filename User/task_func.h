#ifndef __TASK_FUNC_H
#define __TASK_FUNC_H
#include "stdint.h"
#include "stdlib.h"
#include "mytask.h"
#include "RollAvg.h"
#include "gd32f4xx_libopt.h"
#include "Flash.h"
#include "CAN.h"
#include "gd32f4xx_it.h"

#define  LED_R    1U
#define  LED_B    2U
#define  LED_G    3U
#define PERCENT_AVG (10U)
#define PERCENT_MIN (5U)
#define PERCENT_MAX (15U)
/*种子计数范围*/
#define START_SEED_INDEX (5U)
#define END_SEED_INDEX (500U)
//ADC相关数据
typedef struct
{
    uint32_t  allAD_Buf[300]; //DMA数组
    uint32_t  ADC1_Buf[100];  //ADC1数组
    uint32_t  ADC2_Buf[100];	//ADC2数组
    __IO	uint32_t  ADC1_Val;
    __IO	uint32_t	ADC2_Val;
} ADCVoltage;

//CAN数据接收
typedef struct
{
    __IO uint16_t	   Car_Speed; 	 //车速
    __IO uint16_t	   seed_Speed; 	 //种轴转速
    __IO uint16_t    Gear_num; 		 //测速齿数
    __IO uint16_t    Scalar;        //标定量
    __IO uint8_t	   Alarm_clear;   //报警
    __IO uint16_t    Spacing; 		 //株距
    __IO uint64_t    ROW_NUM;      //行数
    __IO uint8_t     Sensitivity;   //灵敏度
    __IO uint8_t     Reset_SeedNum;   //清零

    uint8_t     rx_data[8];  	//数据缓存数组
} CAN_RXDATA;

///CAN数据发送
typedef struct
{
    __IO uint8_t     SeedCnt_err; 		//落种异常  堵种0x02  漏种 0x01
    __IO uint32_t    Seeding_Rate; 		//播种量
    __IO uint32_t 	 SeedNum_Cnt; 		//种子计数值
    __IO uint16_t    Variation_Gain;		//变异系数
    __IO bool     Work_State;           //工作状态
    __IO uint16_t    Seed_detection;		//落种检测次数
} CAN_SENDATA;
//DA输出
typedef struct
{
    __IO	 uint16_t DA2_OUT;   //灵敏度调节（DA2）
    __IO	 uint16_t DA1_OUT;   //发光强度调节(DA1)

} DAout;

extern  CAN_RXDATA CAN_REC_DATA;
extern  CAN_SENDATA SeedMES;
extern ADCVoltage GetAD;
extern  DAout da_out;

void GetAD_Filter(ADCVoltage *GetADC);  //ADC数据处理函数
void LED_Spark(uint8_t LEDX);  //led反转函数
void Timer_CapConfig(uint16_t  edge);
void CANUpload(uint8_t *txbuf,uint8_t CANID);
void NodeID_Send(uint8_t ID);
void NodeID_Check(void);
void Can_Rx_Mes(void);
void bianyixishu_count_in_handler(void);//数据处理
void bianyixishu_calc_in_mainloop(unsigned char record_data[PERCENT_MAX-PERCENT_MIN]);//计算变异系数
void bianyixishu_clear_func(void);//变异系数计算数据清零

void CAN_Type_Check(uint32_t Read_Type,uint8_t ID);
#endif





