
TARGET   =gbemu
SRCSPATH =. ../../src ../../dmgcpu ../../libemb/drv/tft ../../libemb/display src 
OBJSPATH =.
INCSPATH =. ../../inc inc

CSRCS    =debug.c disassembler.c tests.c testdata.c io.c main.c readline.c lcd.c display.c fonts.c
CSRCS   +=decoder.c alu.c dmgcpu.c cartridge.c video.c
LIBS     =-lSDL2main -lSDL2
SYMBOLS  =-D__DEBUG__ -D__EMU__ -D__LCDFB__

CC      =arm-linux-gnueabihf-gcc
GCC     =gcc 
RM      =rm -f
CFLAGS  =-Og -g -Wall -gdwarf-2 -gstrict-dwarf $(addprefix -I, $(INCSPATH)) $(SYMBOLS)
OBJS    =$(addprefix $(OBJSPATH)/, $(CSRCS:.c=.o))
VPATH   +=$(SRCSPATH)

all: $(TARGET) 
	
clean: 
	@${RM} $(TARGET) $(OBJSPATH)/*.o


$(TARGET): $(OBJS) $(OBJSPATH)/lcdsdl2.o
	$(GCC) $(OBJS) $(OBJSPATH)/lcdsdl2.o $(LIBS) -o $(TARGET)
#@./$(TARGET)
	
$(OBJSPATH)/%.o : %.c
	@echo "-----" $< "---->" $@
	@$(GCC) $(CFLAGS) -c $< -o $@
	
rpi: $(OBJS) $(OBJSPATH)/lcdfb.o 
	$(CC) $(OBJS) $(OBJSPATH)/lcdfb.o -lSDLmain -lSDL -o $(TARGET)
	



