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
INCS = -Iinclude -I$(MW_INC) -I$(ISP_INC) -I$(COMM_INC) -I$(KERNEL_INC)
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
	# -Wl,--format=binary \
	# -Wl,data/00_352x288_y.yuv -Wl,data/bin_352x288_y.yuv -Wl,data/sky_640x480.yuv \
	# -Wl,data/result/sample_LBP_Normal.yuv -Wl,data/result/sample_LBP_Abs.yuv \
	# -Wl,data/result/sample_NormGrad_Hor5x5.yuv -Wl,data/result/sample_NormGrad_Ver5x5.yuv \
	# -Wl,data/result/sample_NormGrad_Hor3x3.yuv -Wl,data/result/sample_NormGrad_Ver3x3.yuv \
	# -Wl,data/result/sample_NormGrad_Combine3x3.yuv \
	# -Wl,data/result/sample_Add.yuv -Wl,data/result/sample_Xor.yuv -Wl,data/result/sample_Or.yuv -Wl,data/result/sample_And.yuv \
	# -Wl,data/result/sample_Sub_Abs.yuv -Wl,data/result/sample_Erode_3x3.bin.only_erode \
	# -Wl,data/result/sample_Erode_5x5.bin.only_erode -Wl,data/result/sample_Dilate_3x3_dilate_only.bin \
	# -Wl,data/result/sample_Dilate_5x5_dilate_only.bin -Wl,data/result/sample_Sobel_Hor3x3.yuv -Wl,data/result/sample_Sobel_Ver3x3.yuv \
	# -Wl,data/result/sample_Sobel_Hor5x5.yuv -Wl,data/result/sample_Sobel_Ver5x5.yuv -Wl,data/result/sample_Integ_Sum.yuv -Wl,data/result/sample_Integ_Combine.yuv \
	# -Wl,data/result/sample_Integ_Sqsum.yuv -Wl,data/result/sample_Hist.bin -Wl,data/result/sample_Map.yuv \
	# -Wl,data/result/sample_Bernsen_3x3.yuv -Wl,data/result/sample_Bernsen_3x3_Paper.yuv -Wl,data/result/sample_Bernsen_3x3_Thresh.yuv \
	# -Wl,data/result/sample_Bernsen_5x5.yuv -Wl,data/result/sample_Bernsen_5x5_Paper.yuv -Wl,data/result/sample_Bernsen_5x5_Thresh.yuv \
	# -Wl,data/result/sample_Canny_Mem_3x3_0.bin -Wl,data/result/sample_Canny_Mem_5x5_0.bin -Wl,data/result/sample_CannyHysEdge_3x3.yuv \
	# -Wl,data/result/sample_CannyEdge_3x3.yuv -Wl,data/result/sample_CannyEdge_5x5.yuv -Wl,data/result/sample_CannyHysEdge_5x5.yuv \
	# -Wl,data/result/sample_MagAndAng_MagAndAng5x5_Mag.yuv \
	# -Wl,data/result/sample_MagAndAng_MagAndAng3x3_Ang.yuv -Wl,data/result/sample_MagAndAng_MagAndAng3x3_Mag.yuv \
	# -Wl,data/result/sample_MagAndAng_MagAndAng5x5_Ang.yuv -Wl,data/result/sample_MagAndAng_Thresh5x5_Mag.yuv \
	# -Wl,data/result/sample_MagAndAng_Thresh3x3_Mag.yuv -Wl,data/result/sample_tile_MagAndAng_MagAndAng3x3_Ang.yuv -Wl,data/result/sample_tile_MagAndAng_Thresh3x3_Mag.yuv \
	# -Wl,data/result/sample_tile_MagAndAng_Thresh5x5_Mag.yuv -Wl,data/result/sample_tile_MagAndAng_MagAndAng3x3_Mag.yuv \
	# -Wl,data/result/sample_tile_MagAndAng_MagAndAng5x5_Ang.yuv -Wl,data/result/sample_tile_MagAndAng_MagAndAng5x5_Mag.yuv \
	# -Wl,data/result/sample_tile_Dilate_3x3.yuv -Wl,data/result/sample_tile_Dilate_5x5.yuv \
	# -Wl,data/result/sample_tile_Erode_3x3.yuv -Wl,data/result/sample_tile_Erode_5x5.yuv \
	# -Wl,data/result/sample_tile_Sobel_Ver5x5.yuv -Wl,data/result/sample_tile_Sobel_Hor3x3.yuv \
	# -Wl,data/result/sample_tile_Sobel_Hor5x5.yuv -Wl,data/result/sample_tile_Sobel_Ver3x3.yuv \
	# -Wl,data/result/sample_tile_NormGrad_Combine3x3.yuv \
	# -Wl,data/result/sample_tile_NormGrad_Hor3x3.yuv -Wl,data/result/sample_tile_NormGrad_Hor5x5.yuv \
	# -Wl,data/result/sample_tile_NormGrad_Ver3x3.yuv -Wl,data/result/sample_tile_NormGrad_Ver5x5.yuv \
	# -Wl,data/result/sample_tile_Canny_Mem_3x3_0.bin -Wl,data/result/sample_tile_Canny_Mem_5x5_0.bin \
	# -Wl,data/result/sample_tile_CannyHysEdge_3x3.yuv -Wl,data/result/sample_tile_CannyHysEdge_5x5.yuv \
	# -Wl,data/result/sample_tile_CannyEdge_3x3.yuv -Wl,data/result/sample_tile_CannyEdge_5x5.yuv \
	# -Wl,data/result/sample_tile_Add.yuv -Wl,data/result/sample_tile_And.yuv \
	# -Wl,data/result/sample_tile_Sub_Abs.yuv -Wl,data/result/sample_tile_Xor.yuv -Wl,data/result/sample_tile_Or.yuv \
	# -Wl,--format=default
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)
	@cp $(TARGET1) ../auto_test/

$(TARGET2) : $(OBJ_2nd)
	@$(CXX) -o $@ $^ $(EXTRA_LDFLAGS)
	# -Wl,--format=binary \
	# -Wl,data/00_352x288_y.yuv -Wl,data/01_352x288_y.yuv -Wl,data/bin_352x288_y.yuv -Wl,data/sky_640x480.yuv \
	# -Wl,data/00_352x288_SP420.yuv -Wl,data/00_352x288_SP422.yuv \
	# -Wl,data/lena_480x480_planar.yuv -Wl,data/00_352x288_444.yuv -Wl,data/00_704x576.s16 -Wl,data/00_704x576.u16 \
	# -Wl,data/campus_352x288.gray -Wl,data/campus_352x288.rgb -Wl,data/md1_480x480.yuv -Wl,data/md2_480x480.yuv \
	# -Wl,data/result/sample_NCC_Mem.bin -Wl,data/result/sample_Thresh_Binary.yuv -Wl,data/result/sample_Thresh_Trunc.yuv -Wl,data/result/sample_Thresh_ToMinVal.yuv \
	# -Wl,data/result/sample_Thresh_MinMidMax.yuv -Wl,data/result/sample_Thresh_MinMidOri.yuv -Wl,data/result/sample_Thresh_MinOriMax.yuv \
	# -Wl,data/result/sample_Thresh_OriMidMax.yuv -Wl,data/result/sample_Thresh_OriMidOri.yuv \
	# -Wl,data/result/sample_Thresh_S16_To_S8_MinMidMax.yuv -Wl,data/result/sample_Thresh_S16_To_S8_MinOriMax.yuv \
	# -Wl,data/result/sample_Thresh_S16_To_U8_MinMidMax.yuv -Wl,data/result/sample_Thresh_S16_To_U8_MinOriMax.yuv \
	# -Wl,data/result/sample_Thresh_U16_To_U8_MinMidMax.yuv -Wl,data/result/sample_Thresh_U16_To_U8_MinOriMax.yuv \
	# -Wl,data/result/sample_tile_Thresh_Binary.yuv -Wl,data/result/sample_tile_Thresh_Trunc.yuv -Wl,data/result/sample_tile_Thresh_ToMinVal.yuv \
	# -Wl,data/result/sample_tile_Thresh_MinMidMax.yuv -Wl,data/result/sample_tile_Thresh_MinMidOri.yuv -Wl,data/result/sample_tile_Thresh_MinOriMax.yuv \
	# -Wl,data/result/sample_tile_Thresh_OriMidMax.yuv -Wl,data/result/sample_tile_Thresh_OriMidOri.yuv \
	# -Wl,data/result/sample_DMA_Direct.bin -Wl,data/result/sample_DMA_Interval.bin \
	# -Wl,data/result/sample_DMA_Set3Byte.bin -Wl,data/result/sample_DMA_Set8Byte.bin \
	# -Wl,data/result/sample_Resize_Area_240p.rgb -Wl,data/result/sample_Resize_Area_gray.yuv \
	# -Wl,data/result/sample_Resize_Area_rgb.rgb -Wl,data/result/sample_Resize_Bilinear_240p.rgb \
	# -Wl,data/result/sample_Resize_Bilinear_gray.yuv -Wl,data/result/sample_Resize_Bilinear_rgb.rgb \
	# -Wl,data/result/sample_16BitTo8Bit_U16ToU8.yuv -Wl,data/result/sample_16BitTo8Bit_Abs.yuv \
	# -Wl,data/result/sample_16BitTo8Bit_S16ToS8.yuv -Wl,data/result/sample_16BitTo8Bit_Shift.yuv \
	# -Wl,data/result/sample_GradFg_FIND_MIN_GRAD.out -Wl,data/result/sample_GradFg_USE_CUR_GRAD.out \
	# -Wl,data/result/sample_tile_GradFg_FIND_MIN_GRAD.out -Wl,data/result/sample_tile_GradFg_USE_CUR_GRAD.out \
	# -Wl,data/result/sample_Sad_sad_mode0_out0.bin -Wl,data/result/sample_Sad_sad_mode0_out1.bin \
	# -Wl,data/result/sample_Sad_sad_mode1_out0.bin -Wl,data/result/sample_Sad_sad_mode1_out1.bin \
	# -Wl,data/result/sample_Sad_sad_mode2_out0.bin -Wl,data/result/sample_Sad_sad_mode2_out1.bin \
	# -Wl,data/result/sample_Sad_thr_mode0_out0.bin -Wl,data/result/sample_Sad_thr_mode0_out1.bin \
	# -Wl,data/result/sample_Sad_thr_mode1_out0.bin -Wl,data/result/sample_Sad_thr_mode1_out1.bin \
	# -Wl,data/result/sample_Sad_thr_mode2_out0.bin -Wl,data/result/sample_Sad_thr_mode2_out1.bin \
	# -Wl,data/result/sample_tile_FrameDiffMotion.yuv -Wl,data/result/sample_FrameDiffMotion.yuv \
	# -Wl,data/result/sample_Shitomasi_CandiCorner.yuv -Wl,data/penguin_352x288.gray.shitomasi.raw -Wl,data/result/sample_tile_Shitomasi_sky_640x480.yuv \
	# -Wl,data/result/sample_CSC_BT601_YUV2HSV_480x480.vsh -Wl,data/result/sample_CSC_BT601_YUV2LAB_480x480.bal \
	# -Wl,data/result/sample_CSC_BT709_YUV2HSV_480x480.vsh -Wl,data/result/sample_CSC_BT709_YUV2LAB_480x480.bal -Wl,data/result/sample_CSC_YUV2RGB.rgb \
	# -Wl,data/result/sample_FilterAndCSC_420SPToVideoPlanar3x3.yuv -Wl,data/result/sample_FilterAndCSC_420SPToVideoPlanar5x5.yuv \
	# -Wl,data/result/sample_Filter_Y3x3.yuv -Wl,data/result/sample_Filter_422SP3x3.yuv -Wl,data/result/sample_Filter_420SP3x3.yuv \
	# -Wl,data/result/sample_Filter_Y5x5.yuv -Wl,data/result/sample_Filter_420SP5x5.yuv -Wl,data/result/sample_Filter_422SP5x5.yuv \
	# -Wl,data/result/sample_OrdStaFilter_Min.yuv -Wl,data/result/sample_OrdStaFilter_Max.yuv \
	# -Wl,data/result/sample_OrdStaFilter_Median.yuv -Wl,--format=default
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)
	@cp $(TARGET2) ../auto_test/

$(TARGET_bg) : $(OBJ_bg)
	@$(CXX) -o $@ $^ $(EXTRA_LDFLAGS)
	# -Wl,--format=binary \
	# -Wl,data/result/sample_BgModelSample2_BgMdl_100.bin -Wl,data/result/sample_tile_BgModelSample2_BgMdl_100.bin \
	# -Wl,data/result/sample_GMM_U8C1_fg_31.yuv -Wl,data/result/sample_GMM_U8C1_bg_31.yuv \
	# -Wl,data/result/sample_GMM2_U8C1_fg_31.yuv -Wl,data/result/sample_GMM2_U8C1_bg_31.yuv \
	# -Wl,data/result/sample_GMM2_U8C1_PixelCtrl_fg_31.yuv -Wl,data/result/sample_GMM2_U8C1_PixelCtrl_bg_31.yuv -Wl,data/result/sample_GMM2_U8C1_PixelCtrl_match_31.yuv \
	# -Wl,data/result/sample_tile_GMM_U8C1_bg_31.yuv -Wl,data/result/sample_tile_GMM_U8C1_fg_31.yuv \
	# -Wl,data/result/sample_tile_GMM2_U8C1_bg_31.yuv -Wl,data/result/sample_tile_GMM2_U8C1_fg_31.yuv \
	# -Wl,data/campus.u8c1.1_100.raw -Wl,--format=default
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)
	@cp $(TARGET_bg) ../auto_test/

clean:
	@rm -f $(OBJS) $(DEPS) $(TARGET1) $(TARGET2) $(TARGET_bg)

-include $(DEPS)
