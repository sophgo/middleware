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

TOOLCHAIN_PATH=$(PWD)/../../../../../../../../host-tools
CROSS_COMPILE_32=$(TOOLCHAIN_PATH)/gcc/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
CROSS_COMPILE_64=$(TOOLCHAIN_PATH)/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
CROSS_COMPILE_UCLIBC=$(TOOLCHAIN_PATH)/gcc/arm-cvitek-linux-uclibcgnueabihf/bin/arm-cvitek-linux-uclibcgnueabihf-
CROSS_COMPILE_MUSL_RISCV_64=$(TOOLCHAIN_PATH)/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-

ifeq ($(SDK_VER), 64bit)
CROSS_COMPILE = $(CROSS_COMPILE_64)
LDFLAGS = -shared -fPIC --gc-sections -export-dynamic
OUTPUT_DIR = ../libmad_output/64bit/lib
RELEASE_DIR = $(MW_PATH)/3rdparty/libmad/lib_64
LIBMAD_INCLUDE = ../libmad_output/64bit/include
LIB_DIR = -L$(PWD)/ -L$(OUTPUT_DIR)/ -L$(MW_LIB) -L$(MW_3RD_LIB)
endif

ifeq ($(SDK_VER), 32bit)
CROSS_COMPILE = $(CROSS_COMPILE_32)
LDFLAGS = -shared -fPIC --gc-sections -export-dynamic
OUTPUT_DIR = ../libmad_output/32bit/lib
RELEASE_DIR = $(MW_PATH)/3rdparty/libmad/lib_32
LIBMAD_INCLUDE = ../libmad_output/32bit/include
LIB_DIR = -L$(PWD)/ -L$(OUTPUT_DIR)/ -L$(MW_LIB) -L$(MW_3RD_LIB)
endif

ifeq ($(SDK_VER), uclibc)
CROSS_COMPILE = $(CROSS_COMPILE_UCLIBC)
LDFLAGS = -shared -fPIC --gc-sections -export-dynamic
LIB_DIR = -L$(PWD)/ -L$(OUTPUT_DIR)/ -L$(MW_LIB) -L$(MW_3RD_LIB)
OUTPUT_DIR = ../libmad_output/uclibc/lib
RELEASE_DIR = $(MW_PATH)/3rdparty/libmad/lib_uclibc
LIBMAD_INCLUDE = ../libmad_output/uclibc/include
LIB_DIR = -L$(PWD)/ -L$(OUTPUT_DIR)/ -L$(MW_LIB) -L$(MW_3RD_LIB)
endif

ifeq ($(SDK_VER), musl_riscv64)
CROSS_COMPILE = $(CROSS_COMPILE_MUSL_RISCV_64)
LDFLAGS = -shared -fPIC --gc-sections -export-dynamic
OUTPUT_DIR = ../libmad_output/musl64/lib
RELEASE_DIR = $(MW_PATH)/3rdparty/libmad/lib_riscv64
LIBMAD_INCLUDE = ../libmad_output/musl64/include
LIB_DIR = -L$(PWD)/ -L$(OUTPUT_DIR)/ -L$(MW_LIB) -L$(MW_3RD_LIB)
endif

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld

EXENAME := cvimp3
DEFS=
CFLAGS += $(DEFS)
TARGET_SO = $(OUTPUT_DIR)/libcvi_mp3.so
TARGET_A = $(OUTPUT_DIR)/libcvi_mp3.a

SDIR = $(PWD)
SRC_C += $(shell find ./ -name 'cvi_mp3_decode.c')
INCS = -I$(MW_INC) -I$(COMMON_INC) -I$(LIBMAD_INCLUDE)
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
BUILD_LIST += $(TARGET_A)

all:$(BUILD_LIST)


$(EXENAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^  $(LIB_DIR) $(LIBS)

$(TARGET_SO): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ --start-group $(OBJS) --end-group

$(TARGET_A):$(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

export_mp3dec_lib:
#	cp $(OUTPUT_DIR)/*  $(MW_LIB)/.
	cp -a $(OUTPUT_DIR)/libcvi*  $(RELEASE_DIR)/.
	cp -a $(OUTPUT_DIR)/libmad.a  $(RELEASE_DIR)/.
	cp -a $(OUTPUT_DIR)/libmad.la  $(RELEASE_DIR)/.
	cp -a $(OUTPUT_DIR)/libmad.so*  $(RELEASE_DIR)/.
	cp -a $(PWD)/*.h $(LIBMAD_INCLUDE)
clean:
	$(RM) $(OBJS) $(EXENAME) $(SHARED) *.o
	rm -f $(MW_LIB)/libmad*
	rm -f $(MW_LIB)/libcvi_mp3*
	rm -f $(RELEASE_DIR)/*
	rm -f $(OUTPUT_DIR)/libcvi_mp3*

.PHONY:clean all
