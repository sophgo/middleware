ifeq ($(PARAM_FILE), )
     PARAM_FILE:=../Makefile.param
     include $(PARAM_FILE)
endif

.PHONY: all clean $(MODULES)

MODULES = sys vpss gdc stitch bin vi vo rgn audio mipi_tx cipher vcodec misc ive dpu tde hdmi eis

all:
	@echo "########################################################################################################"
	@echo "#"
	@echo "#  Compiling 'module libs' Configs as below ..."
	@echo "#    "CROSS_COMPILE=$(CROSS_COMPILE)
	@echo "#    "KERNEL_DIR=$(KERNEL_DIR)
	@echo "#"
	@echo "########################################################################################################"
	@for x in $(MODULES); \
	    do cd $$x; if [ $$? ]; then $(MAKE) all || exit 1; cd -; fi; done
	@make -C isp all
	@echo "#"

clean:
	@for x in `find ./ -maxdepth 2 -mindepth 2 -name "Makefile" `; \
	    do cd `dirname $$x`; if [ $$? ]; then $(MAKE) clean; cd -; fi; done
	@make -C isp clean
	@make -C audio clean
	@echo "#"
