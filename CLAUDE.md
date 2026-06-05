# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Flash

PlatformIO is at `/c/Users/10631/.platformio/penv/Scripts/platformio.exe`. Python (with pyserial) is at the same path.

```bash
# PlatformIO build (recommended)
/c/Users/10631/.platformio/penv/Scripts/platformio.exe run

# Flash with ST-Link
/c/Users/10631/.platformio/penv/Scripts/platformio.exe run -t upload

# Build + flash + serial monitor (full cycle)
/c/Users/10631/.platformio/penv/Scripts/platformio.exe run -t upload && \
  /c/Users/10631/.platformio/penv/Scripts/python.exe tools/serial_monitor.py --capture 10

# GNU Make build (needs arm-none-eabi-gcc in PATH)
make clean && make

# STM32CubeIDE: open .cproject or import .project
```

Target: `genericSTM32F407VGT6` (Cortex-M4F, 168MHz, 1MB Flash, 192KB RAM). Linker script: `STM32F407VGTx_FLASH.ld`.

Build excludes `Source/SCMD/App/scmd_adc.c` and `Source/SCMD/App/scmd_dac.c` (filtered in platformio.ini).

## Serial Monitor

`tools/serial_monitor.py` captures STM32 UART output (COM4, 115200 baud):

```bash
# Interactive mode (Ctrl+C to stop)
python tools/serial_monitor.py

# Capture 10 seconds, save to tools/logs/
python tools/serial_monitor.py --capture 10

# Send a SCMD command and capture response
python tools/serial_monitor.py --capture 5 --cmd ">dvm get(1)\r\n"

# View last log file path
python tools/serial_monitor.py --last-log
```

Logs are saved to `tools/logs/serial_YYYYMMDD_HHMMSS.log`. After capture, read the log file to verify firmware output.

## Architecture

Network-capable instrument firmware on STM32F407VGT6 with **FreeRTOS** (CMSIS-RTOS v2), **lwIP** TCP/IP stack, and **W5500** Ethernet controller. Uses STM32 HAL drivers directly (no CubeMX framework layer).

### Directory Map

```
Core/                STM32 HAL init (CubeMX-generated: main, gpio, usart, i2c, spi, adc, dac, dma, tim, etc.)
Drivers/             CMSIS + STM32F4xx_HAL_Driver
Middlewares/         FreeRTOS + LwIP
Source/
  App/               Application entry: app.c (SCMD callback), bps.c (BSP init), os.c (FreeRTOS tasks)
  Common/            Utilities: buffer, str, sPrintf, font, cpu, linux_list
  Devices/           Device drivers: ad5667, ads124s0x, cat9555, pca9847, w25qxx, adg2128, ntc, husb238
  Electronic/        Analog helpers: res_divider
  Internet/
    Application/     TCP server, DHCP client, DNS
    W5500/           Wiznet W5500 driver + socket layer
  Module/            High-level HW modules: DAC5667 (DAC+CCS), DVM_V2 (precision ADC), SwitchMatrix, PD, EMIO
  SCMD/
    Core/            SCMD protocol engine (scmd.c/h, scmd_str, scmd_uops) — command parser + register system
    App/             SCMD app bindings: scmd_dvm, scmd_switch, scmd_emio, scmd_em_cvs_ccs, scmd_pd, scmd_i2c, scmd_spi, scmd_pwm, scmd_config, scmd_capture
  StdPort/           Standard port abstractions: std_gpio, std_i2c, std_spi, std_usart, std_adc, std_dac, std_pwm, std_capture
  uLWIP/             lwIP integration
```

### Layered Architecture

```
Application     app.c, bps.c, os.c
SCMD Commands   scmd_*.c (dvm, switch, emio, pd, i2c, spi, pwm, config...)
Module          Module_DAC5667, Module_DVM_V2, Module_SwitchMatrix, Module_PD, Module_EMIO
Device Drivers  ad5667, ads124s0x, cat9555, pca9847, w25qxx, adg2128, ntc, husb238
StdPort         std_gpio, std_i2c, std_spi, std_usart, std_adc, std_dac, std_pwm, std_capture
HAL/Foundation  STM32 HAL, CMSIS, FreeRTOS, lwIP, W5500
```

### FreeRTOS Tasks

| Task | Priority | Stack | Purpose |
|------|----------|-------|---------|
| `vStart_thread` (defaultTask) | High | 4KB | Heartbeat LED blink, general init |
| `vSCMD_thread` | AboveNormal | 32KB | SCMD command parser: reads UART ring buffer + 8 TCP socket buffers, dispatches commands |
| `vTCP_Server_Thread` | Low | 32KB | TCP server via W5500 + lwIP |
| `vDvmTask` (disabled) | Normal7 | 2KB | DVM measurement task — code exists but task creation is commented out |

### SCMD Protocol System

The SCMD engine (`Source/SCMD/Core/scmd.h`) is a text-based command parser inspired by SCPI:

- **Commands** register via `scmd_cmd_def` structs (function pointer + name + description)
- **Registers** register via `scmd_reg_def` structs (pointer + name + type: int/float/string/pointer)
- **Format**: configurable start/end codes (default uses `\r\n` as delimiter)
- **Multi-source**: commands can come from UART (source=1) or TCP sockets (source=2-9)
- **Callback**: `scmd_callback()` in `app.c` routes responses back to the originating source

SCMD app modules (`Source/SCMD/App/`) each expose a specific hardware subsystem as SCMD commands:
- `scmd_dvm` — precision voltage measurement (ADS124S0x)
- `scmd_dac` / `scmd_adc` — DAC/ADC control (excluded from build)
- `scmd_switch` — switch matrix control
- `scmd_emio` — extended IO (CAT9555 GPIO expanders)
- `scmd_em_cvs_ccs` — constant voltage/current source control
- `scmd_pd` — USB PD (HUSB238)
- `scmd_pwm` / `scmd_spi` / `scmd_i2c` — peripheral control
- `scmd_config` — system configuration
- `scmd_capture` — input capture

### Key Hardware

- **AD5667 DAC** — dual-channel 16-bit DAC on I2C2 via PCA9847 mux (CH2), drives voltage paths and CCS current source
- **ADS124S0x** — 24-bit ADC for precision voltage measurement, with auto-ranging (DVM_V2)
- **CAT9555** — I2C GPIO expanders (3 chips: chip0 IO1-16, chip1 IO17-32, chip2 IO33-48) for relay/switch control
- **PCA9847** — I2C mux, 8 channels, for routing I2C to different devices
- **W5500** — SPI Ethernet controller with hardware TCP/IP offload
- **ADG2128** — analog crosspoint switch matrix
- **HUSB238** — USB PD sink controller

### Module Pattern

High-level modules in `Source/Module/` wrap device drivers with application logic:
- `Module_DAC5667` — DAC with 3 voltage paths (NORMAL 0-5V, AMPLIFIED 0-35V, NEGATIVE 0 to -5V) and CCS current source (4 ranges: 100R/499R/10K/1M)
- `Module_DVM_V2` — precision voltmeter with auto-ranging and calibration
- `Module_SwitchMatrix` — ADG2128 crosspoint routing
- `Module_PD` — USB PD negotiation
- `Module_EMIO` — extended I/O management

### Key Include Pattern

Unlike Core_Board_Eload, this project does NOT use a single `app_includes.h`. Files include what they need directly. The main entry chain is:

```c
// main.c includes:
#include "bsp.h"    // → pulls in all hardware init
#include "os.h"     // → pulls in FreeRTOS + task creation
```

`os.c` is where FreeRTOS tasks are created (not the auto-generated `freertos.c` — the Makefile doesn't even list it).

## Session Notes (2026-06-05, V3.0)

Key facts established during V3.0 development — new sessions should consult these:

- **AD5667**: I2C addr **0x0F** (not 0x0C). Command byte format: [R=0][S=0][C2][C1][C0][A2][A1][A0]. Write DAC A=0x18, DAC B=0x19, Power-up=0x27, Ref-on=0x3F(data=0x0001 DB0=1). Internal ref MUST be enabled (data DB0, NOT DB15).
- **CCS IO mapping** (all on chip0, pin 0-4): IO1=Range_A1(MSB), IO2=Range_A0(LSB), IO3=MUX_A1, IO4=MUX_A0, IO5=MUX_EN. Range: 00=100R,01=499R,10=10K,11=1M. MUX: S1(00)=pos, S2(01)=neg. IO1-48 locked for internal use.
- **DAC output path**: IO7(chip0 pin6) and IO33(chip2 pin0) control 3 modes — LP(0,0), HP(1,0), NP(0,1).
- **CCS readback**: DVM CH4 reads diff-amp output. Current = V_DVM / R_sense. DVM preserves sign (no manual negation needed).
- **Y6** reserved for measurement bus (`switch meas` uses Y6→T13→DVM CH2).
- **DVM global instance** needs `DVM_V2_Init(&DVM_V2)` before use (was commented out).
- **newlib-nano** doesn't support `%f` in printf/sprintf — format as `%d.%02d` integer+fraction.
- **EMIO chips 1-48 (chips 0-2) locked** for internal use, 49-96 (chips 3-5) user-configurable via `em_io config`.
- **I2C buses** are now init'd once in `bsp_init()` before all modules.
- **SCMD** buffer size: scmd_msgBuf 2048, scmd_buff 1024 (watch RAM limit ~128KB).
- **RAM state**: ~98.3% used (~2.5KB free). Disabled ADC/DAC (SCMD + StdPort) from build. PWM/Capture are ACTIVE and must stay.
- **Command naming**: `em_cvs` (constant voltage) / `em_ccs` (constant current) — separate top-level commands, not sub-commands of dac5667. File: `scmd_em_cvs_ccs.c`. Modes: LP(0-5V), HP(0-35V), NP(0~-5V). Parameters with units: `em_cvs set(LP, 2.5V)`, `em_ccs set(10mA)`.
- **`switch meas`** uses `Dvm_V2_Rang25V` range (not Dvm_V2_Auto). DVM global instance must be init'd.
- **PD module**: `pd cal` (CH3) and `pd test` (CH8) — two separate HUSB238 instances. HUSB238 negotiation: write SRC_PDO(0x08) then GO_COMMAND(0x09, 0x01). PDO encoding is non-linear: 5V=0x10,9V=0x20,12V=0x30,15V=0x80,18V=0x90,20V=0xA0.
- **Serial tools**: `tools/serial_monitor.py` for quick command testing via COM4. `tools/serial_mcp.py` is an MCP server (may not be loaded).
- **Permissions**: project `.claude/settings.json` has allowlist for build/flash/git/serial. `python -c *` deliberately NOT allowlisted (arbitrary exec risk).

## Differences from Core_Board_Eload (sibling project)

| Feature | Core_Board_Eload | SailCode_3 |
|---------|-----------------|------------|
| Networking | None | lwIP + W5500 Ethernet |
| CMSIS-RTOS | v1 | v2 |
| Command protocol | Simple UART parsing | SCMD framework (UART + TCP) |
| Include style | Single `app_includes.h` | Direct includes per file |
| Module layer | None (drivers in iCore/) | `Source/Module/` with wrapper pattern |
| Port abstraction | Direct HAL calls | `Source/StdPort/` wrappers |
| Build sources | `iCore/` | `Source/` + `Core/Src/` |
