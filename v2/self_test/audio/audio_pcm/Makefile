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


COMMON_DIR := $(MW_PATH)/sample/common
COMM_INC := $(COMMON_DIR)
SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c)
SRCS += ../audio_utility/cvi_audio_loadcheck.c
INCS = -I$(MW_INC) -I$(ISP_INC) -I$(KERNEL_INC) -I ../include -I$(COMM_INC) -I ../audio_utility
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
DEFS += -DSUPPORT_EXTERNAL_AAC
DEFS += -DCVI_MODIFIED

TARGET = sample_audio_internal
TARGET_WITH_ASAN =

ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
TARGET_WITH_ASAN = sample_audio_internal_asan
endif


AUD_LIB += -lcvi_audio -lcvi_vqe -lcvi_VoiceEngine -lcvi_RES1  -ltinyalsa -lm -lpthread  -ldl
AUD_LIB += -lpthread -pthread
AUD_LIB += -lcvi_dnvqe -lcvi_ssp2
ifeq ($(CVIARCH), $(filter $(CVIARCH), CV181X CV180X CV186X))
AUD_LIB += -lsys
endif
LIBS = $(AUD_LIB)

ifeq ($(MULTI_PROCESS_SUPPORT), 1)
DEFS += -DRPC_MULTI_PROCESS
endif

ifeq ($(USE_NEXT_SSP_DUAL), yes)
ifeq ($(USE_CVIAUDIO_STATIC), yes)
AUD_LIB += -lcvi_ssp
endif
else
ifeq ($(USE_CVIAUDIO_STATIC), yes)
AUD_LIB += -lcvi_ssp
AUD_LIB += -lcvi_ssp2
endif
AUD_LIB += -ldnvqe
endif

ifeq ($(MULTI_PROCESS_SUPPORT_AUDIO), yes)
DEFS += -DRPC_MULTI_PROCESS_AUDIO
endif
EXTRA_LDFLAGS +=  -L $(MW_LIB)

ifeq ($(USE_CVIAUDIO_STATIC), yes)
DEFS += -DCVIAUDIO_STATIC
STATIC_FLAG = -static
LIBS += -lm -lpthread -ldl
else
STATIC_FALG =
endif

EXTRA_CFLAGS += $(DEFS) $(INCS)  -Wno-unused-parameter
EXTRA_LDFLAGS += $(LIBS)
CFLAGS2  = -fPIC $(OPT_LEVEL) -Wno-implicit-function-declaration

EXTRA_WITH_ASAN = -lasan

all : $(TARGET) $(TARGET_WITH_ASAN)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(CFLAGS2) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET) : $(OBJS)
	@$(CXX) -o $@ $(OBJS) $(ELFFLAGS) $(STATIC_FLAG) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

$(TARGET_WITH_ASAN) : $(OBJS)
	@$(CXX) -o $@ $(OBJS) $(ELFFLAGS) $(EXTRA_WITH_ASAN) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)



.PHONY : clean
clean:
	@rm -f $(OBJS) $(DEPS) $(COMM_DEPS) $(TARGET) $(TARGET_WITH_ASAN)

-include $(DEPS)
