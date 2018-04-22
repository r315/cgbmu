TARGET_OUT = image.elf
all : $(TARGET_OUT)

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
BUILD:=REGULAR
MAIN_MHZ?=320 #Pick from *52, *80, 104 or *115, 160, *173, *189#, 231, 346, 378#  * = peripheral clock at processor clock. # = Mine won't boot + on ESP8285, Clock Lower and unreliable.  Warning. Peripheral clocks of >115 will NOT boot without a full power-down and up. (Don't know why)
USE_I2S?=YES
USE_PRINT:=YES

ESP_OPEN_SDK:=~/esp-open-sdk
GCC_FOLDER:=$(ESP_OPEN_SDK)/xtensa-lx106-elf
ESPTOOL:=esptool.py#$(ESP_OPEN_SDK)/esptool/esptool.py
PREFIX:=xtensa-lx106-elf-
GCC:=$(PREFIX)gcc
OBJDUMP:=$(PREFIX)objdump
SIZE:=$(PREFIX)size
OBJCOPY:=$(PREFIX)objcopy

FW_1 = $(TARGET_OUT)-0x10000.bin
FW_2 = $(TARGET_OUT)-0x00000.bin
ESPTOOLOPTS?=-b 115200
ADDITIONAL_DEPS?=
PORT:=/dev/ttyUSB0


LDFLAGS:=-T $(SRCPREFIX)ld/linkerscript.ld -T $(SRCPREFIX)ld/addresses.ld
FOLDERPREFIX:=$(GCC_FOLDER)/bin


ifeq (YES, $(USE_PRINT))
	CFLAGS:=$(CFLAGS)
else
	CFLAGS:=$(CFLAGS) -DPICONOPRINT
endif


INC =-I../../libemb/include    \
-I../../inc
#Adding the -g flag makes our assembly easier to read and does not increase size of final executable.
CFLAGS:=$(CFLAGS) -Os $(INC) -I$(SRCPREFIX)include -nostdlib -D__ESP03__ -DMAIN_MHZ=$(MAIN_MHZ)  -mno-serialize-volatile -mlongcalls -g

SRCS:=$(SRCS)                  \
$(SRCPREFIX)src/startup.S      \
$(SRCPREFIX)src/nosdk8266.c    \
../../libemb/drv/tft/ili9341.c \
../../libemb/drv/spi/hspi.c    \
../../libemb/display/lcd.c     \
../../libemb/display/display.c \
../../libemb/display/fonts.c   \
../../src/main.c               \
../../src/io.c                 \
../../dmgcpu/video.c           \
../../dmgcpu/dmgcpu.c          \
../../dmgcpu/decoder.c         \
../../dmgcpu/cartridge.c       \
../../dmgcpu/alu.c             \
 

$(TARGET_OUT) : $(ADDITIONAL_DEPS) $(SRCS)
#@echo $(shell echo $(shell cat count.txt)+1) | bc > count.txt
	$(GCC) $(CFLAGS) $(SRCS) $(LDFLAGS) -o $@
#objdump -t $(TARGET_OUT) > image.map
#nm -S -n $(TARGET_OUT) > image.map
	$(SIZE) -A $@
	$(PREFIX)objdump -S $@ > image.lst
#PATH=$(FOLDERPREFIX):$$PATH
	$(ESPTOOL) elf2image $(TARGET_OUT) 

burn : $(FW_FILE_1) $(FW_FILE_2) $(TARGET_OUT)
	($(ESPTOOL) --port $(PORT) write_flash 0x00000 image.elf-0x00000.bin -ff 80m -fm dout)||(true)
#sleep .1
#($(ESPTOOL) --port $(PORT) run)||(true)

clean :
	rm -rf $(TARGET_OUT) image.map image.lst $(FW_1) $(FW_2)

