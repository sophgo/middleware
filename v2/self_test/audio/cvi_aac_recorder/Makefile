SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
ifeq ($(MW_VER), )
PARAM_FILE:=../../../Makefile.param
include $(PARAM_FILE)
else
PARAM_FILE:=../../../../$(shell echo $(MW_VER))/Makefile.param
include $(PARAM_FILE)
endif
endif

include ../../self_test.mk
include $(MW_PATH)/modules/audio/audio.mk

EXENAME = cvi_aac_recorder
SDIR = $(PWD)
PWD = $(shell pwd)
MW_PATCH = $(PWD)/../../..
C_SRCS = $(wildcard $(SDIR)/*.c)
C_OBJS = $(C_SRCS:.c=.o)

ifeq ($(SDK_VER), 64bit)
CC = aarch64-linux-gnu-gcc
LIBS = -L$(PWD)/aac_lib/64bit -lfdkaac_enc
LIBS += -L$(MW_PATCH)/3rdparty/libaac/64bit -lcvi_aac_enc
endif

ifeq ($(SDK_VER), 32bit)
CC = arm-linux-gnueabihf-gcc
LIBS = -L$(PWD)/aac_lib/32bit -lfdkaac_enc
LIBS += -L$(MW_PATCH)/3rdparty/libaac/32bit -lcvi_aac_enc
endif

DEFINES =
#INC = -I$(MW_INC) -I$(PWD)/
INCS = -I$(MW_INC) -I../../sample/common -I$(KERNEL_INC) -I ./include -I ./
LIBS += -L$(MW_PATCH)/lib -lcvi_audio -ldl -lcvi_VoiceEngine -lcvi_vqe -ltinyalsa -lcvi_RES1 -lm -lpthread  -ldl

##CFLAGS = -g -Wall -O0 -fPIC -fgnu89-inline $(DEFINES) $(INC)
##LDFLAGS = -lpthread -pthread  -lm

ifeq ($(MULTI_PROCESS_SUPPORT), 1)
DEFS += -DRPC_MULTI_PROCESS
endif

ifeq ($(MULTI_PROCESS_SUPPORT_AUDIO), yes)
DEFS += -DRPC_MULTI_PROCESS_AUDIO
endif

ifeq ($(SDK_VER), 32bit)
EXTRA_LDFLAGS += -L $(MW_PATH)/modules/audio/src/cvifdk_aac/32bit/
EXTRA_LDFLAGS += -L $(MW_PATH)/modules/audio/src/cvifdk_aac/aac_enc/32bit/
endif
ifeq ($(SDK_VER), 64bit)
EXTRA_LDFLAGS += -L $(MW_PATH)/modules/audio/src/cvifdk_aac/64bit/
EXTRA_LDFLAGS += -L $(MW_PATH)/modules/audio/src/cvifdk_aac/aac_enc/64bit/
endif
ifeq ($(SDK_VER), uclibc)
EXTRA_LDFLAGS += -L $(MW_PATH)/modules/audio/src/cvifdk_aac/lib_uclibc/
EXTRA_LDFLAGS += -L $(MW_PATH)/modules/audio/src/cvifdk_aac/aac_enc/lib_uclibc/
endif


EXTRA_LDFLAGS += -L $(LIBS)
EXTRA_CFLAGS += $(DEFINES) $(INCS)  -Wno-unused-parameter

DEPS = $(C_SRCS:.c=.d)
CFLAGS2  = -fPIC $(OPT_LEVEL) -Wno-implicit-function-declaration


all: $(EXENAME)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(CFLAGS2) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

#$(EXENAME): $(C_OBJS)
#	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ $^
$(EXENAME) : $(C_OBJS)
	@$(CXX) -o $@ $(C_OBJS) $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)


.PHONY : clean
clean:
	@rm -f $(C_OBJS) $(DEPS) $(COMM_DEPS) $(EXENAME)

-include $(DEPS)


