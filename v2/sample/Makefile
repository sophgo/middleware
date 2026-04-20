export INSTALL_MW_DIR = $(TOP_DIR)/middleware/$(MW_VER)/sample/install/bin

all:$(INSTALL_MW_DIR)
	@echo "##############################"
	@echo "#                            #"
	@echo "#   Compiling 'samples'...   #"
	@echo "#                            #"
	@echo "##############################"
	@for x in `find ./ -maxdepth 2 -mindepth 2 -name "Makefile" `; \
	    do cd `dirname $$x`; if [ $$? ]; then $(MAKE) all || exit 1; cd ../; fi; done
$(INSTALL_MW_DIR):
	@rm -rf $@
	@mkdir -p $@


clean:
	@for x in `find ./ -maxdepth 2 -mindepth 2 -name "Makefile" `; \
	    do cd `dirname $$x`; if [ $$? ]; then $(MAKE) clean; cd ../; fi; done
