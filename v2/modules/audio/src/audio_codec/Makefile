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

SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c)
INCS = -I$(MW_INC) -I$(KERNEL_INC) -I$(COMMON_INC) -I../audio_aacenc -I../audio_aacdec
DEFS = -DUSE_ALGO_SOURCE -DCHIP_CV1835 -DCVI_MODIFIED
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
TARGET_A = $(MW_LIB)/libcvi_VoiceEngine.a
TARGET_SO = $(MW_LIB)/libcvi_VoiceEngine.so

EXTRA_CFLAGS = $(INCS) $(DEFS)
EXTRA_LDFLAGS =

.PHONY : clean all
all : $(OBJS) $(TARGET_A) $(TARGET_SO)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET_A): $(OBJS)
	@$(AR) $(ARFLAGS) $@ $(OBJS)
	@echo -e $(YELLOW)[LINK]$(END)[$(notdir $(AR))] $(notdir $@)

$(TARGET_SO): $(OBJS)
	@$(LD) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $@ --start-group $(OBJS) --end-group
	@echo -e $(GREEN)[LINK]$(END)[$(notdir $(LD))] $(notdir $@)

clean:
	@rm -f $(DEPS) $(OBJS) $(TARGET_A) $(TARGET_SO)

-include $(DEPS)

