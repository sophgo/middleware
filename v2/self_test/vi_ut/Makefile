SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif
include ../self_test.mk
include $(MW_PATH)/modules/isp/isp.mk

PANEL_INC = ../../component/panel/$(shell echo $(CHIP_ARCH) | tr A-Z a-z)
SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c)
SRCS := $(filter-out $(SDIR)/vi_ut_client.c, $(SRCS))
INCS = -I$(MW_INC) -I$(ISP_INC) -I../../sample/common -I$(KERNEL_INC) -I$(PANEL_INC) -I$(SYS_INC)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

SRCS_CLNT := $(SDIR)/vi_ut_client.c
SRCS_CLNT += $(SDIR)/vi_ut_dump.c
OBJS_CLNT = $(SRCS_CLNT:.c=.o)
DEPS_CLNT = $(SRCS_CLNT:.c=.d)

COMM_SRC := $(wildcard $(INIH_DIR)/*.c)
COMM_SRC += $(COMMON_DIR)/sample_common_peripheral.c
COMM_SRC += $(COMMON_DIR)/sample_common_platform.c
COMM_SRC += $(COMMON_DIR)/sample_common_sys.c
COMM_SRC += $(COMMON_DIR)/sample_common_isp.c
COMM_SRC += $(COMMON_DIR)/sample_common_vi.c
COMM_SRC += $(COMMON_DIR)/sample_common_bin.c
COMM_SRC += $(COMMON_DIR)/sample_common_vpss.c
COMM_OBJ := $(COMM_SRC:%.c=%.o)

TARGET = vi_ut
TARGET_CLNT = vi_ut_client
LIBS = -lvi -lvpss $(ISP_LIB) -lsys $(SNS_LIB) -lmisc -lcvi_bin -lcvi_bin_isp
ifeq ($(MULTI_PROCESS_SUPPORT), 1)
DEFS += -DRPC_MULTI_PROCESS
endif

ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
PKG_CONFIG_PATH = $(MW_PATH)/pkgconfig
LIBS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs --define-variable=mw_dir=$(MW_PATH) cvi_common cvi_sample)
else ifeq ($(SELF_TEST_STATIC), 1)
ELFFLAGS += -static
endif


ifeq ($(DEBUG), 1)
ldflags-y += -g -O0
endif

ifeq ($(MTRACE), y)
ldflags-y += -g
endif

EXTRA_CFLAGS += $(DEFS) $(INCS)
ldflags-y += $(LIBS) -lpthread -lm -lini -latomic

all : $(TARGET) $(TARGET_CLNT)

$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET) : $(COMM_OBJ) $(OBJS) $(ISP_OBJ) $(MW_LIB)/libsys.a
	@$(CXX) -o $@ -Wl,--start-group $(OBJS) $(COMM_OBJ) $(MW_LIB)/libsys.a -Wl,--end-group $(ELFFLAGS) $(ldflags-y)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)
	@cp $@ ../auto_test/

$(TARGET_CLNT) : $(COMM_OBJ) $(OBJS_CLNT) $(OBJS) $(ISP_OBJ) $(MW_LIB)/libsys.a
	@$(CXX) -o $@ -Wl,--start-group $(OBJS_CLNT) $(COMM_OBJ) $(MW_LIB)/libsys.a -Wl,--end-group $(ELFFLAGS) $(ldflags-y)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)
	@cp $@ ../auto_test/

.PHONY : clean
clean:
	@rm -f $(OBJS) $(DEPS) $(COMM_OBJ) $(COMM_DEPS) $(TARGET) $(OBJS_CLNT) $(DEPS_CLNT) $(TARGET_CLNT)

-include $(DEPS)
