SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif

include ../self_test.mk
include ../../modules/isp/isp.mk

SDIR = $(PWD)
VENC_DIR = $(SDIR)/../../sample/venc/src
SENSOR_TEST_DIR = $(SDIR)/../../sample/sensor_test/src
ISP_PATH := $(SDIR)/../../modules/isp/$(isp_chip_dir)
ISP_DIR = $(ISP_PATH)/isp_test
SRCS = $(wildcard $(SDIR)/*.c) $(wildcard $(VENC_DIR)/*.c) $(wildcard $(ISP_DIR)/*.c)
SRCS += $(wildcard $(SENSOR_TEST_DIR)/*.c)
ifeq ($(CHIP_ARCH), $(filter $(CHIP_ARCH), CV183X))
	SRCS := $(filter-out $(SDIR)/vpss_test_1822.c, $(SRCS))
	SRCS := $(filter-out $(SDIR)/vpss_test_mars.c, $(SRCS))
	INCS = -I$(ISP_PATH)/src
else ifeq ($(CHIP_ARCH), $(filter $(CHIP_ARCH), CV182X))
	SRCS := $(filter-out $(SDIR)/vpss_test.c, $(SRCS))
	SRCS := $(filter-out $(SDIR)/vpss_test_mars.c, $(SRCS))
	INCS = -I$(ISP_PATH)/isp_algo/inc -I$(ISP_PATH)/isp/src
else ifeq ($(CHIP_ARCH), $(filter $(CHIP_ARCH), CV180X CV181X CV186X))
	SRCS := $(filter-out $(SDIR)/vpss_test.c, $(SRCS))
	SRCS := $(filter-out $(SDIR)/vpss_test_1822.c, $(SRCS))
	INCS = -I$(ISP_PATH)/isp_algo/inc -I$(ISP_PATH)/isp/src
endif
INCS += -I$(MW_INC) -I$(ISP_INC) -I$(VI_INC) -I$(COMM_INC) -I$(ISP_PATH)/include -I$(ISP_PATH)/isp/inc -I$(ISP_PATH)/isp_test -I$(KERNEL_INC) -I$(ISP_PATH) -I$(ISP_PATH)/../algo -I../../sample/venc/include
INCS += -I$(ISP_PATH)/../common/clog
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

TARGET = cvi_test
ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
TARGET = cvi_test_asan
endif

ifeq ($(SUBTYPE), fpga)
LIBS = -lvi -lvenc -lvdec $(ISP_LIB) -lvpss -lvo -lrgn -lgdc -lsys $(SNS_LIB) -lmisc
else
LIBS = -lvi -lvenc -lvdec $(ISP_LIB) -lvpss -lvo -lrgn -lgdc -lsys $(SNS_LIB) -lmisc
endif
ifeq ($(MULTI_PROCESS_SUPPORT), 1)
DEFS += -DRPC_MULTI_PROCESS
endif

EXTRA_CFLAGS = $(DEFS) $(INCS)
EXTRA_LDFLAGS = $(LIBS) -lini -lpthread -latomic
EXTRA_LDFLAGS += -lcvi_bin -lcvi_bin_isp

EXTRA_LDFLAGS_WITH_ASAN = -lasan

ifeq ($(SELF_TEST_STATIC), 1)
ELFFLAGS += -static
endif

ifeq ($(DEBUG), 1)
EXTRA_LDFLAGS += -g -O0
endif

ifeq ($(MTRACE), y)
EXTRA_LDFLAGS += -g
endif

.PHONY : clean all
all: $(TARGET)

$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET): $(COMM_OBJ) $(OBJS) $(ISP_OBJ) $(MW_LIB)/libsys.a
	@$(CXX) -o $@ $(OBJS) $(COMM_OBJ) $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)
	@cp $@ ../auto_test/cvi_test

clean:
	@rm -f $(OBJS) $(DEPS) $(COMM_DEPS) $(TARGET)

-include $(DEPS)
