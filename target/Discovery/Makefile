######################################
# target
######################################
TARGET =disco-cgbmu

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
BUILD_DIR  :=build
EMU_PATH   :=../..
ROMS_PATH  :=$(EMU_PATH)/roms
DRIVERS_PATH =$(CURDIR)/../Drivers
DRIVERS_HAL_PATH =$(DRIVERS_PATH)/STM32F7xx_HAL_Driver
DRIVERS_COMPONENT_PATH =$(DRIVERS_PATH)/Component
DRIVERS_BSP_PATH =$(DRIVERS_PATH)/BSP
LIB_PATH   :=$(EMU_PATH)/lib
######################################
# source
######################################
C_SOURCES =  \
Src/main_discovery.c \
Src/board_discovery.c \
Src/fifo.c \
Src/serial.c \
Src/it.c \
Src/hal_msp.c \
Src/syscalls.c \
Src/system_stm32f7xx.c \
$(DRIVERS_PATH)/Costum/src/i2c_disco.c \
$(DRIVERS_COMPONENT_PATH)/src/pcf8574.c \
$(DRIVERS_HAL_PATH)/Src/stm32f7xx_hal.c \
$(DRIVERS_HAL_PATH)/Src/stm32f7xx_hal_cortex.c \
$(DRIVERS_HAL_PATH)/Src/stm32f7xx_hal_dma2d.c \
$(DRIVERS_HAL_PATH)/Src/stm32f7xx_hal_dsi.c \
$(DRIVERS_HAL_PATH)/Src/stm32f7xx_hal_sdram.c \
$(DRIVERS_HAL_PATH)/Src/stm32f7xx_hal_ltdc.c \
$(DRIVERS_HAL_PATH)/Src/stm32f7xx_hal_ltdc_ex.c \
$(DRIVERS_HAL_PATH)/Src/stm32f7xx_hal_tim.c \
$(DRIVERS_HAL_PATH)/Src/stm32f7xx_hal_tim_ex.c \
$(DRIVERS_HAL_PATH)/Src/stm32f7xx_hal_rcc.c \
$(DRIVERS_HAL_PATH)/Src/stm32f7xx_hal_rcc_ex.c \
$(DRIVERS_HAL_PATH)/Src/stm32f7xx_hal_gpio.c \
$(DRIVERS_HAL_PATH)/Src/stm32f7xx_hal_dma.c \
$(DRIVERS_HAL_PATH)/Src/stm32f7xx_hal_dma_ex.c \
$(DRIVERS_HAL_PATH)/Src/stm32f7xx_hal_pwr.c \
$(DRIVERS_HAL_PATH)/Src/stm32f7xx_hal_pwr_ex.c \
$(DRIVERS_HAL_PATH)/Src/stm32f7xx_hal_i2c.c \
$(DRIVERS_HAL_PATH)/Src/stm32f7xx_hal_i2c_ex.c \
$(DRIVERS_HAL_PATH)/Src/stm32f7xx_hal_uart.c \
$(DRIVERS_HAL_PATH)/Src/stm32f7xx_ll_fmc.c \

#$(DRIVERS_HAL_PATH)/Src/stm32f7xx_hal_qspi.c \
$(DRIVERS_HAL_PATH)/Src/stm32f7xx_hal_flash.c \
$(DRIVERS_HAL_PATH)/Src/stm32f7xx_hal_flash_ex.c \
$(DRIVERS_HAL_PATH)/Src/stm32f7xx_hal_exti.c \
$(DRIVERS_HAL_PATH)/Src/stm32f7xx_hal_sd.c \
$(DRIVERS_HAL_PATH)/Src//stm32f7xx_ll_sdmmc.c \
#$(REPOSITORY_DIR)/Middlewares/Third_Party/FatFs/src/diskio.c \
$(REPOSITORY_DIR)/Middlewares/Third_Party/FatFs/src/ff.c \
$(REPOSITORY_DIR)/Middlewares/Third_Party/FatFs/src/ff_gen_drv.c \
Src/sd_diskio.c \
Src/fatfs.c \
Src/syscalls.c \

# ASM sources
ASM_SOURCES =  \
startup/startup_stm32f769xx.s

BSP_C_SOURCES = \
$(DRIVERS_BSP_PATH)/Components/otm8009a/otm8009a.c \
$(DRIVERS_BSP_PATH)/STM32F769I-Discovery/stm32f769i_discovery.c \
$(DRIVERS_BSP_PATH)/STM32F769I-Discovery/stm32f769i_discovery_lcd.c \
$(DRIVERS_BSP_PATH)/STM32F769I-Discovery/stm32f769i_discovery_sdram.c \
#$(DRIVERS_BSP_PATH)/STM32F769I-Discovery/stm32f769i_discovery_sd.c \

CGBEMU_SOURCES = \
$(EMU_PATH)/src/instrs.c \
$(EMU_PATH)/src/cartridge.c \
$(EMU_PATH)/src/dmgcpu.c \
$(EMU_PATH)/src/decoder.c \
$(EMU_PATH)/src/cgbmu.c \
$(EMU_PATH)/src/video.c \

ROM_FILE=$(ROMS_PATH)/cpu_instrs.gb

# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
Inc \
$(DRIVERS_HAL_PATH)/Inc \
$(DRIVERS_HAL_PATH)/Inc/Legacy \
$(DRIVERS_PATH)/CMSIS/Device/ST/STM32F7xx/Include \
$(DRIVERS_PATH)/CMSIS/Core/Include \
$(DRIVERS_BSP_PATH)/STM32F769I-Discovery \
$(DRIVERS_COMPONENT_PATH)/inc \
$(EMU_PATH)/inc \
$(LIB_PATH)/inc \
#$(REPOSITORY_DIR)/Middlewares/Third_Party/FatFs/src \

LDSCRIPT = startup/STM32F769NIHx_FLASH.ld

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
CPU =-mcpu=cortex-m7 -mthumb
# fpu
FPU =-mfpu=fpv5-d16
# float-abi
FLOAT-ABI =-mfloat-abi=hard
# mcu
MCU = $(CPU) $(FPU) $(FLOAT-ABI)
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
SPECS =-specs=nano.specs
LIBS =-lc -lm #-lnosys
MAP =-Wl,-Map=$(BUILD_DIR)/$(TARGET).map
LDFLAGS = $(MCU) $(SPECS) -T$(LDSCRIPT) $(LIBS) -Wl,--gc-sections $(MAP)

#######################################
# Objects to be generated
#######################################
ROM_OBJ=$(BUILD_DIR)/rom.o

OBJECTS = $(addprefix $(BUILD_DIR)/, $(notdir $(C_SOURCES:.c=.o))) $(ROM_OBJ)
vpath %.c $(sort $(dir $(C_SOURCES)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

#######################################
# Rules
#######################################
# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf 
#$(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

program: all
	openocd -f startup/STM32F769-DISCO.cfg -c "program $(BUILD_DIR)/$(TARGET).elf verify reset exit"

aslist: $(BUILD_DIR)/$(TARGET).elf
	@$(DP) -D $< > $(BUILD_DIR)/$(TARGET).lst
test:
	 @echo $(OBJECTS)

$(ROM_OBJ): $(ROM_FILE)
	$(CP) -I binary -O elf32-littlearm -B arm --rename-section .data=.rodata --add-symbol _binary_rom_start=.rodata:0 $< $@
	$(DP) -t $@

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@echo "[CC]  $<"
	@$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s | $(BUILD_DIR)
	@echo "[AS]  $<"
	@$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) $(ROM_OBJ)
	@echo "[LD]  $@"
	@$(CC) $^ $(LDFLAGS) -o $@
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

clean-rom:
	-rm -f $(ROM_OBJ)
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***
