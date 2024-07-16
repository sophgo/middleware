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
INCS = -I$(MW_INC) -I$(ISP_INC) -I../../sample/common -I../../sample/sample_panel -I$(KERNEL_INC) -I$(PANEL_INC) -I$(SYS_INC)
INCS += -I../../modules/vo/include
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

COMM_SRC := $(wildcard $(INIH_DIR)/*.c)
COMM_SRC += $(COMMON_DIR)/sample_common_peripheral.c
COMM_SRC += $(COMMON_DIR)/sample_common_platform.c
COMM_SRC += $(COMMON_DIR)/sample_common_sys.c
COMM_SRC += $(COMMON_DIR)/sample_common_isp.c
COMM_SRC += $(COMMON_DIR)/sample_common_vi.c
COMM_SRC += $(COMMON_DIR)/sample_common_vo.c
COMM_SRC += $(COMMON_DIR)/sample_common_vpss.c
COMM_OBJ := $(COMM_SRC:%.c=%.o)

TARGET = vo_ut
LIBS = -lvo -lvpss $(ISP_LIB) -lsys $(SNS_LIB)
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

ifeq ("$(CVIARCH)", "CV180X")
all :
else
all : $(TARGET)
endif

$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET) : $(COMM_OBJ) $(OBJS) $(ISP_OBJ) $(MW_LIB)/libsys.a
	@$(CXX) -o $@ -Wl,--start-group $(OBJS) $(COMM_OBJ) $(MW_LIB)/libsys.a -Wl,--end-group $(ELFFLAGS) $(ldflags-y)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)
	@cp ../../sample/sample_panel/sample_panel ../auto_test/
	@cp $(TARGET) ../auto_test/

.PHONY : clean
clean:
	@rm -f $(OBJS) $(DEPS) $(COMM_OBJ) $(COMM_DEPS) $(TARGET)

-include $(DEPS)
