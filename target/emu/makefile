
TARGET   =cgbemu
BUILD_DIR :=build
TARGET_PATH =$(CURDIR)
EXEC =$(BUILD_DIR)/$(TARGET)
EMU_PATH  :=../..
LIB_PATH  :=$(EMU_PATH)/lib
ROMS_PATH :=$(EMU_PATH)/roms

C_INCLUDES = \
$(EMU_PATH)/inc \
$(TARGET_PATH)/inc \
$(LIB_PATH)/inc \

CSRCS = \
$(TARGET_PATH)/src/disassembler.c \
$(TARGET_PATH)/src/tests.c \
$(TARGET_PATH)/src/testdata.c \
$(TARGET_PATH)/src/main.c \
$(TARGET_PATH)/src/readline.c \
$(TARGET_PATH)/src/lcdsdl2.c \
$(LIB_PATH)/src/liblcd.c \
$(LIB_PATH)/src/lib2d.c \
$(LIB_PATH)/src/font.c \
$(EMU_PATH)/src/debug.c \
$(EMU_PATH)/src/decoder.c \
$(EMU_PATH)/src/instrs.c \
$(EMU_PATH)/src/dmgcpu.c \
$(EMU_PATH)/src/cartridge.c \
$(EMU_PATH)/src/video.c \
$(EMU_PATH)/src/cgbmu.c

LIBS     =-lSDL2main -lSDL2 -lGL
SYMBOLS  =-D__DEBUG__ -D__EMU__ -D$(ROMS_PATH)
CFLAGS  =-Og -g -Wall -gdwarf-2 -gstrict-dwarf  $(addprefix -I, $(C_INCLUDES)) $(SYMBOLS) $(shell sdl2-config --cflags)
LDFLAGS =-LSDL2/lib/x64
OBJS    =$(addprefix $(BUILD_DIR)/, $(notdir $(CSRCS:.c=.o)))

CC      =arm-linux-gnueabihf-gcc
GCC     =gcc 
RM      =rm -f

VPATH   +=$(sort $(dir $(CSRCS)))

all: $(EXEC) 
	
clean: 
	@${RM} $(BUILD_DIR)/*.o


$(EXEC): $(OBJS)
	@$(GCC) $(OBJS) -o $@ $(shell sdl2-config --libs) -lGL -lpthread
	
$(BUILD_DIR)/%.o : %.c | $(BUILD_DIR)
	@echo "[GCC]  $@"
	@$(GCC) $(CFLAGS) -c $< -o $@
	

$(BUILD_DIR):
	mkdir -p $@

