#include "stdlib.h"
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "Flash.h"
#include "CAN.h"
#include "gd32f4xx_it.h"


can_trasnmit_message_struct transmit_message; 											 // 发送数据结构体
can_receive_message_struct receive_message,receive_message_Fifo1;    // 接收数据结构体


/**********************************
CAN ID过滤设置 
***********************************/
void CanIDFliter(void)
{
	/*CAN1 过滤器编号从14开始*/
	// 配置过滤器0，用于接收0x00000013的CAN消息，使用列表模式（List Mode），32位滤波，放入FIFO0中
	CanFliterConfig(0x00000013,0x00,0,CAN_FILTERMODE_LIST,CAN_FILTERBITS_32BIT,CAN_FIFO0,1);	//上位机节点查询	
	//	CanFliterConfig(0x18BC1111,0x00,1,CAN_FILTERMODE_LIST,CAN_FILTERBITS_32BIT,CAN_FIFO0,1);	//主控发送
	// 配置过滤器1，用于接收标识符在范围0x18BA0000-0xFFFFFFFF之间的CAN消息，使用掩码模式（Mask Mode），32位滤波，放入FIFO0中
	CanFliterConfig(0x18BA0000,0xFFFF0000,1,CAN_FILTERMODE_MASK,CAN_FILTERBITS_32BIT,CAN_FIFO0,1);	//主控发送
	
	// 配置过滤器5，用于接收0x184C5701的CAN消息，使用列表模式（List Mode），32位滤波，放入FIFO0中
	CanFliterConfig(0x184C5701,0x00,5,CAN_FILTERMODE_LIST,CAN_FILTERBITS_32BIT,CAN_FIFO0,1);	//上位机节点查询	
	// 配置过滤器6，用于接收0x184C5702的CAN消息，使用列表模式（List Mode），32位滤波，放入FIFO0中
	CanFliterConfig(0x184C5702,0x00,6,CAN_FILTERMODE_LIST,CAN_FILTERBITS_32BIT,CAN_FIFO0,1);	//上位机节点查询	

	CanFliterConfig(0x1ABCDE01,0x18FFA5D2,2,CAN_FILTERMODE_LIST,CAN_FILTERBITS_32BIT,CAN_FIFO0,1);	//上位机Bootloader跳转，控制器广播升级请求
	
#if defined CAN_SeedType	
	CanFliterConfig(0x18BC1112,0x00,3,CAN_FILTERMODE_LIST,CAN_FILTERBITS_32BIT,CAN_FIFO0,1);	
#endif
	//控制器单独升级请求
	CanFliterConfig(0x18BCA400,0xFFFFFF00,4,CAN_FILTERMODE_MASK,CAN_FILTERBITS_32BIT,CAN_FIFO0,1);
	
}

/*********************************************
 * 函数：void CAN_Init(void)
 * 说明：CAN初始化
 		AHB =SYSCLK= 200 M  
		APB1 = AHB / 4 =50 M
		pclk1 =APB1= 50M
		波特率计算：Pclk1 / Prescaler /( Tbs1 + Tbs2 + 1 ) 
**********************************************/
void CAN_HardInit(void)
{	
    can_parameter_struct can_parameter; 
    rcu_periph_clock_enable(RCU_CAN0);	// 使能CAN时钟

    can_deinit(CAN0);  //CAN0复位	
    can_parameter.time_triggered = DISABLE; // 禁用时间触发模式
	can_parameter.auto_bus_off_recovery = ENABLE; // 启用自动总线关闭恢复（启用自动离线恢复）
	
    can_parameter.auto_wake_up = ENABLE;  //启用自动唤醒
    can_parameter.auto_retrans = ENABLE;  //启用自动重传
    can_parameter.rec_fifo_overwrite = DISABLE; //禁用接收FIFO满时覆盖
    can_parameter.trans_fifo_order = DISABLE;  //禁用发送FIFO顺序
    can_parameter.working_mode = CAN_NORMAL_MODE;  //工作模式
    can_parameter.resync_jump_width = CAN_BT_SJW_1TQ;  //再同步补偿宽度
    can_parameter.time_segment_1 = CAN_BT_BS1_5TQ;  //位段1
    can_parameter.time_segment_2 = CAN_BT_BS2_4TQ;  //位段2
   
    can_parameter.prescaler = 20;  //250    // CAN波特率预分频器
    can_init(CAN0,&can_parameter); // 使用初始化后的结构体初始化CAN

		can_struct_para_init(CAN_TX_MESSAGE_STRUCT, &transmit_message);  //存储发送帧结构体
		can_struct_para_init(CAN_RX_MESSAGE_STRUCT,&receive_message);    //接收帧结构体
		can_struct_para_init(CAN_RX_MESSAGE_STRUCT,&receive_message_Fifo1);

   //使能CAN中断
		can_interrupt_enable(CAN0, CAN_INT_RFNE0);  // 使能CAN接收FIFO0消息挂起中断
//		can_interrupt_enable(CAN0, CAN_INT_RFNE1);
}



/*********************************************************************************
 * 函数：	can 数据发送函数
 * 参数：	id: 报文ID
			ens： 0 标准帧 1拓展帧
			eos： 0 数据帧 1遥控帧
			buf： 发送数据
			dat_len： 数据长度
**********************************************************************************/
void Can_Tx_Mes(uint32_t can_periph,uint32_t id,uint8_t ens,uint8_t eos,uint8_t *buf,uint8_t dat_len )
{
	
	if(!ens){transmit_message.tx_sfid = id;             	//ID
					 transmit_message.tx_ff = CAN_FF_STANDARD;} 	//标准帧 	
	else   	{transmit_message.tx_efid = id;             	//ID
	         transmit_message.tx_ff = CAN_FF_EXTENDED; } 	//拓展帧 	
	if(!eos){transmit_message.tx_ft = CAN_FT_DATA; } 			
	else   	{transmit_message.tx_ft = CAN_FT_REMOTE; }		
	
	transmit_message.tx_dlen = dat_len;   				
	
	for(int i=0;i<dat_len;i++)
	{	
		transmit_message.tx_data[i] = buf [i];						
	}
	switch(can_periph)
	{
			case CAN0:
					can_message_transmit(CAN0, &transmit_message);		//CAN数据发送	
			break;
      case  CAN1:
					can_message_transmit(CAN1, &transmit_message);	
			break;
	}	
}

/**********************
CAN过滤器配置
************************/
void CanFliterConfig(uint32_t listID,uint32_t Mskid,uint8_t Fliter_number,uint8_t MODE,uint8_t BITS ,uint8_t fifo,uint8_t SorE)
{
		uint32_t fltid=0,mskid=0;
	
		can_filter_parameter_struct can_filter;
		can_struct_para_init(CAN_FILTER_STRUCT, &can_filter);
		
		can_filter.filter_number = Fliter_number;     				//过滤器编号   0
		can_filter.filter_mode =MODE;// CAN_FILTERMODE_LIST;	//列表模式
		can_filter.filter_bits =BITS;//CAN_FILTERBITS_32BIT;	//设置过滤器32位
		
		can_filter.filter_enable = ENABLE;							//使能过滤器
		can_filter.filter_fifo_number = fifo;//CAN_FIFO0;			//关联FIFO
	  switch(SorE)
		{
		  case 0:  //标准帧
					can_filter.filter_list_high =(uint16_t)listID<< 5;
					can_filter.filter_list_low = 0x0000U;
					can_filter.filter_mask_high =(uint16_t)Mskid<< 5;
					can_filter.filter_mask_low = 0x0000U;  				
			break;
			
			case 1:  //拓展帧	
					fltid = ((listID << 3)|0x00000004U|0x00000000U);
					mskid = ((Mskid << 3)|0x00000004U|0x00000000U);	
					can_filter.filter_list_high = (fltid>>16)&0xFFFF;
					can_filter.filter_list_low = fltid&0xFFFF;
					can_filter.filter_mask_high = (mskid>>16)&0xFFFF;
					can_filter.filter_mask_low =  mskid&0xFFFF; 
			break;			
		  default:  //默认拓展帧
					fltid = ((listID << 3)|0x00000004U|0x00000000U);
					mskid = ((Mskid << 3)|0x00000004U|0x00000000U);	
					can_filter.filter_list_high = (fltid>>16)&0xFFFF;
					can_filter.filter_list_low = fltid&0xFFFF;
					can_filter.filter_mask_high = (mskid>>16)&0xFFFF;
					can_filter.filter_mask_low =  mskid&0xFFFF; 
			break;
		}	
		can_filter_init(&can_filter);//初始化过滤器			
}


