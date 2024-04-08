SHELL = /bin/bash
SDK_VER? := 64bit

ifeq ($(CHIP_ARCH), MARS)
	CHIP_ARCH:=CV181X
else ifeq ($(CHIP_ARCH), PHOBOS)
	CHIP_ARCH:=CV180X
else ifeq ($(CHIP_ARCH), SOPHON)
	CHIP_ARCH:=CV186X
endif


ifeq ($(PARAM_FILE), )
     PARAM_FILE:=Makefile.param
     include $(PARAM_FILE)
endif

CHIP_DIR := $(MW_PATH)/chips/$(shell echo $(CHIP_ARCH) | tr A-Z a-z)

ifeq ($(DESTDIR),)
    DESTDIR := $(shell pwd)/install
endif

define package_mw
    @install -d $(1)
    @tar -c --exclude='chips' --exclude='modules' --exclude='$(1)' --exclude='.git' --exclude='tool' --exclude='self_test' --exclude='component/panel/cv1835/customer' -f - . | (cd $(1); tar xf -)
    @rm $(1)/3rdparty/gtest -r
    @rm $(1)/sample/ -r
    @make -C sample/ clean
    @cp sample $(1)/sample -r
endef

.PHONY: clean all module component sample install uninstall package 3rdparty self_test

all: prepare 3rdparty module component sample self_test

module: prepare 3rdparty
	@make -C modules/

prepare:
	@if [ -d mod_tmp ]; then mv mod_tmp modules; fi;
ifeq ($(CHIP_ARCH), $(filter $(CHIP_ARCH), CV180X CV181X CV182X CV186X))
else
	@cp $(CHIP_DIR)/cvi_defines.h $(MW_INC)
endif
	@mkdir -p include/isp
	@cp -rf modules/isp/include/* include/isp
ifeq ($(CHIP_ARCH), $(filter $(CHIP_ARCH), CV181X CV180X CV186X))
	@cp -rf modules/isp/$(shell echo $(CHIP_ARCH) | tr A-Z a-z)/isp-daemon2/inc/cvi_ispd2.h include/isp/$(shell echo $(CHIP_ARCH) | tr A-Z a-z)/
endif

3rdparty:
	@make -C 3rdparty/

sample: module component
	@mv modules mod_tmp
	@make -C sample/
	@mv mod_tmp modules

self_test: sample
	@make -C self_test/

component:
	@make -C component/isp/ all

dl_daemon: sample
	@cmake self_test/dl_daemon -Bself_test/dl_daemon/build
	@make -C self_test/dl_daemon/build/

ifneq ($(SUBTYPE), fpga)
install:
	@mkdir -p $(DESTDIR)/usr/bin
	@mkdir -p $(DESTDIR)/usr/lib/3rd
ifneq ($(FLASH_SIZE_SHRINK),y)
	# copy sample_xxx
	@cp -f sample/mipi_tx/sample_dsi $(DESTDIR)/usr/bin
	@cp -f sample/cipher/sample_cipher $(DESTDIR)/usr/bin
	@cp -f sample/cvg/sample_cvg $(DESTDIR)/usr/bin
	@cp -f sample/venc/sample_venc $(DESTDIR)/usr/bin
	@cp -f sample/venc/sample_vcodec $(DESTDIR)/usr/bin
	@cp -f sample/vdec/sample_vdec $(DESTDIR)/usr/bin

	@cp -f self_test/cvi_test/cvi_test $(DESTDIR)/usr/bin
	@cp -f self_test/cvi_test/sensor_cfg.ini $(DESTDIR)/usr/bin
endif

ifneq ($(FLASH_SIZE_SHRINK),y)
	# copy venc
	#@cp -f modules/venc/vc_lib/bin/cvi_h265_enc_test $(DESTDIR)/usr/bin
	#@cp -f modules/venc/vc_lib/bin/cvi_h265_dec $(DESTDIR)/usr/bin
	#@cp -f modules/venc/vc_lib/bin/cvi_h264_dec $(DESTDIR)/usr/bin
	#@cp -f modules/venc/vc_lib/bin/cvi_jpg_codec $(DESTDIR)/usr/bin
endif

ifneq ($(FLASH_SIZE_SHRINK),y)
	# copy audio libs and elf
	@cp -f sample/audio/sample_audio*  $(DESTDIR)/usr/bin
	@if [ -e "sample/audio/cvi_mp3player" ]; then cp -f sample/audio/cvi_mp3player $(DESTDIR)/usr/bin; fi
endif

	# copy mw lib
	@cp -a lib/*.so*  $(DESTDIR)/usr/lib
	@cp -a lib/3rd/*.so*  $(DESTDIR)/usr/lib/3rd

uninstall:
	@rm $(DESTDIR) -rf

package:
	$(call package_mw,tmp)
	@install -d $(DESTDIR)
	@tar fcz $(DESTDIR)/mw.tar.gz -C tmp .
	@echo $(KERNEL_INC)
	@tar fcz $(DESTDIR)/kernel_header.tar.gz -C $(KERNEL_INC) ./
	@rm tmp -r
	@echo "package done"
endif
clean:
	@if [ -d mod_tmp ]; then mv mod_tmp modules; fi;
ifeq ($(CHIP_ARCH), $(filter $(CHIP_ARCH), CV180X CV181X CV182X CV186X))
else
	@rm -f $(MW_INC)/cvi_defines.h
endif
	@rm -rf include/isp
	@make -C modules/ clean
	@make -C 3rdparty/ clean
	@make -C sample/ clean
	@make -C self_test/ clean
	@make -C component/isp/ clean
	@rm -rf self_test/dl_daemon/build
