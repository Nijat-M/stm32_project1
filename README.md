
# STM32-based Environment Control System (基于STM32的环境控制系统)

## 🇬🇧 English

### Project Overview

This is an environment monitoring and control system based on the STM32F103C8T6 microcontroller. It reads temperature and humidity data from a DHT11 sensor, displays the values on an SSD1306 OLED screen, and controls a fan and a steam generator based on predefined thresholds.

### Hardware Components

*   **Microcontroller:** STM32F103C8T6 (Blue Pill board)
*   **Sensor:** DHT11 (for temperature and humidity)
*   **Display:** SSD1306 OLED Display (I2C) - Driver library from [afiskon/stm32-ssd1306](https://github.com/afiskon/stm32-ssd1306)
*   **Actuators:**
    *   A 5V fan connected to pin `PB4`.
    *   A steam generator/humidifier connected to pin `PB10`.

### Functionality

1.  **Data Acquisition:** Reads temperature (°C) and humidity (%) from the DHT11 sensor every second.
2.  **OLED Display:**
    *   Shows the current temperature.
    *   Shows the current humidity.
    *   Displays the status of the fan (`FAN: ON` or `FAN: OFF`).
3.  **Fan Control:**
    *   The fan is connected to `PB4` (high-level trigger).
    *   It turns **ON** when the temperature exceeds 26°C.
    *   It turns **OFF** when the temperature is 26°C or lower.
4.  **Steam/Humidity Control:**
    *   The steam generator is connected to `PB10` (low-level trigger).
    *   It turns **ON** when the humidity drops below 65%.
    *   It turns **OFF** when the humidity is 65% or higher.
5.  **Data Integrity:** Includes a checksum validation for the data from the DHT11 sensor to ensure that readings are accurate. If the checksum fails, the system uses the last valid readings.

### Pinout

*   **I2C for OLED:**
    *   `PB6`: SCL
    *   `PB7`: SDA
*   **DHT11 Sensor:** `PB9`
*   **Fan:** `PB4`
*   **Steam Generator:** `PB10`

---

## 🇨🇳 中文

### 项目概述

这是一个基于 STM32F103C8T6 微控制器的环境监控系统。它通过 DHT11 传感器读取温湿度数据，并将其显示在 SSD1306 OLED 屏幕上。同时，系统会根据预设的阈值自动控制风扇和蒸汽发生器（加湿器）。

### 硬件清单

*   **微控制器:** STM32F103C8T6 (蓝色核心板)
*   **传感器:** DHT11 (用于测量温度和湿度)
*   **显示屏:** SSD1306 OLED 显示屏 (I2C) - 驱动库来源 [afiskon/stm32-ssd1306](https://github.com/afiskon/stm32-ssd1306)
*   **执行器:**
    *   一个5V风扇，连接到 `PB4` 引脚。
    *   一个蒸汽发生器/加湿器，连接到 `PB10` 引脚。

### 主要功能

1.  **数据采集:** 每秒从 DHT11 传感器读取一次温度（°C）和湿度（%）。
2.  **OLED 显示:**
    *   显示当前温度。
    *   显示当前湿度。
    *   显示风扇状态 (`FAN: ON` 或 `FAN: OFF`)。
3.  **风扇控制:**
    *   风扇连接到 `PB4` 引脚（高电平触发）。
    *   当温度 **高于** 26°C 时，风扇启动。
    *   当温度 **等于或低于** 26°C 时，风扇关闭。
4.  **蒸汽/湿度控制:**
    *   蒸汽发生器连接到 `PB10` 引脚（低电平触发）。
    *   当湿度 **低于** 65% 时，启动加湿。
    *   当湿度 **等于或高于** 65% 时，停止加湿。
5.  **数据校验:** 对 DHT11 传感器的数据进行校验和检查，以确保读数的准确性。如果校验失败，系统将保留上一次的有效读数。

### 引脚连接

*   **OLED (I2C):**
    *   `PB6`: SCL
    *   `PB7`: SDA
*   **DHT11 传感器:** `PB9`
*   **风扇:** `PB4`
*   **蒸汽发生器:** `PB10`

