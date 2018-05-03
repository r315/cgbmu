OBJSPATH  =obj


all: emu
	
clean: 
	@${RM} $(OBJSPATH)/*.o $(TARGET)
	@$(MAKE) -C bsp/emu clean
	@$(MAKE) -C bsp/Blueboard clean
	@$(MAKE) -C bsp/ESP03 clean
	
emu:
	@$(MAKE) -C bsp/emu OBJSPATH=../../$(OBJSPATH)

rpi: 
	@$(MAKE) -C bsp/emu OBJSPATH=../../$(OBJSPATH) rpi

bb:
	@$(MAKE) -C bsp/Blueboard OBJSPATH=../../$(OBJSPATH)


esp: 
	@$(MAKE) -C bsp/ESP03 OBJSPATH=../../$(OBJSPATH)

esp-burn: 
	@$(MAKE) -C bsp/ESP03 OBJSPATH=../../$(OBJSPATH) burn	



