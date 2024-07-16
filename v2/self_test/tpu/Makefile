SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif
include ../self_test.mk
include ../../modules/isp/isp.mk

SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c)
INCS = -I$(MW_INC) -I$(ISP_INC) -I$(VI_INC) -I../../sample/common -I$(KERNEL_INC) -I../../3rdparty/opencv/include
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
CXX_SRCS = $(wildcard $(SDIR)/*.cpp)
CXX_OBJS = $(CXX_SRCS:.cpp=.o)
CXX_DEPS = $(CXX_SRCS:.cpp=.d)

ifeq ($(SAMPLE_TPU_ENABLE), 1)
TARGET = sample_tpu
else
TARGET =
endif

ifeq ($(SDK_VER), 32bit)
  OPENCV_LIB=-lopencv_core -lzlib -ltegra_hal
else
  OPENCV_LIB=-lopencv_core -lz -ltegra_hal
endif
ifeq ($(MULTI_PROCESS_SUPPORT), 1)
DEFS = -DRPC_MULTI_PROCESS
endif

LIBS += -lvpu -lvenc -lvdec -lcvi_jpeg -lcvi_vcodec -lsys $(ISP_LIB) $(SNS_LIB) $(OPENCV_LIB)

EXTRA_CFLAGS = $(DEFS) $(INCS)
EXTRA_CPPFLAGS = -fno-use-cxa-atexit
EXTRA_LDFLAGS = $(LIBS) -lini -lpthread
EXTRA_LDFLAGS += -lcvi_bin -lcvi_bin_isp

ifneq ($(SDK_VER),uclibc)
all : $(TARGET)
else
all:
endif

$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(SDIR)/%.o: $(SDIR)/%.cpp
	@$(CXX) $(DEPFLAGS) $(CXXFLAGS) $(EXTRA_CFLAGS) $(EXTRA_CPPFLAGS) -o $@ -c $<
	@echo [$(notdir $(CXX))] $(notdir $@)

$(TARGET) : $(COMM_OBJ) $(OBJS) $(CXX_OBJS) $(MW_LIB)/libisp.a $(MW_LIB)/libvpu.a $(MW_LIB)/libsys.a
	@$(CXX) -o $@ -Wl,--start-group $(COMM_OBJ) $(OBJS) $(CXX_OBJS) $(MW_LIB)/libsys.a -Wl,--end-group $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

.PHONY : clean
clean:
	@rm -f $(OBJS) $(DEPS) $(CXX_OBJS) $(CXX_DEPS) $(TARGET)

-include $(DEPS) $(CXX_DEPS)
