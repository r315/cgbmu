BUILD_DIR  	=$(CURDIR)/build
EMU_PATH   	=$(CURDIR)
ROMS_PATH   =$(CURDIR)/roms

EMU_PARAM	=EMU_PATH=$(EMU_PATH) ROMS_PATH=$(ROMS_PATH)
TARGET_EMU_PARAM =-C target/emu BUILD_DIR=$(BUILD_DIR)/emu
TARGET_BB_PARAM =-C target/Blueboard BUILD_DIR=$(BUILD_DIR)/blueboard
TARGET_ARTERY_PARAM =-C target/artery BUILD_DIR=$(BUILD_DIR)/artery
TARGET_DISCO_PARAM =-C target/Discovery BUILD_DIR=$(BUILD_DIR)/discovery

all: bb disco artery

clean: 
	@${RM} -Rf $(BUILD_DIR)
	
emu:
	@"$(MAKE)" $(TARGET_EMU_PARAM) $(EMU_PARAM)

bb:
	@"$(MAKE)" $(TARGET_BB_PARAM) $(EMU_PARAM)
bb-program: bb
	@"$(MAKE)" $(TARGET_BB_PARAM) $(EMU_PARAM) program
bb-rebuild-rom:
	@"$(MAKE)" $(TARGET_BB_PARAM) clean-rom
	@"$(MAKE)" bb

disco: 
	"$(MAKE)" $(TARGET_DISCO_PARAM) $(EMU_PARAM)
disco-program: disco
	@"$(MAKE)" $(TARGET_DISCO_PARAM) $(EMU_PARAM) program
disco-rebuild-rom:
	@"$(MAKE)" $(TARGET_DISCO_PARAM) clean-rom
	@"$(MAKE)" disco-program

artery: 
	@"$(MAKE)" $(TARGET_ARTERY_PARAM) $(EMU_PARAM)
artery-program: artery
	@"$(MAKE)" $(TARGET_ARTERY_PARAM) $(EMU_PARAM) program
artery-rebuild-rom:
	@"$(MAKE)" $(TARGET_ARTERY_PARAM) clean-rom
	@"$(MAKE)" artery