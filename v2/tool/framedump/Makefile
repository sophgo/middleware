#CC=riscv64-unknown-linux-gnu-gcc
CC=$(CROSS_COMPILE)gcc
SDIR = $(PWD)
SDK_VER=v2
DEBUG = 1
TOP_DIR = $(SDIR)/../../../..

#include ../../Makefile.param
#$(info $(ELFFLAGS))

SRC := $(wildcard ./*.c)
OBJ := $(SRC:%.c=%.o)
TARGET := $(OBJ:%.o=%)
INC := -I./
INC += -I../../include/
INC += -I$(TOP_DIR)/osdrv/interdrv/$(SDK_VER)/include/common/uapi/

INC += -I$(TOP_DIR)/osdrv/interdrv/$(SDK_VER)/include/chip/cv186x/uapi/
#$(info $(INC))

ELFFLAGS := -Wl,--gc-sections
ELFFLAGS += -static

EXTRA_LDFLAGS = -L../../lib
EXTRA_LDFLAGS += -lvi -lvpss -lvo -lisp -lisp_algo -lawb -lae -laf -lcvi_bin -lsys
EXTRA_LDFLAGS += -lm -lpthread -latomic

CFLAGS := $(INC)
CFLAGS += -DSUPPORT_MULTI_PROCESS

ifeq ($(DEBUG), 1)
CFLAGS += -g -O0
endif

all : $(TARGET)

$(SDIR)/%.o: %.c
	@$(CC) $(CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET):%:%.o
	$(CC) $(CFLAGS) -o $@ $^ $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@rm *.o
ifneq ($(DEBUG), 1)
	@$(CROSS_COMPILE)strip $@
endif

clean:
	@rm -f $(OBJ) $(TARGET)
