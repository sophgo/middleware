SHELL = /bin/bash
PWD := $(shell pwd)
SDK_VER?=64bit
$(info SDK_VER=$(SDK_VER))

ifeq ($(PARAM_FILE), )
ifeq ($(MW_VER), )
     PARAM_FILE:=../../../../Makefile.param
     include $(PARAM_FILE)
else
     PARAM_FILE:=../../../../$(shell echo $(MW_VER))/Makefile.param
     include $(PARAM_FILE)
endif
endif



ifeq ($(SDK_VER), 64bit)
PREBUILD_UAC = prebuild64
CFLAGS += -DTHIS_IS_64
endif

ifeq ($(SDK_VER), 32bit)
PREBUILD_UAC = prebuild32
CFLAGS += -DTHIS_IS_32
endif

ifeq ($(SDK_VER), uclibc)
PREBUILD_UAC = prebuilduclibc
CFLAGS += -DTHIS_IS_UCLIBC
endif

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld

LIB_DIR = -L$(PWD)/lib/
EXENAME := sample_audio_uac
DEFS=

CFLAGS += $(DEFS)
TARGET_SO = $(PWD)/lib/libcvi_uac.so

SRC_C += $(shell find ./ -name 'cvi_audio_uac.c')
SRC_C += $(shell find ./ -name 'cvi_audio_dl_adp.c')
SRC_C += $(shell find ./ -name 'cyclebuffer.c')
SRC_C_EXE  = $(shell find ./ -name 'cvi_sample_uac.c')


LIBS +=  -lm -ldl -lpthread  -ltinyalsa  -lcvi_mp3 -lcvi_RES1 -lmad -lcvi_uac -lgcc_s
AAC_LIBS = -lcvi_aac_dec  -lfdkaac_dec
LIBS += $(AAC_LIBS)

OBJS := $(SRC_C:%.c=%.o)
OBJS_TEST := $(SRC_C_EXE:%.c=%.o)

CFLAGS += -fPIC -pthread
UAC_INC = -I ./include/
CFLAGS += $(UAC_INC)

BUILD_LIST =
BUILD_LIST += $(PREBUILD_UAC)
BUILD_LIST += $(TARGET_SO)
BUILD_LIST += $(EXENAME)


all:$(BUILD_LIST)

prebuild64:
	@echo "prebuild 64bit lib untar"
	@pushd $(PWD)/prebuild/; tar zxvf prebuild.tar.gz -C $(PWD)/lib/; popd
	@mv $(PWD)/lib/asoundlib.h  $(PWD)/include/

prebuild32:
	@echo "prebuild 32bit lib untar"
	@pushd $(PWD)/prebuild/; tar zxvf prebuild32bit.tar.gz -C $(PWD)/lib/; popd
	@mv $(PWD)/lib/asoundlib.h  $(PWD)/include/

prebuilduclibc:
	@echo "prebuild uclibc lib untar"
	@pushd $(PWD)/prebuild/; tar zxvf prebuilduclibc.tar.gz -C $(PWD)/lib/; popd
	@mv $(PWD)/lib/asoundlib.h  $(PWD)/include/

$(EXENAME): $(OBJS_TEST)
	$(CC)   -o $@ $^  $(LIB_DIR) $(LIBS) $(CFLAGS) $(UAC_INC)

$(TARGET_SO): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ --start-group $(OBJS) --end-group

clean:
	$(RM) $(OBJS) $(EXENAME) $(SHARED) *.o
	@pushd $(PWD)/lib/; rm -rf ./*; popd
	@pushd $(PWD)/include/; [ -e asoundlib.h ] && rm -- asoundlib.h; popd

.PHONY:clean all
