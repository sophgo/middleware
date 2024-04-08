#  ------------------------------------------------------------------------
#
#                    (C) COPYRIGHT 2014 SYNOPSYS, INC.
#                            ALL RIGHTS RESERVED
#
#  This software and the associated documentation are confidential and
#  proprietary to Synopsys, Inc.  Your use or disclosure of this
#  software is subject to the terms and conditions of a written
#  license agreement between you, or your company, and Synopsys, Inc.
#
# The entire notice above must be reproduced on all authorized copies.
#
#  -----------------------------------------------------------------------

#
# Makefile snipet to provide support for gcov tool
#

##############################################################################
# Compiler Options	
CFLAGS += \
	-fprofile-arcs \
	-ftest-coverage

##############################################################################
# Linker Options
LDFLAGS += \
	-fprofile-arcs

##############################################################################
# O U T P U T    A R T I F A C T S
##############################################################################

GCOV_DIR := gcov

GCNO_FILES := $(OBJS:.o=.gcno)
GCDA_FILES := $(OBJS:.o=.gcda)

GCOV_FILES := $(addsuffix .c.gcov,$(basename $(notdir $(wildcard $(OBJ_DIR)/*.gcda))))

##############################################################################
# M A K E F I L E    T A R G E T S
##############################################################################
.PHONY : coverage

%.c.gcov: $(OBJ_DIR)/%.gcda
	$(VERBOSE)gcov -o $(OBJ_DIR) $(basename $@)
	$(VERBOSE)mv $@ $(GCOV_DIR)/$(PREFIX).$@

coverage: $(GCOV_DIR) $(GCOV_FILES)

ifndef GCOV_MKDIR
$(GCOV_DIR):
	$(MKDIR) -p $(GCOV_DIR)
GCOV_MKDIR := 1
endif
	
clean::
	$(VERBOSE)$(RM) $(GCDA_FILES) $(GCNO_FILES) 
	$(VERBOSE)$(RM) $(addprefix $(GCOV_DIR)/$(PREFIX),$(GCOV_FILES))

help::
	@echo "make coverage	Create gcov files"
