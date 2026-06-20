#include "PID.h"



/***************************
初始化PID相关参数
用于初始化pid参数的函数
****************************/
void PID_Init(PID *pid,float p,float i,float d,float maxI,float maxOut)
{
    pid->kp=p;
    pid->ki=i;
    pid->kd=d;
    pid->maxIntegral=maxI;
    pid->maxOutput=maxOut;
}

/***********************************
*PID计算
进行一次pid计算
参数为(pid结构体,目标值,反馈值)，计算结果放在pid结构体的output成员中
***********************************/

void PID_Calc(PID *pid,float reference,float feedback)
{
 	//更新数据
    pid->lastError=pid->error;//将旧error存起来
    pid->error=reference-feedback;//计算新error
    //计算微分
    float dout=(pid->error-pid->lastError)*pid->kd;
    //计算比例
    float pout=pid->error*pid->kp;
    //计算积分
    pid->integral+=pid->error*pid->ki;
    //积分限幅
    if(pid->integral > pid->maxIntegral) pid->integral=pid->maxIntegral;
    else if(pid->integral < -pid->maxIntegral) pid->integral=-pid->maxIntegral;
    //计算输出
    pid->output=pout+dout+pid->integral;
    //输出限幅
    if(pid->output > pid->maxOutput) pid->output=pid->maxOutput;
    else if(pid->output < -pid->maxOutput) pid->output=-pid->maxOutput;
}
 

 



pid_p pid;
 
//pid位置式
void PID_init()
{
//    printf("PID_init begin \n");
    pid.SetVoltage= 0.0;		  	// 设定的预期电压值
    pid.ActualVoltage= 0.0;			// adc实际电压值
    pid.err= 0.0;				    // 当前次实际与理想的偏差
    pid.err_last=0.0;			    // 上一次的偏差
    pid.voltage= 0.0;			    // 控制电压值
    pid.integral= 0.0;			  	// 积分值
    pid.Kp= 0.2;				    // 比例系数
    pid.Ki= 0.15;				    // 积分系数
    pid.Kd= 0.2;				    // 微分系数    
}
 
float PID_realize( float v, float v_r)
{
    pid.SetVoltage = v;			// 固定电压值传入
    pid.ActualVoltage = v_r;	// 实际电压传入 = ADC_Value * 3.3f/ 4096
    pid.err = pid.SetVoltage - pid.ActualVoltage;	//计算偏差
    pid.integral += pid.err;						//积分求和
    pid.result = pid.Kp * pid.err + pid.Ki * pid.integral + pid.Kd * ( pid.err - pid.err_last);//位置式公式
    pid.err_last = pid.err;				//留住上一次误差
    return pid.result;
}

























