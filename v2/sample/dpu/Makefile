SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif
include ../sample.mk

#SYS_INC:= "/media/cvitek/zhongbin.wang/athena2_0908/middleware/v2/modules/sys/include"

SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c)
INCS = -I$(MW_INC) -I$(ISP_INC) -I$(SYS_INC) -I../common/ -I$(KERNEL_INC)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

TARGET = sample_dpu
ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
TARGET = sample_dpu_asan
endif

PKG_CONFIG_PATH = $(MW_PATH)/pkgconfig
REQUIRES = cvi_common cvi_sample

#MW_LIBS = $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs --define-variable=mw_dir=$(MW_PATH) $(REQUIRES))

#LIBS = $(MW_LIBS)
ifeq ($(MULTI_PROCESS_SUPPORT), 1)
DEFS += -DRPC_MULTI_PROCESS
LIBS += -lnanomsg
endif

EXTRA_CFLAGS = $(INCS) $(DEFS)
EXTRA_LDFLAGS = $(LIBS) -lpthread -lm


.PHONY : clean all
all: $(TARGET)
	@echo $(MW_INC)

$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET):  $(OBJS) $(MW_LIB)/libdpu.a $(MW_LIB)/libsys.a $(MW_LIB)/libdwa.a
	@$(CXX) -o $@ -Wl,--start-group $(OBJS)  $(MW_LIB)/libdpu.a $(MW_LIB)/libsys.a $(MW_LIB)/libdwa.a -Wl,--end-group $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

clean:
	@rm -f $(OBJS) $(DEPS) $(COMM_OBJ) $(COMM_DEPS) $(TARGET)

-include $(DEPS)
