SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif
include ../sample.mk
export SDK_DIR = $(PWD)
export LDFLAGS = -L$(MW_PATH)/lib/3rd

.phony: all clean

all: test_cases

test_util:
	@$(MAKE) -C samples/util

test_cases: test_util
	@$(MAKE) -C samples/blend
	@$(MAKE) -C samples/shape

clean:
	@$(MAKE) -C samples/blend clean
	@$(MAKE) -C samples/shape clean
