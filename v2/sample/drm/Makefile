SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif
include ../sample.mk

DRM_INC = $(MW_INC)/3rd/drm
LIBKMS_INC = $(MW_INC)/3rd/drm/libkms
LIBDRM_INC = $(MW_INC)/3rd/drm/libdrm
DRM_COMMON_INC = $(PWD)/common
LIBS = $(MW_PATH)/lib/3rd/drm

SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c)
INCS = -I$(MW_INC) -I$(DRM_INC) -I$(LIBKMS_INC) -I$(LIBDRM_INC) -I$(DRM_COMMON_INC)
OBJS = $(SRCS:.c=.o)

TARGET = $(SRCS:.c=)

EXTRA_CFLAGS = $(INCS)
EXTRA_LDFLAGS = -L$(LIBS) -ldrm -lkms -lm

.PHONY : clean all
all: $(TARGET)

$(SDIR)/%: $(SDIR)/%.c $(SDIR)/common/*.c
	@$(CC) -o $@ $^ $(CFLAGS) $(ELFFLAGS) $(EXTRA_CFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CC))] $(notdir $@)

clean:
	@rm -f $(OBJS) $(notdir $(TARGET))
