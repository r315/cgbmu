
CC   = gcc -D__DEBUG__ -D__EMU__ -DNO_STDIO_REDIRECT -O0 -Wall -gdwarf-2 -gstrict-dwarf

CSRCPATH = ../src .
VPATH += $(CSRCPATH)
BIN  = gbemu
OBJ  =lcdsdl2.o debug.o decoder.o alu.o dmgcpu.o main.o cartridge.o video.o graphics.o io.o tests.o readline.o disassembler.o
LINKOBJ  = $(OBJ) $(RES)
LIBS =  -lSDL2main -lSDL2 -g3 
INCS =  -I"." -I"../inc"

CFLAGS = $(INCS) -g3
RM = rm -f

all: $(BIN)
#./$(BIN)
	
clean: 
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CC) $(LINKOBJ) -o $(BIN) $(LIBS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

.PHONY: all all-before all-after clean clean-custom
