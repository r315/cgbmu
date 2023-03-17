######################################
# target
######################################
TARGET =discocgbemu

######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og

#######################################
# paths
#######################################
# Build path
BUILD_DIR := build
REPOSITORY_DIR =$(REPO_F769)
EMU_PATH 	:=../../
######################################
# source
######################################
C_SOURCES =  \
Src/video_stm32.c \
Src/main_discovery.c \
Src/board_discovery.c \
Src/fifo.c \
Src/serial.c \
Src/it.c \
Src/hal_msp.c \
Src/system_stm32f7xx.c \
Src/sd_diskio.c \
Src/fatfs.c \
Src/syscalls.c \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_cortex.c \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma2d.c \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dsi.c \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_ll_fmc.c \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_sdram.c \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_ltdc.c \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_ltdc_ex.c \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_qspi.c \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim.c \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim_ex.c \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc.c \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc_ex.c \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash.c \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash_ex.c \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_gpio.c \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma.c \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma_ex.c \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr.c \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr_ex.c \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.c \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_i2c.c \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_i2c_ex.c \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_exti.c \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_uart.c \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_sd.c \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Src//stm32f7xx_ll_sdmmc.c \
$(REPOSITORY_DIR)/Middlewares/Third_Party/FatFs/src/diskio.c \
$(REPOSITORY_DIR)/Middlewares/Third_Party/FatFs/src/ff.c \
$(REPOSITORY_DIR)/Middlewares/Third_Party/FatFs/src/ff_gen_drv.c \

# ASM sources
ASM_SOURCES =  \
startup/startup_stm32f769xx.s

BSP_C_SOURCES = \
$(REPOSITORY_DIR)/Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery.c \
$(REPOSITORY_DIR)/Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_lcd.c \
$(REPOSITORY_DIR)/Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_sdram.c \
$(REPOSITORY_DIR)/Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_sd.c \
$(REPOSITORY_DIR)/Drivers/BSP/Components/otm8009a/otm8009a.c \

# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
Inc \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Inc \
$(REPOSITORY_DIR)/Drivers/STM32F7xx_HAL_Driver/Inc/Legacy \
$(REPOSITORY_DIR)/Drivers/CMSIS/Device/ST/STM32F7xx/Include \
$(REPOSITORY_DIR)/Drivers/CMSIS/Include \
$(REPOSITORY_DIR)/Drivers/CMSIS/Include \
$(REPOSITORY_DIR)/Drivers/BSP/STM32F769I-Discovery \
$(REPOSITORY_DIR)/Middlewares/Third_Party/FatFs/src \
$(EMU_PATH)/inc

CGBEMU_SOURCES = \
$(EMU_PATH)/src/alu.c \
$(EMU_PATH)/src/cartridge.c \
$(EMU_PATH)/src/dmgcpu.c \
$(EMU_PATH)/src/decoder.c \
$(EMU_PATH)/src/cgbmu.c \
$(EMU_PATH)/src/debug.c \

C_SOURCES +=$(CGBEMU_SOURCES) $(BSP_C_SOURCES)
#######################################
# binaries
#######################################
PREFIX =arm-none-eabi
CC = $(PREFIX)-gcc
AS = $(PREFIX)-gcc -x assembler-with-cpp
CP = $(PREFIX)-objcopy
SZ = $(PREFIX)-size
DP = $(PREFIX)-objdump

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
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32F769xx

# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(addprefix -I, $(C_INCLUDES)) $(OPT) -Wall -fdata-sections -ffunction-sections -Wno-unused-const-variable

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
#CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = startup/STM32F769NIHx_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

program: all
	openocd -f startup/STM32F769-DISCO.cfg -c "program $(BUILD_DIR)/$(TARGET).elf verify reset exit"

aslist: $(BUILD_DIR)/$(TARGET).elf
	@$(DP) -D $< > $(BUILD_DIR)/$(TARGET).lst
test:
	 @echo $(OBJECTS)

$(BUILD_DIR)/extbin.o : ../../../roms/mario.gb 
#$(BUILD_DIR)/extbin.o : ../../../roms/tests/cpu_instrs.gb
	$(CP) -I binary -O elf32-littlearm -B arm --rename-section .data=.extbinary $< $@
	$(DP) -t $@
#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/, $(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	@echo "[CC]  $<"
	@$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	@echo "Assemble " $<
	@$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile $(BUILD_DIR)/extbin.o
	@$(CC) $(OBJECTS) $(BUILD_DIR)/extbin.o $(LDFLAGS) -o $@
	@$(SZ) $@ -A

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	
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

# *** EOF ***
