#ifndef __CVI_PATTERN_H__
#define __CVI_PATTERN_H__

// input data
extern char data_00_352x288_y[] asm("_binary_data_00_352x288_y_yuv_start");
extern char data_01_352x288_y[] asm("_binary_data_01_352x288_y_yuv_start");
extern char data_bin_352x288_y[] asm("_binary_data_bin_352x288_y_yuv_start");
extern char data_tile_640x480_y[] asm("_binary_data_sky_640x480_yuv_start");
extern char data_campus_raw[] asm("_binary_data_campus_u8c1_1_100_raw_start");
extern char
	data_00_352x288_SP420[] asm("_binary_data_00_352x288_SP420_yuv_start");
extern char
	data_00_352x288_SP422[] asm("_binary_data_00_352x288_SP422_yuv_start");
extern char data_00_352x288_444[] asm("_binary_data_00_352x288_444_yuv_start");
extern char data_lena_480x480_planar[] asm(
	"_binary_data_lena_480x480_planar_yuv_start");
extern char data_00_704x576_u16[] asm("_binary_data_00_704x576_u16_start");
extern char data_00_704x576_s16[] asm("_binary_data_00_704x576_s16_start");
extern char data_md1_480x480_y[] asm("_binary_data_md1_480x480_yuv_start");
extern char data_md2_480x480_y[] asm("_binary_data_md2_480x480_yuv_start");
extern char data_PixelCtrl_Factor[] asm(
	"_binary_data_sample_GMM2_U8C1_PixelCtrl_Factor_raw_start");
extern char data_00_campus_352x288_rgb[] asm(
	"_binary_data_campus_352x288_rgb_start");
extern char data_00_campus_352x288_gray[] asm(
	"_binary_data_campus_352x288_gray_start");
extern char data_penguin_352x288_y[] asm(
	"_binary_data_penguin_352x288_gray_shitomasi_raw_start");

// 16BitTo8Bit result data
extern char data_U16ToU8_U16ToU8[] asm(
	"_binary_data_result_sample_16BitTo8Bit_U16ToU8_yuv_start");
extern char data_U16ToU8_Abs[] asm(
	"_binary_data_result_sample_16BitTo8Bit_Abs_yuv_start");
extern char data_U16ToU8_S16ToS8[] asm(
	"_binary_data_result_sample_16BitTo8Bit_S16ToS8_yuv_start");
extern char data_U16ToU8_Shift[] asm(
	"_binary_data_result_sample_16BitTo8Bit_Shift_yuv_start");

// Add result data
extern char data_Add[] asm("_binary_data_result_sample_Add_yuv_start");
extern char
	data_tile_Add[] asm("_binary_data_result_sample_tile_Add_yuv_start");

// And result data
extern char data_And[] asm("_binary_data_result_sample_And_yuv_start");
extern char
	data_tile_And[] asm("_binary_data_result_sample_tile_And_yuv_start");

// Bernsen result data
extern char result_Bernsen_5x5_Normal[] asm(
	"_binary_data_result_sample_Bernsen_5x5_yuv_start");
extern char result_Bernsen_5x5_Thresh[] asm(
	"_binary_data_result_sample_Bernsen_5x5_Thresh_yuv_start");
extern char result_Bernsen_5x5_Paper[] asm(
	"_binary_data_result_sample_Bernsen_5x5_Paper_yuv_start");
extern char result_Bernsen_3x3_Normal[] asm(
	"_binary_data_result_sample_Bernsen_3x3_yuv_start");
extern char result_Bernsen_3x3_Thresh[] asm(
	"_binary_data_result_sample_Bernsen_3x3_Thresh_yuv_start");
extern char result_Bernsen_3x3_Paper[] asm(
	"_binary_data_result_sample_Bernsen_3x3_Paper_yuv_start");

// BgModel result data
extern char data_BgMdl[] asm(
	"_binary_data_result_sample_BgModelSample2_BgMdl_100_bin_start");
extern char data_tile_BgMdl[] asm(
	"_binary_data_result_sample_tile_BgModelSample2_BgMdl_100_bin_start");

// CSC result data
extern char data_CSC_BT601_YUV2HSV[] asm(
	"_binary_data_result_sample_CSC_BT601_YUV2HSV_480x480_vsh_start");
extern char data_CSC_BT601_YUV2LAB[] asm(
	"_binary_data_result_sample_CSC_BT601_YUV2LAB_480x480_bal_start");
extern char data_CSC_BT709_YUV2HSV[] asm(
	"_binary_data_result_sample_CSC_BT709_YUV2HSV_480x480_vsh_start");
extern char data_CSC_BT709_YUV2LAB[] asm(
	"_binary_data_result_sample_CSC_BT709_YUV2LAB_480x480_bal_start");
extern char data_CSC_YUV2RGB[] asm(
	"_binary_data_result_sample_CSC_YUV2RGB_rgb_start");

// CannyHysEdge result data
extern char data_CannyHysEdge_Mem3x3[] asm(
	"_binary_data_result_sample_Canny_Mem_3x3_0_bin_start");
extern char data_CannyHysEdge_3x3[] asm(
	"_binary_data_result_sample_CannyHysEdge_3x3_yuv_start");
extern char data_CannyHysEdge_Mem5x5[] asm(
	"_binary_data_result_sample_Canny_Mem_5x5_0_bin_start");
extern char data_CannyHysEdge_5x5[] asm(
	"_binary_data_result_sample_CannyHysEdge_5x5_yuv_start");

extern char data_tile_CannyHysEdge_Mem3x3[] asm(
	"_binary_data_result_sample_tile_Canny_Mem_3x3_0_bin_start");
extern char data_tile_CannyHysEdge_3x3[] asm(
	"_binary_data_result_sample_tile_CannyHysEdge_3x3_yuv_start");
extern char data_tile_CannyHysEdge_Mem5x5[] asm(
	"_binary_data_result_sample_tile_Canny_Mem_5x5_0_bin_start");
extern char data_tile_CannyHysEdge_5x5[] asm(
	"_binary_data_result_sample_tile_CannyHysEdge_5x5_yuv_start");

// CannyEdge result data
extern char data_CannyEdge_3x3[] asm(
	"_binary_data_result_sample_CannyEdge_3x3_yuv_start");
extern char data_CannyEdge5x5[] asm(
	"_binary_data_result_sample_CannyEdge_5x5_yuv_start");

extern char data_tile_CannyEdge_3x3[] asm(
	"_binary_data_result_sample_tile_CannyEdge_3x3_yuv_start");
extern char data_tile_CannyEdge_5x5[] asm(
	"_binary_data_result_sample_tile_CannyEdge_5x5_yuv_start");

// Dilate result data
extern char data_Dilate3x3[] asm(
	"_binary_data_result_sample_Dilate_3x3_dilate_only_bin_start");
extern char data_Dilate5x5[] asm(
	"_binary_data_result_sample_Dilate_5x5_dilate_only_bin_start");
extern char data_tile_Dilate_3x3[] asm(
	"_binary_data_result_sample_tile_Dilate_3x3_yuv_start");
extern char data_tile_Dilate_5x5[] asm(
	"_binary_data_result_sample_tile_Dilate_5x5_yuv_start");

// DMA result data
extern char data_DMA_Direct[] asm(
	"_binary_data_result_sample_DMA_Direct_bin_start");
extern char data_DMA_Interval[] asm(
	"_binary_data_result_sample_DMA_Interval_bin_start");
extern char data_DMA_Set3Byte[] asm(
	"_binary_data_result_sample_DMA_Set3Byte_bin_start");
extern char data_DMA_Set8Byte[] asm(
	"_binary_data_result_sample_DMA_Set8Byte_bin_start");

// Filter result data
extern char result_Filter_Y3x3[] asm(
	"_binary_data_result_sample_Filter_Y3x3_yuv_start");
extern char result_Filter_420SP3x3[] asm(
	"_binary_data_result_sample_Filter_420SP3x3_yuv_start");
extern char result_Filter_422SP3x3[] asm(
	"_binary_data_result_sample_Filter_422SP3x3_yuv_start");
extern char result_Filter_Y5x5[] asm(
	"_binary_data_result_sample_Filter_Y5x5_yuv_start");
extern char result_Filter_420SP5x5[] asm(
	"_binary_data_result_sample_Filter_420SP5x5_yuv_start");
extern char result_Filter_422SP5x5[] asm(
	"_binary_data_result_sample_Filter_422SP5x5_yuv_start");

// Erode result data
extern char data_Erode_3x3[] asm(
	"_binary_data_result_sample_Erode_3x3_bin_only_erode_start");
extern char data_Erode_5x5[] asm(
	"_binary_data_result_sample_Erode_5x5_bin_only_erode_start");
extern char data_tile_Erode_3x3[] asm(
	"_binary_data_result_sample_tile_Erode_3x3_yuv_start");
extern char data_tile_Erode_5x5[] asm(
	"_binary_data_result_sample_tile_Erode_5x5_yuv_start");

// MD result data
extern char
	data_MD[] asm("_binary_data_result_sample_FrameDiffMotion_yuv_start");
extern char data_tile_MD[] asm(
	"_binary_data_result_sample_tile_FrameDiffMotion_yuv_start");

// GMM result data
extern char data_GMM_FG[] asm(
	"_binary_data_result_sample_GMM_U8C1_fg_31_yuv_start");
extern char data_GMM_BG[] asm(
	"_binary_data_result_sample_GMM_U8C1_bg_31_yuv_start");

extern char data_tile_GMM_FG[] asm(
	"_binary_data_result_sample_tile_GMM_U8C1_fg_31_yuv_start");
extern char data_tile_GMM_BG[] asm(
	"_binary_data_result_sample_tile_GMM_U8C1_bg_31_yuv_start");

// GMM2 result data
extern char data_GMM2_FG[] asm(
	"_binary_data_result_sample_GMM2_U8C1_fg_31_yuv_start");
extern char data_GMM2_BG[] asm(
	"_binary_data_result_sample_GMM2_U8C1_bg_31_yuv_start");

extern char data_tile_GMM2_FG[] asm(
	"_binary_data_result_sample_tile_GMM2_U8C1_fg_31_yuv_start");
extern char data_tile_GMM2_BG[] asm(
	"_binary_data_result_sample_tile_GMM2_U8C1_bg_31_yuv_start");

extern char data_GMM2_PixelCtrl_bg_31[] asm(
	"_binary_data_result_sample_GMM2_U8C1_PixelCtrl_bg_31_yuv_start");
extern char data_GMM2_PixelCtrl_fg_31[] asm(
	"_binary_data_result_sample_GMM2_U8C1_PixelCtrl_fg_31_yuv_start");
extern char data_GMM2_PixelCtrl_match_31[] asm(
	"_binary_data_result_sample_GMM2_U8C1_PixelCtrl_match_31_yuv_start");

// GradFg result data
extern char data_GradFg_MIN[] asm(
	"_binary_data_result_sample_GradFg_FIND_MIN_GRAD_out_start");
extern char data_GradFg_CUR[] asm(
	"_binary_data_result_sample_GradFg_USE_CUR_GRAD_out_start");
extern char data_tile_GradFg_MIN[] asm(
	"_binary_data_result_sample_tile_GradFg_FIND_MIN_GRAD_out_start");
extern char data_tile_GradFg_CUR[] asm(
	"_binary_data_result_sample_tile_GradFg_USE_CUR_GRAD_out_start");

// Hist result data
extern char result_Hist[] asm("_binary_data_result_sample_Hist_bin_start");

// Integ result data
extern char result_Integ_Combine[] asm(
	"_binary_data_result_sample_Integ_Combine_yuv_start");
extern char result_Integ_Sum[] asm(
	"_binary_data_result_sample_Integ_Sum_yuv_start");
extern char result_Integ_Sqsum[] asm(
	"_binary_data_result_sample_Integ_Sqsum_yuv_start");

// LBP result data
extern char result_LBP_Normal[] asm(
	"_binary_data_result_sample_LBP_Normal_yuv_start");
extern char
	result_LBP_Abs[] asm("_binary_data_result_sample_LBP_Abs_yuv_start");

// MagAndAng result data
extern char data_MagAndAng_3x3_Mag[] asm(
	"_binary_data_result_sample_MagAndAng_MagAndAng3x3_Mag_yuv_start");
extern char data_MagAndAng_3x3_Ang[] asm(
	"_binary_data_result_sample_MagAndAng_MagAndAng3x3_Ang_yuv_start");
extern char data_MagAndAng_5x5_Mag[] asm(
	"_binary_data_result_sample_MagAndAng_MagAndAng5x5_Mag_yuv_start");
extern char data_MagAndAng_5x5_Ang[] asm(
	"_binary_data_result_sample_MagAndAng_MagAndAng5x5_Ang_yuv_start");
extern char data_MagAndAng_3x3_MagThr[] asm(
	"_binary_data_result_sample_MagAndAng_Thresh3x3_Mag_yuv_start");
extern char data_MagAndAng_5x5_MagThr[] asm(
	"_binary_data_result_sample_MagAndAng_Thresh5x5_Mag_yuv_start");

extern char data_tile_MagAndAng_3x3_Mag[] asm(
	"_binary_data_result_sample_tile_MagAndAng_MagAndAng3x3_Mag_yuv_start");
extern char data_tile_MagAndAng_3x3_Ang[] asm(
	"_binary_data_result_sample_tile_MagAndAng_MagAndAng3x3_Ang_yuv_start");
extern char data_tile_MagAndAng_5x5_Mag[] asm(
	"_binary_data_result_sample_tile_MagAndAng_MagAndAng5x5_Mag_yuv_start");
extern char data_tile_MagAndAng_5x5_Ang[] asm(
	"_binary_data_result_sample_tile_MagAndAng_MagAndAng5x5_Ang_yuv_start");
extern char data_tile_MagAndAng_3x3_MagThr[] asm(
	"_binary_data_result_sample_tile_MagAndAng_Thresh3x3_Mag_yuv_start");
extern char data_tile_MagAndAng_5x5_MagThr[] asm(
	"_binary_data_result_sample_tile_MagAndAng_Thresh5x5_Mag_yuv_start");

// Map result data
extern char result_Map[] asm("_binary_data_result_sample_Map_yuv_start");

// NCC result data
extern char data_NCC[] asm("_binary_data_result_sample_NCC_Mem_bin_start");

// NormGrad result data
extern char data_NormGrad_Hor3x3[] asm(
	"_binary_data_result_sample_NormGrad_Hor3x3_yuv_start");
extern char data_NormGrad_Ver3x3[] asm(
	"_binary_data_result_sample_NormGrad_Ver3x3_yuv_start");
extern char data_NormGrad_Hor5x5[] asm(
	"_binary_data_result_sample_NormGrad_Hor5x5_yuv_start");
extern char data_NormGrad_Ver5x5[] asm(
	"_binary_data_result_sample_NormGrad_Ver5x5_yuv_start");
extern char data_NormGrad_Combine3x3[] asm(
	"_binary_data_result_sample_NormGrad_Combine3x3_yuv_start");
extern char data_tile_NormGrad_Hor3x3[] asm(
	"_binary_data_result_sample_tile_NormGrad_Hor3x3_yuv_start");
extern char data_tile_NormGrad_Ver3x3[] asm(
	"_binary_data_result_sample_tile_NormGrad_Ver3x3_yuv_start");
extern char data_tile_NormGrad_Hor5x5[] asm(
	"_binary_data_result_sample_tile_NormGrad_Hor5x5_yuv_start");
extern char data_tile_NormGrad_Ver5x5[] asm(
	"_binary_data_result_sample_tile_NormGrad_Ver5x5_yuv_start");
extern char data_tile_NormGrad_Combine3x3[] asm(
	"_binary_data_result_sample_tile_NormGrad_Combine3x3_yuv_start");

// Or result data
extern char data_Or[] asm("_binary_data_result_sample_Or_yuv_start");
extern char data_tile_Or[] asm("_binary_data_result_sample_tile_Or_yuv_start");

// OrdStatFilter result data
extern char data_OrdStaFilter_Max[] asm(
	"_binary_data_result_sample_OrdStaFilter_Max_yuv_start");
extern char data_OrdStaFilter_Min[] asm(
	"_binary_data_result_sample_OrdStaFilter_Min_yuv_start");
extern char data_OrdStaFilter_Median[] asm(
	"_binary_data_result_sample_OrdStaFilter_Median_yuv_start");

// Resize result data
extern char data_Resize_Area_240p[] asm(
	"_binary_data_result_sample_Resize_Area_240p_rgb_start");
extern char data_Resize_Area_gray[] asm(
	"_binary_data_result_sample_Resize_Area_gray_yuv_start");
extern char data_Resize_Area_rgb[] asm(
	"_binary_data_result_sample_Resize_Area_rgb_rgb_start");
extern char data_Resize_Bilinear_240p[] asm(
	"_binary_data_result_sample_Resize_Bilinear_240p_rgb_start");
extern char data_Resize_Bilinear_gray[] asm(
	"_binary_data_result_sample_Resize_Bilinear_gray_yuv_start");
extern char data_Resize_Bilinear_rgb[] asm(
	"_binary_data_result_sample_Resize_Bilinear_rgb_rgb_start");

// Sad result data
extern char data_Sad_mode0_out0[] asm(
	"_binary_data_result_sample_Sad_sad_mode0_out0_bin_start");
extern char data_Sad_mode0_out1[] asm(
	"_binary_data_result_sample_Sad_sad_mode0_out1_bin_start");
extern char data_Sad_mode1_out0[] asm(
	"_binary_data_result_sample_Sad_sad_mode1_out0_bin_start");
extern char data_Sad_mode1_out1[] asm(
	"_binary_data_result_sample_Sad_sad_mode1_out1_bin_start");
extern char data_Sad_mode2_out0[] asm(
	"_binary_data_result_sample_Sad_sad_mode2_out0_bin_start");
extern char data_Sad_mode2_out1[] asm(
	"_binary_data_result_sample_Sad_sad_mode2_out1_bin_start");
extern char data_Sad_thr_mode0_out0[] asm(
	"_binary_data_result_sample_Sad_thr_mode0_out0_bin_start");
extern char data_Sad_thr_mode0_out1[] asm(
	"_binary_data_result_sample_Sad_thr_mode0_out1_bin_start");
extern char data_Sad_thr_mode1_out0[] asm(
	"_binary_data_result_sample_Sad_thr_mode1_out0_bin_start");
extern char data_Sad_thr_mode1_out1[] asm(
	"_binary_data_result_sample_Sad_thr_mode1_out1_bin_start");
extern char data_Sad_thr_mode2_out0[] asm(
	"_binary_data_result_sample_Sad_thr_mode2_out0_bin_start");
extern char data_Sad_thr_mode2_out1[] asm(
	"_binary_data_result_sample_Sad_thr_mode2_out1_bin_start");

// Sobel result data
extern char data_Sobel_Hor3x3[] asm(
	"_binary_data_result_sample_Sobel_Hor3x3_yuv_start");
extern char data_Sobel_Ver3x3[] asm(
	"_binary_data_result_sample_Sobel_Ver3x3_yuv_start");
extern char data_Sobel_Hor5x5[] asm(
	"_binary_data_result_sample_Sobel_Hor5x5_yuv_start");
extern char data_Sobel_Ver5x5[] asm(
	"_binary_data_result_sample_Sobel_Ver5x5_yuv_start");

extern char data_tile_Sobel_Hor3x3[] asm(
	"_binary_data_result_sample_tile_Sobel_Hor3x3_yuv_start");
extern char data_tile_Sobel_Ver3x3[] asm(
	"_binary_data_result_sample_tile_Sobel_Ver3x3_yuv_start");
extern char data_tile_Sobel_Hor5x5[] asm(
	"_binary_data_result_sample_tile_Sobel_Hor5x5_yuv_start");
extern char data_tile_Sobel_Ver5x5[] asm(
	"_binary_data_result_sample_tile_Sobel_Ver5x5_yuv_start");

// CandiCorner result data
extern char data_CandiCorner[] asm(
	"_binary_data_result_sample_Shitomasi_CandiCorner_yuv_start");
extern char data_tile_CandiCorner[] asm(
	"_binary_data_result_sample_tile_Shitomasi_sky_640x480_yuv_start");

// Sub result data
extern char data_Sub[] asm("_binary_data_result_sample_Sub_Abs_yuv_start");
extern char data_tile_Sub[] asm(
	"_binary_data_result_sample_tile_Sub_Abs_yuv_start");

// Thresh result data
extern char data_Thresh_Binary[] asm(
	"_binary_data_result_sample_Thresh_Binary_yuv_start");
extern char data_Thresh_Trunc[] asm(
	"_binary_data_result_sample_Thresh_Trunc_yuv_start");
extern char data_Thresh_ToMinVal[] asm(
	"_binary_data_result_sample_Thresh_ToMinVal_yuv_start");
extern char data_Thresh_MinMidMax[] asm(
	"_binary_data_result_sample_Thresh_MinMidMax_yuv_start");
extern char data_Thresh_MinMidOri[] asm(
	"_binary_data_result_sample_Thresh_MinMidOri_yuv_start");
extern char data_Thresh_MinOriMax[] asm(
	"_binary_data_result_sample_Thresh_MinOriMax_yuv_start");
extern char data_Thresh_OriMidMax[] asm(
	"_binary_data_result_sample_Thresh_OriMidMax_yuv_start");
extern char data_Thresh_OriMidOri[] asm(
	"_binary_data_result_sample_Thresh_OriMidOri_yuv_start");

extern char data_tile_Thresh_Binary[] asm(
	"_binary_data_result_sample_tile_Thresh_Binary_yuv_start");
extern char data_tile_Thresh_Trunc[] asm(
	"_binary_data_result_sample_tile_Thresh_Trunc_yuv_start");
extern char data_tile_Thresh_ToMinVal[] asm(
	"_binary_data_result_sample_tile_Thresh_ToMinVal_yuv_start");
extern char data_tile_Thresh_MinMidMax[] asm(
	"_binary_data_result_sample_tile_Thresh_MinMidMax_yuv_start");
extern char data_tile_Thresh_MinMidOri[] asm(
	"_binary_data_result_sample_tile_Thresh_MinMidOri_yuv_start");
extern char data_tile_Thresh_MinOriMax[] asm(
	"_binary_data_result_sample_tile_Thresh_MinOriMax_yuv_start");
extern char data_tile_Thresh_OriMidMax[] asm(
	"_binary_data_result_sample_tile_Thresh_OriMidMax_yuv_start");
extern char data_tile_Thresh_OriMidOri[] asm(
	"_binary_data_result_sample_tile_Thresh_OriMidOri_yuv_start");

// ThreshS16 result data
extern char data_ThreshS16_S16ToS8_MinMidMax[] asm(
	"_binary_data_result_sample_Thresh_S16_To_S8_MinMidMax_yuv_start");
extern char data_ThreshS16_S16ToS8_MinOriMax[] asm(
	"_binary_data_result_sample_Thresh_S16_To_S8_MinOriMax_yuv_start");
extern char data_ThreshS16_S16ToU8_MinMidMax[] asm(
	"_binary_data_result_sample_Thresh_S16_To_U8_MinMidMax_yuv_start");
extern char data_ThreshS16_S16ToU8_MinOriMax[] asm(
	"_binary_data_result_sample_Thresh_S16_To_U8_MinOriMax_yuv_start");

// ThreshU16 result data
extern char data_ThreshU16_MinMidMax[] asm(
	"_binary_data_result_sample_Thresh_U16_To_U8_MinMidMax_yuv_start");
extern char data_ThreshU16_MinOriMax[] asm(
	"_binary_data_result_sample_Thresh_U16_To_U8_MinOriMax_yuv_start");

// FilterAndCSC result data
extern char data_FilterAndCSC_Y3x3[] asm(
	"_binary_data_result_sample_FilterAndCSC_420SPToVideoPlanar3x3_yuv_start");
extern char data_FilterAndCSC_Y5x5[] asm(
	"_binary_data_result_sample_FilterAndCSC_420SPToVideoPlanar5x5_yuv_start");

// Xor result data
extern char data_Xor[] asm("_binary_data_result_sample_Xor_yuv_start");
extern char
	data_tile_Xor[] asm("_binary_data_result_sample_tile_Xor_yuv_start");

#endif /* __CVI_PATTERN_H__ */