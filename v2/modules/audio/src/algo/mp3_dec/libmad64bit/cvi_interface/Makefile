SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
ifeq ($(MW_VER), )
PARAM_FILE:=../../../../../../../Makefile.param
include $(PARAM_FILE)
else
PARAM_FILE:=../../../../../../../../$(shell echo $(MW_VER))/Makefile.param
include $(PARAM_FILE)
endif
endif


PWD := $(shell pwd)
SDK_VER?=64bit
$(info SDK_VER=$(SDK_VER))

SOURCE_DIR=/home/vincentyu/master_repo5
CROSS_COMPILE32=$(SOURCE_DIR)/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
CROSS_COMPILE64=$(SOURCE_DIR)/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-

ifeq ($(SDK_VER), 64bit)
CROSS_COMPILE = $(CROSS_COMPILE64)
LDFLAGS = -shared -fPIC -O3 --gc-sections -export-dynamic
endif

ifeq ($(SDK_VER), 32bit)
CROSS_COMPILE = $(CROSS_COMPILE32)
LDFLAGS = -shared -fPIC -O3 --gc-sections -export-dynamic
endif

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld

LIB_DIR = -L$(PWD)/../lib64bit -L$(MW_LIB) -L$(MW_3RD_LIB)
EXENAME := cvimp3
DEFS=
CFLAGS += $(DEFS)
TARGET_SO = ../lib64bit/libcvi_mp3.so

SDIR = $(PWD)
SRC_C += $(shell find ./ -name 'cvi_mp3_decode.c')
INCS = -I$(MW_INC) -I$(COMMON_INC) -I$(PWD)/../
LIBS +=  -lm -lpthread
LIBS += -lmad

OBJS := $(SRC_C:%.c=%.o)

CFLAGS += -fPIC -pthread
CFLAGS += $(INCLUDES)
CFLAGS += $(INCS)
##turn on below if you want to build unit test in cvi_mp3_decode.c
##CFLAGS += -DTEST_IN_DECODE_UNIT

EXPORT_LIBMAD = export_mp3dec_lib

BUILD_LIST =
##BUILD_LIST += $(EXENAME)
BUILD_LIST += $(TARGET_SO)
BUILD_LIST += $(EXPORT_LIBMAD)

all:$(BUILD_LIST)


$(EXENAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^  $(LIB_DIR) $(LIBS)

$(TARGET_SO): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ --start-group $(OBJS) --end-group

export_mp3dec_lib:
#	@cp ../lib64bit/*  $(MW_LIB)/.
	@cp ../lib64bit/*  $(MW_PATH)/3rdparty/libmad/lib_64/.
clean:
	$(RM) $(OBJS) $(EXENAME) $(SHARED) *.o
	@rm -f $(MW_LIB)/libmad*
	@rm -f $(MW_LIB)/libcvi_mp3*
	@rm -f $(MW_PATH)/3rdparty/libmad/lib_64/*

.PHONY:clean all
