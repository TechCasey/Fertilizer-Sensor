#include "GPIO.h"

void GPIO_Init(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    //rcu_periph_clock_enable(RCU_GPIOD);


//-------------------------------------------CAN----------------------------------------------------------------------------
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);  // 输出参数设置  CAN TX  发送引脚
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8);   // GPIO 模式设置
    gpio_af_set(GPIOB, GPIO_AF_9, GPIO_PIN_8);

    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);  // 输出参数设置  CAN RX  接收引脚
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_9);   // GPIO 模式设置
    gpio_af_set(GPIOB, GPIO_AF_9, GPIO_PIN_9);
    //--------------------------------------------_ID--------------------------------------------------------------------------------

    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_3);   	// ID OUT		设置GPIO模式，带下拉电阻  PB3
    gpio_output_options_set(GPIOB,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_3);		//设置GPIO输出模式和速度

    gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_10);   	// ID IN-------带上拉电阻
    gpio_output_options_set(GPIOA,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_10);

//------------------------------------------------------------------------------------------------------------------------
    gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_4);   //Signal_out  PB4-->  种子信号检测
//    gpio_output_options_set(GPIOB,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_4);     //设置GPIO输出模式和速度
    gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_0);//Signal_DU  PA0-->  种子堵种信号检测
//--------------------------------------------DA1_DA2----------------------------------------------------------------------------
    //gpio_mode_set(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_4);
    //gpio_output_options_set(GPIOA,GPIO_OTYPE_OD,GPIO_OSPEED_50MHZ,GPIO_PIN_4);		 //PA4(DAC0) ->DA1

    //gpio_mode_set(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_5);
    // gpio_output_options_set(GPIOA,GPIO_OTYPE_OD,GPIO_OSPEED_50MHZ,GPIO_PIN_5);	   //PA5(DAC1) ->DA2

//-------------------------------------------GPIO_DA3_ADC3-----------------------------------------------------------------------------

    //gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_1);
    // gpio_output_options_set(GPIOB,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_1);       //PB1 选择性打开，打开测玉米大豆
    //关闭测 高粱

    //gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_5);
    //gpio_output_options_set(GPIOC,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_5);     //PC5一直高电平


//--------------------------------------------LED_RGB--------------------------------------------------------------

    //gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_3);   //PB3
    //gpio_output_options_set(GPIOB,GPIO_OTYPE_OD,GPIO_OSPEED_50MHZ,GPIO_PIN_3);

    // gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_4);   //PB4
    //gpio_output_options_set(GPIOB,GPIO_OTYPE_OD,GPIO_OSPEED_50MHZ,GPIO_PIN_4);

    //gpio_mode_set(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_2);   //PD2
    // gpio_output_options_set(GPIOD,GPIO_OTYPE_OD,GPIO_OSPEED_50MHZ,GPIO_PIN_2);


//-----------------------------------TIMER2_CH0---------------------------------------------

    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_6);
    gpio_af_set(GPIOA, GPIO_AF_2, GPIO_PIN_6);     //PA6

//-----------------------初始化输出-----------------------------------------------------------
    //gpio_bit_set(GPIOD,GPIO_PIN_2);
    // gpio_bit_set(GPIOB,GPIO_PIN_4);
    //  gpio_bit_set(GPIOB,GPIO_PIN_3);
    gpio_bit_reset(GPIOB,GPIO_PIN_3);
//    gpio_bit_set(GPIOC,GPIO_PIN_5);
    gpio_bit_reset(GPIOA,GPIO_PIN_0);
}






























