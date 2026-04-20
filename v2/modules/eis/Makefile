SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
     PARAM_FILE:=../../Makefile.param
     include $(PARAM_FILE)
endif

SDIR = $(PWD)/src
SRCS = $(wildcard $(SDIR)/*.c)
INCS = -I$(MW_INC) -I$(KERNEL_INC) -I$(SYS_INC) -I./include -I$(MW_INC)/isp/cv186x
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
TARGET_A = $(MW_LIB)/libcvi_eis.a
TARGET_SO = $(MW_LIB)/libcvi_eis.so

# Prebuilt algo library path
PREBUILD_DIR = $(PWD)/prebuilt/lib64/cpu_eis/
PREBUILD_OBJ = $(PREBUILD_DIR)/cvi_cpu_eis.o

EXTRA_CFLAGS = $(INCS)

.PHONY : clean all
all : $(TARGET_A) $(TARGET_SO)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -c $< -o $@
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET_A): $(OBJS) $(PREBUILD_OBJ)
	@$(AR) $(ARFLAGS) $(TARGET_A) $(OBJS) $(PREBUILD_OBJ)
	@echo -e $(GREEN)[LINK]$(END)[$(notdir $(AR))] $(notdir $(TARGET_A))

$(TARGET_SO): $(OBJS) $(PREBUILD_OBJ)
	@$(LD) $(LDFLAGS) -o $(TARGET_SO) --start-group $(OBJS) $(PREBUILD_OBJ) --end-group
	@echo -e $(GREEN)[LINK]$(END)[$(notdir $(LD))] $(notdir $(TARGET_SO))

clean:
	@rm -f $(OBJS) $(DEPS) $(TARGET_A) $(TARGET_SO)

-include $(DEPS)
