SHELL = /bin/bash
include ./Makefile.param
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=$(MW_DIR)/Makefile.param
	include $(PARAM_FILE)
endif
include ../../modules/isp/isp.mk

SNS_LIB = -lsns_full

COMMON_DIR ?= $(MW_DIR)/sample/common

CFLAGS += -DSENSOR0_TYPE=$(SENSOR0_TYPE)
CFLAGS += -DSENSOR1_TYPE=$(SENSOR1_TYPE)

ifeq ($(DEBUG), 1)
CFLAGS += -g -O0
endif

ifeq ($(SELF_TEST_STATIC), 1)
ELFFLAGS += -static
endif

#########################################################################
COMM_SRC := $(wildcard $(COMMON_DIR)/*.c)
COMM_OBJ := $(COMM_SRC:%.c=%.o)
COMM_INC := $(COMMON_DIR)
COMM_DEPS = $(COMM_SRC:.c=.d)

#$(info $(MW_INC))

SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/app/src/*.c)
PANEL_INC =../../component/panel/$(shell echo $(CHIP_ARCH) | tr A-Z a-z)
INCS = -I$(MW_INC) -I$(ISP_INC) -I../common/ -I$(KERNEL_INC) -I$(COMM_INC) -I$(PANEL_INC)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

TARGET = ipc_test
ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
TARGET = ipc_test_asan
endif

PKG_CONFIG_PATH = $(MW_PATH)/pkgconfig
REQUIRES = cvi_common cvi_sample
MW_LIBS = $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs --define-variable=mw_dir=$(MW_PATH) $(REQUIRES))

LIBS += $(MW_LIBS)

ifeq ($(MULTI_PROCESS_SUPPORT), 1)
DEFS += -DRPC_MULTI_PROCESS
endif

EXTRA_CFLAGS = $(INCS) $(DEFS) -Wno-unused-parameter
EXTRA_LDFLAGS = $(LIBS) -lpthread -lm -lini

EXTRA_LDFLAGS_WITH_ASAN = -lasan

#disable rtsp in uclibc version
ifneq ($(SDK_VER),uclibc)
INCS += -I./rtsp/include
LIBS += -L./rtsp/lib$(SDK_VER:bit=) -lcvi_rtsp -lliveMedia -lgroupsock -lBasicUsageEnvironment -lUsageEnvironment
#LIBS += -L./rtsp/lib -Wl,-Bdynami -lcvi_rtsp
EXTRA_CFLAGS += -D__ENABLE_RTSP__
endif

.PHONY : clean all install
#all : $(TARGET)

all:

$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET) : $(COMM_OBJ) $(OBJS) $(MW_LIB)/libisp.a $(MW_LIB)/libvpu.a $(MW_LIB)/libsys.a
	@$(CXX) -o $@ $(OBJS) $(COMM_OBJ) $(COMM_OBJS) $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@$(STRIP) $@
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

clean:
	@rm -f $(OBJS) $(DEPS) $(COMM_OBJ) $(COMM_DEPS) $(TARGET)

install:
	mkdir install -p
	cp $(TARGET) ./install
	mkdir install/lib -p
	cp rtsp/lib/*.so ./install/lib

-include $(DEPS)
