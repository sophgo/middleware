SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif
include ../sample.mk

SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c)
INCS = -I$(MW_INC) -I$(ISP_INC) -I$(VI_INC) -I$(KERNEL_INC) $(COMM_INC)
INCS += -I$(TOP_DIR)/middleware/v2/modules/isp/$(shell echo $(CHIP_ARCH) | tr A-Z a-z)/isp_algo/inc

OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

TARGET = dpc_cali

PKG_CONFIG_PATH = $(MW_PATH)/pkgconfig
REQUIRES = cvi_sample

MW_LIBS = $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs --define-variable=mw_dir=$(MW_PATH) $(REQUIRES))

LIBS = $(MW_LIBS)
ifeq ($(MULTI_PROCESS_SUPPORT), 1)
DEFS += -DRPC_MULTI_PROCESS
endif

EXTRA_CFLAGS = $(INCS) $(DEFS)
EXTRA_LDFLAGS = $(LIBS) -lpthread -lm -lini
EXTRA_LDFLAGS += -lisp_algo

.PHONY : clean all
all: $(TARGET)

$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET): $(COMM_OBJ) $(OBJS) $(ISP_OBJ) $(MW_LIB)/libsys.a
	@$(CXX) -o $@ -Wl,--start-group $(OBJS) $(COMM_OBJ) $(COMM_OBJS) -lsys $(MW_LIB)/libsys.a -Wl,--end-group $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)
	@cp $@ $(INSTALL_MW_DIR)/
	@echo "Installed $@ to $(INSTALL_MW_DIR)"
clean:
	@rm -f $(OBJS) $(DEPS) $(COMM_OBJ) $(COMM_DEPS) $(TARGET)

-include $(DEPS)
