
#include "ADC.h"
/************************************************************ 
 * 函数:       Get_ADC_val(void)
 * 说明:       ADC值获取
 * 参数:       adc_periph =ADCx,x=0,1,2       
************************************************************/
//double Get_ADC_val(uint32_t adc_periph)
//{
//  uint16_t ADC_val=0;
//  adc_flag_clear(adc_periph,ADC_FLAG_EOC);              //  清除结束标志	
//	
//  while(SET != adc_flag_get(adc_periph,ADC_FLAG_EOC)); //  获取转换结束标志
//		
//  ADC_val = (uint16_t)ADC_RDATA(adc_periph); 			 		   // 读取ADC	
//  return	ADC_val;	
//}

/************************************************************ 
 * 函数:    ADC_Init(void)
 * 参数：   *buf = AD数值存放数组  len =数组长度
 * 时间：  	2023.7.21
			AHB =SYSCLK= 200 M  
			APB2 = AHB / 2 =100 M
			PCLK2=APB2 =100M
			ADC_CLK =APB2/2=50m
Tconv = (3+12.5)*(1/50M)=15.5*0.00000002 =0.00000031S = 0.31us
f=1/Tconv =1/0.00000031 =3,225,806.45HZ
************************************************************/
void ADC_Init(void)
{
	 rcu_periph_clock_enable(RCU_ADC0);        // 使能ADC时钟		
	 adc_clock_config(ADC_ADCCK_PCLK2_DIV2);   // 配置时钟  50m  0.2US
	//ADC配置
	adc_deinit();                             
	adc_sync_mode_config(ADC_SYNC_MODE_INDEPENDENT);   	//		ADC_ALL_ROUTINE_PARALLEL       ADC_SYNC_MODE_INDEPENDENT
	adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, ENABLE);    	// 使能连续转换模式 ADC_SCAN_MODE
	adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);	
		
	adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);     					
	adc_channel_length_config(ADC0,ADC_ROUTINE_CHANNEL,2);   //ADC通道长度设置
	
	adc_routine_channel_config(ADC0,0,ADC_CHANNEL_14,ADC_SAMPLETIME_28);//PC4 ADC0-CH14  ADC1    
	adc_routine_channel_config(ADC0,1,ADC_CHANNEL_8,ADC_SAMPLETIME_28); //PB0 ADC0-CH8   ADC2
	

	adc_resolution_config(ADC0,ADC_RESOLUTION_12B); //ADC分辨率 		
	adc_external_trigger_config(ADC0, ADC_ROUTINE_CHANNEL, EXTERNAL_TRIGGER_DISABLE); //外部触发失能 ,软件触发   
	adc_dma_mode_enable(ADC0);       	
	adc_dma_request_after_last_enable(ADC0);  		
	
	adc_enable(ADC0);      
	delay_1ms(10);			
	adc_calibration_enable(ADC0);     //校准	
	delay_1ms(10);
	adc_software_trigger_enable(ADC0, ADC_ROUTINE_CHANNEL);	//ADC软件触发		
}

/**************
*DMA 初始化函数
***************/
void DMA_Init(uint32_t *buf,uint32_t len)
{
	rcu_periph_clock_enable(RCU_DMA1);  			 
	dma_deinit(DMA1, DMA_CH0);   
	dma_single_data_parameter_struct dma_single_data_parameter;  

	dma_single_data_parameter.periph_addr = (uint32_t) (&ADC_RDATA(ADC0)); //(&ADC_RDATA(ADC0));  
	dma_single_data_parameter.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
	dma_single_data_parameter.memory0_addr = (uint32_t)(buf);   		   //AD数值存取内部地址
	dma_single_data_parameter.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
	dma_single_data_parameter.circular_mode= DMA_CIRCULAR_MODE_ENABLE;        //循环模式
	dma_single_data_parameter.periph_memory_width = DMA_PERIPH_WIDTH_32BIT;   //宽度32
	dma_single_data_parameter.direction = DMA_PERIPH_TO_MEMORY;  
	dma_single_data_parameter.number = len;                 	  						
	dma_single_data_parameter.priority = DMA_PRIORITY_HIGH;     
	dma_single_data_mode_init(DMA1, DMA_CH0, &dma_single_data_parameter);
	
	dma_channel_subperipheral_select(DMA1, DMA_CH0, DMA_SUBPERI0);			
	dma_circulation_enable(DMA1, DMA_CH0);
//	dma_interrupt_enable(DMA1,DMA_CH0,DMA_CHXCTL_FTFIE);
	dma_channel_enable(DMA1, DMA_CH0);	
	
}
/************
DAC初始化
*************/
void DAC_Init(void)
{   
	rcu_periph_clock_enable(RCU_DAC);	
	//DAC0
		dac_deinit();
		dac_trigger_enable(DAC0);
    dac_trigger_source_config(DAC0,DAC_TRIGGER_SOFTWARE);   
    dac_wave_mode_config(DAC0,DAC_WAVE_DISABLE);
    dac_output_buffer_enable(DAC0);  
    dac_enable(DAC0);		




	//DAC1
		dac_trigger_enable(DAC1);
    dac_trigger_source_config(DAC1, DAC_TRIGGER_SOFTWARE);
    dac_wave_mode_config(DAC1, DAC_WAVE_DISABLE);
    dac_output_buffer_enable(DAC1);  
    dac_enable(DAC1);	

		dac_software_trigger_enable(DAC0);	//DAC0软件触发使能

}




