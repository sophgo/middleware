SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif

CHIP_NAME = cv186x
V4L2_INC = $(TOP_DIR)/linux_5.10/drivers/media/platform/soph/osdrv_include

SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c)

INCS += -I$(V4L2_INC)/chip/$(CHIP_NAME)/uapi/
INCS += -I$(V4L2_INC)/common/uapi/

ifeq ($(V4L2_ISP_ENABLE),1)
#for isp mw
INCS += -I$(TOP_DIR)/middleware/$(MW_VER)/modules/isp/cv186x/v4l2_adapter/inc
LIBS += -L$(TOP_DIR)/middleware/$(MW_VER)/lib
LIBS += -lispv4l2_adapter -lcvi_bin -lcvi_bin_isp -lsns_full
LIBS += -lisp -lawb -lae -laf -lisp_algo
LIBS += -lispv4l2_helper
#end

OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

TARGET = ispv4l2_ut
endif

EXTRA_CFLAGS = $(INCS)

ldflags-y += $(LIBS) -lpthread -latomic

.PHONY : clean all
all: $(TARGET)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET): $(OBJS)
	@$(CXX) -o $@ -Wl,--start-group $(OBJS) -Wl,--end-group $(ELFFLAGS) $(EXTRA_LDFLAGS) $(ldflags-y)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

clean:
	@rm -f $(OBJS) $(DEPS)  $(TARGET)

-include $(DEPS)
