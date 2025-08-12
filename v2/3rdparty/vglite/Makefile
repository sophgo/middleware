SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
     PARAM_FILE:=../../Makefile.param
     include $(PARAM_FILE)
endif

VG_LITE_INC = $(MW_INC)/3rd/drm

SDIR = $(PWD)/src
SRCS := $(wildcard $(SDIR)/*.c)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

ifeq ($(SDK_VER), 32bit)
SRCS_ASM = $(wildcard $(SDIR)/*.S)
else ifeq ($(SDK_VER), 64bit)
SRCS_ASM = $(wildcard $(SDIR)/*.S)
else
SRCS_ASM =
endif

OBJS_ASM = $(SRCS_ASM:.S=.o)
DEPS_ASM = $(SRCS_ASM:.S=.d)

TARGET_A = $(MW_3RD_LIB)/libvg_lite.a
TARGET_SO = $(MW_3RD_LIB)/libvg_lite.so

INCS = -I$(MW_INC) -I$(KERNEL_INC) -I$(SYS_INC) -I./inc

EXTRA_CFLAGS = $(INCS) -fpack-struct=8 -Wno-unused-parameter -Wno-missing-braces
EXTRA_CFLAGS += -DMMF_VERSION=\"$(shell git describe --always)\"
EXTRA_CFLAGS += -DSDK_VER=\"$(SDK_VER)\"
EXTRA_CFLAGS += -DENABLE_PCIE
ifeq ($(SDK_VER), 32bit)
EXTRA_CFLAGS += -D_FILE_OFFSET_BITS=64
endif

LDFLAGS  = -shared -fPIC -Wl,--gc-sections -export-dynamic -L$(MW_LIB) -L$(MW_3RD_LIB) --sysroot $(SYSROOT) $(CVI_TARGET_PACKAGES_LIBDIR)
ifeq ($(MULTI_PROCESS_SUPPORT), 1)
EXTRA_CFLAGS += -DRPC_MULTI_PROCESS
endif

.PHONY : clean all
all : $(TARGET_A) $(TARGET_SO)

$(SDIR)/%.o: $(SDIR)/%.c
	@echo $(CC) $(CFLAGS) -o $@ -c $< $(LDFLAGS)
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	#@echo [$(notdir $(CC))] $(notdir $@)
$(SDIR)/%.o: $(SDIR)/%.S
	$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET_A): $(OBJS) $(OBJS_ASM)
	@$(AR) $(ARFLAGS) $(TARGET_A) $(OBJS) $(OBJS_ASM)
	@echo -e $(YELLOW)[LINK]$(END)[$(notdir $(AR))] $(notdir $(TARGET_A))
	@echo "$$AR_MRI" | $(AR) -M

$(TARGET_SO): $(OBJS) $(OBJS_ASM)
	@$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $(TARGET_SO) -Wl,--start-group $(OBJS) $(OBJS_ASM) $(LIBS) -Wl,--end-group
	@echo -e $(GREEN)[LINK]$(END)[$(notdir $(LD))] $(notdir $(TARGET_SO))
	@cp -f inc/vg_lite.h $(MW_3RD_INC)
	@cp -f provider/Vivante_VGLite_Src_tst/samples/util/vg_lite_util.h $(MW_3RD_INC)

clean:
	@rm -rf $(OBJS) $(OBJS_ASM) $(DEPS) $(DEPS_ASM) $(TARGET_SO) $(TARGET_A)
	@rm -rf $(MW_3RD_INC)/vg_lite.h
	@rm -rf $(MW_3RD_INC)/vg_lite_util.h

-include $(DEPS)
