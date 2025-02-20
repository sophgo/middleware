SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
     PARAM_FILE:=../../Makefile.param
     include $(PARAM_FILE)
endif

SDIR = $(PWD)/src
SRCS := $(wildcard $(SDIR)/*.c)
HDIR = $(PWD)/include
TMP_FOLDER = $(PWD)/tmp

INDEX_STRATEGY = 1
ARFLAGS_x = x

INCS = -I$(MW_INC) -I$(MOD_INC) -I$(SYS_INC) -I$(ISP_INC) -Iinclude
INCS += -I$(MW_PATH)/modules/vpss/include -I../isp/algo
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

PARAM_SRC = $(HDIR)/cvi_bin_param_header.h
PARAM_PRE = $(addsuffix .i,$(basename $(addprefix $(TMP_FOLDER)/,$(notdir $(PARAM_SRC)))))
PARAM_DEP = $(addsuffix .d,$(basename $(addprefix $(TMP_FOLDER)/,$(notdir $(PARAM_SRC)))))

TARGET_A = $(MW_LIB)/libcvi_bin.a
TARGET_SO = $(MW_LIB)/libcvi_bin.so

EXTRA_CFLAGS = $(INCS) -fpack-struct=8
EXTRA_CFLAGS += -DSDK_VER=\"$(SDK_VER)\"
ifeq ($(MULTI_PROCESS_SUPPORT), 1)
EXTRA_CFLAGS += -DRPC_MULTI_PROCESS
endif

EXTRA_CFLAGS += -DISP_BIN_COMMIT=\"$(shell git rev-parse --short HEAD)\"
gerrit_id=$(shell git log -n1 | grep Reviewed-on | tail -n1 | sed "s/.*+\///g")
ifneq ($(gerrit_id), )
	EXTRA_CFLAGS += -DISP_BIN_GERRIT=\"$(gerrit_id)\"
endif

.PHONY : clean all cleanall gencode
all : gencode
target : $(TARGET_A) $(TARGET_SO)

gencode: $(PARAM_PRE)

$(TMP_FOLDER):
	@echo "Create $(TMP_FOLDER) for object file"
	-@mkdir -p $(TMP_FOLDER)

$(PARAM_PRE): $(PARAM_SRC) | $(TMP_FOLDER)
ifeq ($(INDEX_STRATEGY),1)
	@$(CC) -E $(DEPFLAGS) $(EXTRA_CFLAGS) -c $< -o $@
	@sed -i '1s|.*|$@: \\|' $(PARAM_DEP)
	@echo "Generate code"
	@python3 python/gencode.py $@
endif
	$(MAKE) target

result = $(shell ./CalPqBinMD5.sh)
ifeq ($(result),-1)
abort:
	@echo $(result)
	$(error "md5 error: please check CalPqBinMD5.sh")
else
	EXTRA_CFLAGS += -DISP_BIN_MD5=\"$(result)\"
endif

ifeq ($(INDEX_STRATEGY),1)
	EXTRA_CFLAGS += -DINDEX_STRATEGY
endif

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(SDIR)/%.o: $(SDIR)/%.S
	$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET_A): $(OBJS)
	@$(AR) $(ARFLAGS) $(TARGET_A) $(OBJS)
	@echo -e $(YELLOW)[LINK]$(END)[$(notdir $(AR))] $(notdir $(TARGET_A))
	@echo "$$AR_MRI" | $(AR) -M

$(TARGET_SO): $(OBJS)
	@$(LD) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $(TARGET_SO) --start-group $(OBJS) $(LIBS) --end-group
	@echo -e $(GREEN)[LINK]$(END)[$(notdir $(LD))] $(notdir $(TARGET_SO))
	@$(STRIP) $(TARGET_SO)

clean:
	@rm -f $(OBJS) $(DEPS) $(TARGET_A) $(TARGET_SO)
	@rm -f $(PARAM_DEP) $(PARAM_PRE)
	@rm -f $(SDIR)/*.autogen.*
	@rm -f $(HDIR)/*.autogen.*

cleanall: clean
	@rm -rf $(TMP_FOLDER)

-include $(DEPS)
-include $(PARAM_DEP)
