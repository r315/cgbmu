
TARGET   =gbemu
SRCSPATH =. ../../src ../../dmgcpu src 
OBJSPATH =.
INCSPATH =. ../../inc inc

CSRCS    =debug.c disassembler.c tests.c lcdsdl2.c io.c main.c readline.c lcd.c display.c fonts.c
CSRCS   +=decoder.c alu.c dmgcpu.c cartridge.c video.c
LIBS     =-lSDL2main -lSDL2
SYMBOLS  =-D__DEBUG__ -D__EMU__ -DNO_STDIO_REDIRECT

GCC     =gcc 
RM      =rm -f
CFLAGS  =-Og -g -Wall -gdwarf-2 -gstrict-dwarf $(addprefix -I, $(INCSPATH)) $(SYMBOLS)
OBJS    =$(addprefix $(OBJSPATH)/, $(CSRCS:.c=.o))
VPATH   +=$(SRCSPATH)

all: $(TARGET) 
	
clean: 
	@${RM} $(OBJS) $(TARGET)


$(TARGET): $(OBJS)
	@$(GCC) $(OBJS) $(LIBS) -o $(TARGET)
	@./$(TARGET)
	
$(OBJSPATH)/%.o : %.c
	@echo "-----" $< "---->" $@
	@$(GCC) $(CFLAGS) -c $< -o $@



