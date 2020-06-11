# Minimal Computer Vision Application

Simple application that grabs images from webcam, does a color conversion and display to LCD.


## Requirements

These should be accessible from your PATH

- make
- arm-none-eabi-gcc
- STM32_Programmer_CLI (Usualy installed in C/Program Files/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer_CLI.exe)
- (Optional) ST-LINK_gdbserver.exe (C/ST/STM32CubeIDE_1.0.2/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.stlink-gdb-server.win32_1.1.0.201910081157/tools/bin/ST-LINK_gdbserver.exe)

## How build

``` shell
make 
```

## How to flash


```shell
make flash 
```

> Warning, if you are using external flash, you should use the provided external flash loader. Refer to the `make flash` section of the readme


