# SPDX-License-Identifier: GPL-2.0
CROSS_COMPILE=aarch64-linux-gnu-
# Utilities
CC    		:= $(CROSS_COMPILE)gcc
CPP   		:= $(CROSS_COMPILE)g++
LD    		:= $(CROSS_COMPILE)ld
AR    		:= $(CROSS_COMPILE)ar
RM    		:= rm -Rf
CP    		:= cp
MKDIR 		:= mkdir -p
CHMOD 		:= chmod 755
INDENT 		:= indent -nbad -bap -nbc -bbo -hnl -br -brs -c33 -cd33 -ncdb -ce -ci4 -cli0 -d0 -di1 -nfc1 -i8 -ip0 -l80 -lp -npcs -nprs -npsl -sai -saf -saw -ncs -nsc -sob -nfca -cp33 -ss -ts8 -il1

# Optional Utilities
SWIG  := swig
TCLSH := tclsh
DOXYGEN := doxygen

# Compiler Options
CFLAGS := 

# Linker Options
#LDFLAGS += \

# Output options
#PROG_EXT = 
SHARED_EXT = .so

# Export all variables
export
