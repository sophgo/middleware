SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
PARAM_FILE:=../../Makefile.param
include $(PARAM_FILE)
endif

PREBUILT_LIB = lib_$(SDK_VER)
DRM_INC = $(MW_INC)/3rd/drm
DRM_LIBS = $(MW_PATH)/lib/3rd/drm

all :
	@rm -rf lib
ifeq ($(SDK_VER),64bit)
	@if [ -f "$(PWD)/$(PREBUILT_LIB)/libdrm.a" ]; then rm $(PWD)/$(PREBUILT_LIB)/libdrm.a; fi
	@if [ -f "$(PWD)/$(PREBUILT_LIB)/libkms.a" ]; then rm $(PWD)/$(PREBUILT_LIB)/libkms.a; fi
ifeq ($(CONFIG_TOOLCHAIN_GLIBC_ARM64_V930),y)
	@cp $(PWD)/$(PREBUILT_LIB)/libdrm.9.3.0 $(PWD)/$(PREBUILT_LIB)/libdrm.a
	@cp $(PWD)/$(PREBUILT_LIB)/libkms.9.3.0 $(PWD)/$(PREBUILT_LIB)/libkms.a
else
	@cp $(PWD)/$(PREBUILT_LIB)/libdrm.6.3.1 $(PWD)/$(PREBUILT_LIB)/libdrm.a
	@cp $(PWD)/$(PREBUILT_LIB)/libkms.6.3.1 $(PWD)/$(PREBUILT_LIB)/libkms.a
endif
endif
	@ln -s $(PREBUILT_LIB) lib
	@rm -rf $(DRM_INC)
	@rm -rf $(DRM_LIBS)
	@mkdir $(DRM_INC) -p
	@mkdir $(DRM_LIBS) -p
	@cp -a include/* $(DRM_INC)
	@cp -a lib/* $(DRM_LIBS)

clean :
	@rm -rf lib
ifeq ($(SDK_VER),64bit)
	@if [ -f "$(PWD)/$(PREBUILT_LIB)/libdrm.a" ]; then rm $(PWD)/$(PREBUILT_LIB)/libdrm.a; fi
	@if [ -f "$(PWD)/$(PREBUILT_LIB)/libkms.a" ]; then rm $(PWD)/$(PREBUILT_LIB)/libkms.a; fi
endif
	@rm -rf $(DRM_INC)
	@rm -rf $(DRM_LIBS)
