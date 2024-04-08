# SPDX-License-Identifier: GPL-2.0

##############################################################################
# B U I L D    E N V I R O N M E N T
##############################################################################
VERBOSE = @

##############################################################################
# Required definitions
ifndef ROOTDIR_ABS
ROOTDIR_ABS := $(shell pwd)
endif

##############################################################################
# Included makefiles
# Include host/target specific options
-include $(ROOTDIR_ABS)/makefiles/os/linux/configuration.mk

##############################################################################
# Source search paths
VPATH := \
	$(ROOTDIR_ABS)/src \
	$(ROOTDIR_ABS)/src/api \
	$(ROOTDIR_ABS)/src/bsp \
	$(ROOTDIR_ABS)/src/cec \
	$(ROOTDIR_ABS)/src/core \
	$(ROOTDIR_ABS)/src/core/audio \
	$(ROOTDIR_ABS)/src/core/interrupt \
	$(ROOTDIR_ABS)/src/core/color_space \
	$(ROOTDIR_ABS)/src/core/frame_composer \
	$(ROOTDIR_ABS)/src/core/identification \
	$(ROOTDIR_ABS)/src/core/interrupt \
	$(ROOTDIR_ABS)/src/core/main_controller \
	$(ROOTDIR_ABS)/src/core/video \
	$(ROOTDIR_ABS)/src/edid \
	$(ROOTDIR_ABS)/src/hdcp \
	$(ROOTDIR_ABS)/src/identification \
	$(ROOTDIR_ABS)/src/phy \
	$(ROOTDIR_ABS)/src/reg_scan \
	$(ROOTDIR_ABS)/src/scdc \
	$(ROOTDIR_ABS)/src/util \
	$(ROOTDIR_ABS)/src/ipc

##############################################################################
# Header directories
INCL_DIR := \
	$(ROOTDIR_ABS)/src \
	$(ROOTDIR_ABS)/include \
	$(ROOTDIR_ABS)/include/api \
	$(ROOTDIR_ABS)/include/bsp \
	$(ROOTDIR_ABS)/include/cec \
	$(ROOTDIR_ABS)/include/core \
	$(ROOTDIR_ABS)/src/core/audio \
	$(ROOTDIR_ABS)/include/edid \
	$(ROOTDIR_ABS)/include/hdcp \
	$(ROOTDIR_ABS)/include/phy \
	$(ROOTDIR_ABS)/include/reg_scan \
	$(ROOTDIR_ABS)/include/scdc \
	$(ROOTDIR_ABS)/include/util \
	$(ROOTDIR_ABS)/include/ipc \
	$(ROOTDIR_ABS)/include/identification \
	$(ROOTDIR_ABS)/src/core/audio \
	$(ROOTDIR_ABS)/src/core/color_space \
	$(ROOTDIR_ABS)/src/core/frame_composer \
	$(ROOTDIR_ABS)/src/core/interrupt \
	$(ROOTDIR_ABS)/src/core/main_controller \
	$(ROOTDIR_ABS)/src/core/video

##############################################################################
# Source file(s)
SOURCES := $(shell find $(ROOTDIR_ABS)/src/ -name '*.c')
HEADERS := $(shell find $(ROOTDIR_ABS)/include/ -name '*.h')

##############################################################################
# Compiler Options
CFLAGS += \
    $(foreach dir,$(INCL_DIR),-I$(dir)) \
    -g -ggdb -O0\
    -Wall -Werror

##############################################################################
# Linker Options
LDFLAGS += \

##############################################################################
# Output Objects & directories
OBJ_DIR := $(ROOTDIR_ABS)/build

LIB_OBJS  := $(addprefix $(OBJ_DIR)/,$(notdir $(SOURCES:.c=.o)))

LIB_DIR := $(ROOTDIR_ABS)/lib

LIBS := \
	hdmiTx

OUTPUT_LIBS := \
               $(foreach lib,$(LIBS),$(LIB_DIR)/lib$(lib).a)

##############################################################################
# M A K E F I L E    T A R G E T S
##############################################################################
.PHONY : all clean help hdmiTxLib debug
.EXPORT_ALL_VARIABLES:

all:: hdmi_header hdmiTxLib

# Standard compile make target
$(OBJ_DIR)/%.o: %.c
	@echo "Compiling $(notdir $<)"
	$(VERBOSE)$(CC) $(CFLAGS) -c -o $@ $<

hdmi_header:
	@echo "############################################################"
	@echo "# B U I L D I N G    H D M I   T X   A P I   L I B R A R Y #"
	@echo "############################################################"

debug:
	@echo "##############################################################################"
	@echo "# BH D M I   T X   A P I   L I B R A R Y"
	@echo "# ROOTDIR_ABS = $(ROOTDIR_ABS)"
	@echo "# OUTPUT_LIBS = $(OUTPUT_LIBS)"
	@echo "# LIB_DIR  = $(LIB_DIR)"
	@echo "# LIB_OBJS = $(LIB_OBJS)"
	@echo "# OBJ_DIR = $(OBJ_DIR)"
	@echo "# VERBOSE  = $(VERBOSE)"
	@echo "# LDFLAGS  = $(LDFLAGS)"
	@echo "# RM = $(RM)"
	@echo "##############################################################################"

hdmiTxLib: $(OUTPUT_LIBS)

$(LIB_DIR)/%.so: $(LIB_OBJS)
	@echo "Creating shared library $(notdir $@)"
	@echo $(CC)
	$(VERBOSE)$(LD) $(LDFLAGS) -shared -Wl,-soname,$(notdir $@) -o $@ $(LIB_OBJS)

$(LIB_DIR)/%.a: $(OBJ_DIR) $(LIB_DIR) $(LIB_OBJS)
	$(VERBOSE)echo "Creating static library $(notdir $@)"
	@echo $(CC)
	$(VERBOSE)$(AR) $@ $(LIB_OBJS)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(LIB_DIR):
	@mkdir -p $(LIB_DIR)

docs:
	$(VERBOSE)echo "Creating Doxygen documentation"
	cd doc; \
	rm -rf output/*; \
	$(DOXYGEN) Doxyfile; \
	cp -R images output/html; \
	cp -R images output/latex; \
	sed -i -e 's/Module Documentation/API Documentation/' output/latex/refman.tex; \
	make -C output/latex; \
	cd ..

clean:
	@$(RM) $(LIB_OBJS) $(OUTPUT_LIBS)
	@$(RM) *~

indent:
	@echo "Code indentation - kernel style"
	@$(foreach file,$(SOURCES),$(CHMOD) $(file); $(INDENT) $(file);)
	@$(foreach file,$(HEADERS),$(CHMOD) $(file); $(INDENT) $(file);)

help:
	@echo "make		same as make hdmiTxLib"
	@echo "make hdmiTxLib 	Build everything"
	@echo "make help	Print this message"
	@echo "make clean	Delete build artifacts"
	@echo "make debug	Print internal variables"

PREFIX := $(notdir $(TEST_DIR))
