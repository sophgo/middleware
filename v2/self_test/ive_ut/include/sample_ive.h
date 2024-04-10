#ifndef __SAMPLE_IVE_H__
#define __SAMPLE_IVE_H__

enum TILE_MODE {
	SMALL_PIC_TEST = 0,
	TILE_MODE_TEST = 1,
};

enum TESTCASE_ID {
	TESTCASE_All = 0,
	TESTCASE_Add = 1,
	TESTCASE_Sub,
	TESTCASE_And,
	TESTCASE_Xor,
	TESTCASE_Or = 5,
	TESTCASE_Erode,
	TESTCASE_Dilate,
	TESTCASE_SAD,
	TESTCASE_Resize,
	TESTCASE_16BitTo8Bit = 10,
	TESTCASE_Threshold,
	TESTCASE_Thresh_S16,
	TESTCASE_Thresh_U16,
	TESTCASE_NCC,
	TESTCASE_Filter = 15,
	TESTCASE_LBP,
	TESTCASE_CSC,
	TESTCASE_FilterAndCSC,
	TESTCASE_DMA,
	TESTCASE_OrdStatFilter = 20,
	TESTCASE_Sobel,
	TESTCASE_MagAndAng,
	TESTCASE_Integ,
	TESTCASE_HIST,
	TESTCASE_MAP = 25,
	TESTCASE_NormGrad,
	TESTCASE_Bernsen,
	TESTCASE_GradFg,
	TESTCASE_CannyHysEdge,
	TESTCASE_CannyEdge = 30,
	TESTCASE_StCandiCorner,
	TESTCASE_FrameDiff,
	TESTCASE_CCL,
	TESTCASE_Multi_thread,
	TESTCASE_GMM,
	TESTCASE_GMM2,
	TESTCASE_BgModel,
	TESTCASE_TILE_Add = 50,
	TESTCASE_TILE_Sub,
	TESTCASE_TILE_And,
	TESTCASE_TILE_Xor,
	TESTCASE_TILE_Or,
	TESTCASE_TILE_Erode = 55,
	TESTCASE_TILE_Dilate,
	TESTCASE_TILE_Sobel,
	TESTCASE_TILE_MagAndAng,
	TESTCASE_TILE_NormGrad,
	TESTCASE_TILE_GradFg = 60,
	TESTCASE_TILE_CannyHysEdge,
	TESTCASE_TILE_CannyEdge,
	TESTCASE_TILE_StCandiCorner,
	TESTCASE_TILE_Threshold,
	TESTCASE_TILE_GMM,
	TESTCASE_TILE_GMM2,
	TESTCASE_TILE_BgModel,
	TESTCASE_TILE_FrameDiff = 68,
	TESTCASE_TILE_LBP,
	TESTCASE_TILE_OrdStatFilter = 70,
	TESTCASE_TILE_Filter,
	TESTCASE_TILE_Bernsen,
	TESTCASE_TILE_Map, //73
	TESTCASE_TILE_FilterAndCSC,
	TESTCASE_TILE_THresh_U16, // 75
	TESTCASE_TILE_THresh_S16,
	TESTCASE_TILE_16BitTo8Bit,
	TESTCASE_CMDQ,
	TESTCASE_IVE_RESET = 100,
	TESTCASE_PRINT_REG = 101,
	TESTCASE_Base_All,
	TESTCASE_Tile_All,
	TESTCASE_ImgToOdma,
	TESTCASE_TILE_ImgToOdma,
};
typedef struct {
	int bTileMode;
	int bWrite;
	int bInstant;
	int loop;
} multi_thread_param;

int test_add(int bTileMode, int bWrite, int bInstant);
int test_sub(int bTileMode, int bWrite, int bInstant);
int test_xor(int bTileMode, int bWrite, int bInstant);
int test_and(int bTileMode, int bWrite, int bInstant);
int test_or(int bTileMode, int bWrite, int bInstant);
int test_erode(int bTileMode, int bWrite, int bInstant);
int test_dilate(int bTileMode, int bWrite, int bInstant);
int test_16botto8bit(int bTileMode, int bWrite, int bInstant);
int test_thresh_s16(int bTileMode, int bWrite, int bInstant);
int test_thresh_u16(int bTileMode, int bWrite, int bInstant);
int test_threshold(int bTileMode, int bWrite, int bInstant);
int test_filterandcsc(int bTileMode, int bWrite, int bInstant);
int test_csc(int bTileMode, int bWrite, int bInstant);
int test_lbp(int bTileMode, int bWrite, int bInstant);
int test_ordstatfilter(int bTileMode, int bWrite, int bInstant);
int test_filter(int bTileMode, int bWrite, int bInstant);
int test_sobel(int bTileMode, int bWrite, int bInstant);
int test_magandang(int bTileMode, int bWrite, int bInstant);
int test_map(int bTileMode, int bWrite, int bInstant);
int test_normgrad(int bTileMode, int bWrite, int bInstant);
int test_bernsen(int bTileMode, int bWrite, int bInstant);
int test_gradfg(int bTileMode, int bWrite, int bInstant);
int test_cannyhysedge(int bTileMode, int bWrite, int bInstant);
int test_cannyedge(int bTileMode, int bWrite, int bInstant);
int test_imgtoodma(int bTileMode, int bWrite, int bInstant);
int test_stcandicorner(int bTileMode, int bWrite, int bInstant);
int test_framediffmotion(int bTileMode, int bWrite, int bInstant);
int test_ccl(int bWrite, int bInstant);
int test_multi_thread(int bTileMode, int bWrite, int bInstant);
int test_gmm(int bTileMode, int bWrite, int bInstant);
int test_gmm2(int bTileMode, int bWrite, int bInstant);
int test_bgmodel(int bTileMode, int bWrite, int bInstant);
int test_resize(int bWrite, int bInstant);
int test_sad(int bWrite, int bInstant);
int test_ncc(int bWrite, int bInstant);
int test_integ(int bWrite, int bInstant);
int test_hist(int bWrite, int bInstant);
int test_dma(int bWrite, int bInstant);
int test_cmdq(void);
int reset(void);
int dump(void);
#endif /* __SAMPLE_IVE_H__ */
