##
# 'make'        build executable file 
# 'make clean'  removes all .o and executable files

# define the C compiler to use
BROOT_PATH	:= $(BUILDROOT_HOME)
LINUX_PATH	:= $(BROOT_PATH)/output/build/linux-arc-3.18-2
TOOLCHAIN_PATH	:= $(BROOT_PATH)/output/host/usr/bin
CROSS_COMPILE	:= $(TOOLCHAIN_PATH)/arc-buildroot-linux-uclibc-
CC 		:= $(CROSS_COMPILE)gcc

# define any compile-time flags
CFLAGS = -g -Wall -O2

# define any directories containing header files other than /usr/include
#
#INCLUDES = -I $(LINUX_PATH)/include/video \
#	   -I $(PWD)/../../../general_includes
	   
# define the C source files
SRCS = esm_logdump.c 

# define the C object files 
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#
OBJS = $(SRCS:.c=.o)

# define the executable file 
MAIN = esm_logdump

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean

all:    $(MAIN)
#	@mkdir -p ../bin
#	@mv arcpgu ../bin
	@rm *.o
	@echo  esmlogdump application has been compiled

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) *.o *~ ../$(MAIN)
	
depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it

