SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif
include ../self_test.mk

SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c)
INCS = -I$(MW_INC) -I$(ISP_INC) -I../../sample/common -I$(KERNEL_INC) -I$(SYS_INC)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

H264_DRV_TEST_SRCS = $(SDIR)/cvi_h264_enc_test.c
JPG_DRV_ENC_SRCS = $(SDIR)/cvi_jpg_enc_test.c
JPG_DRV_DEC_SRCS = $(SDIR)/cvi_jpg_dec_test.c
CVI_API_TEST_SRCS = $(SDIR)/cvi_api_test.c
MULTI_PROCESS_ENC_TEST_SRCS = $(SDIR)/multi_process_enc_test.c
MULTI_PROCESS_DEC_TEST_SRCS = $(SDIR)/multi_process_dec_test.c
MULTI_PROCESS_CHANNELS_ENC_TEST_SRCS = $(SDIR)/multi_process_channels_enc_test.c
MULTI_PROCESS_CHANNELS_DEC_TEST_SRCS = $(SDIR)/multi_process_channels_dec_test.c


H264_DRV_TEST_OBJS = $(H264_DRV_TEST_SRCS:.c=.o)
H264_DRV_TEST_DEPS = $(H264_DRV_TEST_SRCS:.c=.d)
JPG_DRV_ENC_OBJS = $(JPG_DRV_ENC_SRCS:.c=.o)
JPG_DRV_ENC_DEPS = $(JPG_DRV_ENC_SRCS:.c=.d)
JPG_DRV_DEC_OBJS = $(JPG_DRV_DEC_SRCS:.c=.o)
JPG_DRV_DEC_DEPS = $(JPG_DRV_DEC_SRCS:.c=.d)
CVI_API_TEST_OBJS = $(CVI_API_TEST_SRCS:.c=.o)
MULTI_PROCESS_ENC_TEST_OBJS = $(MULTI_PROCESS_ENC_TEST_SRCS:.c=.o)
MULTI_PROCESS_DEC_TEST_OBJS = $(MULTI_PROCESS_DEC_TEST_SRCS:.c=.o)
MULTI_PROCESS_CHANNELS_ENC_TEST_OBJS = $(MULTI_PROCESS_CHANNELS_ENC_TEST_SRCS:.c=.o)
MULTI_PROCESS_CHANNELS_DEC_TEST_OBJS = $(MULTI_PROCESS_CHANNELS_DEC_TEST_SRCS:.c=.o)

H264_DRV_TEST = cvi_h264_enc_test
H265_DRV_TEST = cvi_h265_enc_test
JPG_DRV_ENC_TEST = cvi_jpg_enc
JPG_DRV_DEC_TEST = cvi_jpg_dec
CVI_API_TEST = cvi_api_test
MULTI_PROCESS_ENC_TEST = multi_pro_enc_test
MULTI_PROCESS_DEC_TEST = multi_pro_dec_test
MULTI_PROCESS_CHANNELS_ENC_TEST = multi_pro_chns_enc
MULTI_PROCESS_CHANNELS_DEC_TEST = multi_pro_chns_dec

COMM_OBJ :=

LIBS =


EXTRA_CFLAGS += $(DEFS) $(INCS) -Wno-error=unused-variable -Wno-error=sign-compare -Wno-error=unused-parameter
EXTRA_CFLAGS += -ffunction-sections -fdata-sections

EXTRA_LDFLAGS += $(LIBS) -pthread -lm -latomic

# ELFFLAGS +=  -march=rv64imafdcvxthead -mcmodel=medany -mabi=lp64dv
LD_FLAGS = -lvdec -lsys -lvenc -pthread -static

all : $(JPG_DRV_ENC_TEST) $(JPG_DRV_DEC_TEST) $(H264_DRV_TEST) $(H265_DRV_TEST) $(CVI_API_TEST) $(MULTI_PROCESS_ENC_TEST) $(MULTI_PROCESS_DEC_TEST) $(MULTI_PROCESS_CHANNELS_ENC_TEST) $(MULTI_PROCESS_CHANNELS_DEC_TEST)

$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.c
	$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

	#/mnt/workspace/mars_daily_32_2/host-tools/gcc/gcc-linaro-12.0.0-2021.11-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc -fanalyzer -fdump-analyzer-callgraph $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
$(SDIR)/%.o: $(SDIR)/%.c
	$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(H264_DRV_TEST) : $(H264_DRV_TEST_OBJS)
	$(CXX) -o $@ $(H264_DRV_TEST_OBJS) $(ELFFLAGS) $(LD_FLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

$(H265_DRV_TEST) : $(H264_DRV_TEST_OBJS)
	$(CXX) -o $@ $(H264_DRV_TEST_OBJS) $(ELFFLAGS) $(LD_FLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

$(JPG_DRV_ENC_TEST) : $(JPG_DRV_ENC_OBJS)
	@$(CXX) -o $@ $(JPG_DRV_ENC_OBJS) $(ELFFLAGS) $(LD_FLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

$(JPG_DRV_DEC_TEST) : $(JPG_DRV_DEC_OBJS)
	@$(CXX) -o $@ $(JPG_DRV_DEC_OBJS) $(ELFFLAGS) $(LD_FLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

$(CVI_API_TEST) : $(CVI_API_TEST_OBJS)
	@$(CXX) -o $@ $(CVI_API_TEST_OBJS) $(ELFFLAGS) $(LD_FLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

$(MULTI_PROCESS_ENC_TEST) : $(MULTI_PROCESS_ENC_TEST_OBJS)
	@$(CXX) -o $@ $(MULTI_PROCESS_ENC_TEST_OBJS) $(ELFFLAGS) $(LD_FLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

$(MULTI_PROCESS_DEC_TEST) : $(MULTI_PROCESS_DEC_TEST_OBJS)
	@$(CXX) -o $@ $(MULTI_PROCESS_DEC_TEST_OBJS) $(ELFFLAGS) $(LD_FLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

$(MULTI_PROCESS_CHANNELS_ENC_TEST) : $(MULTI_PROCESS_CHANNELS_ENC_TEST_OBJS)
	@$(CXX) -o $@ $(MULTI_PROCESS_CHANNELS_ENC_TEST_OBJS) $(ELFFLAGS) $(LD_FLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

$(MULTI_PROCESS_CHANNELS_DEC_TEST) : $(MULTI_PROCESS_CHANNELS_DEC_TEST_OBJS)
	@$(CXX) -o $@ $(MULTI_PROCESS_CHANNELS_DEC_TEST_OBJS) $(ELFFLAGS) $(LD_FLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)


.PHONY : clean
clean:
	@rm -f $(OBJS) $(DEPS) $(COMM_OBJ) $(COMM_DEPS) 
	@rm -f $(MULTI_PROCESS_ENC_TEST) $(H264_DRV_TEST) $(H265_DRV_TEST) $(JPG_DRV_ENC_TEST) $(JPG_DRV_DEC_TEST) $(CVI_API_TEST)
	@rm -f $(MULTI_PROCESS_DEC_TEST) $(MULTI_PROCESS_CHANNELS_ENC_TEST) $(MULTI_PROCESS_CHANNELS_DEC_TEST)


-include $(DEPS)
