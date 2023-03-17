BUILD_DIR  =$(PWD)/build
EMU_PATH   =$(PWD)

all: emu bb disco disco

clean: 
	@${RM} -Rf $(BUILD_DIR)
#@$(MAKE) -C bsp/emu clean
#@$(MAKE) -C bsp/Blueboard clean
#@$(MAKE) -C bsp/Discovery clean
	
emu:
	@"$(MAKE)" -C target/emu BUILD_DIR=$(BUILD_DIR)/emu EMU_PATH=$(EMU_PATH)

rpi: 
	@"$(MAKE)" -C target/emu BUILD_DIR=$(BUILD_DIR)/rpi rpi EMU_PATH=$(EMU_PATH)

bb:
	@"$(MAKE)" -C target/Blueboard BUILD_DIR=$(BUILD_DIR)/blueboard EMU_PATH=$(EMU_PATH)
	
bb-program:
	@"$(MAKE)" -C target/Blueboard BUILD_DIR=$(BUILD_DIR)/blueboard program

disco: 
	@"$(MAKE)" -C target/Discovery BUILD_DIR=$(BUILD_DIR)/discovery EMU_PATH=$(EMU_PATH)

artery: 
	@"$(MAKE)" -C target/artery BUILD_DIR=$(BUILD_DIR)/artery EMU_PATH=$(EMU_PATH)
