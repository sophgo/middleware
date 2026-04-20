SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif
include ../sample.mk

SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c)
INCS = -I$(MW_INC) -I$(ISP_INC) -I$(VI_INC) -I$(KERNEL_INC) $(COMM_INC) -I$(PANEL_INC)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

PKG_CONFIG_PATH = $(MW_PATH)/pkgconfig
REQUIRES = cvi_common cvi_vdec cvi_misc
MW_LIBS = $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs --define-variable=mw_dir=$(MW_PATH) $(REQUIRES))

TARGET = sample_fisheye
ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
TARGET = sample_fisheye_asan
endif

LIBS = $(MW_LIBS)
ifeq ($(MULTI_PROCESS_SUPPORT), 1)
DEFS += -DRPC_MULTI_PROCESS
endif

CFLAGS += -Wno-format-truncation
EXTRA_CFLAGS = $(INCS) $(DEFS)
EXTRA_LDFLAGS = $(LIBS) -lpthread -lm -lini

.PHONY : clean all
all: $(TARGET)

$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET): $(COMM_OBJ) $(OBJS) $(ISP_OBJ) $(MW_LIB)/libsys.a $(MW_LIB)/libgdc.a
	@$(CXX) -o $@ $(OBJS) $(COMM_OBJ) $(ELFFLAGS) $(EXTRA_LDFLAGS) $(MW_LIB)/libgdc.a
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)
	@cp $@ $(INSTALL_MW_DIR)/
	@echo "Installed $@ to $(INSTALL_MW_DIR)"
clean:
	@rm -f $(OBJS) $(DEPS) $(COMM_OBJ) $(COMM_DEPS) $(TARGET)

-include $(DEPS)
