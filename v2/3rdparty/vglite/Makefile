SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
     PARAM_FILE:=../../Makefile.param
     include $(PARAM_FILE)
endif

TARGET_A = ./lib/libvg_lite.a
TARGET_SO = ./lib/libvg_lite.so

.PHONY : clean all
all: install
	@# using pre-compiled dynamic library

install:
	@cp -f $(TARGET_A) $(MW_LIB)/3rd/libvg_lite.a
	@cp -f $(TARGET_SO) $(MW_LIB)/3rd/libvg_lite.so

clean:
	@# clean