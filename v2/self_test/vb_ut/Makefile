SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif

SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c)
INCS = -I$(MW_INC) -I$(KERNEL_INC) -I$(SYS_INC) -I$(ISP_INC) -I$(VI_INC)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

TARGET = vb_ut
LIBS = -lsys -lmisc

ifeq ($(SELF_TEST_STATIC), 1)
ELFFLAGS += -static
endif

ifeq ($(DEBUG), 1)
EXTRA_LDFLAGS += -g -O0
endif

ifeq ($(MTRACE), y)
EXTRA_LDFLAGS += -g
endif

EXTRA_CFLAGS += $(DEFS) $(INCS)
EXTRA_LDFLAGS += $(LIBS) -lpthread -lm -lini -latomic

all : $(TARGET)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET) : $(OBJS) $(ISP_OBJ) $(MW_LIB)/libsys.a
	@$(CXX) -o $@ -Wl,--start-group $(OBJS) $(MW_LIB)/libsys.a -Wl,--end-group $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)
	@cp $(TARGET) ../auto_test/

.PHONY : clean
clean:
	@rm -f $(OBJS) $(DEPS) $(COMM_DEPS) $(TARGET)

-include $(DEPS)
