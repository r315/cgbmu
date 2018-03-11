#/*Copyright (C) 2011 by Hugo MR
#
#Permission is hereby granted, free of charge, to any person obtaining a copy
#of this software and associated documentation files (the "Software"), to deal
#in the Software without restriction, including without limitation the rights
#to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#copies of the Software, and to permit persons to whom the Software is
#furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in
#all copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#THE SOFTWARE.*/

#update modificado macro GCC_EXEC_PREFIX = arm-none-eabi
#03-09-2011 limpeza
#########################################################################
#  Project Name
PROJECT = gameboy
GNUCORE  = core

CSRCS  = startup.c blueboard.c ili9328.c spi.c main.c display.c fonts.c debug.c io.c #mystdlib.c
CSRCS += alu.c cartridge.c dmgcpu.c video.c decoder.c
CSRCS += pff.c mmc.c

INCDIRS  = . ../inc
DEFS = __NEWLIB__ __thumb2__
#MULTIPLE_FONTS

#########################################################################
#  List of the objects files to be compiled/assembled
# system sources and common includes and directories

INCDIRS += $(wildcard $(GNUCORE)/)

OBJECTS = $(CSRCS:.c=.o) $(ASRCS:.S=.o) $(CPPSRCS:.cpp=.o)
LSCRIPT = linker.ld
#########################################################################
#  Compiler Options

INCLUDES = $(patsubst %,-D%,$(DEFS)) $(patsubst %,-I%,$(INCDIRS)) -I.

GCFLAGS = -g -Wall -gdwarf-2 -mcpu=cortex-m3 -mthumb -mthumb-interwork -ffunction-sections -fdata-sections -Wnested-externs -fno-stack-protector
GCFLAGS += $(INCLUDES)
#linux
LDFLAGS = -mcpu=cortex-m3 -mthumb -O$(OPTIMIZATION) -Wl,-Map=$(PROJECT).map,--cref,--gc-sections #-nostdlib
#normal
#LDFLAGS = -mcpu=cortex-m3 -mthumb -O$(OPTIMIZATION) -Wl,-Map=$(PROJECT).map,--cref,--gc-sections -lc -lm -lgcc -lstdc++
ASFLAGS = $(LISTING) -mcpu=cortex-m3 -mthumb

#########################################################################
#  Compiler/Assembler/Linker Paths

VPATH +=../src pff

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
GCC_EXEC_PREFIX = arm-linux-gnueabi
else
GCC_EXEC_PREFIX = arm-none-eabi
endif


GCC = $(GCC_EXEC_PREFIX)-gcc
GPP = $(GCC_EXEC_PREFIX)-g++
AS = $(GCC_EXEC_PREFIX)-as
LD = $(GCC_EXEC_PREFIX)-gcc
SIZE = $(GCC_EXEC_PREFIX)-size
OBJCOPY = $(GCC_EXEC_PREFIX)-objcopy
OBJDUMP = $(GCC_EXEC_PREFIX)-objdump
REMOVE = rm -f
#########################################################################

all: firmware.bin stats list

rebuild: clean all

list: $(PROJECT).elf
	@$(OBJDUMP) -S $(PROJECT).elf > $(PROJECT).lst

firmware.bin: $(PROJECT).elf
	@$(OBJCOPY) -O binary -j .text -j .data $(PROJECT).elf firmware.bin

$(PROJECT).elf: $(OBJECTS)
	@echo "---- Linking ----"
	@$(LD) $(LDFLAGS) $(LDOBJECTS) -T $(LSCRIPT) $(OBJECTS) -o $(PROJECT).elf

stats: $(PROJECT).elf
	@$(SIZE) -A -x $<

clean:
	$(REMOVE) $(OBJECTS)
	$(REMOVE) $(PROJECT).hex $(PROJECT).elf $(PROJECT).map $(PROJECT).bin
	

	
	
#########################################################################
#  Default rules to compile .c and .cpp file to .o
#  and assemble .s files to .o
#########################################################################
%.o : %.c
	@echo "-----" $< "---->" $@
	@$(GCC) $(GCFLAGS) -c $< -o  $(notdir $(<:.c=.o))

.cpp.o :
	@echo "-----" $< "---->" $@
	@$(GPP) $(GPFLAGS) -c $< -o $(<:.cpp=.o)

.S.o :
	@echo "-----" $< "---->" $@
	@$(AS) $(ASFLAGS) -o $(<:.S=.o) $< > $(<:.S=.lst)
#########################################################################
#
#########################################################################
