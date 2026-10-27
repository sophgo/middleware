SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif
include ../sample.mk

SDIR = $(PWD)
SRCS_HDMI = $(SDIR)/sample_hdmi.c
INCS = -I$(MW_INC) -I$(ISP_INC) -I$(VI_INC) -I$(KERNEL_INC) $(COMM_INC)
OBJS_HDMI = $(SRCS_HDMI:.c=.o)
DEPS = $(SRCS_HDMI:.c=.d)

TARGET = sample_hdmi
ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
TARGET = sample_hdmi_asan
endif

PKG_CONFIG_PATH = $(MW_PATH)/pkgconfig
REQUIRES = cvi_common cvi_sample

MW_LIBS = $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs --define-variable=mw_dir=$(MW_PATH) $(REQUIRES))

LIBS = $(MW_LIBS)
EXTRA_CFLAGS = $(INCS) $(DEFS)
EXTRA_LDFLAGS = $(LIBS) -lpthread -lm

.PHONY : clean all
all: $(TARGET)

$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET): $(COMM_OBJ) $(OBJS_HDMI) $(ISP_OBJ) $(MW_LIB)/libsys.a $(MW_LIB)/libhdmitx.a
	@$(CXX) -o $@ -Wl,--start-group $(OBJS_HDMI) $(COMM_OBJ) $(COMM_OBJS) -lsys $(MW_LIB)/libhdmitx.a $(MW_LIB)/libsys.a -Wl,--end-group $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)
	@if [ -d "$(INSTALL_MW_DIR)" ]; then cp $@ $(INSTALL_MW_DIR)/ && echo "Installed $@ to $(INSTALL_MW_DIR)"; fi
clean:
	@rm -f $(OBJS_HDMI) $(OBJS_AUDIO_SEND) $(DEPS) $(COMM_OBJ) $(COMM_DEPS) $(TARGET) $(TARGET_AUDIO_SEND)

-include $(DEPS)
