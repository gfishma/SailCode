# SailCode V3.2 功能操作手册 (SOP)

> 适用版本：SailCode Program V3.2.x  
> 硬件平台：OQC Switch Board V3.0A + Common Emulation Board V5.0 + Sail Core Board Mini (STM32F407)

---

## 1. 系统架构

```
┌─────────────────────┐    68pin     ┌──────────────────────┐
│  Switch Board V3.0A │ ←──────────→ │ Emulation Board V5.0 │
│  (Input Side)       │              │  (Output Side)       │
│                     │              │                      │
│  ADG2128 交叉开关    │              │  DVM 模块 (ADS124S0x) │
│  HIGH_CURR_CH 继电器 │              │  HIGH_CURR/VOLT 路由  │
│  HIGH_VOLT_CH 继电器 │              │  AD5667 DAC (CVS/CCS) │
│  CAT9555 IO 扩展     │              │  HUSB238 PD 控制器    │
└─────────────────────┘              └──────────────────────┘
          │                                    │
          └──────────┬─────────────────────────┘
                     │
            ┌────────┴────────┐
            │ Sail Core Board │
            │   (STM32F407)   │
            │   SCMD 命令行    │
            └─────────────────┘
```

**信号流**：信号源 → Switch Board (X→Y 交叉开关) → Emulation Board (继电器路由 → Y7/Y8 → T13 → DVM CH2，或直接 DVM CH1)

**命令格式**：所有命令以 `>` 开头，以 `\r\n` 结尾。例如：`>em_dmm help\r\n`

---

## 2. em_dmm — 电压测量

### 2.1 功能概述

`em_dmm` 提供三种电压测量方式：
- **X 通道测量**：通过交叉开关矩阵 X→Y6→T13→DVM CH2，范围 0~10V
- **HIGH_CURR_CH 测量**：大电流继电器通道，15 路 (CH1-15)
- **HIGH_VOLT_CH 测量**：高电压继电器通道，28 路 (CH1-28)

### 2.2 命令用法

```
>em_dmm help                          -- 显示帮助
>em_dmm info                          -- 显示测量类型说明
>em_dmm(Xn)                           -- 测量 X 通道 (n: 1-300)
>em_dmm(HIGH_CURR_CHn)                -- 测量 HIGH_CURR 通道 (n: 1-15)
>em_dmm(HIGH_VOLT_CHn)                -- 测量 HIGH_VOLT 通道 (n: 1-28)
```

### 2.3 测量路径说明

> **重要：X 通道和 HIGH_xxx 通道的使用方式不同！**

#### X 通道（直接测量，无需任何配置）

X1~X300 的测量路径是固定的：`X→Y6→T13→DVM CH2`。不需要任何 IO 配置，直接发指令即可：

```
>em_dmm(X1)
<em_dmm(ok) X1 2495mV
```

#### HIGH_CURR_CH / HIGH_VOLT_CH（需要预先配置信号路径）

与 X 不同，HIGH_CURR_CH 和 HIGH_VOLT_CH 是**继电器选通的输入通道**，信号需要通过 IO 配置才能到达 DVM。`em_dmm` 命令**只读 IO 状态判断路径，不主动切换 IO**。

**三种使用场景：**

| 场景 | 前置条件 | 指令 | 说明 |
|------|---------|------|------|
| **1. 已切换到普通输出 (T1-48)** | 通道通过矩阵连到了 Y 总线 | `em_dmm(HIGH_CURR_CHn)` | LV 路径，等同于 X 测量 |
| **2. 已切换到 DVM_MES_HIGH** | IO 配置为 HV 路径 | `em_dmm(HIGH_CURR_CHn)` | HV 路径，DVM CH1 测量 |
| **3. 未配置（默认/不满足 1 或 2）** | — | 报错或读数异常 | 需先手动配 IO 再测量 |

**LV/HV 自动判定逻辑：**

`em_dmm` 通过读取以下 IO 状态判定走哪条路：

| 通道类型 | LV 路径 (→Y→T13→DVM CH2) | HV 路径 (→DVM CH1, ×6) |
|---------|--------------------------|------------------------|
| HIGH_CURR_CH | IO79=1, IO87=0 (IO77 不在意) | IO79=0, IO87=1, **IO77=0** |
| HIGH_VOLT_CH | IO78=1, IO87=0 (IO77 不在意) | IO78=0, IO87=0, **IO77=0** |

> **三个 IO 条件必须全部满足才走 HV 路径**，否则走 LV 路径。IO87 和 IO77 在 HIGH_CURR 和 HIGH_VOLT 之间共享——两者不能同时使用 HV 模式。

**场景 1 详解——切换到普通输出 (LV)：**

当 HIGH_CURR_CH 或 HIGH_VOLT_CH 被切换到矩阵输出侧的 **T1-T48** 上时，`em_dmm` 自动识别 LV 路径，内部调用 `switch yf set(Y, T13, ON)` 把信号接到 DVM CH2。

```
步骤 1: 用继电器把通道信号路由到 Y 总线
步骤 2: em_dmm(HIGH_CURR_CHn)  →  自动识别 LV → Y→T13→DVM CH2
```

**场景 2 详解——切换到 DVM_MES_HIGH (HV)：**

当通道被配置为直接连接 DVM CH1 时（三个 IO 条件全部满足），`em_dmm` 走 HV 路径，直接读数 ×6。

```
步骤 1: em_io set([xx, 路径IO])  →  配好 HV 通路的三个 IO
步骤 2: em_io set([xx, 通道IO])   →  选通道
步骤 3: em_dmm(HIGH_CURR_CHn)     →  自动识别 HV → DVM CH1 ×6
```

> **未配好路径时**：如果 HIGH_CURR_CH/HIGH_VOLT_CH 没有连到 T1-48 也没有切到 DVM_MES_HIGH，`em_dmm` 会报 IO 状态不匹配。

### 2.4 使用示例

```
# === X 通道：直接测 ===
>em_dmm(X1)
<em_dmm(ok) X1 2495mV

# === 场景 1: HIGH_CURR_CH1 已切到矩阵输出 (LV) ===
>em_dmm(HIGH_CURR_CH1)
<em_dmm(ok) HIGH_CURR_CH1 3310mV

# === 场景 2: HIGH_VOLT_CH1 切到 DVM_MES_HIGH (HV) ===
# 先配 HV 三条件
>em_io set([78,0],[87,0],[77,0])
>em_io set([65,1],[72,0])            -- 选 HIGH_VOLT_CH1
>em_dmm(HIGH_VOLT_CH1)
<em_dmm(ok) HIGH_VOLT_CH1 2508mV

# === 场景 3: 未配路径 → 报错 ===
>em_dmm(HIGH_CURR_CH1)
<em_dmm(error) HIGH_CURR_CH IO state unknown (LV/HV?)
```

### 2.5 通道 IO 映射

**HIGH_CURR_CH (15 通道，one-hot 编码，所有 IO=0 为 OFF)**：

| 通道 | IO | 通道 | IO |
|------|-----|------|-----|
| CH1 | 57 | CH9 | 89 |
| CH2 | 58 | CH10 | 90 |
| CH3 | 59 | CH11 | 91 |
| CH4 | 60 | CH12 | 92 |
| CH5 | 61 | CH13 | 93 |
| CH6 | 62 | CH14 | 94 |
| CH7 | 63 | CH15 | 95 |
| CH8 | 64 | OFF | 全 0 |

**HIGH_VOLT_CH (28 通道，行 IO + IO72 列)**：

| 通道 | 行 IO | IO72 | 通道 | 行 IO | IO72 |
|------|-------|------|------|-------|------|
| CH1 | 65 | 0 | CH15 | 52 | 0 |
| CH2 | 65 | 1 | CH16 | 52 | 1 |
| CH3 | 66 | 0 | CH17 | 53 | 0 |
| CH4 | 66 | 1 | CH18 | 53 | 1 |
| CH5 | 67 | 0 | CH19 | 54 | 0 |
| CH6 | 67 | 1 | CH20 | 54 | 1 |
| CH7 | 68 | 0 | CH21 | 55 | 0 |
| CH8 | 68 | 1 | CH22 | 55 | 1 |
| CH9 | 69 | 0 | CH23 | 56 | 0 |
| CH10 | 69 | 1 | CH24 | 56 | 1 |
| CH11 | 70 | 0 | CH25 | 85 | 0 |
| CH12 | 70 | 1 | CH26 | 85 | 1 |
| CH13 | 71 | 0 | CH27 | 86 | 0 |
| CH14 | 71 | 1 | CH28 | 86 | 1 |

---

## 3. em_cvs — 恒压源

### 3.1 功能概述

通过 AD5667 DAC B 通道输出恒定电压，支持三种模式。

### 3.2 命令用法

```
>em_cvs help                          -- 显示帮助
>em_cvs info                          -- 显示 CVS 配置
>em_cvs set(LP, V)                    -- 低压模式 0~5V
>em_cvs set(HP, V)                    -- 高压模式 0~35V
>em_cvs set(NP, V)                    -- 负压模式 0~-5V
```

### 3.3 输出通路说明

> **重要**：三种模式的物理输出通路不同，不可互换。

```
┌──────────┐   DAC B → LP    → T16 (交叉开关输出侧)
│ AD5667   │   DAC B → HP    → 独立继电器通路 (IO7=1, IO34=1)
│ DAC B    │   DAC B → NP    → 独立继电器通路 (IO33=1)
└──────────┘
```

**LP 通路**：DAC → Output Side 开关矩阵 → T16 → 通过 `switch set(X, Y, T16, ON)` 把电压路由到任意 X 通道 (Y 为任意可用通道: 1-5, 7-8)
**HP 通路**：DAC → IO7 继电器 → IO34 继电器 → 专用 HP 输出端 (不在开关矩阵上)
**NP 通路**：DAC → IO33 继电器 → 专用 NP 输出端 (不在开关矩阵上)

> LP 可以通过矩阵路由到 300 路输入，HP/NP 只能从各自的专用输出端测量。
> **HP/NP 暂不支持电压回读**。

### 3.4 IO 配置

| 模式 | IO7 | IO33 | IO34 | 输出范围 |
|------|-----|------|------|----------|
| **LP** (低压) | 0 | 0 | x | 0 ~ 5V |
| **HP** (高压) | 1 | 0 | 1 | 0 ~ 35V |
| **NP** (负压) | 0 | 1 | x | 0 ~ -5V |

### 3.5 使用示例

```
# LP: 输出 2.5V，通过矩阵 X1→Y→T16 接出 (Y 为 1-5/7-8 任意通道)
>em_cvs set(LP, 2.5V)
>switch set(X1, Y2, T16, ON)
>em_dmm(X1)
<em_cvs set(ok) LP 2.50V
<em_dmm(ok) X1 2495mV

# HP: 输出 15V，从 HP 专用输出端接出 (需配置 IO7=1, IO34=1, IO6=1, IO20=1, IO87=1)
>em_cvs set(HP, 15V)
>em_io set([7,1],[34,1],[6,1],[20,1],[87,1])
<em_cvs set(ok) HP 15.00V

# NP: 输出 -3V，从 NP 专用输出端接出 (需配置 IO33=1)
>em_cvs set(NP, 3V)
>em_io set([33,1])
<em_cvs set(ok) NP 3.00V
```

---

## 4. em_ccs — 恒流源

### 4.1 功能概述

通过 AD5667 DAC A 通道输出恒定电流，支持自动量程选择。

### 4.2 命令用法

```
>em_ccs help                          -- 显示帮助
>em_ccs info                          -- 显示 CCS 配置
>em_ccs set(mA)                       -- 设置目标电流 (最大 10mA)
>em_ccs read                          -- 回读当前电流
```

### 4.3 量程说明

CCS 支持自动量程选择，无需手动指定。系统根据目标电流自动选最高取样电阻（V = I × R ≤ 5V）。

| 量程 | 取样电阻 | IO1/IO2 |
|------|---------|---------|
| 0 | 100Ω | 00 |
| 1 | 499Ω | 01 |
| 2 | 10KΩ | 10 |
| 3 | 1MΩ | 11 |

> **注意**：当前运放最大推 10mA，超过会报错 `max 10mA (op-amp limit)`。
> 自动量程根据 |I|×R≤5V 选最高档位。实际可设置电流受量程和 10mA 上限共同约束。

### 4.4 极性控制

| MUX | IO3/IO4 | IO5 (EN) | 说明 |
|-----|---------|----------|------|
| S1 (正) | 00 | 1 | 电流从正端流出 |
| S2 (负) | 01 | 1 | 电流从负端流出 |

### 4.5 使用示例

```
# 设置 10mA 恒流
>em_ccs set(10mA)
<em_ccs set(ok) 10.00mA

# 回读电流 (通过 DVM CH4)
>em_ccs read
<em_ccs read(ok) 9.98mA
```

---

## 5. em_io — 扩展 IO 控制

### 5.1 功能概述

通过 CAT9555 I2C GPIO 扩展器控制 96 路 IO。IO 1-48 为内部使用（锁定），IO 49-96 可用户配置。

### 5.2 命令用法

```
>em_io help                           -- 显示帮助
>em_io info                           -- 显示所有 IO 芯片状态
>em_io scan                           -- 扫描检测 CAT9555 芯片
>em_io init                           -- 初始化/重置所有 IO
>em_io reset                          -- 恢复默认 IO 状态
>em_io set(io_num, level)             -- 单 IO 设置 (level: 0/1)
>em_io set([io1, lv1],[io2, lv2],...) -- 多 IO 批量设置
>em_io read(io_num)                   -- 单 IO 读取
>em_io read([io1],[io2],...)          -- 多 IO 批量读取
>em_io config(IO_start-IO_end, i2c_x, mux_ch, addr)  -- IO 映射配置
```

### 5.3 IO 芯片分布

| 芯片 | IO 范围 | 默认总线 | 默认 MUX | 用途 |
|------|--------|---------|---------|------|
| Chip 0 | 1-16 | I2C2 CH7 | 6 | 内部 (锁定) |
| Chip 1 | 17-32 | I2C2 CH7 | 6 | 内部 (锁定) |
| Chip 2 | 33-48 | I2C2 CH7 | 6 | 内部 (锁定) |
| Chip 3 | 49-64 | I2C1 CH1 | 0 | **用户可配** |
| Chip 4 | 65-80 | I2C1 CH1 | 0 | **用户可配** |
| Chip 5 | 81-96 | I2C1 CH1 | 0 | **用户可配** |

### 5.4 路径控制 IO 速查（常用）

| IO | 功能 | 说明 |
|----|------|------|
| 77 | DVM_HV 路由 | 0=HV 路径, 1=LV 路径 |
| 78 | HIGH_VOLT 路径选择 | 1→Y7(LV), 0→DVM_CH1(HV) |
| 79 | HIGH_CURR 路径选择 | 1→Y8(LV), 0→DVM_CH1(HV) |
| 87 | DVM 测量总线隔离 | 0=LV(Y总线), 1=HV(DVM CH1) |

### 5.5 诊断：scan 扫描 IO 芯片

`em_io scan` 扫描所有 6 颗 CAT9555 芯片是否在线，**快速定位 I2C 通信故障**。

```
>em_io scan
<em_io scan:
  Chip 0: I2C2 CH6 0x20 OK
  Chip 1: I2C2 CH6 0x21 OK
  Chip 2: I2C2 CH6 0x22 OK
  Chip 3: I2C1 CH0 0x20 OK
  Chip 4: I2C1 CH0 0x21 OK
  Chip 5: I2C1 CH0 0x22 OK
```

- **OK** — 芯片通信正常
- **no response** — 芯片无应答，检查 I2C 总线连接、MUX 通道、供电

### 5.6 使用示例

```
# 设置 HIGH_CURR_CH 为 LV 路径
>em_io set([79,1],[87,0])

# 设置 HIGH_VOLT_CH 为 HV 路径
>em_io set([78,0],[87,0],[77,0])

# 查看当前 IO 状态
>em_io read([77],[78],[79],[87])
<em_io read(ok) IO77=0, IO78=0, IO79=0, IO87=0
```

---

## 6. switch — 交叉开关矩阵

### 6.1 功能概述

通过 ADG2128 模拟交叉开关矩阵连接信号。300 路输入 (X) × 8 路 Y 总线 × 48 路输出 (T)。

### 6.2 命令用法

```
>switch help                          -- 显示帮助
>switch info                          -- 显示矩阵配置
>switch scan                          -- 扫描检测所有 ADG2128 芯片
>switch reset                         -- 关闭所有开关
>switch set(X, Y, T, ON/OFF)          -- 连接 X→Y→T (X:1-300, Y:1-5/7-8, T:1-48)
>switch yf set(Y, T, ON/OFF)           -- 仅连接 Y→T (不切换输入侧)
```

### 6.3 注意事项

- **Y6 预留给测量总线**，不可用于 switch set/switch yf set
- T13 是 DVM CH2 的测量输入点
- Y7/Y8 在 HIGH_VOLT/HIGH_CURR 测量时用于 LV 路径；**不测量时 Y7/Y8 可正常作为矩阵通道使用**

### 6.4 诊断：scan 扫描矩阵芯片

`switch scan` 自动扫描 Input 和 Output 两侧所有 ADG2128 芯片，并与当前配置对比，**快速定位矩阵硬件故障**。

```
>switch scan
<scan input (I2C1 PCA9847 0x59):
  CH1: 0x70 0x71 0x72 0x73 0x74 0x75 0x76 0x77 OK
  CH2: 0x70 0x71 0x72 0x73 0x74 0x75 0x76 0x77 OK
  CH3: 0x70 0x71 0x72 0x73 0x74 0x75 0x76 0x77 OK
  CH4: 0x70 OK
<scan output (I2C2 PCA9847 0x59):
  CH4: 0x70 0x71 0x72 0x73 OK
```

- **OK** — 芯片数量与配置匹配
- **MISMATCH** — 芯片缺失或多余，检查对应 MUX 通道和芯片供电
- 每个 `0xXX` 是一个 I2C 地址，对应一颗 ADG2128

> **常用诊断组合**：先 `em_io scan` 确认 IO 扩展正常，再 `switch scan` 确认矩阵正常，基本覆盖所有 I2C 设备。

### 6.5 使用示例

```
# 将 X1 通过 Y2 连接到 T16 (CVS LP 输出)
>switch set(X1, Y2, T16, ON)
<switch set(ok) X1-Y2-T16 ON

# 断开
>switch set(X1, Y2, T16, OFF)

# Y7/Y8 也可用作普通矩阵通道，例如 Y8→T1
>switch yf set(Y8, T1, ON)
```

---

## 7. pd — USB PD 控制

### 7.1 功能概述

通过 HUSB238 芯片协商 USB PD 电压，双通道独立控制。

### 7.2 命令用法

```
>pd help                              -- 显示帮助
>pd cal(request, V)                   -- PD CAL 通道 (CH3), 请求电压
>pd test(request, V)                  -- PD TEST 通道 (CH8), 请求电压
```

### 7.3 支持的电压

| 命令值 | 实际电压 | PDO 编码 |
|--------|---------|---------|
| 5V | 5V | 0x10 |
| 9V | 9V | 0x20 |
| 12V | 12V | 0x30 |
| 15V | 15V | 0x80 |
| 18V | 18V | 0x90 |
| 20V | 20V | 0xA0 |

### 7.4 使用示例

```
# PD CAL 请求 12V
>pd cal(request, 12V)

# PD TEST 请求 9V
>pd test(request, 9V)
```

---

## 8. dvm — 精密电压测量

### 8.1 功能概述

通过 ADS124S0x 24-bit ADC 进行精密电压测量，4 通道。

### 8.2 命令用法

```
>dvm help                             -- 显示帮助
>dvm info                             -- 显示 DVM 配置
>dvm get(dvm_0, ch)                   -- 读取通道电压 (ch:1-4)
>dvm config(dvm_0, i2c_x, spi_x, cs=ioXX, drdy=ioXX, start=ioXX)  -- 配置 DVM
```

### 8.3 通道说明

| 通道 | 用途 |
|------|------|
| CH1 | 高压测量 (1/6 分压, 0~25V) |
| CH2 | 标准测量 (T13 输入) |
| CH3 | — |
| CH4 | CCS 电流回读 |

### 8.4 使用示例

```
# 读取 DVM CH2 电压
>dvm get(dvm_0, 2)
<dvm get(164mV)

# 读取 DVM CH1 (高压)
>dvm get(dvm_0, 1)
<dvm get(2500mV)
```

---

## 附录 A：完整测量流程示例

### A.1 用 em_dmm 测量 X 通道电压

```
>em_dmm(X5)
<em_dmm(ok) X5 3300mV
```

### A.2 用 HIGH_CURR_CH LV 路径测量电流通道电压

```
# 步骤 1: 配置路径 IO (LV)
>em_io set([79,1],[87,0])

# 步骤 2: 选择通道 (例: CH1)
>em_io set([57,1],[95,0])

# 步骤 3: 测量
>em_dmm(HIGH_CURR_CH1)
<em_dmm(ok) HIGH_CURR_CH1 3310mV
```

### A.3 CVS LP 输出电压并测量验证

CVS LP 通过输出侧开关矩阵 T16 输出，所以需要矩阵路由到 X 通道再测量。

```
# 步骤 1: 设置 CVS LP 输出 2.5V
>em_cvs set(LP, 2.5V)
<em_cvs set(ok) LP 2.50V

# 步骤 2: 通过开关矩阵连接 X1→Y2→T16 (T16 = CVS LP 输出点)
>switch set(X1, Y2, T16, ON)

# 步骤 3: 测量 X1 电压
>em_dmm(X1)
<em_dmm(ok) X1 2495mV
```

> HP/NP 输出在专用继电器端子上，不在开关矩阵上。HP 测量需配置 IO6/IO7/IO20/IO34/IO87=1 后用 `dvm get(dvm_0, 1)` 读 DVM CH1。

### A.4 CCS 设置并回读

```
# 步骤 1: 设置 10mA 恒流
>em_ccs set(10mA)

# 步骤 2: 回读
>em_ccs read
<em_ccs read(ok) 9.98mA
```

---

## 附录 B：常用 IO 速查表

| IO | 芯片 | 引脚 | 功能 |
|----|------|------|------|
| 1-5 | Chip0 | 0-4 | CCS 量程/MUX (内部) |
| 6-7 | Chip0 | 5-6 | CVS 模式选择 (内部) |
| 52-56 | Chip3 | 3-7 | HIGH_VOLT_CH 行选择 |
| 57-64 | Chip3 | 8-15 | HIGH_CURR_CH1-8 选择 |
| 65-71 | Chip4 | 0-6 | HIGH_VOLT_CH 行选择 |
| 72 | Chip4 | 7 | HIGH_VOLT_CH 列选择 |
| 77 | Chip4 | 12 | DVM_HV 路由 |
| 78 | Chip4 | 13 | HIGH_VOLT 路径选择 |
| 79 | Chip4 | 14 | HIGH_CURR 路径选择 |
| 85-86 | Chip5 | 4-5 | HIGH_VOLT_CH 行选择 |
| 87 | Chip5 | 6 | DVM 总线隔离 |
| 89-95 | Chip5 | 8-14 | HIGH_CURR_CH9-15 选择 |
