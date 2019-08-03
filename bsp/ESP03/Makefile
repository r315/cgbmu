TARGET_OUT = cgbmu


#First, select whether you are targeting PICO or REGULAR.
#
#PICO Operates at 52 MHz, and makes no calls to ROM.
# It's ideal for the HackADay 1kB Challenge.
# You can also run PICO at 104 MHz but it takes a couple extra bytes to set the overclocking bits.
#
#REGULAR Operates at a variety of frequencies and allows for a number of
# ROM functions.

#SUBMODULE if we are including this in another project.
SUBMODULE?=NO

#BUILD?=PICO
BUILD    :=REGULAR
MAIN_MHZ ?=160 #Pick from *52, *80, 104 or *115, 160, *173, *189#, 231, 346, 378#  * = peripheral clock at processor clock. # = Mine won't boot + on ESP8285, Clock Lower and unreliable.  Warning. Peripheral clocks of >115 will NOT boot without a full power-down and up. (Don't know why)

LIBEMBPATH      =$(HOME)/Dropbox/Projects/software/libemb
ESP_OPEN_SDK    :=~/esp-open-sdk
GCC_FOLDER      :=$(ESP_OPEN_SDK)/xtensa-lx106-elf
ESPTOOL         :=esptool.py#$(ESP_OPEN_SDK)/esptool/esptool.py

PREFIX          :=xtensa-lx106-elf-
GCC             :=$(PREFIX)gcc
OBJDUMP         :=$(PREFIX)objdump
SIZE            :=$(PREFIX)size
OBJCOPY         :=$(PREFIX)objcopy

FW_1             =$(TARGET_OUT).elf-0x10000.bin
FW_0             =$(TARGET_OUT).elf-0x00000.bin

ESPTOOLOPTS     ?=-b 115200
ADDITIONAL_DEPS ?=
PORT:=/dev/ttyUSB0


LDFLAGS:=-T $(SRCPREFIX)startup/linkerscript.ld -T $(SRCPREFIX)startup/addresses.ld
FOLDERPREFIX:=$(GCC_FOLDER)/bin

INC =-I$(LIBEMBPATH)/include -I../../inc -Ispiffs/include
CFLAGS:=$(CFLAGS) -Os -g -std=c99 $(INC) -I$(SRCPREFIX)include -nostdlib -D__ESP03__ -D_ST7735_ -DMAIN_MHZ=$(MAIN_MHZ)  -mno-serialize-volatile -mlongcalls

SRCS =                         \
$(SRCPREFIX)startup/startup.S   \
$(SRCPREFIX)startup/nosdk8266.c \
$(LIBEMBPATH)/drv/tft/ili9341.c \
$(LIBEMBPATH)/drv/spi/hspi.c    \
$(LIBEMBPATH)/display/lcd.c     \
$(LIBEMBPATH)/display/display.c \
$(LIBEMBPATH)/display/fonts.c   \
../../dmgcpu/video.c           \
../../dmgcpu/dmgcpu.c          \
../../dmgcpu/decoder.c         \
../../dmgcpu/cartridge.c       \
../../dmgcpu/alu.c             \
../../src/cgbmu.c              \
../../src/debug.c              \
main.c                         \
#hw_timer.c                     \

all : $(TARGET_OUT).elf

$(TARGET_OUT).elf : $(ADDITIONAL_DEPS) #$(SRCS)
#@echo $(shell echo $(shell cat count.txt)+1) | bc > count.txt
	$(GCC) $(CFLAGS) $(SRCS) $(LDFLAGS) -o $@
#objdump -t $(TARGET_OUT) > image.map
#nm -S -n $(TARGET_OUT) > image.map
	$(SIZE) -A -x $@
	$(PREFIX)objdump -S $@ > $(TARGET_OUT).lst
#PATH=$(FOLDERPREFIX):$$PATH
	$(ESPTOOL) elf2image $(TARGET_OUT).elf 

burn : $(TARGET_OUT).elf
	$(ESPTOOL) --port $(PORT) write_flash 0x00000 $(FW_0) -ff 80m -fm qio
#sleep .1
#($(ESPTOOL) --port $(PORT) run)||(true)

clean :
	rm -rf $(TARGET_OUT).elf $(TARGET_OUT).map $(TARGET_OUT).lst $(FW_1) $(FW_0)

upload:
	$(ESPTOOL) --port $(PORT) write_flash 0x80000 $(FILE) -ff 80m -fm qio

