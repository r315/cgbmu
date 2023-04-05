TARGET 		=$(BUILD_DIR)/bb-cgbmu

BUILD_DIR   :=build
BOARD_PATH 	=$(CURDIR)
EMU_PATH 	:=../..
LIB_PATH    :=$(EMU_PATH)/lib
DRIVERS_COMPONENT_PATH =../Drivers/Component
CMSIS_PATH  =../Drivers/CMSIS

CSRCS = \
$(CMSIS_PATH)/Device/NXP/LPC17xx/src/startup_LPC17xx.c \
$(CMSIS_PATH)/Device/NXP/LPC17xx/src/system_LPC17xx.c \
$(BOARD_PATH)/board_blueboard.c \
$(BOARD_PATH)/main_blueboard.c \
$(DRIVERS_COMPONENT_PATH)/src/ili9328.c \
$(LIB_PATH)/src/liblcd.c \
$(LIB_PATH)/src/font.c \
$(LIB_PATH)/src/libbutton.c \
$(LIB_PATH)/src/lib2d.c \
$(EMU_PATH)/src/instrs.c \
$(EMU_PATH)/src/cartridge.c \
$(EMU_PATH)/src/dmgcpu.c \
$(EMU_PATH)/src/video.c \
$(EMU_PATH)/src/decoder.c \
$(EMU_PATH)/src/cgbmu.c \

#$(DRIVERS_PATH)/drv/spi/spi_lpc17xx.c \
$(BSP_PATH)/src/spi.c \
$(FATFS_PATH)/pff.c \
$(FATFS_PATH)/mmc.c \

ROM_FILE=$(ROMS_PATH)/tests/cpu_instrs.gb

C_INCLUDES  = \
$(BOARD_PATH) \
$(BOARD_PATH)/Drivers/Costum/inc \
$(CMSIS_PATH)/Device/NXP/LPC17xx/inc \
$(CMSIS_PATH)/Core/Include \
$(LIB_PATH)/inc \
$(EMU_PATH)/inc \
$(DRIVERS_COMPONENT_PATH)/inc/

C_DEFS  =__BB__ NO_LCD_HW_SCROLL SCREEN_OFFSET_X=80 SCREEN_OFFSET_Y=48 USE_FAST_CODE
#########################################################################
#  List of the objects files to be compiled/assembled
# system sources and common includes and directories
ROM_OBJ=$(BUILD_DIR)/rom.o
OBJECTS = $(addprefix $(BUILD_DIR)/, $(notdir $(CSRCS:.c=.o)))
#$(addprefix $(BUILD_DIR)/, $(CPPSRCS:.cpp=.o)) 
LSCRIPT =$(BOARD_PATH)/lpc1768_gb.ld

#########################################################################
#  Compiler Options
DEVICE =LPC1768

CPU =-mcpu=cortex-m3 -mthumb

MCU =$(CPU)

OPT = -Og -g -Wall -gdwarf-2 

ifdef SEMIHOSTING
SPECS =--specs=rdimon.specs
LDLIBS =-nostartfiles -lc -lrdimon
else
SPECS =--specs=nano.specs
#-nostdlib -lstd++ -lnosys -lm 
LDLIBS =
endif

ifndef V
VERBOSE =@
else
VERBOSE =
endif

GCFLAGS =$(MCU) $(OPT) $(addprefix -D, $(C_DEFS)) $(addprefix -I, $(C_INCLUDES)) -ffunction-sections -fdata-sections -std=gnu11 #-fno-stack-protector
LDFLAGS =$(MCU) $(SPECS) -Wl,-Map=$(TARGET).map,--cref,--gc-sections #-nostdlib
ASFLAGS =$(LISTING) -mcpu=cortex-m3 -mthumb

VPATH +=$(sort $(dir $(CSRCS)))
#########################################################################
#  Compiler/Assembler/Linker Paths

GCC_EXEC_PREFIX =arm-none-eabi-
GCC = $(GCC_EXEC_PREFIX)gcc
GPP = $(GCC_EXEC_PREFIX)g++
AS = $(GCC_EXEC_PREFIX)as
LD = $(GCC_EXEC_PREFIX)gcc
SIZE = $(GCC_EXEC_PREFIX)size
OBJCOPY = $(GCC_EXEC_PREFIX)objcopy
OBJDUMP = $(GCC_EXEC_PREFIX)objdump
REMOVE = rm -fR


ifeq ($(shell uname -s), Linux)
JLK ="/opt/SEGGER/JLink/JLinkExe"
else
CHECKSUM ="C:\Tools\bin\checksum"
JLK =$(JLINK)/jlink
endif
#########################################################################

all: $(TARGET).elf size

$(TARGET).elf: $(OBJECTS) $(ROM_OBJ)
	@echo "[LD]  $@"
	$(VERBOSE)$(LD) $(LDFLAGS) -T $(LSCRIPT) $^ -o $@

$(TARGET).bin: $(TARGET).elf 
	$(OBJCOPY) -O binary $(TARGET).elf $@
#hd -n 128 $@
	@echo "[CK] $@"
	@$(CHECKSUM) $@

#https://balau82.wordpress.com/2012/02/19/linking-a-binary-blob-with-gcc
$(ROM_OBJ): $(ROM_FILE) 
	$(OBJCOPY) -I binary -O elf32-littlearm --rename-section .data=.rodata --add-symbol _binary_rom_start=.rodata:0 $< $@
	@$(OBJDUMP) -t $@
	
$(TARGET)_.cfg:
	@echo "Creating configuration file for st-link probe"
	@echo "adapter driver st-link" > $@
	@echo "transport select dapdirect_swd" >> $@
	@echo "st-link vid_pid 0x0483 0x3748" >> $@
	@echo "adapter speed 4000" >> $@
	@echo "source [find board/mcb1700.cfg]" >> $@

$(TARGET).cfg: #openocd with jlink probe
	@echo "Creating configuration file for jlink probe"
	@echo "# windows driver must be libusb" > $@
	@echo "adapter driver jlink" >> $@
	@echo "transport select "swd"" >> $@
	@echo "source [find board/mcb1700.cfg]" >> $@
	
$(TARGET).jlink: $(TARGET).bin
	@echo "Creating Jlink configuration file"
	@echo "loadfile $< 0x00000000" >> $@
	@echo "r" >> $@
	@echo "q" >> $@

list: $(TARGET).elf
	@echo "---- Listing ----"
	@$(OBJDUMP) -S $(TARGET).elf > $(TARGET).lst

size: $(TARGET).elf
	@echo "---- Size ----"
	@$(SIZE) -A -x $<

clean:
	$(REMOVE) $(BUILD_DIR) 

clean-rom:
	$(VERBOSE)-rm -f $(ROM_OBJ)

program: $(TARGET).bin $(TARGET).cfg
	openocd -f $(TARGET).cfg -c "program $< 0x00000000 verify reset exit"

_program: $(TARGET).jlink $(TARGET).bin
	$(VERBOSE)$(JLK) -device $(DEVICE) -if SWD -speed auto -CommanderScript $<

$(BUILD_DIR):
	mkdir -p $@
#########################################################################
#  Default rules to compile .c and .cpp file to .o
#  and assemble .s files to .o
#########################################################################
$(BUILD_DIR)/%.o : %.c | $(BUILD_DIR)
	@echo "[CC]  $< "
	$(VERBOSE)$(GCC) $(GCFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o : %.S | $(BUILD_DIR)
	@echo "[AS]  $< "
	$(VERBOSE)$(AS) $(ASFLAGS) -o $(<:.S=.o) $< > $(<:.S=.lst)
#########################################################################
#
#########################################################################
#.PHONY: $(BUILD_DIR)
