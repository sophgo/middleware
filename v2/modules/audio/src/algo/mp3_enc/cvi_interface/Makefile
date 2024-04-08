SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
ifeq ($(MW_VER), )
PARAM_FILE:=../../../../../../Makefile.param
include $(PARAM_FILE)
else
PARAM_FILE:=../../../../../../../$(shell echo $(MW_VER))/Makefile.param
include $(PARAM_FILE)
endif
endif

PWD := $(shell pwd)
SDK_VER?=32bit
$(info SDK_VER=$(SDK_VER))

TOOLCHAIN_PATH=../../../../../../../host-tools
CROSS_COMPILE_32=$(TOOLCHAIN_PATH)/gcc/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
CROSS_COMPILE_64=$(TOOLCHAIN_PATH)/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
CROSS_COMPILE_UCLIBC=$(TOOLCHAIN_PATH)/gcc/arm-cvitek-linux-uclibcgnueabihf/bin/arm-cvitek-linux-uclibcgnueabihf-

ifeq ($(SDK_VER), 64bit)
CROSS_COMPILE = $(CROSS_COMPILE_64)
LDFLAGS = -shared -fPIC -O3 --gc-sections -export-dynamic
OUTPUT_DIR = ../lame_output/64bit/lib
RELEASE_DIR = $(MW_PATH)/3rdparty/lame/lib_64
LAME_INCLUDE = ../lame_output/64bit/include/lame
LIB_DIR = -L$(PWD)/ -L$(OUTPUT_DIR)/ -L$(MW_LIB) -L$(MW_3RD_LIB)
endif

ifeq ($(SDK_VER), 32bit)
CROSS_COMPILE = $(CROSS_COMPILE_32)
LDFLAGS = -shared -fPIC -O3 --gc-sections -export-dynamic
OUTPUT_DIR = ../lame_output/32bit/lib
RELEASE_DIR = $(MW_PATH)/3rdparty/lame/lib_32
LAME_INCLUDE = ../lame_output/32bit/include/lame
LIB_DIR = -L$(PWD)/ -L$(OUTPUT_DIR)/ -L$(MW_LIB) -L$(MW_3RD_LIB)
endif

ifeq ($(SDK_VER), uclibc)
CROSS_COMPILE = $(CROSS_COMPILE_UCLIBC)
LDFLAGS = -shared -fPIC -O3 --gc-sections -export-dynamic
OUTPUT_DIR = ../lame_output/uclibc/lib
RELEASE_DIR = $(MW_PATH)/3rdparty/lame/lib_uclibc
LAME_INCLUDE = ../lame_output/uclibc/include/lame
LIB_DIR = -L$(PWD)/ -L$(OUTPUT_DIR)/ -L$(MW_LIB) -L$(MW_3RD_LIB)
endif

ifeq ($(SDK_VER), glibc_riscv64)
CROSS_COMPILE = $(CROSS_COMPILE_UCLIBC)
LDFLAGS = -shared -fPIC -O3 --gc-sections -export-dynamic
OUTPUT_DIR = ../lame_output/riscv64/lib
RELEASE_DIR = $(MW_PATH)/3rdparty/lame/lib_riscv64
LAME_INCLUDE = ../lame_output/riscv64/include/lame
LIB_DIR = -L$(PWD)/ -L$(OUTPUT_DIR)/ -L$(MW_LIB) -L$(MW_3RD_LIB)
endif

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld

EXENAME := cvimp3enc
DEFS=
CFLAGS += $(DEFS)
##CFLAGS += -DTEST_IN_ENCODE_UNIT
TARGET_SO = $(OUTPUT_DIR)/libcvi_mp3enc.so

SDIR = $(PWD)
SRC_C += $(shell find ./ -name 'cvi_mp3_encode.c')
INCS = -I$(MW_INC) -I$(COMMON_INC) -I$(PWD)/../ -I$(LAME_INCLUDE) -I$(KERNEL_INC)
LIBS +=  -lm -lpthread
LIBS += -lmp3lame

OBJS := $(SRC_C:%.c=%.o)

CFLAGS += -fPIC -pthread
CFLAGS += $(INCLUDES)
CFLAGS += $(INCS)
##turn on below if you want to build unit test in cvi_mp3_decode.c
##CFLAGS += -DTEST_IN_DECODE_UNIT

EXPORT_LIBMAD = export_mp3enc_lib

BUILD_LIST =
##BUILD_LIST += $(EXENAME)
BUILD_LIST += $(TARGET_SO)
BUILD_LIST += $(EXPORT_LIBMAD)

all:$(BUILD_LIST)


$(EXENAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^  $(LIB_DIR) $(LIBS)

$(TARGET_SO): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ --start-group $(OBJS) --end-group

export_mp3enc_lib:
#	@cp $(OUTPUT_DIR)/*  $(MW_LIB)/.
	@cp -a $(OUTPUT_DIR)/libcvi*  $(RELEASE_DIR)/.
	@cp -a $(OUTPUT_DIR)/libmp3lame.a  $(RELEASE_DIR)/.
	@cp -a $(OUTPUT_DIR)/libmp3lame.la  $(RELEASE_DIR)/.
	@cp -a $(OUTPUT_DIR)/libmp3lame.so*  $(RELEASE_DIR)/.
clean:
	$(RM) $(OBJS) $(EXENAME) $(SHARED) *.o
	@rm -f $(MW_LIB)/libcvi_mp3enc*
	@rm -f $(MW_LIB)/libmp3lame*
	@rm -f $(RELEASE_DIR)/*
	@rm -f $(OUTPUT_DIR)/libcvi_mp3enc*

.PHONY:clean all
