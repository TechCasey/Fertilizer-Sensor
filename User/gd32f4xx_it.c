#include "gd32f4xx_it.h"
#include "mytask.h"
#include "task_func.h"
#include "FreeRTOS.h"
#include "task.h"
//#define us1 (SystemCoreClock/1000000U) //1us

TimCnt Dust_compensation;			//灰尘补偿计时
TimCnt Dust_while1; 					  //灰尘补偿死循环
TimCnt Du; 					  //堵种死循环
TimCnt Que; 					  //缺种死循环
TimCnt Luo_Err; 					  //漏种死循环
TimCnt CanDisWait;      			 	//CAN数据接收超时时间
static  TimCnt ErrTim;          //缺种两粒种子间隔时间判断
__IO uint8_t  Tim2IT_Flg=0;
__IO uint16_t timer_cnt=0;

TimCnt ID_WaitTIM;  					    //等待ID判断时间
static  TimCnt ID_TimOut; 				//两段ID间隔时间判断
static  uint16_t CANTX_TIM=0; 		//can发送时间
__IO bool Cnt_flag=false;
static __IO  uint8_t Timer_UpdateFlg=0; //定时器0计数溢出中断标志

/****************************************************************************************************************
		NVIC_PriorityGroupConfig有5个选项，选择优先级使用哪一组，他们在不同组的取值范围是：
		        分组             抢占优先级                            	响应优先级
		NVIC_PriorityGroup_0	PreemptionPriority=0（只看SubPriority）	SubPriority=0-15
		NVIC_PriorityGroup_1	PreemptionPriority=0-1	                SubPriority=0-7
		NVIC_PriorityGroup_2	PreemptionPriority=0-3	                SubPriority=0-3
		NVIC_PriorityGroup_3	PreemptionPriority=0-7	                SubPriority=0-1
		NVIC_PriorityGroup_4	PreemptionPriority=0-15	 				SubPriority=0（只看PreemptionPriority）
		1组内相同的Preemption Priority不能抢占，SubPriority 低的优先执行（响应优先级顺序只在同抢占分组的条件下有效）
		如果中断比较多，又要抢占，选NVIC_PriorityGroup_4
 **   RTOS需要将分组选为4，RTOS中断全部为抢占，无子优先级
*****************************************************************************************************************/
void NVIC_Config(void)
{
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);

    /*RTOS不管理中断*/

    nvic_irq_enable(TIMER0_Channel_IRQn,1,0);	    //使能中断，配置中断的优先级,1代表抢占优先级，0代表响应优先级
    nvic_irq_enable(TIMER0_UP_TIMER9_IRQn,5,0);


    nvic_irq_enable(TIMER1_IRQn, 2,0);	           	//定时器1 1ms中断
    nvic_irq_enable(CAN0_RX0_IRQn,4,0);	         		//CAN0_FIFO0


    /*RTOS管理中断*/

}
//定时器0 更新中断使能函数
void Time0_UpIntEnable(bool enable)
{
    if(enable)
    {
        timer_interrupt_enable(TIMER0, TIMER_INT_UP);		//外设TIMER0中断使能
    }
    else
    {
        timer_interrupt_disable(TIMER0, TIMER_INT_UP);
    }
    timer_interrupt_flag_clear(TIMER0,TIMER_INT_FLAG_UP);//清除外设TIMER0的中断标志
    timer_flag_clear(TIMER0, TIMER_FLAG_UP);	//清除外设TIMER0更新标志

}
/*************************************
*定时器 0 更新中断，用于判断溢出
***************************************/
void TIMER0_UP_TIMER9_IRQHandler(void)
{
    if(timer_interrupt_flag_get(TIMER0,TIMER_FLAG_UP))
    {
        timer_flag_clear(TIMER0,TIMER_FLAG_UP); //清除外设TIMER0更新标志

        Timer_UpdateFlg++;//计数溢出中断标志

    }

}
//定时器 0 输入捕获中断，用于ID接收判断
void TIMER0_Channel_IRQHandler(void)
{
    uint16_t time=0;
    static uint16_t TimeValue1=0,TimeValue2=0;
    static uint8_t SensorID=0;

    if(timer_interrupt_flag_get(TIMER0,TIMER_INT_CH2))//获取外设TIMER0中断标志
    {
        timer_flag_clear(TIMER0,TIMER_INT_CH2); //清除外设TIMER0状态标志

        CANTX_TIM=0;
        SensorID++;
        //当前时间=读取的通道捕获值
        TimeValue1=	timer_channel_capture_value_register_read(TIMER0,TIMER_CH_2);
        //开启更新中断
        Time0_UpIntEnable(ENABLE);

        switch(Timer_UpdateFlg)
        {
        case 0:
        case 1:
        case 2:
            if(TimeValue1>TimeValue2)
            {
                time=TimeValue1-TimeValue2;
            }
            else if(TimeValue2>TimeValue1)
            {
                time=65535-TimeValue2+TimeValue1;
            }
            if(time>=30000)
            {
                NodeID=SensorID-1;
                SensorID=1;	 //下一组开头
            }
            break;

        default:
            SensorID=1;
            break;
        }
        //记录本次时间，历史值
        TimeValue2=TimeValue1;
        TimeValue1=0;
        Timer_UpdateFlg=0;
    }
}

/************************************************************
 * 函数:       TIMER1_IRQHandler(void)
 * 说明:       定时器1中断服务函数
	中段时间：   1ms
**************************************************************/
void TIMER1_IRQHandler(void)
{
    uint8_t   CanTXbuf[8]= {0};  	 	//CAN数据发送数组

    static		uint8_t  delay_cnt=0,delay_CAN=0;

    if(timer_interrupt_flag_get(TIMER1,TIMER_FLAG_UP))
    {
        timer_flag_clear(TIMER1,TIMER_FLAG_UP);

        delay_decrement(); //delayms延时

        //节点ID等待时间
        if(ID_WaitTIM.Flag) {
            ID_WaitTIM.CNT++;
        }

        //缺种种子间隔时间(防溢出)
        if(ErrTim.Flag) {
            ErrTim.CNT++;
            if(ErrTim.CNT>4294967000) {
                ErrTim.CNT=0;
            }
        }

        if(ID_TimOut.Flag) {
            ID_TimOut.CNT++;
        }
        //灰尘补偿计时
        if(Dust_compensation.Flag) {
            Dust_compensation.CNT++;
        }
        //灰尘死循环计时
        if(Dust_while1.Flag) {
            Dust_while1.CNT++;
        }

        /*---------堵种死循环计时--------*/
        if(Du.Flag) {
            Du.CNT++;
        }
        /*---------缺种死循环计时--------*/
        if(Que.Flag) {
            Que.CNT++;
        }
        /*---------漏种死循环计时--------*/
        if(Luo_Err.Flag) {
            Luo_Err.CNT++;
        }

        //CAN断开连接计时
        if(++CanDisWait.CNT>=1000) {
            CanDisWait.Flag=true;
            CanDisWait.CNT=0;
        }

        //CAN发送
        if(++CANTX_TIM>=100) //100MS
        {
            /*
            	数据放定时器中断发送，可实现CAN一直有数据输出
            	如果建立独立任务，由于PID灰尘补偿用死循环调节，在PID调节时会造成CAN发送任务等待，
            	从而导致短时CAN无数据发出（特别是判断堵种时）
            */
            if(delay_CAN<10)
            {
                delay_CAN++;
            }
            else
            {
                CANTX_TIM=0;
                CANUpload(CanTXbuf,NodeID);
            }
        }
        if(Cnt_flag) {
            if(++delay_cnt>=10) {
                delay_cnt=0;
                Cnt_flag=false;
            }
        }
    }
}
/*******************************************************
 * 函数:       void CAN0_RX0_IRQHandler(void)
 * 说明:       CAN0接收 中断函数，can fifo0 有数据就触发
********************************************************/
extern can_receive_message_struct receive_message;
void CAN0_RX0_IRQHandler(void)
{
    if(can_interrupt_flag_get(CAN0,CAN_INT_FLAG_RFL0)) //检查CAN0接口的接收FIFO 0是否已满
    {

        can_interrupt_flag_clear(CAN0,CAN_INT_FLAG_RFL0);  //清除接收FIFO 0的满标志
        can_message_receive(CAN0, CAN_FIFO0, &receive_message);//从该FIFO中读取一条CAN消息，并将其存储在receive_message结构体中

        /*CAN任务处理函数*/
        Can_Rx_Mes();		//  CAN接收数据处理函数
        memset(receive_message.rx_data,0,8);
    }
}

/*****************
定时器2 中断
******************/
void TIMER2_IRQHandler(void)
{
    static uint16_t TimeValue1=0,TimeValue2=0,time=0;
    static uint8_t IT_NUM=0;

    if(timer_interrupt_flag_get(TIMER2,TIMER_INT_CH0))
    {
        timer_flag_clear(TIMER2,TIMER_INT_CH0);

        if(Work_State)
        {

            switch(IT_NUM)
            {
            case 0:
                IT_NUM=1;
                TimeValue1=	timer_channel_capture_value_register_read(TIMER2,TIMER_CH_0);//从 TIMER2 的通道0的捕获值寄存器中读取值
                Timer_CapConfig(TIMER_IC_POLARITY_FALLING); //下降沿
                break;
            case 1:
                Tim2IT_Flg =1;
                IT_NUM=0;
                TimeValue2=	timer_channel_capture_value_register_read(TIMER2,TIMER_CH_0);
                Timer_CapConfig(TIMER_IC_POLARITY_RISING); //上升沿
                (TimeValue2>TimeValue1)?(time=(TimeValue2-TimeValue1)):(time =((60000-1)-TimeValue1)+TimeValue2);
                timer_cnt=time;
                switch(Seed_Type)
                {
                case 1:
                    if((time>=Corn_LTim)&&(time<=Corn_HTim))  	/*玉米*/
                    {
                        SeedMES.SeedNum_Cnt++; //正常计数
                    }
                    break;
                case 2:
                    if((time>=Soybean_LTim)&&(time<=Soybean_HTim)) /*大豆*/
                    {
                        SeedMES.SeedNum_Cnt++;
                    }
                    break;
                case 3:
                    if((time>=Sorghum_LTim)&&(time<=Sorghum_HTim)) /*高粱*/
                    {
                        SeedMES.SeedNum_Cnt++;
                    }
                    break;
                default:
                    if((time>=Corn_LTim)&&(time<=Corn_HTim))
                    {
                        SeedMES.SeedNum_Cnt++;
                    }
                    break;
                }

                time=0;
                TimeValue2=0;
                TimeValue1=0;
                break;
            default:
                IT_NUM=0;
                TimeValue1=0;
                TimeValue2=0;
                Timer_CapConfig(TIMER_IC_POLARITY_RISING);
                break;
            }


        }
    }
}

/************************************************************
 * 函数:       void DMA1_Channel0_IRQHandler(void))
 * 说明:       DMA1通道0 中断函数，DMA全部完成触发中断
用来检查并清除DMA传输完成的中断标志
**************************************************************/
void DMA1_Channel0_IRQHandler(void)
{
    if(dma_interrupt_flag_get(DMA1,DMA_CH0,DMA_INT_FLAG_FTF))   //获取DMA通道传输中断完成标志
    {
        dma_interrupt_flag_clear(DMA1,DMA_CH0,DMA_INT_FLAG_FTF); //清除DMA通道传输中断完成标志
    }
}




void NMI_Handler(void)
{



}


/************************************************************
 * 函数:       void EXTI4_IRQHandler(void)
* 说明:       外部中断服务函数，下降沿触发，当下降沿来临时，开始计时，然后改变为上升沿触发，跳出；
              等上升沿来临时，将计时清零，然后改变为下降沿触发，跳出。
              下一次下降沿来临时，重新开始计时，若一直计时，则证明没有上升沿，表示播种异常。
**************************************************************/
int interrupt_occurred = 0;//定义一个全局中断标志位
int interrupt_occurred_check = 0;//检测次数标志位
int Seed_detection=0;    //种子检测次数
//void EXTI4_IRQHandler(void) {
//    static uint8_t EXTI4_NUM=0;
//    //有种子信号变化时首先将种子异常状态清零
//    SeedMES.SeedCnt_err = 0x00;//
//    Que.CNT=0;//清除缺种计时
//    Du.CNT=0;//清除堵种计时
//    Que.Flag =false;
//    Du.Flag =false;
//    Luo_Err.CNT = 0;//清除落种计时
////    //任务通知（进入中断就进行播种量计算的任务通知）
////    xTaskNotify((TaskHandle_t	)	Dust_compensation_Task_Handler,//灰尘补偿任务
////                (uint32_t		)	Dust_compensation_EventB0,
////                (eNotifyAction	)	eSetBits);

//    if (exti_interrupt_flag_get(EXTI_4) != RESET)  //==SET
//    {
//        interrupt_occurred = 1;//进中断标志位置1
//        interrupt_occurred_check = 1;//检测次数标志位
//        SeedMES.Work_State = true;//有落种就将工作状态置1
//
//        // 改变边沿
//        switch(EXTI4_NUM)
//        {
//        case 0:
//            EXTI4_NUM=1;
////            Luo_Err.CNT = 0;
////            Luo_Err.Flag=false;
//            exti_init(EXTI_4, EXTI_INTERRUPT, EXTI_TRIG_RISING);    //下降
//            break;
//        case 1:
//            EXTI4_NUM=0;
////            Luo_Err.Flag=true;
//			sDecSeedAbnormal.seedCnt++;
//            if((CAN_REC_DATA.Car_Speed==0)||(CAN_REC_DATA.seed_Speed==0))
//            {
//                Seed_detection++;//中断检测到落种次数一次，检测次数+1
//            }
//            exti_init(EXTI_4, EXTI_INTERRUPT, EXTI_TRIG_FALLING);    //上升
//            break;
//        }

//        // 清除中断标志
//        exti_interrupt_flag_clear(EXTI_4);
//    }

//}

uint32_t seed_axle_cnt;
uint8_t flag = 0;

/* 定时器中断处理函数 */
void TIMER5_DAC_IRQHandler(void)
{
    if (timer_interrupt_flag_get(TIMER5,TIMER_INT_FLAG_UP))
    {

        //获取IO状态
        if(gpio_input_bit_get(GPIOB,GPIO_PIN_4))
        {
            flag = 1;
        }

        if ((gpio_input_bit_get(GPIOB,GPIO_PIN_4) == 0) && flag == 1)
        {
			sDecSeedAbnormal.seedCnt++;
            
            if ((CAN_REC_DATA.Car_Speed == 0) || (CAN_REC_DATA.seed_Speed == 0))
            {
                Seed_detection++;//中断检测到落种次数一次，检测次数+1
            }
            interrupt_occurred = 1;//进中断标志位置1
            interrupt_occurred_check = 1;//检测次数标志位
            SeedMES.Work_State = true;//有落种就将工作状态置1
            flag = 0;
        }

    }
    timer_interrupt_flag_clear(TIMER5,TIMER_INT_FLAG_UP);
}






/*!
    \brief      this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void HardFault_Handler(void)
{
    /* if Hard Fault exception occurs, go to infinite loop */
    while (1) {

    }
}

/*!
    \brief      this function handles MemManage exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void MemManage_Handler(void)
{
    /* if Memory Manage exception occurs, go to infinite loop */
    while (1) {

    }
}

/*!
    \brief      this function handles BusFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void BusFault_Handler(void)
{
    /* if Bus Fault exception occurs, go to infinite loop */
    while (1) {
    }
}

/*!
    \brief      this function handles UsageFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UsageFault_Handler(void)
{
    /* if Usage Fault exception occurs, go to infinite loop */
    while (1) {
    }
}

/*!
    \brief      this function handles SVC exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
//void SVC_Handler(void)
//{
//}

/*!
    \brief      this function handles DebugMon exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void DebugMon_Handler(void)
{
}

/*!
    \brief      this function handles PendSV exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
//void PendSV_Handler(void)
//{
//}





