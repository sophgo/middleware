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

INCS = -I$(MW_INC) -I$(MOD_INC) -I$(SYS_INC) -I$(ISP_INC) -Iinclude
INCS += -I$(MW_PATH)/modules/vpss/include -I../isp/algo
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

PARAM_SRC = $(HDIR)/cvi_bin_param_header.h
PARAM_PRE = $(addsuffix .i,$(basename $(addprefix $(TMP_FOLDER)/,$(notdir $(PARAM_SRC)))))
PARAM_DEP = $(addsuffix .d,$(basename $(addprefix $(TMP_FOLDER)/,$(notdir $(PARAM_SRC)))))

isp_chip_dir := $(shell echo $(CHIP_ARCH) | tr A-Z a-z)

PROFILE_ISP = ../isp/common/toolJsonGenerator/$(isp_chip_dir)/cvi_bin_profile.json
PROFILE_CVI_BIN = python/cvi_bin_profile.json

check_isp_profile_ret := $(shell test -f "$(PROFILE_ISP)"; echo $$?)

ifeq ($(check_isp_profile_ret), 0)
	PROFILE := $(PROFILE_ISP)
else
	PROFILE := $(PROFILE_CVI_BIN)
endif

TARGET_A = $(MW_LIB)/libcvi_bin.a
TARGET_SO = $(MW_LIB)/libcvi_bin.so

EXTRA_CFLAGS = $(INCS) -fpack-struct=8
EXTRA_CFLAGS += -DSDK_VER=\"$(SDK_VER)\"
ifeq ($(MULTI_PROCESS_SUPPORT), 1)
EXTRA_CFLAGS += -DRPC_MULTI_PROCESS
endif

check_git_repo_ret := $(shell git log &>/dev/null; echo $$?)

ifeq ($(check_git_repo_ret), 0)
	commit_id = $(shell git rev-parse --short HEAD | cut -c1-9)
	gerrit_id = $(shell git log -n1 | grep Change-Id | tail -n1 | sed -n 's/.*Change-Id: \(.*\)/\1/;s/\(.\{16\}\).*/\1/p')
endif

ifneq ($(commit_id), )
	EXTRA_CFLAGS += -DISP_BIN_COMMIT=\"$(commit_id)\"
endif

ifneq ($(gerrit_id), )
	EXTRA_CFLAGS += -DISP_BIN_GERRIT=\"$(gerrit_id)\"
endif

.PHONY : clean all gencode check

all : gencode
target : check $(TARGET_A) $(TARGET_SO)

gencode: $(PARAM_PRE)

$(TMP_FOLDER):
	@echo "Create $(TMP_FOLDER) for object file"
	-@mkdir -p $(TMP_FOLDER)

$(PARAM_PRE): $(PARAM_SRC) $(SRCS) | $(TMP_FOLDER)
ifeq ($(INDEX_STRATEGY),1)
	@$(CC) -E $(DEPFLAGS) $(EXTRA_CFLAGS) -c $< -o $@
	@sed -i '1s|.*|$@: \\|' $(PARAM_DEP)
	@echo "Generate code"
	@python3 python/gencode.py $@ --profile $(PROFILE)
ifeq ($(check_isp_profile_ret), 0)
	@cp -f $(PROFILE_ISP) $(PROFILE_CVI_BIN)
endif
endif
	@$(MAKE) target

md5_result = $(shell ./CalPqBinMD5.sh)
ifneq ($(md5_result), -1)
	EXTRA_CFLAGS += -DISP_BIN_MD5=\"$(md5_result)\"
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

check:
	@echo ">>>>>> check start"
	@echo "profile: $(PROFILE)"
	@if [ "$(check_git_repo_ret)" == "0" ]; then \
		echo "git repo: Yes"; \
		echo "commit_id: $(commit_id)"; \
		echo "gerrit_id: $(gerrit_id)"; \
	else \
		echo "git repo: No"; \
	fi
	@echo "md5 value: $(md5_result)"
	@if [ "$(md5_result)" == "-1" ]; then \
		echo "Error: md5 is -1, please check the the md5!"; \
		exit 1; \
	fi
	@echo "<<<<<< check end"

clean:
	@rm -f $(OBJS) $(DEPS) $(TARGET_A) $(TARGET_SO)
	@rm -f $(PARAM_DEP) $(PARAM_PRE)
	@rm -f $(SDIR)/*.autogen.*
	@rm -f $(HDIR)/*.autogen.*
	@rm -rf $(TMP_FOLDER)
	@rm -f python/cvi_bin_struct.h

-include $(DEPS)
-include $(PARAM_DEP)
