TARGET     =bbgb
LIBEMBPATH =$(LIBEMB_PATH)
SRCSPATH  +=../../src ../../dmgcpu pff src $(LIBEMBPATH)/display $(LIBEMBPATH)/drv/tft
OBJSPATH   :=build

CSRCS      =startup.c blueboard.c ili9328.c spi.c main.c lcd.c display.c fonts.c debug.c
CSRCS     +=alu.c cartridge.c dmgcpu.c video.c decoder.c cgbmu.c
CSRCS     +=pff.c mmc.c

INCDIRS  =../../inc inc $(LIBEMBPATH)/include
SYMBOLS  =__BB__ NO_LCD_HW_SCROLL
#########################################################################
#  List of the objects files to be compiled/assembled
# system sources and common includes and directories
INCDIRS +=
OBJECTS =$(addprefix $(OBJSPATH)/, $(CSRCS:.c=.o)) $(addprefix $(OBJSPATH)/, $(ASRCS:.S=.o)) $(addprefix $(OBJSPATH)/, $(CPPSRCS:.cpp=.o))
LSCRIPT =linker.ld
#########################################################################
#  Compiler Options

INCLUDES = $(addprefix -D, $(SYMBOLS)) $(addprefix -I, $(INCDIRS)) -I.

GCFLAGS = -g -Wall -gdwarf-2 -mcpu=cortex-m3 -mthumb -ffunction-sections -fdata-sections -fno-stack-protector
GCFLAGS += $(INCLUDES)
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
CHECKSUM =$(LIBEMBPATH)/bsp/tools/checksum
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
	$(REMOVE) $(OBJSPATH) $(TARGET).hex $(TARGET).elf $(TARGET).map $(TARGET).bin
	
$(OBJSPATH):
	mkdir -p $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary -j .startup -j .text $(TARGET).elf $@
	hd -n 128 $@
	$(CHECKSUM) $@

$(TARGET).cfg:
	@echo "Creating opencod configuration file"
	@echo "interface jlink\ntransport select swd\nsource [find target/lpc17xx.cfg]\nadapter_khz 4000" > $(TARGET).cfg

program: $(TARGET).bin $(TARGET).cfg
	openocd -f $(TARGET).cfg -c "program $(TARGET).bin 0x00000000 verify reset exit"

test:
	@echo $(OBJECTS)
#########################################################################
#  Default rules to compile .c and .cpp file to .o
#  and assemble .s files to .o
#########################################################################
$(OBJSPATH)/%.o : %.c | $(OBJSPATH)
	@echo "-----" $< "---->" $@
	@$(GCC) $(GCFLAGS) -c $< -o $@

$(OBJSPATH)/%.o : %.S | $(OBJSPATH)
	@echo "-----" $< "---->" $@
	@$(AS) $(ASFLAGS) -o $(<:.S=.o) $< > $(<:.S=.lst)
#########################################################################
#
#########################################################################
#.PHONY: $(OBJSPATH)