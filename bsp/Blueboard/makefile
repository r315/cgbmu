TARGET =$(BUILD_DIR)/bbcgbemu

ifeq ($(shell uname), Linux)
LIBEMB_PATH =$(HOME)/Dropbox/Projects/software/libemb
EXTBIN_ARCH =-B armv7 # confirm this for linux
else
EXTBIN_ARCH =-B arm
endif

SRCSPATH  +=../../src ../../dmgcpu pff src $(LIBEMB_PATH)/display $(LIBEMB_PATH)/drv/tft
BUILD_DIR   :=build

CSRCS      =startup.c board.c ili9328.c spi.c main.c lcd.c display.c fonts.c debug.c
CSRCS     +=pff.c mmc.c
CSRCS     +=alu.c cartridge.c dmgcpu.c video.c decoder.c cgbmu.c

INCDIRS  =../../inc inc $(LIBEMB_PATH)/include
SYMBOLS  =-D__BB__ -DNO_LCD_HW_SCROLL -DSCREEN_OFFSET_X=80 -DSCREEN_OFFSET_Y=48 -DUSE_FAST_CODE
#########################################################################
#  List of the objects files to be compiled/assembled
# system sources and common includes and directories
INCDIRS +=
OBJECTS = \
$(addprefix $(BUILD_DIR)/, $(CSRCS:.c=.o)) \
$(addprefix $(BUILD_DIR)/, $(ASRCS:.S=.o)) \
$(addprefix $(BUILD_DIR)/, $(CPPSRCS:.cpp=.o)) \
$(BUILD_DIR)/extbin.eo

LSCRIPT =cgbemu_linker.ld
#########################################################################
#  Compiler Options

INCLUDES =$(addprefix -I, $(INCDIRS)) -I.

GCFLAGS = -Og -g -Wall -gdwarf-2 -mcpu=cortex-m3 -mthumb -ffunction-sections -fdata-sections -fno-stack-protector
GCFLAGS +=$(SYMBOLS) $(INCLUDES)
LDFLAGS = -mcpu=cortex-m3 -mthumb -Wl,-Map=$(TARGET).map,--cref,--gc-sections #-nostdlib

ASFLAGS = $(LISTING) -mcpu=cortex-m3 -mthumb

#########################################################################
#  Compiler/Assembler/Linker Paths

VPATH +=$(SRCSPATH)

GCC_EXEC_PREFIX = arm-none-eabi

GCC = $(GCC_EXEC_PREFIX)-gcc
GPP = $(GCC_EXEC_PREFIX)-g++
AS = $(GCC_EXEC_PREFIX)-as
LD = $(GCC_EXEC_PREFIX)-gcc
SIZE = $(GCC_EXEC_PREFIX)-size
OBJCOPY = $(GCC_EXEC_PREFIX)-objcopy
OBJDUMP = $(GCC_EXEC_PREFIX)-objdump
REMOVE = rm -fR
CHECKSUM =$(LIBEMB_PATH)/bsp/tools/checksum
#########################################################################

all: $(TARGET).elf stats

$(TARGET).elf: $(OBJECTS)
	@echo "---- Linking ----> $@"
	@$(LD) $(LDFLAGS) $(LDOBJECTS) -T $(LSCRIPT) $(OBJECTS) -o $(TARGET).elf

rebuild: clean all

list: $(TARGET).elf
	@echo "---- Listing ----"
	@$(OBJDUMP) -S $(TARGET).elf > $(TARGET).lst

firmware.bin: $(TARGET).elf
	@$(OBJCOPY) -O binary -j .text -j .data $(TARGET).elf firmware.bin

stats: $(TARGET).elf
	@echo "---- Size ----"
	@$(SIZE) -A -x $<

clean:
	$(REMOVE) $(BUILD_DIR) $(TARGET).hex $(TARGET).elf $(TARGET).map $(TARGET).bin
	
$(BUILD_DIR):
	mkdir -p $@

$(TARGET).bin: $(TARGET).elf 
	$(OBJCOPY) -O binary -j .startup -j .text -j .data -j .extbinary $(TARGET).elf $@
	hd -n 128 $@
	$(CHECKSUM) $@

$(TARGET).cfg:
	@echo "Generating opencod configuration file"
	@echo 'interface jlink\ntransport select swd\nsource [find target/lpc17xx.cfg]\nadapter_khz 4000' > $(TARGET).cfg

program: $(CHECKSUM) $(TARGET).bin $(TARGET).cfg
	openocd -f $(TARGET).cfg -c "program $(TARGET).bin 0x00000000 verify reset exit"
	
#https://balau82.wordpress.com/2012/02/19/linking-a-binary-blob-with-gcc
$(BUILD_DIR)/extbin.eo : ../../../roms/mario.gb #tests/cpu_instrs.gb
	$(OBJCOPY) -I binary -O elf32-littlearm $(EXTBIN_ARCH) --rename-section .data=.extbinary $< $@
	$(OBJDUMP) -t $@
	
$(CHECKSUM):
	gcc $@.c -o $@
	
test:
	@echo $(OBJECTS)
#########################################################################
#  Default rules to compile .c and .cpp file to .o
#  and assemble .s files to .o
#########################################################################
$(BUILD_DIR)/%.o : %.c | $(BUILD_DIR)
	@echo "-----" $< "---->" $@
	@$(GCC) $(GCFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o : %.S | $(BUILD_DIR)
	@echo "-----" $< "---->" $@
	@$(AS) $(ASFLAGS) -o $(<:.S=.o) $< > $(<:.S=.lst)
#########################################################################
#
#########################################################################
#.PHONY: $(BUILD_DIR)
