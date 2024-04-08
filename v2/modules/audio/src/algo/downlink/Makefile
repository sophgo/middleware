SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
ifeq ($(MW_VER), )
PARAM_FILE:=../../../../../Makefile.param
include $(PARAM_FILE)
else
PARAM_FILE:=../../../../../../$(shell echo $(MW_VER))/Makefile.param
include $(PARAM_FILE)
endif
endif

AUDIO_PARAM_FILE:=../../../audio.mk
include $(AUDIO_PARAM_FILE)


SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c)
INCS = -I. -I$(MW_INC) -I$(KERNEL_INC) -I$(AUD_INC)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
TARGET_A = $(MW_LIB)/libcvi_ssp2.a
TARGET_SO = $(MW_LIB)/libcvi_ssp2.so
SAMPLE_FOLDER = $(MW_LIB)/../sample/audio/

DEFS =
ifeq ($(USE_HEAP_REPLACE_STACK), yes)
DEFS += -DSSP_STACK_TO_HEAP
endif

LOCAL_CFLAGS =  -fgnu89-inline -Wno-unused-variable -Wuninitialized $(DEFS)
EXTRA_CFLAGS = $(INCS)
EXTRA_LDFLAGS =
#EXTRA_CFLAGS += -DFAST_MATH_CALC
.PHONY : clean all
all : $(TARGET_A) $(TARGET_SO)

$(SDIR)/%.o: $(SDIR)/%.c
	@echo $(PWD)
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(LOCAL_CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET_A): $(OBJS)
	@$(AR) $(ARFLAGS) $@ $(OBJS)
	@echo -e $(YELLOW)[LINK]$(END)[$(notdir $(AR))] $(notdir $@)

$(TARGET_SO): $(OBJS)
	@$(LD) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $@ --start-group $(OBJS) --end-group
	@echo -e $(GREEN)[LINK]$(END)[$(notdir $(LD))] $(notdir $@)
#	@$(CC) $(DEPFLAGS) $(CFLAGS) $(LOCAL_CFLAGS) $(EXTRA_CFLAGS) ssp_main.c  -o ssp_Test0128  -lcvi_ssp128 -L$(MW_LIB) -lm -static

clean:
	@rm -f $(DEPS) $(OBJS) $(TARGET_A) $(TARGET_SO) ssp_Test*

-include $(DEPS)