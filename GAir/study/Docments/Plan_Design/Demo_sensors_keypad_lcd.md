


### 功能点
- 键盘（单PIN？matrix？）
- 屏幕
- mic&speaker
- gps+gprs（不在一个板子上，定位先不做）


- I2C
- 带硬件流控的UART1， 不带硬件流控的UART2
- GPO: 6 8 9
- GPIO：1 3 4 6 8 13 14 15 24 25（部分复合i2c和uartPIN使用）


### 资源分配
由于gpio 5 6 8 15各有用途，不能使用

LCD | GPIO | Air200 PIN
---|---|---
CLK   |  GPO  6 | PIN23 
Data  |  GPIO 1 | PIN27
D/C   |  GPIO 3 | PIN26
CS    |  GPO  8 | PIN22
RESET |  GPO  9 | PIN33

KEYPAD | GPIO | Air200 PIN
---|---|---
KEY0 | GPIO 4    | PIN34
KEY1 | GPIO13 | PIN20
KEY2 | GPIO 14   | PIN25

I2C| GPIO | Air200 PIN
---|---|---
SCL | GPIO 24    | PIN16
SDA | GPIO 25    | PIN17








