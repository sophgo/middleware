SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif
include ../self_test.mk
include $(MW_PATH)/modules/isp/isp.mk

SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c)
INCS = -I$(MW_INC) -I$(ISP_INC) -I$(VI_INC) -I../../sample/common -I$(KERNEL_INC)
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

TARGET = vip_fpga
LIBS = -lvpu $(ISP_LIB) -lsys -lvenc $(SNS_LIB)
ifeq ($(MULTI_PROCESS_SUPPORT), 1)
DEFS += -DRPC_MULTI_PROCESS
endif

EXTRA_CFLAGS += $(DEFS) $(INCS)
EXTRA_LDFLAGS += $(LIBS) -lpthread -lm -lini
EXTRA_LDFLAGS += -lcvi_bin -lcvi_bin_isp

all :
#all : $(TARGET)

$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET) : $(COMM_OBJ) $(OBJS) $(ISP_OBJ) $(MW_LIB)/libvpu.a $(MW_LIB)/libsys.a
	@$(CXX) -o $@ -Wl,--start-group $(OBJS) $(COMM_OBJ) $(MW_LIB)/libsys.a -Wl,--end-group $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

.PHONY : clean
clean:
	@rm -f $(OBJS) $(DEPS) $(COMM_OBJ) $(COMM_DEPS) $(TARGET)

-include $(DEPS)
