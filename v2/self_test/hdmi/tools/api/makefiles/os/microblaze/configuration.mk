#  ------------------------------------------------------------------------
#
#                    (C) COPYRIGHT 2014 SYNOPSYS, INC.
#                            ALL RIGHTS RESERVED
#
# This software and the associated  documentation are proprietary to Synopsys,
# Inc. This software may only be used in accordance with the terms and 
# conditions of a written license agreement with Synopsys, Inc. All other use,
# reproduction, or distribution of this software is strictly prohibited.
#
# The entire notice above must be reproduced on all authorized copies.
#
#  -----------------------------------------------------------------------

# Utilities
CROSS_COMPILE := mb-
CC    := ${CROSS_COMPILE}gcc
CPP   := ${CROSS_COMPILE}g++
LD    := ${CROSS_COMPILE}gcc
AR    := ${CROSS_COMPILE}ar rcs
RM    := rm -Rf
CP    := cp
MKDIR := mkdir -p
CHMOD := chmod 644
INDENT := indent -nbad -bap -nbc -bbo -hnl -br -brs -c33 -cd33 -ncdb -ce -ci4 -cli0 -d0 -di1 -nfc1 -i8 -ip0 -l80 -lp -npcs -nprs -npsl -sai -saf -saw -ncs -nsc -sob -nfca -cp33 -ss -ts8 -il1

# Optional Utilities
SWIG  := swig
TCLSH := tclsh
DOXYGEN := doxygen

# Compiler Options
CFLAGS += \
		-c -fmessage-length=0 -fno-strict-aliasing -D __XMK__ \
		  -mxl-pattern-compare -mcpu=v7.30.a -mno-xl-soft-mul -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)"

# Linker Options
#LDFLAGS += \

# Output options
#PROG_EXT = 
SHARED_EXT = .so

# Export all variables
export


