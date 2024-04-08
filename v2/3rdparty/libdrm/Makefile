SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
PARAM_FILE:=../../Makefile.param
include $(PARAM_FILE)
endif

PREBUILT_LIB = lib_$(SDK_VER)
PREBUILT_BIN = bin_$(SDK_VER)
DRM_INC = $(MW_INC)/3rd/drm
DRM_LIBS = $(MW_PATH)/lib/3rd/drm
MODETEST_BIN = $(MW_PATH)/sample/drm

all :
	@rm -rf lib
	@rm -rf bin
	@ln -s $(PREBUILT_LIB) lib
	@ln -s $(PREBUILT_BIN) bin
	@rm -rf $(DRM_INC)
	@rm -rf $(DRM_LIBS)
	@mkdir $(DRM_INC) -p
	@mkdir $(DRM_LIBS) -p
	@cp -a include/* $(DRM_INC)
	@cp -a lib/* $(DRM_LIBS)
	@cp -a bin/modetest $(MODETEST_BIN)

clean :
	@rm -rf lib
	@rm -rf bin
	@rm -rf $(DRM_INC)
	@rm -rf $(DRM_LIBS)
	@rm -f $(MODETEST_BIN)/modetest