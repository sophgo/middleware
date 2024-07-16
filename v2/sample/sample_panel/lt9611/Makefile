SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../../Makefile.param
	include $(PARAM_FILE)
endif
include ../../sample.mk

COMMON_DIR = $(PWD)/../../common
SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c) $(filter-out $(COMMON_DIR)/sample_common_vo.c, $(wildcard $(COMMON_DIR)/*.c))
INCS = -I$(KERNEL_INC) -I$(MW_INC) -I$(ISP_INC) -I$(VI_INC) -I$(COMMON_DIR) -I$(COMMON_DIR)/../../component/panel/$(shell echo $(CVIARCH) | tr A-Z a-z)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

EXTRA_CFLAGS = $(INCS)
LIBS = -lsys
EXTRA_LDFLAGS = $(LIBS) -lm -latomic -lpthread

TARGET = lt9611

.PHONY : clean all
all: $(TARGET)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET): $(OBJS)
	@$(CXX) -o $@ $(OBJS) $(ELFFLAGS) $(EXTRA_LDFLAGS) -static
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CC))] $(notdir $@)

clean:
	@rm -f $(OBJS) $(DEPS) $(TARGET)

-include $(DEPS)
