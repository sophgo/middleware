SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif
include ../sample.mk
include ./Kbuild

SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c) $(filter-out $(COMMON_DIR)/sample_common_vo.c, $(wildcard $(COMMON_DIR)/*.c))
INCS = -I$(MW_INC) -I$(ISP_INC) -I$(KERNEL_INC) $(COMM_INC)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

TARGET = sample_dsi
ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
TARGET = sample_dsi_asan
endif

LIBS = -lsys -lmipi_tx -lmisc

ifeq ($(MULTI_PROCESS_SUPPORT), 1)
DEFS += -DRPC_MULTI_PROCESS
LIBS += -lmisc
endif

CFLAGS += $(KBUILD_DEFINES)
EXTRA_CFLAGS = $(INCS)
EXTRA_LDFLAGS = $(LIBS) -lm -lpthread
# EXTRA_LDFLAGS += -lcvi_bin -lcvi_bin_isp $(ISP_LIB)

.PHONY : clean all
all: $(TARGET)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET): $(OBJS) $(MW_LIB)/libsys.a $(MW_LIB)/libmipi_tx.a
	@$(CXX) -o $@ $(OBJS) $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CC))] $(notdir $@)

clean:
	@rm -f $(OBJS) $(DEPS) $(TARGET)

-include $(DEPS)
