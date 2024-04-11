SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif
include ../self_test.mk

SDIR = $(PWD)
#SRCS = $(wildcard $(SDIR)/*.c)
SRCS := $(SDIR)/dpu_ut.c $(SDIR)/dpu_ut_comm.c
INCS = -I$(MW_INC) -I$(ISP_INC) -I$(KERNEL_INC) -I$(SYS_INC)

SRCS_CLNT := $(SDIR)/dpu_ut_client.c $(SDIR)/dpu_ut_comm.c
OBJS_CLNT = $(SRCS_CLNT:.c=.o)
DEPS_CLNT = $(SRCS_CLNT:.c=.d)

OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)


TARGET = dpu_ut
LIBS = -ldpu -lsys

EXTRA_CFLAGS += $(DEFS) $(INCS)
EXTRA_CFLAGS += -ffunction-sections -fdata-sections

ldflags-y += $(LIBS) -lpthread -lm -lini -latomic

ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
PKG_CONFIG_PATH = $(MW_PATH)/pkgconfig
REQUIRES = cvi_common cvi_vdec
LIBS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs --define-variable=mw_dir=$(MW_PATH) cvi_common)
else
ELFFLAGS += -static
endif

#all : dpu_ut dpu_ut_client
all : dpu_ut
$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

	#/mnt/workspace/mars_daily_32_2/host-tools/gcc/gcc-linaro-12.0.0-2021.11-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc -fanalyzer -fdump-analyzer-callgraph $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

dpu_ut : $(OBJS) $(MW_LIB)/libdpu.a $(MW_LIB)/libsys.a
	@$(CXX) -o $@ -Wl,--start-group $(OBJS) $(MW_LIB)/libdpu.a $(MW_LIB)/libsys.a -Wl,--end-group $(ELFFLAGS) $(ldflags-y)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CC))] $(notdir $@)
	@cp $@ ../auto_test/

dpu_ut_client : $(OBJS_CLNT) $(MW_LIB)/libdpu.a $(MW_LIB)/libsys.a
	@$(CXX) -o $@ -Wl,--start-group $(OBJS_CLNT) $(MW_LIB)/libdpu.a $(MW_LIB)/libsys.a -Wl,--end-group $(ELFFLAGS) $(ldflags-y)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CC))] $(notdir $@)
	@cp $@ ../auto_test/

.PHONY : clean
clean:
	@rm -f $(OBJS) $(OBJS_CLNT) $(DEPS) $(DEPS_CLNT) $(COMM_DEPS) dpu_ut dpu_ut_client

-include $(DEPS)
