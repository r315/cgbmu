BUILD_DIR  =build

all: bb disco disco

$(BUILD_DIR):
	mkdir -p $@
	
clean: 
	@${RM} -Rf $(BUILD_DIR)
#@$(MAKE) -C bsp/emu clean
#@$(MAKE) -C bsp/Blueboard clean
#@$(MAKE) -C bsp/Discovery clean
	
emu: $(BUILD_DIR)
	@$(MAKE) -C bsp/emu BUILD_DIR=../../$(BUILD_DIR)/emu

rpi: 
	@$(MAKE) -C bsp/emu BUILD_DIR=../../$(BUILD_DIR)/rpi rpi

bb:
	@$(MAKE) -C bsp/Blueboard BUILD_DIR=../../$(BUILD_DIR)/blueboard
	
bb-program:
	@$(MAKE) -C bsp/Blueboard BUILD_DIR=../../$(BUILD_DIR)/blueboard program


DISCO: 
	@$(MAKE) -C bsp/Discovery BUILD_DIR=../../$(BUILD_DIR)/discovery
