SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
     PARAM_FILE:=../../Makefile.param
     include $(PARAM_FILE)
endif

INCS = -I$(MW_INC) -I$(KERNEL_INC) -I$(SYS_INC) -I$(PWD)/include

VD_SDIR = $(PWD)/src/vdec
VD_SRCS = $(wildcard $(VD_SDIR)/*.c)
VD_OBJS = $(VD_SRCS:.c=.o)
VD_DEPS = $(VD_SRCS:.c=.d)
VD_TARGET_A = $(MW_LIB)/libvdec.a
VD_TARGET_SO = $(MW_LIB)/libvdec.so

VE_SDIR = $(PWD)/src/venc
VE_SRCS = $(wildcard $(VE_SDIR)/*.c)
VE_OBJS = $(VE_SRCS:.c=.o)
VE_DEPS = $(VE_SRCS:.c=.d)
VE_TARGET_A = $(MW_LIB)/libvenc.a
VE_TARGET_SO = $(MW_LIB)/libvenc.so

EXTRA_CFLAGS = $(INCS)

ifeq ($(DEBUG), 1)
EXTRA_CFLAGS += -g -O0
endif

.PHONY : clean all
all : prepare $(VD_TARGET_A) $(VD_TARGET_SO) $(VE_TARGET_A) $(VE_TARGET_SO)

$(VD_SDIR)/%.o: $(VD_SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(VD_TARGET_A): $(VD_OBJS)
	@$(AR) $(ARFLAGS) $@ $(VD_OBJS)
	@echo -e $(YELLOW)[LINK]$(END)[$(notdir $(AR))] $(notdir $@)

$(VD_TARGET_SO): $(VD_OBJS)
	@$(LD) $(LDFLAGS) $(LEXTRA_LDFLAGS) -o $@ --start-group $(VD_OBJS) $(LIBS) --end-group
	@echo -e $(GREEN)[LINK]$(END)[$(notdir $(LD))] $(notdir $@)

$(VE_SDIR)/%.o: $(VE_SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(VE_TARGET_A): $(VE_OBJS)
	@$(AR) $(ARFLAGS) $@ $(VE_OBJS)
	@echo -e $(YELLOW)[LINK]$(END)[$(notdir $(AR))] $(notdir $@)

$(VE_TARGET_SO): $(VE_OBJS)
	@$(LD) $(LDFLAGS) $(LEXTRA_LDFLAGS) -o $@ --start-group $(VE_OBJS) $(LIBS) --end-group
	@echo -e $(GREEN)[LINK]$(END)[$(notdir $(LD))] $(notdir $@)

clean:
	@rm -f $(VD_OBJS) $(VD_DEPS) $(VD_TARGET_A) $(VD_TARGET_SO)
	@rm -f $(VE_OBJS) $(VE_DEPS) $(VE_TARGET_A) $(VE_TARGET_SO)

prepare:
	@echo "SDK_VER = ${SDK_VER}"

-include $(VD_DEPS) $(VE_DEPS)
