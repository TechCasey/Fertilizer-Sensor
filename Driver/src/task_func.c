/*
其他功能函数
*/
#include "task_func.h"
#include "gd32f4xx_it.h"
#include "math.h"
#include "string.h"
static  uint8_t  Version[8]= {0x00,0x01,0x00,0x01,0xAA,0xAA,0xAA,0xAA}; //回应上位机为APP模式
extern uint16_t timer_cnt;		       //一粒种子落下的时间
static uint32_t Temp_Seednum=0;      //种子数临时变量，判断种子数是否增加



CAN_RXDATA CAN_REC_DATA= {
    .ROW_NUM=0xFFFFFFFF,  //默认行数全开
}; //can接收数据
//CAN_RXDATA CAN_REC_DATA; //can接收数据
CAN_SENDATA SeedMES;     //can发送数据

DAout da_out;            //DAC输出

/*传感器节点ID发送(PB3用于OUT)*/
void NodeID_Send(uint8_t ID)
{
    gpio_bit_reset(GPIOB,GPIO_PIN_3);
    for (int i=0; i<5; i++) //发送5次
    {
        for(int i=0; i<ID; i++)
        {
            gpio_bit_set(GPIOB,GPIO_PIN_3);
            delay_1ms(5);
            gpio_bit_reset(GPIOB,GPIO_PIN_3);
            delay_1ms(5);
        }
        delay_1ms(40);
    }
    gpio_bit_reset(GPIOB,GPIO_PIN_3);
}

///****************************************
//*节点ID检查(PA10用于IN)
//*如果需要定时检查,可以每隔一段时间调用一下此函数
//*notes:delay延时时间不要随意修改！！！
//*****************************************/
void NodeID_Check(void)
{
    uint32_t TimeoOut=16000;  //超时时间
    bool  ID_Disconnect=false;


    /*1号位低电平*/
    NodeID=0;
    if((gpio_input_bit_get(GPIOA,GPIO_PIN_10)!=1)&&(NodeID==0))
    {
        delay_1ms(100);  //防抖
        if((gpio_input_bit_get(GPIOA,GPIO_PIN_10)!=1)&&(NodeID==0))//1号位
        {
            ID_WaitTIM.Flag=true;	//开启中断
            NodeID=1;
            //关闭中断和定时器，停止接收
            timer_deinit(TIMER0); //复位
            nvic_irq_disable(TIMER0_Channel_IRQn);  //TIMER0通道捕获比较中断
            nvic_irq_disable(TIMER0_UP_TIMER9_IRQn);  //TIMER0更新中断和TIMER9全局中断
            delay_1ms(50);	 //等待2号机稳定
        }
        else
        {
            //复用PA10 为tim0_ch2
            gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_10);    //PA11设置为上升沿捕获（备用功能模式，带上拉电阻）
            gpio_af_set(GPIOA, GPIO_AF_1, GPIO_PIN_10);
            Timer0_Init();  //定时器0 初始化

            ID_WaitTIM.Flag=true;//开启中断

            while(1) //等待节点信息
            {
                if(NodeID!=0)break;
                else	if(ID_WaitTIM.CNT>=TimeoOut) //超时等待
                {
                    ID_WaitTIM.CNT=0;
                    ID_WaitTIM.Flag=false;
                    break;
                }
            }
        }
    }
    else//同上
    {
        //复用PA10 为tim0_ch2
        gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_10);
        gpio_af_set(GPIOA, GPIO_AF_1, GPIO_PIN_10);
        Timer0_Init();  //定时器0 初始化

        ID_WaitTIM.Flag=true;//开启中断

        while(1)
        {
            if(NodeID!=0)
            {
                break;
            }
            else	if(ID_WaitTIM.CNT>=TimeoOut) //超时等待
            {
                ID_WaitTIM.CNT=0;
                ID_WaitTIM.Flag=false;
                break;
            }
        }


    }


    //断线，读取ID
    if(!ID_WaitTIM.Flag)
    {
        ID_Disconnect	=true;
        NodeID=0;
        //读取保存ID
        NodeID=Flash_ReadData(SensorID_WR_START_ADDR, SensorIDWordNum);
    }
    else
    {
        ID_WaitTIM.CNT=0;
        ID_WaitTIM.Flag=false;
    }

    //中断停止接收
    timer_deinit(TIMER0);
    nvic_irq_disable(TIMER0_Channel_IRQn);  //TIMER0通道捕获比较中断
    nvic_irq_disable(TIMER0_UP_TIMER9_IRQn);  //TIMER0更新中断和TIMER9全局中断

    //没有断线，发送ID
    if(!ID_Disconnect)
    {
        delay_1ms(100);         //等待后续节点稳定
        NodeID_Send(NodeID+1);	//发送次级ID 5次
    }
    //ID存入Flash
    if(NodeID!=(Flash_ReadData(SensorID_WR_START_ADDR, SensorIDWordNum)))
    {
        Flash_erase_Sector(SensorID_WR_START_ADDR);
        Flash_WriteData (SensorID_WR_START_ADDR,SensorID_WR_END_ADDR,(uint32_t)NodeID);
    }


}

///********
// 求和
//*********/
static uint32_t Sum(double inputbuf[])
{
    uint32_t	sum=0;

    for(uint8_t i=0; i<100; i++)
    {
        sum += (uint32_t)inputbuf[i];
    }
    return sum;
}
///**************************************
//*ADC数据处理函数
//***************************************/
void GetAD_Filter(ADCVoltage *GetADC)
{
    uint32_t sum=0;
    double RES[100]= {0};

    for(uint8_t i=0; i<100; i++)
    {
        GetADC->ADC1_Buf[i]=GetADC->allAD_Buf[(i*2)];  		//ADC1
        GetADC->ADC2_Buf[i]=GetADC->allAD_Buf[((i*2)+1)];   //ADC2

    }

    RollAvg(GetADC->ADC1_Buf,RES);
    sum=Sum(RES);
    GetADC->ADC1_Val=(uint32_t)(sum/100);

    sum =0;
    memset(RES,0,100);

    RollAvg(GetADC->ADC2_Buf,RES);
    sum=Sum(RES);
    GetADC->ADC2_Val=(uint32_t)(sum/100);
}
///****************
//CAN 数据发送函数
//*****************/
void CANUpload(uint8_t *txbuf,uint8_t CANID)
{
    uint32_t  SEND_ID=0;
    //播种量(32位)；因为这里txbuf定义的是8位的，所以就用4个txbuf位，4*8=32位
    txbuf[3]=SeedMES.Seeding_Rate&0xFF;
    txbuf[2]=(SeedMES.Seeding_Rate>>8)&0xFF;
    txbuf[1]=(SeedMES.Seeding_Rate>>16)&0xFF;
    txbuf[0]=(SeedMES.Seeding_Rate>>24)&0xFF;
    //落种检测（16位）因为这里txbuf定义的是8位的，所以就用2个txbuf位，2*8=16位
    txbuf[5]=SeedMES.Seed_detection&0xFF;
    txbuf[4]=(SeedMES.Seed_detection>>8)&0xFF;
    //堵、漏缺状态（8位）因为这里txbuf定义的是8位的，刚好位数匹配，不用再次分配高低位
    txbuf[6]=SeedMES.SeedCnt_err;
    //工作状态（8位）
    txbuf[7]=(uint8_t)SeedMES.Work_State;

#if defined ROW_SET
    //行没有被选中，停止发送
//    if((Seed_Type!=0)&&(!((CAN_REC_DATA.ROW_NUM>>(CANID-1))&0x01)))
    if((!((CAN_REC_DATA.ROW_NUM>>(CANID-1))&0x01)))
    {
        CANID=0;
    }

#endif

    if(CANID==0)
    {
        return;
    }
    else if(CANID>0&&CANID<=36)
    {
        SEND_ID=0x18CC0000+CANID;
        Can_Tx_Mes(CAN0,SEND_ID,1,0,txbuf,8);
    }


    SeedMES.Work_State=false;//can发送结束将工作状态置为false
    memset(txbuf,0,8);  //将 txbuf 指向的内存区域的前 8 个字节设置为 0

}
///********************
//  CAN接收数据处理函数
//*********************/
uint8_t Alarm_clear;
uint8_t one;
extern int Seed_detection;    //检测次数
extern int Seeding_Rate=0;//总播种量
extern uint32_t seed_axle_cnt;
extern __IO uint16_t current_value;
void Can_Rx_Mes(void)
{
    uint32_t  REC_ID=0;
    uint8_t Reset_SeedNum;
    uint8_t ID;
    if((CAN_FF_EXTENDED == receive_message.rx_ff)&&(8 == receive_message.rx_dlen))//帧格式和数据长度
    {
        memset(CAN_REC_DATA.rx_data,0,8);		//清除了 CAN_REC_DATA.rx_data 数组的内容
        memcpy(CAN_REC_DATA.rx_data,receive_message.rx_data,8);//从 receive_message.rx_data 中复制 8 个字节的数据到 CAN_REC_DATA.rx_data

        switch(receive_message.rx_efid)//扩展格式帧标识符
        {
        case 0x00000013:
            Can_Tx_Mes(CAN0,0x00000013,1,0,Version,8);
            break;
        case 0x1F666666:
            Can_Tx_Mes(CAN0,0x00000013,1,0,Version,8);
            break;
        case 0x184D5701:
            /*-------------------------------------------------------------
            			*CAN数据byte0-7 为行数（32位）
            -------------------------------------------------------------*/

            CAN_REC_DATA.ROW_NUM = 	CAN_REC_DATA.rx_data[0]|
                                    (CAN_REC_DATA.rx_data[1]<<8)|
                                    (CAN_REC_DATA.rx_data[2]<<16)|
                                    (CAN_REC_DATA.rx_data[3]<<24)|
                                    ((uint64_t)CAN_REC_DATA.rx_data[4]<<32)|
                                    ((uint64_t)CAN_REC_DATA.rx_data[5]<<40)|
                                    ((uint64_t)CAN_REC_DATA.rx_data[6]<<48)|
                                    ((uint64_t)CAN_REC_DATA.rx_data[7]<<56);
            memset(CAN_REC_DATA.rx_data,0,8);//接收数据清零
            break;

        case 0x184D5702:
            /*-----------------------------------------------------------------
               0 byte为作业速度-高位，1 byte作业速度-低位
               2 byte为灵敏度设置
               3 byte第 0 bit为解除报警位;第 1 bit为清零位
               4、5、 6、7 byte保留
            ------------------------------------------------------------------*/
            //车速
            CAN_REC_DATA.Car_Speed=(CAN_REC_DATA.rx_data[0]<<8)|CAN_REC_DATA.rx_data[1];
            //灵敏度
            CAN_REC_DATA.Sensitivity=CAN_REC_DATA.rx_data[2];
            one=CAN_REC_DATA.Sensitivity;
            sDecSeedAbnormal.gearOffset = CAN_REC_DATA.Sensitivity;
            //解除报警
            Alarm_clear=CAN_REC_DATA.rx_data[3]&0x01;//获取最低位
            if(Alarm_clear)
            {
                Alarm_clear=0;
                SeedMES.SeedCnt_err=0;  //解除 堵种、缺种报警
                Luo_Err.CNT=0;
            }

            //清0
            Reset_SeedNum= ((CAN_REC_DATA.rx_data[3])&0x02); 	//获取右移后的数值的最低位
            if(Reset_SeedNum)//如果清零位为1
            {
                Reset_SeedNum	=0;
                SeedMES.Seeding_Rate=0;  //清除播种量
                Seeding_Rate=0;
            }
            memset(CAN_REC_DATA.rx_data,0,8);//接收数据清零
            break;

        case 0x18CB0000:
            /*-----------------------------------------------------------------
                0 byte为种轴转速高位，1 byte种轴转速低位
                2 byte为主轴转动齿数高位，3 byte主轴转动齿数低位
                4 byte为单齿下种量高位（标定量），5 byte单齿下种量低位（标定量）
                6、7byte保留
             ------------------------------------------------------------------*/
            //种轴转速
            CAN_REC_DATA.seed_Speed=(CAN_REC_DATA.rx_data[0]<<8)|CAN_REC_DATA.rx_data[1];
            //齿数
            static uint8_t initGearFlag = 1;
            CAN_REC_DATA.Gear_num=(CAN_REC_DATA.rx_data[2]<<8)|CAN_REC_DATA.rx_data[3];
            sDecSeedAbnormal.currGear = CAN_REC_DATA.Gear_num;
            current_value=CAN_REC_DATA.Gear_num;
            if(initGearFlag)
            {
                last_value = CAN_REC_DATA.Gear_num;
                sDecSeedAbnormal.lastGear = last_value;
                initGearFlag = 0;
            }



            //标定量
            CAN_REC_DATA.Scalar =	(CAN_REC_DATA.rx_data[4]<<8)|CAN_REC_DATA.rx_data[5];

            memset(CAN_REC_DATA.rx_data,0,8);//接收数据清零
            break;

        case 0x18CB0001 ... 0x18CB0FFF:
            REC_ID=0x18BA0000+NodeID;
//            //1-15行
//            if((NodeID>=1)&&(NodeID<=15))         				//各自ID
//            {
//                REC_ID=0x18BA0000|(NodeID&0x0F);
//            }
//            //16-30行
//            else if((NodeID>=16)&&(NodeID<=30))
//            {
//                REC_ID=0x18BA000F|((NodeID-15)<<4);
//            }
//            //31-45行
//            else if((NodeID>=31)&&(NodeID<=45))
//            {
//                REC_ID=0x18BA00FF|((NodeID-30)<<8);
//            }

            if(REC_ID==receive_message.rx_efid)
            {
                /*-----------------------------------------------------------------
                        0 byte为种轴转速高位，1 byte种轴转速低位
                				2 byte为主轴转动齿数高位，3 byte主轴转动齿数低位
                				4 byte为单齿下种量高位（标定量），5 byte单齿下种量低位（标定量）
                				6、7byte保留
                ------------------------------------------------------------------*/
                //种轴转速
                CAN_REC_DATA.seed_Speed=(CAN_REC_DATA.rx_data[0]<<8)|CAN_REC_DATA.rx_data[1];
                //齿数
                CAN_REC_DATA.Gear_num=(CAN_REC_DATA.rx_data[2]<<8)|CAN_REC_DATA.rx_data[3];
                //标定量
                CAN_REC_DATA.Scalar =	(CAN_REC_DATA.rx_data[4]<<8)|CAN_REC_DATA.rx_data[5];
            }
            memset(CAN_REC_DATA.rx_data,0,8);//接收数据清零
            break;


///***********************************************
//  CAN接收数据处理结束
//***********************************************/

#if defined CAN_SeedType

        case 0x18BC1112:

            Seed_Type= CAN_REC_DATA.rx_data[0]+1;

            memset(CAN_REC_DATA.rx_data,0,8);

            break;

#endif

            //APP跳转Boot
        case 0x18BCA401 ... 0x18BCA49F:  //控制器单独升级
        case 0x18FFA5D2:                 //控制器广播升级
        case 0X1ABCDE01:                 //上位机升级
            if(receive_message.rx_efid==0X1ABCDE01)  //扩展格式帧标识符=0X1ABCDE01
            {
                Can_Tx_Mes(CAN0,0x00000011,1,0,0x00,8);
                Flash_erase_Sector(Update_WRITE_START_ADDR); //擦除相关扇区
                Flash_WriteData (Update_WRITE_START_ADDR,Update_WRITE_END_ADDR,0x13); //写入更新标志
                __set_PRIMASK(1);
                rcu_deinit();
                for (int i= 0; i < 8; i++)
                {
                    NVIC->ICER[i]=0xFFFFFFFF;
                    NVIC->ICPR[i]=0xFFFFFFFF;
                }
                __set_PRIMASK(0);
                NVIC_SystemReset();

            }
            else if(receive_message.rx_efid==0x18FFA5D2) //控制器广播
            {
#if defined ROW_SET
                //行没有被选中，停止发送
                if((!((CAN_REC_DATA.ROW_NUM>>(NodeID-1))&0x01)))
                {
                    ID=0;
                }
                else
                {
                    ID=NodeID;
                }
#endif
                if (ID!=0) {
                    Flash_erase_Sector(Update_WRITE_START_ADDR);
                    Flash_WriteData (Update_WRITE_START_ADDR,Update_WRITE_END_ADDR,0x12);
                    __set_PRIMASK(1);
                    rcu_deinit();
                    for (int i= 0; i < 8; i++)
                    {
                        NVIC->ICER[i]=0xFFFFFFFF;
                        NVIC->ICPR[i]=0xFFFFFFFF;
                    }
                    __set_PRIMASK(0);
                    NVIC_SystemReset();
                }

            }                                               //单独升级
            else  if((receive_message.rx_efid>=0x18BCA401)&&(receive_message.rx_efid<=0x18BCA49F))
            {
                if((((NodeID>=1)&&(NodeID<=15))&&(receive_message.rx_efid==(0x18BCA400|(NodeID&0x0F))))
                        ||(((NodeID>=16)&&(NodeID<=24))&&(receive_message.rx_efid==(0x18BCA40F|((NodeID-15)<<4)))))
                {
                    Flash_erase_Sector(Update_WRITE_START_ADDR);
                    Flash_WriteData (Update_WRITE_START_ADDR,Update_WRITE_END_ADDR,0x11);
                    __set_PRIMASK(1);
                    rcu_deinit();
                    for (int i= 0; i < 8; i++)
                    {
                        NVIC->ICER[i]=0xFFFFFFFF;
                        NVIC->ICPR[i]=0xFFFFFFFF;
                    }
                    __set_PRIMASK(0);
                    NVIC_SystemReset();
                }
            }
            break;
        default:
            break;
        }
    }
    memset(CAN_REC_DATA.rx_data,0,8);
}

///*********************
//定时器输入捕获配置函数
//**********************/

void Timer_CapConfig(uint16_t  edge)
{
    timer_ic_parameter_struct timer_icinitpara;

    timer_icinitpara.icpolarity  = edge;    			//更改触发边沿
    timer_icinitpara.icselection = TIMER_IC_SELECTION_DIRECTTI;
    timer_icinitpara.icprescaler = TIMER_IC_PSC_DIV1;
    timer_icinitpara.icfilter    = 0x4;

    timer_input_capture_config(TIMER2,TIMER_CH_0,&timer_icinitpara);
}

//变异系数数据处理函数
void bianyixishu_count_in_handler()
{
    unsigned int  theoretical_t ;//种子落下理论时间
    unsigned char temp_percent; //理论时间和种子落下的实际时间的比值

    static uint32_t Bianyi_Seednum_cnt=0;      //种子数变量，种子每计数500次清零
    static unsigned char record_data[PERCENT_MAX-PERCENT_MIN] = {0};

    Bianyi_Seednum_cnt=SeedMES.SeedNum_Cnt%500;//种子数每计500次计算一次变异系数
    if(Bianyi_Seednum_cnt >= START_SEED_INDEX &&Bianyi_Seednum_cnt < END_SEED_INDEX)
    {
        if(Bianyi_Seednum_cnt>Temp_Seednum)//如果种子数目有变化
        {
            Temp_Seednum=Bianyi_Seednum_cnt;
            theoretical_t = CAN_REC_DATA.Spacing*360/CAN_REC_DATA.Car_Speed; //时间=路程/速度
            temp_percent = timer_cnt*10/theoretical_t;
            if(temp_percent>PERCENT_MIN && temp_percent<=PERCENT_MAX)
            {
                record_data[temp_percent]+=1U;//第几行被选中
            }
        }
    }
    else if((SeedMES.SeedNum_Cnt >= END_SEED_INDEX)&&(Bianyi_Seednum_cnt==0))//如果种子计数到500，开始计算变异系数
    {
        Temp_Seednum=0;
        Bianyi_Seednum_cnt=0;      //种子数变量，种子每计数500次清零
        bianyixishu_calc_in_mainloop(record_data);
        memset(record_data,0,sizeof(record_data));
    }
}

///*
//*@brief:
//*			 C.V = (SD / MN) x 100%;
//*			 C.V: bianyixishu
//*			 SD: standard deviation
//*/
///*
//*  S^2 = [ (x1 - x_avg)^2 + (x2 - x_avg)^2 + (x3 - x_avg)^2 + ... + (x3 - x_avg)^2] / n
//*      = (x1^2 + x2^2 + x3^3 + .. + xn^2)/n + x_avg^2
//*/
static void bianyixishu_calc_in_mainloop(unsigned char record_data[PERCENT_MAX-PERCENT_MIN])//变异系数计算函数
{
    float total_num = 0.0f;
    float phase0 = 0.0f;
    float phase1 = 0.0f;
    char i = 0;

    for(i=0; i<PERCENT_MAX-PERCENT_MIN; i++) //求和
    {
        total_num+=record_data[i];
    }
    if(total_num != 0)
    {
        //(x1^2 + x2^2 + x3^3 + .. + xn^2)/n
        phase0 = (0.3025f*record_data[0] \
                  + 0.4225f*record_data[1] \
                  +0.5625f*record_data[2] \
                  +0.7225f*record_data[3] \
                  +0.9025f*record_data[4]\
                  +1.1025f*record_data[5]\
                  +1.3225f*record_data[6]\
                  +1.5625f*record_data[7]\
                  +1.8225f*record_data[8]\
                  +2.1025f*record_data[9])/total_num;

        //x_avg^2
        phase1 = (0.55f*record_data[0]\
                  +0.65f*record_data[1]\
                  +0.75f*record_data[2]\
                  +0.85f*record_data[3]\
                  +0.95f*record_data[4]\
                  +1.05f*record_data[5]\
                  +1.15f*record_data[6]\
                  +1.25f*record_data[7]\
                  +1.35f*record_data[8]\
                  +1.45f*record_data[9])/total_num;

        phase1 *= phase1;

        if(phase0>phase1)//计算变异系数
        {
            SeedMES.Variation_Gain= 1000*sqrt(phase0-phase1);
        }
    }
}


///*
//功能：回应控制器OTA升级指令
//参数：ID 传感器自身ID
//			buf 回应字符串
//			ACKnum 升级编号
//			index  升级时要求的包号
//*/
static void CAN_OTA_ACK(uint8_t ID,char *buf,uint8_t ACKnum,uint16_t index)
{
    uint8_t Ack_Buf[8]= {0};
    uint32_t  SEND_ID=0;

    memcpy(Ack_Buf,buf,8);

    if(strncmp("ACK:",buf,4)==0) //比较字符串
    {
        Ack_Buf[4]=ACKnum;
        Ack_Buf[6]=index&0x0FF;            //低位
        Ack_Buf[5]=(index>>8)&0x0FF;  	    //高位
    }

    if(ID==0)
    {
        Can_Tx_Mes(CAN0,0x18FFC1D2,1,0,Ack_Buf,8);//通用ID
    }
    else
    {
        //1-15行
        if((ID>=1)&&(ID<=15))         				//各自ID
        {
            SEND_ID=0x18BD0000|(ID&0x0F);
            Can_Tx_Mes(CAN0,SEND_ID,1,0,Ack_Buf,8);
        }
        //16-24行
        else if((ID>=16)&&(ID<=24))
        {
            SEND_ID=0x18BD000F|((ID-15)<<4);
            Can_Tx_Mes(CAN0,SEND_ID,1,0,Ack_Buf,8);
        }


    }
}
///*
//功能：判断是否升级后跳转到APP
//参数：Read_Type 读取的升级类型
//			ID        传感器自身ID
//*/
void CAN_Type_Check(uint32_t Read_Type,uint8_t ID)
{

    switch(Read_Type)
    {
        //单独升级
    case 0x11:
        CAN_OTA_ACK(0,"ACK:",2,0);
        Flash_erase_Sector(Update_WRITE_START_ADDR);
        break;
        //广播升级
    case 0x12:
        delay_1ms(ID*10); //根据ID进行延时 iD *10ms
        CAN_OTA_ACK(ID,"ACK:",2,0);
        Flash_erase_Sector(Update_WRITE_START_ADDR);
        break;
        //上位机升级
    case 0x13:
        Flash_erase_Sector(Update_WRITE_START_ADDR);
        break;
    default:
        break;
    }

}

