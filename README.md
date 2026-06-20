# GD32F4-Fertilizer-Sensor

**基于GD32F4xx + FreeRTOS的肥料传感器Demo项目**

---

## 📖 项目简介

本项目基于GD32F407，通过ADC采样光电传感器的输出信号，使用PID算法调节LED光源强度，实现肥料成分（如氮、磷、钾含量）的实时检测，并通过CAN总线将检测结果上报。

### 主要功能
- ✅ ADC多通道采样（光电传感器信号）
- ✅ PID算法调节LED光强（DAC输出）
- ✅ FreeRTOS实时操作系统
- ✅ CAN总线数据上报
- ✅ 支持多种肥料类型参数配置
- ✅ 独立看门狗（IWDG）防止程序跑飞
- ✅ OTA远程升级（Bootloader支持）

### 硬件平台
- **MCU**: GD32F407VET6
- **传感器**: 光电传感器（可见光/近红外）
- **通信接口**: CAN总线 (需外接TJA1050收发器)
- **调试接口**: USART串口

---

## 🔧 硬件连接

### GD32F407 ↔ 光电传感器

| 光电传感器引脚 | 说明 | GD32引脚 | 备注 |
|---------------|------|----------|------|
| VCC | 电源正 (3.3V/5V) | 3.3V/5V | 根据传感器规格 |
| GND | 电源地 | GND | - |
| OUT1 | 信号输出1 (可见光) | PA0 (ADC0_CH0) | ADC采样通道0 |
| OUT2 | 信号输出2 (近红外) | PA1 (ADC0_CH1) | ADC采样通道1 |
| LED_CTRL | LED光强控制输入 | DAC0_OUT (PA4) | DAC输出控制LED |

### GD32F407 ↔ TJA1050 (CAN收发器)

| TJA1050引脚 | GD32引脚 | 备注 |
|-------------|----------|------|
| RXD | PB8 (CAN0_RX) | CAN接收引脚 |
| TXD | PB9 (CAN0_TX) | CAN发送引脚 |
| CANH | CAN总线H | 需120Ω终端电阻 |
| CANL | CAN总线L | 需120Ω终端电阻 |

### 调试串口

| USB-TTL模块 | GD32引脚 | 说明 |
|-------------|----------|------|
| VCC | 不连接 | 无需供电 |
| GND | GND | 共地 |
| TXD | PB10 (USART2_RX) | 模块发送 → GD32接收 |
| RXD | PB11 (USART2_TX) | 模块接收 ← GD32发送 |

---

## 📂 目录结构

```
GD32F4-Fertilizer-Sensor/
├── Doc/                          # 文档
│   ├── README.md                 # 本文件
│   ├── Hardware.md               # 硬件连接详细说明
│   ├── Protocol.md               # CAN协议说明
│   └── FreeRTOS_Config.md       # FreeRTOS配置说明
├── Driver/                        # 驱动层
│   ├── ADC/                     # ADC驱动
│   │   ├── adc.c/h             # ADC底层驱动
│   │   └── adc_filter.c/h      # ADC滤波算法
│   ├── DAC/                     # DAC驱动
│   │   └── dac.c/h            # DAC底层驱动
│   ├── CAN/                     # CAN驱动
│   │   └── can.c/h            # CAN底层驱动
│   ├── LED/                     # LED驱动
│   ├── TIMER/                   # 定时器驱动
│   └── USART/                   # 串口驱动
├── FreeRTOS/                     # FreeRTOS源码
│   ├── core/                    # FreeRTOS内核
│   └── portable/                # 移植层
├── Core/                         # GD32核心文件
│   └── GD32F4xx_Firmware_Library/  # GD32标准外设库
├── User/                         # 应用层
│   ├── main.c                   # 主程序
│   ├── main.h
│   ├── mytask.c/h              # FreeRTOS任务创建
│   └── task_func.c/h          # 任务功能函数
├── Bootloader/                   # Bootloader (支持OTA升级)
│   └── ...
└── .gitignore
```

---

## 🚀 快速开始

### 1. 硬件准备
- GD32F407开发板
- 光电传感器模块（双通道：可见光 + 近红外）
- TJA1050 CAN收发器模块 (可选)
- USB-TTL串口模块

### 2. 软件准备
- Keil uVision5
- GD32F4xx标准外设库
- FreeRTOS v10.x

### 3. 编译和下载
1. 打开 `User/` 目录下的Keil工程文件
2. 编译工程 (Build)
3. 使用J-Link或DAP-Link下载到GD32

### 4. 运行
1. 打开串口调试助手 (波特率115200)
2. 复位GD32
3. 观察串口输出的ADC采样值和PID调节结果
4. 观察CAN总线数据（如果连接了CAN分析仪）

---

## 📊 核心代码说明

### 主函数 (main.c)

```c
int main(void)
{
    /* 设置中断向量表偏移 (APP起始地址0x08010000) */
    SCB->VTOR = APP_Start;

    /* 硬件初始化 */
    Hardware_Init();

    /* 创建开始任务 */
    xTaskCreate((TaskFunction_t )start_task,
                (const char*    )"start_task",
                (uint16_t       )START_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )START_TASK_PRIO,
                (TaskHandle_t*  )&StartTask_Handler);

    /* 启动任务调度器 */
    vTaskStartScheduler();

    return 0;
}
```

### 硬件初始化 (Hardware_Init)

```c
void Hardware_Init(void)
{
    NVIC_Config();       // 中断优先级配置
    Timer1_Init();       // 定时器1初始化
    GPIO_Init();         // GPIO初始化
    CAN_HardInit();      // CAN硬件初始化
    CanIDFliter();       // CAN过滤器配置
    DMA_Init();          // DMA初始化 (用于ADC多通道采样)
    ADC_Init();          // ADC初始化
    DAC_Init();          // DAC初始化
    Timer5_Init();       // 定时器5初始化

    /* PID参数初始化 (光强调节) */
    PID_Init(&PidControl_DA1, 0.275, 0.000085, 5.0,
             DA1_OUT_MAX, DA1_OUT_MAX);
}
```

### FreeRTOS任务架构

```
start_task (开始任务)
    │
    ├── Condition_monitor (状态监测任务)
    │       - 监测工作状态
    │       - 检测CAN通信超时
    │       - 控制LED指示灯
    │
    ├── Dust_compensation_Task (AD1 PID调节任务)
    │       - 读取ADC1采样值 (可见光通道)
    │       - PID算法计算DAC输出值
    │       - 调节LED光强
    │
    ├── AD_Voltage_Task (DA2调节任务)
    │       - 读取ADC2采样值 (近红外通道)
    │       - 计算肥料成分含量
    │       - DAC2输出补偿信号
    │
    ├── SmallSeed_Task (数据处理任务)
    │       - 处理ADC采样数据
    │       - 应用滑动平均滤波
    │       - 计算肥料检测结果
    │
    └── can_msg_task (CAN消息处理任务)
            - 接收CAN消息 (参数配置)
            - 发送检测结果 (CAN上报)
```

---

## 📋 CAN协议说明

### 肥料检测数据包 (ID: 可配置，默认0x200)

| 字节 | 名称 | 类型 | 说明 |
|------|------|------|------|
| 0-1  | Visible_ADC | uint16_t | 可见光通道ADC值 (0-4095) |
| 2-3  | NIR_ADC | uint16_t | 近红外通道ADC值 (0-4095) |
| 4    | Fertilizer_Type | uint8_t | 肥料类型 (1:复合肥 2:尿素 3:其他) |
| 5    | Status | uint8_t | 状态位 (bit0:正常 bit1:校准中) |
| 6-7  | Reserved | uint16_t | 预留 |

**数据示例**:
```
CAN ID:  0x200 (标准帧)
DLC:     8字节
Data:    0x01 0x02  0x03 0x04  0x01  0x00  0x00 0x00

解析:
  Visible_ADC = (0x02<<8)|0x01 = 0x0201 = 513
  NIR_ADC      = (0x04<<8)|0x03 = 0x0403 = 1027
  Fertilizer_Type = 0x01 (复合肥)
  Status         = 0x00 (正常)
```

---

## 🔥 可学习的技术点

1. **ADC多通道采样**: 如何使用DMA实现多通道ADC连续采样
2. **PID算法**: 如何使用PID算法调节LED光强
3. **DAC输出**: 如何配置DAC输出模拟电压
4. **FreeRTOS任务管理**: 如何创建、删除、挂起任务
5. **FreeRTOS队列**: 如何在任务间传递消息
6. **CAN总线**: GD32 CAN外设配置和数据收发
7. **OTA升级**: 如何通过CAN总线进行远程升级

---

## ⚠️ 注意事项

1. **ADC参考电压**: 确保ADC参考电压稳定（建议使用外部基准电压源）
2. **PID参数调节**: PID参数需要根据实际硬件调节，避免振荡
3. **DAC输出范围**: DAC输出范围为0-3.3V，注意不要超出范围
4. **CAN终端电阻**: CAN总线两端必须各接120Ω终端电阻
5. **光电传感器选型**: 根据肥料成分选择合适的光电传感器（可见光/近红外）

---

## 📚 参考资料

1. **GD32F4xx数据手册**: [兆易创新官网](https://www.gigadevice.com/)
2. **FreeRTOS官方文档**: [FreeRTOS.org](https://www.freertos.org/)
3. **PID算法教程**: [PID控制原理](https://en.wikipedia.org/wiki/PID_controller)

---

## 📄 许可证

本项目代码仅供参考学习使用。

---

## ✉️ 联系作者

如有问题或建议，欢迎提交Issue或Pull Request！

**GitHub**: [TechCasey/GD32F4-Fertilizer-Sensor](https://github.com/TechCasey/GD32F4-Fertilizer-Sensor)

---

**最后更新**: 2026-06-20
