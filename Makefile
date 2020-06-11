######################################
# target
######################################
TARGET = Project

######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -O3

#######################################
# paths
#######################################
# Build path
BUILD_DIR = Build

######################################
# source
######################################
# C sources

# Application
C_SOURCES = Core/CM7/Src/main.c
C_SOURCES += Core/CM7/Src/display.c
C_SOURCES += Core/CM7/Src/sd_diskio.c
C_SOURCES += Core/CM7/Src/stm32h7xx_hal_msp.c
C_SOURCES += Core/CM7/Src/stm32h7xx_it.c
C_SOURCES += Core/Common/Src/system_stm32h7xx.c

# HAL Drivers
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_crc.c
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_crc_ex.c
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dcmi.c
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.c
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma2d.c
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.c
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dsi.c
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c.c
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c_ex.c
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc.c
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc_ex.c
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdma.c
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_qspi.c
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rng.c
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sd.c
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sd_ex.c
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sdram.c
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart.c
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart_ex.c
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_fmc.c
C_SOURCES += Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_sdmmc.c

# BSP Drivers
C_SOURCES += Drivers/BSP/STM32H747I-Discovery/stm32h747i_discovery.c
# C_SOURCES += Drivers/BSP/STM32H747I-Discovery/stm32h747i_discovery_camera.c # Patched
# C_SOURCES += Drivers/BSP/STM32H747I-Discovery/stm32h747i_discovery_lcd.c # Patched
C_SOURCES += Drivers/BSP/STM32H747I-Discovery/stm32h747i_discovery_qspi.c
C_SOURCES += Drivers/BSP/STM32H747I-Discovery/stm32h747i_discovery_sd.c
C_SOURCES += Drivers/BSP/STM32H747I-Discovery/stm32h747i_discovery_sdram.c

# Patches
C_SOURCES += Extension/Drivers/BSP/STM32H747I-Discovery/stm32h747i_discovery_camera_patch.c
C_SOURCES += Extension/Drivers/BSP/STM32H747I-Discovery/stm32h747i_discovery_lcd_patch.c

# BSP Compenents
C_SOURCES += Drivers/BSP/Components/otm8009a/otm8009a.c
C_SOURCES += Drivers/BSP/Components/ov9655/ov9655.c

# Middlewares
C_SOURCES += Middlewares/Third_Party/FatFs/src/option/ccsbcs.c
C_SOURCES += Middlewares/Third_Party/FatFs/src/diskio.c
C_SOURCES += Middlewares/Third_Party/FatFs/src/ff.c
C_SOURCES += Middlewares/Third_Party/FatFs/src/ff_gen_drv.c
C_SOURCES += Middlewares/Third_Party/FatFs/src/option/syscall.c
C_SOURCES += Middlewares/ST/STM32_Fs/stm32_fs.c
C_SOURCES += Middlewares/ST/STM32_ImgProc/Src/stm32_img_convert.c
C_SOURCES += Middlewares/ST/STM32_ImgProc/Src/stm32_img_crop.c
C_SOURCES += Middlewares/ST/STM32_ImgProc/Src/stm32_img_resize.c

# ASM sources
ASM_SOURCES = startup_stm32h747xx.s

#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size

HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m7

# fpu
FPU = -mfpu=fpv5-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS =

# C defines
C_DEFS = -DUSE_HAL_DRIVER
C_DEFS += -DCORE_CM7
C_DEFS += -D__FPU_PRESENT=1
C_DEFS += -DARM_MATH_CM7
C_DEFS += -DCAMERA_CAPTURE_RES=2
#C_DEFS += -DUSE_IMG_ASSERT=1
C_DEFS += -DSTM32H747xx
C_DEFS += -DUSE_STM32H747I_DISCOVERY

# AS includes
AS_INCLUDES =

# C includes
C_INCLUDES = -ICore/CM7/Inc
C_INCLUDES += -IExtension/Drivers/BSP/STM32H747I-Discovery
C_INCLUDES += -IDrivers/CMSIS/Device/ST/STM32H7xx/Include
C_INCLUDES += -IDrivers/CMSIS/Include
C_INCLUDES += -IDrivers/CMSIS/DSP/Include
C_INCLUDES += -IDrivers/STM32H7xx_HAL_Driver/Inc
C_INCLUDES += -IDrivers/BSP/STM32H747I-Discovery
C_INCLUDES += -IDrivers/BSP/Components/Common
C_INCLUDES += -IUtilities/Log
C_INCLUDES += -IUtilities/Fonts
C_INCLUDES += -IUtilities/CPU
C_INCLUDES += -IMiddlewares/Third_Party/FatFs/src
C_INCLUDES += -IMiddlewares/ST/STM32_Fs
C_INCLUDES += -IMiddlewares/ST/STM32_ImgProc/Inc
C_INCLUDES += -IMiddlewares/ST/STM32_AI/Inc

# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

# disable certain flags for DSP lib
CFLAGS += -fno-strict-aliasing

ifeq ($(DEBUG), 1)
CFLAGS += -g3 -gdwarf-2
endif

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

#######################################
# LDFLAGS
#######################################
# linker script
LDSCRIPT = STM32H747XIHx_CM7.ld

# libraries
LIBS = -lc -lm -lnosys

# Uncomment to use AI Library
# LIBS += -l:NetworkRuntime500_CM7_GCC.a # AI Lib
# LIBDIR = -LMiddlewares/ST/STM32_AI/lib

LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections,--print-memory-usage
LDFLAGS += -u _printf_float

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf

#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@


$(BUILD_DIR):
	mkdir -p $@

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

#######################################
# flash the target
#######################################
flash: $(BUILD_DIR)/$(TARGET).elf
	 STM32_Programmer_CLI.exe -c port=swd -d $(BUILD_DIR)/$(TARGET).elf -s
	 # Uncomment if you want to use an external flash loader (if you use external flash)
	 # STM32_Programmer_CLI.exe --extload "MT25TL01G_STM32H747I-DISCO.stldr" -c port=swd -d $(BUILD_DIR)/$(TARGET).elf -s
