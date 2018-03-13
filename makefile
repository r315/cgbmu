OBJSPATH  =obj


all: emu
	
clean: 
	@${RM} $(OBJSPATH)/*.o $(TARGET)
	@$(MAKE) -C bsp/emu clean
	@$(MAKE) -C bsp/Blueboard clean
	
emu: 
	@$(MAKE) -C bsp/emu OBJSPATH=../../$(OBJSPATH)


bb:
	@$(MAKE) -C bsp/Blueboard OBJSPATH=../../$(OBJSPATH)
	



