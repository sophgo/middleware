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
SRCS = $(wildcard $(SDIR)/*.c)
INCS = -I$(MW_INC) -I$(KERNEL_INC) -I$(COMMON_INC) -I .$(PWD) -I$(AUD_INC)
DEFS = -DCVI_MODIFIED
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
TARGET_A = $(MW_LIB)/libcvi_dnvqe.a
TARGET_SO = $(MW_LIB)/libcvi_dnvqe.so

INCS += -I../../include
DEFS += -DUSE_CVI_AEC

ifeq ($(USE_CVIAUDIO_STATIC), yes)
DEFS += -DCVIAUDIO_STATIC
endif


EXTRA_CFLAGS = $(DEFS) $(INCS)
EXTRA_LDFLAGS = -L$(MW_LIB) $(LDLIB)

.PHONY : clean all
all : $(TARGET_A) $(TARGET_SO)

$(SDIR)/%.o: $(SDIR)/%.c
	@echo $(AUD_INC)
	@echo $(MW_INC)
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET_A): $(OBJS)
	echo $(AUD_INC)
	@$(AR) $(ARFLAGS) $@ $(OBJS) $(LIB_A_FLAGS)
	@echo -e $(YELLOW)[LINK]$(END)[$(notdir $(AR))] $(notdir $@)

$(TARGET_SO): $(OBJS)
	@$(LD) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $@ --start-group $(OBJS) --end-group
	@echo -e $(GREEN)[LINK]$(END)[$(notdir $(LD))] $(notdir $@)

clean:
	@rm -f $(DEPS) $(OBJS) $(TARGET_A) $(TARGET_SO)

-include $(DEPS)
