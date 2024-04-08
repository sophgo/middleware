SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
ifeq ($(MW_VER), )
PARAM_FILE:=../../../../Makefile.param
include $(PARAM_FILE)
else
PARAM_FILE:=../../../../../$(shell echo $(MW_VER))/Makefile.param
include $(PARAM_FILE)
endif
endif

AUDIO_PARAM_FILE :=../../audio.mk
include $(AUDIO_PARAM_FILE)

SDIR = $(PWD)
ifeq ($(USE_CVIAUDIO_STATIC), yes)
SRCS = $(filter-out $(SDIR)/cvi_audio_dl_adp.c, $(wildcard $(SDIR)/*.c))
else
SRCS = $(wildcard $(SDIR)/*.c)
endif

EXTRA_DEF =
ifeq ($(CONFIG_ENABLE_FREERTOS),y)
ifeq ($(CVIARCH), CV181X)
SRCS += $(PWD)/../audio_kernel_mode/cviaudio_kernel_mode.c
EXTRA_DEF += -DAUD_SUPPORT_KERNEL_MODE
endif

ifeq ($(CVIARCH), CV180X)
SRCS += $(PWD)/../audio_kernel_mode/cviaudio_kernel_mode.c
EXTRA_DEF += -DAUD_SUPPORT_KERNEL_MODE
endif
endif

INCS = -I$(MW_INC) -I$(KERNEL_INC) -I$(COMMON_INC) -I .$(PWD)
DEFS = -DCVI_MODIFIED
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
TARGET_A = $(MW_LIB)/libcvi_vqe.a
TARGET_SO = $(MW_LIB)/libcvi_vqe.so

INCS += -I../../include -I../algo/aec/

ifeq ($(USE_CVI_AEC), no)
DEFS += -DEXTERNAL_AEC
else
DEFS += -DUSE_CVI_AEC
endif

ifeq ($(USE_CVIAUDIO_STATIC), yes)
DEFS += -DCVIAUDIO_STATIC
endif

ifeq ($(USE_CVIAUDIO_RTOS_SSP), yes)
DEFS += -DCVIAUDIO_RTOS_SSP
endif

ifeq ($(USE_CVI_AEC), no)
#LDLIB += -laec.v1.0.4
endif

ifeq ($(USE_CVITEK_SOFTWARE_AEC), yes)
DEFS += -DCVIAUDIO_SOFTWARE_AEC
endif

EXTRA_CFLAGS = $(DEFS) $(INCS)
#EXTRA_LDFLAGS = -L$(MW_LIB) -lcvi_AEC $(LDLIB)
EXTRA_LDFLAGS = -L$(MW_LIB) $(LDLIB)
#CFLAGS +=  -Werror
CFLAGS:=$(filter-out -Werror,$(CFLAGS))
#CFLAGS += -Wattributes
.PHONY : clean all
all : $(TARGET_A) $(TARGET_SO)

$(SDIR)/%.o: $(SDIR)/%.c
	$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(EXTRA_DEF) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET_A): $(OBJS)
	@$(AR) $(ARFLAGS) $@ $(OBJS) $(LIB_A_FLAGS)
	@echo -e $(YELLOW)[LINK]$(END)[$(notdir $(AR))] $(notdir $@)

$(TARGET_SO): $(OBJS)
	@$(LD) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $@ --start-group $(OBJS) --end-group
	@echo -e $(GREEN)[LINK]$(END)[$(notdir $(LD))] $(notdir $@)

clean:
	@rm -f $(DEPS) $(OBJS) $(TARGET_A) $(TARGET_SO)

-include $(DEPS)
