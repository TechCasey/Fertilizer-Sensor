#include "gd32f4xx.h"
#include "gd32f4xx_libopt.h"
#include "systick.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "string.h"
#include "mytask.h"
#include "gd32f4xx_it.h"
#include "CAN.h"
#include "Flash.h"


#define APP_Start  ((uint32_t)0x08010000U)
/*种子传感器程序标记，用于跟单行控制器程序区分*/
const uint8_t  SeedSensor_Mark[4] __attribute__ ((at(APP_Start+450)))= {'S','E','E','D'};


/*******************************************
*函数:       main(void)
*说明:       主函数
********************************************/
int  main(void)
{
    SCB->VTOR = APP_Start;

    //硬件初始化
    Hardware_Init();
    //节点检查
#if defined Auto_ID
	
    NodeID_Check();
	
#endif

    //判断更新标志位,是否更新后跳转
    CAN_Type_Check(Flash_ReadData(Update_WRITE_START_ADDR, UpdateWordNum),NodeID);
    //开CAN中断
    can_interrupt_enable(CAN0, CAN_INT_RFNE0);
	

	
    //创建开始任务
    xTaskCreate((TaskFunction_t )start_task,
                (const char*    )"start_task",
                (uint16_t       )START_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )START_TASK_PRIO,
                (TaskHandle_t*  )&StartTask_Handler);
    //开始任务调度
    vTaskStartScheduler();
    return 0;
}




