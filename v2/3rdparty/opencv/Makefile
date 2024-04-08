SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
PARAM_FILE:=../../Makefile.param
include $(PARAM_FILE)
endif

ifeq ($(SDK_VER), 64bit)
PREBUILT_LIB = lib_64
else
PREBUILT_LIB = lib_32
USE_CVI_AEC = no
endif

all :
ifeq ($(SAMPLE_TPU_ENABLE), 1)
ifneq ($(SDK_VER), uclibc)
	@rm -rf lib
	@ln -s $(PREBUILT_LIB) lib
	@cp -a lib/libopencv_*.so* $(MW_3RD_LIB)
	@cp lib/libtegra_hal.* $(MW_3RD_LIB)
	@cp lib/libz*.* $(MW_3RD_LIB)
	#@cp -a include/opencv $(MW_INC)
	#@cp -a include/opencv2 $(MW_INC)
endif #SDK_VER != uclibc
endif #SAMPLE_TPU_ENABLE

clean :
	@rm -rf lib
	@rm -f $(MW_3RD_LIB)/libopencv_*.*
	@rm -f $(MW_3RD_LIB)/libtegra_hal.*
	@rm -f $(MW_3RD_LIB)/libz*.*
	#@rm -rf $(MW_INC)/opencv
	#@rm -rf $(MW_INC)/opencv2
