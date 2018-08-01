# KNX sniffer

- use this sketch as KNX BUS Monitor
- it allows to see all telegrams on the bus
- work in progress... 

## Settings

to turn off simply comment out this lines

```
#define PRINT_CRC         //CRC column
#define PRINT_BYTES       //payload length
#define PRINT_PAYLOAD     //payload it self
#define PRINT_RAW         //raw telegram
#define PRINT_NORMAL_ONLY //print standard frame only, without repeats and so on... 
```

## Supported hardware KNX side

- Siemens BCU 5WG1117-2AB12 (also known as tpuart2)
- NCN5120 / NCN5121
- Elmos E981.03 / E981.23 (not tested yet)

**it strongly recomended to use galvanical isolator between BCU and Arduino (like ADUM1201)**

## Supported hardware Arduino side
- all ATmega32U4 based Arduinos (Leonardo, Micro...)
- all SAMD21G18A based Arduinos (Zero, M0...)
- Arduino MEGA2560 (not really tested yet)
- ESP32 with [Arduino core](https://github.com/espressif/arduino-esp32)
- all STM32 MCUs supported by [Arduino Core STM32](https://github.com/stm32duino/Arduino_Core_STM32)


## ToDo
- long frame support
- maybe something else...