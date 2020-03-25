# KNX sniffer

- use this sketch as KNX BUS Monitor
- it allows to see all telegrams on the bus
- work in progress... 

## Settings

to turn off just comment out some of this lines

```
#define PRINT_PA_GA       //print PA and GA
#define PRINT_CRC         //CRC result
#define PRINT_LENGTH      //payload length
#define PRINT_PAYLOAD     //payload it self
#define PRINT_RAW         //raw telegram
#define PRINT_NORMAL_ONLY //print normal case only (0xBC), without repeats and so on (0xB0, 0xB4, 0xB8, 0x90, 0x94, 0x98, 0x9C)

```

## Supported hardware KNX side

- Siemens BCU 5WG1117-2AB12 (also known as tpuart2)
- NCN5120 / NCN5121 / NCN5130
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