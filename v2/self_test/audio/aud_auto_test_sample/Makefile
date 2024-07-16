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
#SRCS = $(wildcard $(SDIR)/*.c)
INCS = -I$(PWD)/aac/ -I$(MW_INC) -I$(ISP_INC) -I$(VI_INC) -I$(KERNEL_INC) -I ./include -I$(COMM_INC)
#OBJS = $(SRCS:.c=.o)
#DEPS = $(SRCS:.c=.d)
DEFS += -DSUPPORT_EXTERNAL_AAC
DEFS += -DCVI_MODIFIED


ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
APLAY_TARGET = cvi_auto_play_asan
ACAP_TARGET = cvi_auto_record_asan
SPECIAL_TEST = cvi_auto_special_asan
else
APLAY_TARGET = cvi_auto_play
ACAP_TARGET = cvi_auto_record
SPECIAL_TEST = cvi_auto_special
endif

APLAY_OBJ = cvi_auto_player.o cvi_audio_aac_adp.o
ACAP_OBJ = cvi_auto_recorder.o cvi_audio_aac_adp.o
SPECIAL_OBJ = cvi_auto_special_test.o cvi_audio_aac_adp.o

AUD_LIB = -lcvi_audio -lcvi_vqe -lcvi_ssp -lcvi_VoiceEngine -lcvi_RES1  -ltinyalsa -lm -lpthread -pthread -ldl
AUD_LIB += -lcvi_dnvqe -lcvi_ssp2
LIBS = $(AUD_LIB)
LIBS += -laacdec2
LIBS += -laacsbrdec2
LIBS += -laacenc2
LIBS += -laaccomm2
LIBS += -laacsbrenc2

ifeq ($(MULTI_PROCESS_SUPPORT), 1)
DEFS += -DRPC_MULTI_PROCESS
endif

ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
USE_CVIAUDIO_STATIC = no
endif

ifeq ($(CVIARCH), $(filter $(CVIARCH), CV181X CV180X CV186X))
LIBS += -lsys
endif

ifeq ($(USE_CVIAUDIO_STATIC), yes)
DEFS += -DCVIAUDIO_STATIC
STATIC_FLAG = -static
else
STATIC_FALG =
endif

ifeq ($(MULTI_PROCESS_SUPPORT_AUDIO), yes)
DEFS += -DRPC_MULTI_PROCESS_AUDIO
endif
EXTRA_LDFLAGS +=  -L $(MW_LIB)

EXTRA_LDFLAGS += -lrt
EXTRA_CFLAGS += $(DEFS) $(INCS)  -Wno-unused-parameter
EXTRA_LDFLAGS += $(LIBS)
CFLAGS2  = -fPIC $(OPT_LEVEL) -Wno-implicit-function-declaration

ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
EXTRA_LDFLAGS_WITH_ASAN = -fsanitize=address -fno-omit-frame-pointer -fsanitize-recover=address -lasan
EXTRA_LDFLAGS +=$(EXTRA_LDFLAGS_WITH_ASAN)
endif


all : $(APLAY_TARGET) $(ACAP_TARGET) $(SPECIAL_TEST)

%.o: %.c
	$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(APLAY_TARGET): $(APLAY_OBJ)
	$(CXX) -o $@ $(APLAY_OBJ) $(ELFFLAGS) $(STATIC_FLAG) $(EXTRA_LDFLAGS) $(STATIC_FLAG)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

$(ACAP_TARGET): $(ACAP_OBJ)
	@$(CXX) -o $@ $(ACAP_OBJ) $(ELFFLAGS) $(STATIC_FLAG) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

$(SPECIAL_TEST): $(SPECIAL_OBJ)
	@$(CXX) -o $@ $(SPECIAL_OBJ) $(ELFFLAGS) $(STATIC_FLAG) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

.PHONY : clean
clean:
	@rm -f *.o *.d $(DEPS) $(COMM_DEPS) $(APLAY_TARGET) $(ACAP_TARGET) $(SPECIAL_TEST)

-include $(DEPS)

