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

SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c)
INCS = -I$(MW_INC) -I../../sample/common -I$(KERNEL_INC) -I ../include
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)


TARGET = sample_audio_nr
TARGET_WITH_ASAN =
ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
TARGET_WITH_ASAN = sample_audio_nr_asan
endif


AUD_LIB += -lcvi_audio -lcvi_vqe -lcvi_VoiceEngine -lcvi_RES1  -ltinyalsa -lm -lpthread  -ldl
AUD_LIB += -lpthread -pthread
ifeq ($(CVIARCH), $(filter $(CVIARCH), CV181X CV180X CV186X))
AUD_LIB += -lsys
endif
LIBS = $(AUD_LIB)

ifeq ($(MULTI_PROCESS_SUPPORT), 1)
DEFS += -DRPC_MULTI_PROCESS
endif

ifeq ($(USE_NEXT_SSP_DUAL), yes)
ifeq ($(USE_CVIAUDIO_STATIC), yes)
AUD_LIB += -lcvi_ssp -lcvi_audio
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
else
STATIC_FALG =
endif
EXTRA_CFLAGS += $(DEFS) $(INCS)  -Wno-unused-parameter
EXTRA_LDFLAGS += $(LIBS)
CFLAGS2  = -fPIC $(OPT_LEVEL) -Wno-implicit-function-declaration

EXTRA_WITH_ASAN = -lasan

all : $(TARGET) $(TARGET_WITH_ASAN)

$(SDIR)/%.o: $(SDIR)/%.c
	$(CC) $(DEPFLAGS) $(CFLAGS) $(CFLAGS2) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET) : $(OBJS)
	$(CXX) -o $@ $(OBJS) $(ELFFLAGS) $(EXTRA_LDFLAGS) $(STATIC_FLAG)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

$(TARGET_WITH_ASAN) : $(OBJS)
	$(CXX) -o $@ $(OBJS) $(ELFFLAGS) $(EXTRA_WITH_ASAN) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

.PHONY : clean
clean:
	@rm -f $(OBJS) $(DEPS) $(COMM_DEPS) $(TARGET) $(TARGET_WITH_ASAN)

-include $(DEPS)
