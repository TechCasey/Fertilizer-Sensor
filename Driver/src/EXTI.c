#include "EXTI.h"
#include "gd32f4xx_gpio.h"
#include "gd32f4xx_rcu.h"


/***********
*EXTI初始化
***********/
void Exti_init(void)
{

    rcu_periph_clock_enable(RCU_SYSCFG);
    //使能外设时钟
    rcu_periph_clock_enable(RCU_GPIOB);
//    rcu_periph_clock_enable(RCU_AF);
    //配置引脚为输入
    gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_4);
	 syscfg_exti_line_config(EXTI_SOURCE_GPIOB,EXTI_SOURCE_PIN4);
    //配置外部中断线
    exti_init(EXTI_4, EXTI_INTERRUPT, EXTI_TRIG_FALLING);    //下降
    //配置中断优先级
//    nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);  //配置优先组位的长度
    nvic_irq_enable(EXTI4_IRQn, 0, 0);  //抢占优先级，响应优先级
    //标志位
    exti_flag_clear(EXTI_4);   //清除EXTI线4标志位
    exti_interrupt_flag_clear(EXTI_4);   //清除EXTI线x中断标志位
		//						使能外部中断
            exti_interrupt_enable(EXTI_4);

}






