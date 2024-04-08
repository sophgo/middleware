# SPDX-License-Identifier: GPL-2.0

# Required definitions
ifndef ROOTDIR_ABS
ROOTDIR_ABS := $(shell pwd)
endif

-include $(ROOTDIR_ABS)/makefiles/utils.mk

##############################################################################
# B U I L D    E N V I R O N M E N T
##############################################################################
VERBOSE = @
MAKE_VERBOSE = -s

##############################################################################
# Included makefiles
-include $(ROOTDIR_ABS)/makefiles/os/linux/configuration.mk 

##############################################################################
# Source search paths
VPATH := \
	$(ROOTDIR_ABS)/src \
	$(ROOTDIR_ABS)/src/app \
	$(ROOTDIR_ABS)/src/app/cmd_line \
	$(ROOTDIR_ABS)/src/app/hdcp \
	$(ROOTDIR_ABS)/src/app/edid \
	$(ROOTDIR_ABS)/src/app/scdc \
	$(ROOTDIR_ABS)/src/app/video \
	$(ROOTDIR_ABS)/src/app/audio \
	$(ROOTDIR_ABS)/src/app/phy \
	$(ROOTDIR_ABS)/src/app/cec \
	$(ROOTDIR_ABS)/src/hdmitx_ipk_api \
	$(ROOTDIR_ABS)/src/hdmitx_ipk_api/audio_bridge \
	$(ROOTDIR_ABS)/src/hdmitx_ipk_api/clock_mng \
	$(ROOTDIR_ABS)/src/hdmitx_ipk_api/hdcp \
	$(ROOTDIR_ABS)/src/hdmitx_ipk_api/phy_interface \
	$(ROOTDIR_ABS)/src/hdmitx_ipk_api/reset_mng \
	$(ROOTDIR_ABS)/src/hdmitx_ipk_api/video_bridge

##############################################################################
# Header directories
INCL_DIR := \
	$(ROOTDIR_ABS)/../api/include \
	$(ROOTDIR_ABS)/../api/include/api \
	$(ROOTDIR_ABS)/../api/include/bsp \
	$(ROOTDIR_ABS)/../api/include/core \
	$(ROOTDIR_ABS)/../api/include/edid \
	$(ROOTDIR_ABS)/../api/include/hdcp \
	$(ROOTDIR_ABS)/../api/include/reg_scan \
	$(ROOTDIR_ABS)/../api/include/phy \
	$(ROOTDIR_ABS)/../api/include/platform \
	$(ROOTDIR_ABS)/../api/include/scdc \
	$(ROOTDIR_ABS)/../api/include/util \
	$(ROOTDIR_ABS) \
	$(ROOTDIR_ABS)/include \
	$(ROOTDIR_ABS)/include/hdmitx_ipk_api/audio_bridge \
	$(ROOTDIR_ABS)/include/hdmitx_ipk_api/clock_mng \
	$(ROOTDIR_ABS)/include/hdmitx_ipk_api/hdcp \
	$(ROOTDIR_ABS)/include/hdmitx_ipk_api/phy_interface \
	$(ROOTDIR_ABS)/include/hdmitx_ipk_api/reset_mng \
	$(ROOTDIR_ABS)/include/hdmitx_ipk_api/video_bridge

ifdef KERNEL_PATH
#INCL_DIR += $(KERNEL_PATH)/include
endif

#############################################################################
# Source file(s)
SOURCES := $(shell find $(ROOTDIR_ABS)/src/ -name '*.c')
HEADERS := $(shell find $(ROOTDIR_ABS)/include/ -name '*.h')

##############################################################################
# Compiler Options
CFLAGS += \
    $(foreach dir,$(INCL_DIR),-I$(dir)) \
    -g \
    -Wall -error

LDFLAGS += -static

##############################################################################
# Linker Options
LIB_DIR := \
	-L $(ROOTDIR_ABS)/../api/lib



STATIC_LIBS := -lhdmiTx

##############################################################################
# Output Objects & directories
OBJ_DIR := $(ROOTDIR_ABS)/build

UTIL_OBJS := $(addprefix $(OBJ_DIR)/,$(notdir $(SOURCES:.c=.o)))

#LIB_DIR := $(ROOTDIR_ABS)/lib

BIN_DIR := $(ROOTDIR_ABS)/bin

UTIL_PROGNAME := $(BIN_DIR)/HDMITX$(PROG_EXT)

##############################################################################
# M A K E F I L E    T A R G E T S
##############################################################################
.PHONY: all clean help debug
.EXPORT_ALL_VARIABLES:

all:: $(UTIL_PROGNAME)

# Standard compile make target
$(OBJ_DIR)/%.o: %.c
	@echo "Compiling $(notdir $<)"
	@echo $(CC)
	$(VERBOSE)$(CC) $(CFLAGS) -o $@ -c $<

hdmi_header:
	@echo "############################################################"
	@echo "# B U I L D I N G    H D M I   T X   A P I   L I B R A R Y #"
	@echo "############################################################"

debug:
	@echo "##############################################################################"
	@echo "# H D M I   T X   A P I   L I B R A R Y"
	@echo "# ROOTDIR_ABS = $(ROOTDIR_ABS)"
	@echo "# SOURCES     = $(SOURCES)"
	@echo "# OBJ_DIR     = $(OBJ_DIR)"
	@echo "# LDFLAGS     = $(LDFLAGS)"
	@echo "# C R O S S    C O M P I L E R "
	@echo "# CC = $(CC)"
	@echo "# RM = $(RM)"
	@echo "# LD = $(LD)"
	@echo "# L Y B R A R I E S "
	@echo "# LIB_DIR     = $(LIB_DIR)"
	@echo "# STATIC_LIBS = $(STATIC_LIBS)"
	@echo "##############################################################################"

$(UTIL_PROGNAME): $(OBJ_DIR) $(BIN_DIR) $(UTIL_OBJS) $(ROOTDIR_ABS)/include/
	@echo "Generate binary: $@"
	@$(CC) -static -o $@ $(UTIL_OBJS) $(LIB_DIR) $(STATIC_LIBS) $(ELFFLAGS)-lpthread

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

clean::
	$(VERBOSE)$(RM) $(UTIL_OBJS)
	$(VERBOSE)$(RM) $(UTIL_PROGNAME)
	$(VERBOSE)$(RM) *~

help::
	@echo "make		same as make all"
	@echo "make help	Print this message"
	@echo "make all 	Build everything"
	@echo "make clean	Delete build artifacts"
	@echo "make debug	Print internal variables"

PREFIX := $(notdir $(TEST_DIR))
