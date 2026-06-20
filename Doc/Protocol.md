# CAN协议说明

本文档描述GD32F4-Fertilizer-Sensor项目使用的CAN协议数据格式。

---

## 📋 协议概述

本项目通过CAN总线发送肥料传感器的检测数据（可见光ADC值、近红外ADC值、肥料类型等）。

- **CAN ID**: 可配置（默认0x200，标准帧）
- **数据长度**: 8字节
- **数据速率**: 10Hz（每100ms发送一帧）
- **波特率**: 500Kbps

---

## 📊 数据格式

### 肥料检测数据包（ID: 0x200）

| 字节 | 名称 | 类型 | 说明 |
|------|------|------|------|
| 0-1  | Visible_ADC | uint16_t | 可见光通道ADC值（0-4095） |
| 2-3  | NIR_ADC | uint16_t | 近红外通道ADC值（0-4095） |
| 4    | Fertilizer_Type | uint8_t | 肥料类型（1:复合肥 2:尿素 3:其他） |
| 5    | Status | uint8_t | 状态位（bit0:正常 bit1:校准中 bit2:故障） |
| 6-7  | Reserved | uint16_t | 预留 |

**数据格式**: 小端模式（Little-Endian）  
**数值范围**:
- Visible_ADC: 0 ~ 4095（12位ADC）
- NIR_ADC: 0 ~ 4095（12位ADC）
- Fertilizer_Type: 1 ~ 255
- Status: 0 ~ 255（位掩码）

---

## 📟 示例数据

### 示例1: 正常检测（复合肥）
```
CAN ID:  0x200（标准帧）
DLC:     8字节
Data:    0x01 0x02  0x03 0x04  0x01  0x00  0x00 0x00

解析:
  Visible_ADC = (0x02<<8)|0x01 = 0x0201 = 513
  NIR_ADC      = (0x04<<8)|0x03 = 0x0403 = 1027
  Fertilizer_Type = 0x01 (复合肥)
  Status         = 0x00 (正常)
```

### 示例2: 校准中状态
```
CAN ID:  0x200（标准帧）
DLC:     8字节
Data:    0xFF 0x0F  0xFE 0x0F  0x02  0x02  0x00 0x00

解析:
  Visible_ADC = (0x0F<<8)|0xFF = 0x0FFF = 4095 (最大值)
  NIR_ADC      = (0x0F<<8)|0xFE = 0x0FFE = 4094
  Fertilizer_Type = 0x02 (尿素)
  Status         = 0x02 (校准中)
```

---

## 🔢 数据解析代码

### C语言解析示例

```c
#include <stdint.h>
#include <stdio.h>

// CAN消息结构体
typedef struct
{
    uint32_t id;       // CAN ID
    uint8_t  dlc;      // 数据长度
    uint8_t  data[8];  // 数据缓冲区
} CanRxMsg;

// 解析肥料检测数据
void Parse_Fertilizer_Data(CanRxMsg *msg)
{
    if(msg->id != 0x200) return;
    
    // 小端模式解析
    uint16_t visible_adc = (msg->data[1] << 8) | msg->data[0];
    uint16_t nir_adc     = (msg->data[3] << 8) | msg->data[2];
    uint8_t  fert_type   = msg->data[4];
    uint8_t  status      = msg->data[5];
    
    // 打印解析结果
    printf("Visible_ADC: %d, NIR_ADC: %d, ", visible_adc, nir_adc);
    
    switch(fert_type)
    {
        case 1:  printf("Type: 复合肥"); break;
        case 2:  printf("Type: 尿素"); break;
        case 3:  printf("Type: 其他"); break;
        default: printf("Type: 未知");
    }
    
    printf(", Status: %s%s\r\n",
           (status & 0x01) ? "正常" : "异常",
           (status & 0x02) ? " 校准中" : "");
}
```

### Python解析示例（使用python-can库）

```python
import can

def parse_fertilizer_data(can_msg):
    if can_msg.arbitration_id != 0x200:
        return None
    
    # 小端模式解析
    visible_adc = int.from_bytes(can_msg.data[0:2], byteorder='little')
    nir_adc     = int.from_bytes(can_msg.data[2:4], byteorder='little')
    fert_type   = can_msg.data[4]
    status      = can_msg.data[5]
    
    # 肥料类型字符串
    type_str = {1: "复合肥", 2: "尿素", 3: "其他"}.get(fert_type, "未知")
    
    # 状态字符串
    status_str = []
    if status & 0x01: status_str.append("正常")
    if status & 0x02: status_str.append("校准中")
    if status & 0x04: status_str.append("故障")
    
    return {
        'visible_adc': visible_adc,
        'nir_adc':     nir_adc,
        'fert_type':   fert_type,
        'fert_type_str': type_str,
        'status':      status,
        'status_str':  ' '.join(status_str)
    }

# 使用python-can接收数据
bus = can.interface.Bus(channel='can0', bustype='socketcan')

for msg in bus:
    data = parse_fertilizer_data(msg)
    if data:
        print(f"Visible: {data['visible_adc']}, "
              f"NIR: {data['nir_adc']}, "
              f"Type: {data['fert_type_str']}, "
              f"Status: {data['status_str']}")
```

---

## 🔧 CAN ID配置

### 修改CAN ID

编辑 `User/task_func.c` 中的 `CAN_Send_Fertilizer_Data()` 函数：

```c
void CAN_Send_Fertilizer_Data(uint16_t visible_adc,
                              uint16_t nir_adc,
                              uint8_t fert_type,
                              uint8_t status)
{
    CanTxMsg tx_msg;
    
    tx_msg.StdId = 0x200;  // ← 改成你想要的CAN ID（0x000 ~ 0x7FF）
    tx_msg.IDE = 0;         // 标准帧（11位ID）
    tx_msg.RTR = 0;         // 数据帧
    tx_msg.DLC = 8;
    
    // ... 填充数据 ...
    
    CAN_Transmit(CAN0, &tx_msg);
}
```

---

## ⚡ 数据速率配置

默认数据速率为10Hz（每100ms发送一帧）。修改方法：

编辑 `User/mytask.c` 中的软件定时器创建代码：

```c
/* 创建软件定时器 (100ms周期) */
xTimeHandle[0] = xTimerCreate(
    "DataUpload",
    pdMS_TO_TICKS(100),  // ← 修改此值改变发送周期
    pdTRUE,
    (void *)0,
    DataUpload_CAN_Callback
);

xTimerStart(xTimeHandle[0], 0);
```

---

## 📈 CAN总线负载计算

假设：
- CAN波特率: 500Kbps
- 本设备数据速率: 10Hz
- 每帧数据: ~64 bit（ID + DLC + 8字节数据 + CRC等）

**计算**:
```
本设备占用带宽 = 10帧/s × 64 bit/帧 = 640 bit/s
总线负载 = 640 / 500000 × 100 = 0.128%
```

结论：单个设备占用带宽很小，可以同时连接多个设备。

---

## 🚨 注意事项

1. **CAN ID冲突**: 确保总线上每个设备的CAN ID唯一
2. **总线负载**: 不要超过70%总线负载，否则可能丢包
3. **终端电阻**: CAN总线两端必须各接1个120Ω终端电阻
4. **波特率匹配**: 总线上所有设备的CAN波特率必须一致
5. **ADC值范围**: ADC值为12位（0-4095），对应0-3.3V输入电压

---

## 📚 参考资料

1. **CAN总线协议**: [ISO 11898](https://www.iso.org/standard/63648.html)
2. **python-can库**: [python-can文档](https://python-can.readthedocs.io/)
3. **GD32 CAN外设**: [GD32F4xx参考手册 - CAN章节](https://www.gigadevice.com/)

---

**最后更新**: 2026-06-20
