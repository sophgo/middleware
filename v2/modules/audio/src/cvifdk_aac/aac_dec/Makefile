#!bash/bin
SDK_VER ?= 64bit
ifeq ($(PARAM_FILE), )
ifeq ($(MW_VER), )
PARAM_FILE:=../../../../../Makefile.param
include $(PARAM_FILE)
else
PARAM_FILE:=../../../../../../$(shell echo $(MW_VER))/Makefile.param
include $(PARAM_FILE)
endif
endif


AUDIO_PARAM_FILE :=../../../audio.mk
include $(AUDIO_PARAM_FILE)

PWD = $(shell pwd)
SDIR = $(PWD)
C_SRCS = $(wildcard $(SDIR)/*.c)


ifeq ($(SDK_VER), 32bit)
TARGET_A = $(SDIR)/32bit/libcvi_aac_dec.a
TARGET_SO = $(SDIR)/32bit/libcvi_aac_dec.so
CC = arm-linux-gnueabihf-gcc
LIBS = -L$(PWD)/../32bit/ -lfdkaac_dec
PREBUILT_LIB = $(MW_LIB)/../3rdparty/libaac/32bit
else
TARGET_A = $(SDIR)/64bit/libcvi_aac_dec.a
TARGET_SO = $(SDIR)/64bit/libcvi_aac_dec.so
CC = aarch64-linux-gnu-gcc
LIBS = -L$(PWD)/../64bit/ -lfdkaac_dec
PREBUILT_LIB = $(MW_LIB)/../3rdparty/libaac/64bit
endif

DEFINES =

INC = -I$(PWD)/
INC += -I ../fdkaac/libFDK/include/
INC += -I ../fdkaac/libSYS/include/
INC += -I ../fdkaac/libAACdec/include/
INC += -I ../fdkaac/libArithCoding/include/
INC += -I ../fdkaac/libAACdec/src/
INC += -I ../fdkaac/libMpegTPDec/include/
INC += -I ../fdkaac/libSBRdec/include/
INC += -I ../fdkaac/libPCMutils/include/
INC += -I ../fdkaac/libDRCdec/include/
INC += -I ../fdkaac/libSACdec/include/

INC += -I$(MW_PATH)/include

CFLAGS = -g -Wall -O0 -fPIC -fgnu89-inline  $(DEFINES) $(INC)
#LDFLAGS = -lpthread -pthread  -lm

ARFLAGS = rcs
ARFLAGS_S = -shared -fPIC -o

C_OBJS = $(C_SRCS:.c=.o)

#all: $(EXENAME)
all: $(TARGET_A) $(TARGET_SO)
	cp -a $(TARGET_A) $(TARGET_SO) $(PREBUILT_LIB)

$(SDIR)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS) $(LIBS) -o $@ -c $<

$(TARGET_A): $(C_OBJS)
	$(AR) $(ARFLAGS) $@ $(C_OBJS)

$(TARGET_SO): $(C_OBJS)
	$(CC) $(LDFLAGS) $(ARFLAGS_S) $@ $(C_OBJS)

clean:
	$(RM) $(C_OBJ) server *.o $(TARGET_A) $(TARGET_SO)
	$(RM) -rf $(PREBUILT_LIB)/libcvi_aac_dec.*

.PHONY:clean all




