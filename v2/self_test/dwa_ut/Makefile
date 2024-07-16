SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif
include ../self_test.mk

SDIR = $(PWD)
#SRCS = $(wildcard $(SDIR)/*.c)
SRCS := $(SDIR)/dwa_ut.c $(SDIR)/dwa_ut_comm.c
INCS = -I$(MW_INC) -I$(ISP_INC) -I$(VI_INC) -I$(KERNEL_INC) -I$(SYS_INC)

OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)


TARGET = dwa_ut
LIBS = -ldwa -lsys

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

all : dwa_ut
#dwa_ut_client

$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

dwa_ut : $(OBJS) $(MW_LIB)/libsys.a
	@$(CXX) -o $@ -Wl,--start-group $(OBJS) $(MW_LIB)/libsys.a -Wl,--end-group $(ELFFLAGS) $(ldflags-y)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CC))] $(notdir $@)
	@cp $@ ../auto_test/

#dwa_ut_client : $(OBJS_CLNT) $(MW_LIB)/libvpu.a $(MW_LIB)/libsys.a
	#@$(CXX) -o $@ -Wl,--start-group $(OBJS_CLNT) $(MW_LIB)/libsys.a -Wl,--end-group $(ELFFLAGS) $(ldflags-y)
	#@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CC))] $(notdir $@)
	#@cp $@ ../auto_test/

.PHONY : clean
clean:
	@rm -f $(OBJS) $(DEPS) $(COMM_DEPS) dwa_ut

-include $(DEPS)
