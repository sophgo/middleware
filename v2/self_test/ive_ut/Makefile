SAMPLE_STATIC = 1
SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif
include ../self_test.mk

TARGET1 = ive_1st_ut
TARGET2 = ive_2nd_ut
TARGET_bg = ive_bg_ut

SDIR = $(PWD)/src
SRCS = $(wildcard $(SDIR)/*.c)
INCS = -Iinclude -I$(MW_INC) -I$(ISP_INC) -I$(VI_INC) -I$(COMM_INC) -I$(KERNEL_INC)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
OBJ_1st = 	$(SDIR)/ive_dump.o $(SDIR)/ive_reset.o $(SDIR)/sample_add.o \
			$(SDIR)/sample_and.o $(SDIR)/sample_xor.o $(SDIR)/sample_or.o \
			$(SDIR)/sample_bernsen.o $(SDIR)/sample_cannyedge.o \
			$(SDIR)/sample_cannyhysedge.o $(SDIR)/sample_dilate.o \
			$(SDIR)/sample_erode.o $(SDIR)/sample_hist.o $(SDIR)/sample_imgtoodma.o \
			$(SDIR)/sample_integ.o $(SDIR)/sample_lbp.o $(SDIR)/sample_map.o \
			$(SDIR)/sample_normgrad.o $(SDIR)/sample_sobel.o \
			$(SDIR)/sample_sub.o $(SDIR)/sample_magandang.o $(SDIR)/$(TARGET1).o
OBJ_2nd = 	$(SDIR)/ive_dump.o $(SDIR)/ive_reset.o  $(SDIR)/sample_ccl.o\
			$(SDIR)/sample_ncc.o $(SDIR)/sample_thresh.o \
			$(SDIR)/sample_thresh_S16.o $(SDIR)/sample_thresh_U16.o $(SDIR)/sample_dma.o \
			$(SDIR)/sample_resize.o $(SDIR)/sample_16bitto8bit.o $(SDIR)/sample_gradfg.o $(SDIR)/sample_multi_thread.o \
			$(SDIR)/sample_stcandicorner.o $(SDIR)/sample_sad.o $(SDIR)/sample_framediffmotion.o \
			$(SDIR)/sample_csc.o $(SDIR)/sample_filterandcsc.o $(SDIR)/sample_filter.o \
			$(SDIR)/sample_ordstatfilter.o $(SDIR)/sample_cmdq.o $(SDIR)/$(TARGET2).o
OBJ_bg =  	$(SDIR)/ive_dump.o $(SDIR)/ive_reset.o \
			$(SDIR)/sample_gmm.o $(SDIR)/sample_gmm2.o $(SDIR)/sample_bgmodel.o $(SDIR)/$(TARGET_bg).o

PKG_CONFIG_PATH = $(MW_PATH)/pkgconfig
MW_LIBS = $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs --define-variable=mw_dir=$(MW_PATH) cvi_ive)

EXTRA_CFLAGS = $(INCS) $(DEFS)
EXTRA_LDFLAGS = $(CXXFLAGS) $(LIBS) -lm -lpthread

ifeq ($(SAMPLE_STATIC), 1)
LIBS += -static $(MW_PATH)/lib/libcvi_ive.a $(MW_PATH)/lib/libsys.a
endif

ifeq ($(SUBTYPE), asic)
LIBS += $(MW_LIBS)
endif

ifneq ($(CONFIG_ENABLE_SDK_ASAN), y)
ELFFLAGS += -static
endif

.PHONY : clean all
all : $(TARGET1) $(TARGET2) $(TARGET_bg)

$(SDIR)/%.o : $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET1) : $(OBJ_1st)
	@$(CXX) -o $@ $^ $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)
	@cp $(TARGET1) ../auto_test/

$(TARGET2) : $(OBJ_2nd)
	@$(CXX) -o $@ $^ $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)
	@cp $(TARGET2) ../auto_test/

$(TARGET_bg) : $(OBJ_bg)
	@$(CXX) -o $@ $^ $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)
	@cp $(TARGET_bg) ../auto_test/

clean:
	@rm -f $(OBJS) $(DEPS) $(TARGET1) $(TARGET2) $(TARGET_bg)

-include $(DEPS)
