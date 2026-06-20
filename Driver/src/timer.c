#include "timer.h"
///*

/*
定时器0 1MS
200M
*/
void Timer0_Init(void)
{
    timer_ic_parameter_struct timer_icinitpara; //输入捕获
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER0);
    timer_deinit(TIMER0);

    // 定时器参数配置
    timer_initpara.prescaler         = 199;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = (65536-1);
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER0,&timer_initpara);

    //输入捕获
    timer_icinitpara.icpolarity  = TIMER_IC_POLARITY_RISING;
    timer_icinitpara.icselection = TIMER_IC_SELECTION_DIRECTTI;
    timer_icinitpara.icprescaler = TIMER_IC_PSC_DIV1;
    timer_icinitpara.icfilter    = 0x0;     //不滤波
    timer_input_capture_config(TIMER0,TIMER_CH_2,&timer_icinitpara); //tim0_ch3
    timer_auto_reload_shadow_enable(TIMER0);
    //定时器0 通道3中断
    timer_flag_clear(TIMER0, TIMER_FLAG_CH2);
    timer_interrupt_enable(TIMER0, TIMER_INT_CH2);
    timer_interrupt_flag_clear(TIMER0,TIMER_INT_CH2);

    timer_interrupt_enable(TIMER0, TIMER_INT_CH2);
    timer_interrupt_flag_clear(TIMER0,TIMER_INT_CH2);
    timer_enable(TIMER0);// 使能定时器

    /*定时器模式*/

////    中断使能
//	  timer_interrupt_enable(TIMER0, TIMER_INT_UP);
////    自动加载定时器参数
//    timer_auto_reload_shadow_enable(TIMER0);
////    使能定时器
//    timer_enable(TIMER0);
}
/****************
定时器1  1毫秒延时
100M
******************/
void Timer1_Init(void)
{
    timer_parameter_struct timer_initpara;
    rcu_periph_clock_enable(RCU_TIMER1);
    timer_deinit(TIMER1);
    // 定时器参数配置
    timer_initpara.prescaler         = 99;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = (1000-1);//1MS  中断
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER1,&timer_initpara);
//    中断使能
    timer_interrupt_enable(TIMER1, TIMER_INT_UP);
//    自动加载定时器参数
    timer_auto_reload_shadow_enable(TIMER1);
//    使能定时器
    timer_enable(TIMER1);
}
/*
定时器2
*/
void Timer2_Init(void)
{
    timer_ic_parameter_struct timer_icinitpara; //输入捕获
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER2);
    timer_deinit(TIMER2);//复位外设TIMER2

    // 定时器参数配置
    timer_initpara.prescaler         =99;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = (60000-1);//60MS
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER2,&timer_initpara);

    //输入捕获配置
    timer_icinitpara.icpolarity  = TIMER_IC_POLARITY_RISING;     //上升沿
    timer_icinitpara.icselection = TIMER_IC_SELECTION_DIRECTTI;
    timer_icinitpara.icprescaler = TIMER_IC_PSC_DIV1;
    timer_icinitpara.icfilter    = 0x4;                          //使能滤波功能
    timer_input_capture_config(TIMER2,TIMER_CH_0,&timer_icinitpara); //tim_ch0

    timer_auto_reload_shadow_enable(TIMER2);	//  自动加载定时器参数

    timer_interrupt_enable(TIMER2, TIMER_INT_CH0);
    timer_interrupt_flag_clear(TIMER2,TIMER_INT_CH0);
    timer_enable(TIMER2);  // 使能定时器
    //定时器2 优先级与Tim0 一样，TIM2位于TIM0后，TIM0被deinit
    nvic_irq_enable(TIMER5_DAC_IRQn,1,0);
}
/*TIMER3*/
void Timer3_init(void)
{
    timer_parameter_struct timer_initpara;
    timer_oc_parameter_struct timer_ocintpara;
    timer_deinit(TIMER3);
    rcu_periph_clock_enable(RCU_TIMER3);


    timer_initpara.prescaler         =((uint16_t)(99900/5)-1);
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 1000-1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER3, &timer_initpara);

    /* CH1 configuration in PWM mode0     PWM 模式下的 CH1 配置0*/
    timer_ocintpara.ocpolarity  = TIMER_OC_POLARITY_HIGH;
    timer_ocintpara.outputstate = TIMER_CCX_ENABLE;
    timer_ocintpara.ocnpolarity = TIMER_OCN_POLARITY_HIGH;
    timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
    timer_ocintpara.ocidlestate = TIMER_OC_IDLE_STATE_LOW;
    timer_ocintpara.ocnidlestate =TIMER_OCN_IDLE_STATE_LOW;
    timer_channel_output_config(TIMER3, TIMER_CH_1, &timer_ocintpara);

    timer_channel_output_pulse_value_config(TIMER3, TIMER_CH_1, 499);//设置通道占空比
    timer_channel_output_mode_config(TIMER3, TIMER_CH_1, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER3, TIMER_CH_1, TIMER_OC_SHADOW_DISABLE);

    timer_primary_output_config(TIMER3,ENABLE);
    timer_auto_reload_shadow_enable(TIMER3);  // 自动加载定时器参数
    timer_enable(TIMER3);	 // 使能定时器
}



void Timer5_init(void)
{
    //使能
    rcu_periph_clock_enable(RCU_TIMER5);
    timer_parameter_struct timer_initpara;
    timer_deinit(TIMER5);//复位外设TIMER5

    // 定时器参数配置
    timer_initpara.prescaler         =99;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = (500-1);//60MS
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER5,&timer_initpara);//初始化定时器5
    //  自动加载定时器参数
    timer_auto_reload_shadow_enable(TIMER5);
    //  中断使能
    timer_interrupt_enable(TIMER5, TIMER_INT_FLAG_UP);
    // 使能定时器
    timer_enable(TIMER5);
    //定时器2 优先级
    nvic_irq_enable(TIMER5_DAC_IRQn,6,0);
	
}








