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
#SRCS += $(PWD)/../audio_utility/*.c
INCS = -I$(MW_INC) -I../../sample/common -I$(KERNEL_INC) -I ../include -I ../audio_utility/
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)


TARGET = sample_audio_aec

AUD_LIB = -lcvi_audio -lcvi_vqe -lcvi_VoiceEngine -lcvi_RES1  -ltinyalsa -lm -lpthread  -ldl
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
AEC_TEST_LDLIBS =

all : $(TARGET) post_remove

post_remove:
	@rm -f ../audio_utility/*.o
	@rm -f ../audio_utility/*.d

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(CFLAGS2) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET) : $(OBJS)
	@$(CXX) -o $@ $(OBJS) $(ELFFLAGS) $(EXTRA_LDFLAGS) $(AEC_TEST_LDLIBS) $(STATIC_FLAG)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)


.PHONY : clean
clean:
	echo "clean...audio_aec"
	@rm -f $(OBJS) $(DEPS) $(COMM_DEPS) $(TARGET)

-include $(DEPS)
