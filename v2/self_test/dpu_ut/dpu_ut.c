#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h> // seg fault

#include "cvi_buffer.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_dpu.h"
#include "dpu_ut_comm.h"

#undef DPU_ALIGN
#define DPU_ALIGN(src,align) ((src+(align-1)) & ~((align-1)))

#define UT_MODE 0
#define FILE_IN_LEFT "res/dpu/imgL_sofa_512x284.bin"
#define FILE_IN_RIGHT "res/dpu/imgR_sofa_512x284.bin"
#define FILE_IN_SGBM_RESULT "res/dpu/sgbm_result_sofa_512x284.bin"
#define FILE_OUT "res/dpu/dpu_out_512x284"
#define FILE_COSTMAP_IN_LEFT "res/dpu/64x64_left_img.bin"
#define FILE_COSTMAP_IN_RIGHT "res/dpu/64x64_right_img.bin"
#define FILE_COSTMAP_OUT "res/dpu/btcost_res.bin"
#define FILE_RESULT_SGBM_MUX0 "res/dpu/dpu_result_sgbm_mux0.bin"
#define FILE_RESULT_SGBM_MUX1 "res/dpu/dpu_result_sgbm_mux1.bin"
#define FILE_RESULT_SGBM_MUX2 "res/dpu/dpu_result_sgbm_mux2.bin"
#define FILE_RESULT_ONLINE_MUX0 "res/dpu/dpu_result_online_mux0.bin"
#define FILE_RESULT_ONLINE_MUX1 "res/dpu/dpu_result_online_mux1.bin"
#define FILE_RESULT_ONLINE_MUX2 "res/dpu/dpu_result_online_mux2.bin"
#define FILE_RESULT_FGS_MUX0 "res/dpu/dpu_result_fgs_mux0.bin"
#define FILE_RESULT_FGS_MUX1 "res/dpu/dpu_result_fgs_mux1.bin"
#define FILE_RESULT_SGBM_COSTMAP "res/dpu/btcost_ref.bin"
#define FILE_DPU_INI "dpu.ini"

#define DEFAULT_W 1920
#define DEFAULT_H 1080
#define ALIGN_16  16
#define ALIGN_32  32

#define TIMEOUT_GET_FRAME 10000

typedef enum _DPU_GOLDEN_MODE{
	DPU_GOLDEN_JADEPENT,
	DPU_GOLDEN_PIANO,
	DPU_GOLDEN_TEDDY,
}DPU_GOLDEN_MODE_E;

static DPU_GRP_ATTR_S grp_attr;
static DPU_GRP_ATTR_S grp_attr_zero;
static DPU_GRP_ATTR_S grp_attr_res;
static DPU_CHN_ATTR_S chn_attr_zero;
static DPU_CHN_ATTR_S chn_attr;
DPU_GRP dpuGrp ;
DPU_CHN dpuChn ;
static char fileNameInLeft[128] ="0";
static char fileNameInRight[128]="0";
static char fileNameOut[128]="0";
static char fileNameResult[128]="0";
static char fileNameOutBtcost[128]="0";

#define DPU_SOFA_L "input/sofa_left_img.bin"
#define DPU_SOFA_R "input/sofa_right_img.bin"
#define DPU_SGBM_R "input/sgbm_u8_median_res.bin"

#define DPU_OCTOGONS_L "input/Octogons_left_img.bin"
#define DPU_OCTOGONS_R "input/Octogons_right_img.bin"

#define DPU_VINTAGE_L "input/vintage_left_img.bin"
#define DPU_VINTAGE_R "input/vintage_right_img.bin"

#define DPU_RANDOM_L "input/Random_left_img.bin"
#define DPU_RANDOM_R "input/Random_right_img.bin"

#define DPU_PENDULUM_L "input/Pendulum_left_img.bin"
#define DPU_PENDULUM_R "input/Pendulum_right_img.bin"
#define DPU_PENDULUM_SGBM_R "input/Pendulum_sgbm_u8_median_res.bin"

#define DPU_OHTA_L "input/Ohta_left_img.bin"
#define DPU_OHTA_R "input/Ohta_right_img.bin"
#define DPU_OHTA_SGBM_R "input/Ohta_sgbm_u8_median_res.bin"

#define DPU_RECYCLE_L "input/Recycle_left_img.bin"
#define DPU_RECYCLE_R "input/Recycle_right_img.bin"

#define DPU_TEDDY_L "input/Teddy_left_img.bin"
#define DPU_TEDDY_R "input/Teddy_right_img.bin"

#define DPU_PIANO_L "input/Piano_left_img.bin"
#define DPU_PIANO_R "input/Piano_right_img.bin"

#define DPU_JADEPLANT_L "input/Jadeplant_left_img.bin"
#define DPU_JADEPLANT_R "input/Jadeplant_right_img.bin"

#define DPU_BTCOST_L "input/64x64_left_img.bin"
#define DPU_BTCOST_R "input/64x64_right_img.bin"

#define DPU_804_L "input/804_left_img.bin"
#define DPU_804_R "input/804_right_img.bin"
#define DPU_804_SGBM_R "input/804_sgbm_u8_median_res.bin"

#define DPU_1608_L "input/1608_left_img.bin"
#define DPU_1608_R "input/1608_right_img.bin"
#define DPU_1608_SGBM_R "input/1608_sgbm_u8_median_res.bin"

CVI_U32 tmp = 0;
#define cvi_get_value(str, var) \
	{ \
		DPU_UT_PRT(str); \
		scanf("%d", &tmp); \
		var = tmp; \
	}

CVI_CHAR fileName[128] = "0";
#define cvi_get_filename(str, var) \
	{ \
		DPU_UT_PRT(str); \
		scanf("%128s", fileName); \
		strcpy(var, fileName); \
	}

#define DPU_DEFAULT_FILE_IN  "res/1080p.yuv420"  //1920*1080


// void set_attr_ut(DPU_GRP_ATTR_S *pstGrpAttr)
// {
// 	pstGrpAttr->enDpuMode =DPU_MODE_SGBM_MUX2;
// 	pstGrpAttr->enMaskMode=DPU_MASK_MODE_7x7;
// 	pstGrpAttr->enDispRange =DPU_DISP_RANGE_16;
// 	pstGrpAttr->u16DispStartPos=0;
// 	pstGrpAttr->u32Rshift1 =3;
// 	pstGrpAttr->u32Rshift2 =2;
// 	pstGrpAttr->u32CaP1=1800;
// 	pstGrpAttr->u32CaP2=14400;
// 	pstGrpAttr->u32UniqRatio=25;
// 	pstGrpAttr->u32DispShift =4;
// 	pstGrpAttr->u32CensusShift=1;
// 	pstGrpAttr->u32FxBaseline=864000;
// 	pstGrpAttr->enDccDir = DPU_DCC_DIR_A12;
// 	pstGrpAttr->u32FgsMaxCount=19;
// 	pstGrpAttr->u32FgsMaxT=3;
// 	pstGrpAttr->enDpuDepthUnit=DPU_DEPTH_UNIT_MM;
// 	pstGrpAttr->bIsBtcostOut=false;
// }

typedef enum _DPU_TEST_OP {
	DPU_CHECK_REG_READ,
	DPU_CHECK_REG_WRITE,
	DPU_CHECK_SGBM_STATUS,
	DPU_CHECK_FGS_STATUS,
	DPU_MODE_SGBM_MUX0_TEST = 4,
	DPU_MODE_SGBM_MUX1_TEST,
	DPU_MODE_SGBM_MUX2_TEST,
	DPU_MODE_ONLINE_MUX0_TEST,
	DPU_MODE_ONLINE_MUX1_TEST,
	DPU_MODE_ONLINE_MUX2_TEST,
	DPU_MODE_FGS_MUX0_TEST,
	DPU_MODE_FGS_MUX1_TEST,
	DPU_MODE_SGBM_COSTMAP_TEST,
	DPU_MODE_BTCOST_TEST,
	DPU_MODE_AUTO_TEST = 100,
	DPU_MODE_PRESS_TEST1 = 151,
	DPU_MODE_PRESS_TEST2,
	DPU_MODE_TEST = 200,
	DPU_MODE_FGS_TEST,
	DPU_DISP_RANGE_TEST,
	DPU_CENSUS_TEST,
	DPU_ADD_TEST,
	DPU_BOXFILTER_TEST,
	DPU_DCC_TEST,
	DPU_UNIQ_CHECK_TEST,
	DPU_DISPINTERP_TEST,
	DPU_U16TOU8_TEST,
	DPU_FGS_COUNT_TEST,
	DPU_FGS_MAX_T_TEST,
	DPU_DEPTH_TEST,
	DPU_UNIT_CHOOSE_TEST,
	DPU_BTCOST_TEST,
	DPU_SIZE_TEST,
	DPU_GOLDEN_TEST,
	DPU_AUTO_REGRESSION_TEST,
	DPU_FILE_TEST,
} DPU_TEST_OP;

void set_attr_default()
{
	grp_attr.enDpuMode = DPU_MODE_SGBM_MUX2;
	grp_attr.enMaskMode= DPU_MASK_MODE_7x7;
	grp_attr.enDispRange = DPU_DISP_RANGE_16;
	grp_attr.u16DispStartPos = 0;
	grp_attr.u32Rshift1 = 3;
	grp_attr.u32Rshift2 = 2;
	grp_attr.u32CaP1 = 1800;
	grp_attr.u32CaP2 = 14400;
	grp_attr.u32UniqRatio = 25;
	grp_attr.u32DispShift = 4;
	grp_attr.u32CensusShift = 1;
	grp_attr.u32FxBaseline = 864000;
	grp_attr.enDccDir = DPU_DCC_DIR_A12;
	grp_attr.u32FgsMaxCount = 19;
	grp_attr.u32FgsMaxT = 3;
	grp_attr.enDpuDepthUnit = DPU_DEPTH_UNIT_MM;
	grp_attr.bIsBtcostOut = false;

	dpuGrp =0;
	dpuChn =0;
}

void set_attr_golden_jadepent()
{
	grp_attr.enDpuMode = DPU_MODE_SGBM_MUX2;
	grp_attr.enMaskMode= DPU_MASK_MODE_7x7;
	grp_attr.enDispRange = DPU_DISP_RANGE_128;
	grp_attr.u16DispStartPos = 0;
	grp_attr.u32Rshift1 = 0;
	grp_attr.u32Rshift2 = 2;
	grp_attr.u32CaP1 = 1800;
	grp_attr.u32CaP2 = 14400;
	grp_attr.u32UniqRatio = 25;
	grp_attr.u32DispShift = 4;
	grp_attr.u32CensusShift = 1;
	grp_attr.u32FxBaseline = 864000;
	grp_attr.enDccDir = DPU_DCC_DIR_A14;
	grp_attr.u32FgsMaxCount = 19;
	grp_attr.u32FgsMaxT = 3;
	grp_attr.enDpuDepthUnit = DPU_DEPTH_UNIT_MM;
	grp_attr.bIsBtcostOut = false;

	grp_attr.stLeftImageSize.u32Width = 656;
	grp_attr.stLeftImageSize.u32Height = 496;

	grp_attr.stRightImageSize.u32Width = 656;
	grp_attr.stRightImageSize.u32Height = 496;

	chn_attr.stImgSize.u32Width =656;
	chn_attr.stImgSize.u32Height =496;

	dpuGrp =0;
	dpuChn =0;
}

void set_attr_golden_piano()
{
	grp_attr.enDpuMode = DPU_MODE_SGBM_MUX2;
	grp_attr.enMaskMode= DPU_MASK_MODE_7x7;
	grp_attr.enDispRange = DPU_DISP_RANGE_64;
	grp_attr.u16DispStartPos = 0;
	grp_attr.u32Rshift1 = 0;
	grp_attr.u32Rshift2 = 2;
	grp_attr.u32CaP1 = 1800;
	grp_attr.u32CaP2 = 14400;
	grp_attr.u32UniqRatio = 25;
	grp_attr.u32DispShift = 4;
	grp_attr.u32CensusShift = 1;
	grp_attr.u32FxBaseline = 864000;
	grp_attr.enDccDir = DPU_DCC_DIR_A14;
	grp_attr.u32FgsMaxCount = 19;
	grp_attr.u32FgsMaxT = 3;
	grp_attr.enDpuDepthUnit = DPU_DEPTH_UNIT_MM;
	grp_attr.bIsBtcostOut = false;

	grp_attr.stLeftImageSize.u32Width = 704;
	grp_attr.stLeftImageSize.u32Height = 480;

	grp_attr.stRightImageSize.u32Width = 704;
	grp_attr.stRightImageSize.u32Height = 480;

	chn_attr.stImgSize.u32Width =704;
	chn_attr.stImgSize.u32Height =480;

	dpuGrp =0;
	dpuChn =0;
}

void set_attr_golden_teddy()
{
	grp_attr.enDpuMode = DPU_MODE_SGBM_MUX2;
	grp_attr.enMaskMode= DPU_MASK_MODE_7x7;
	grp_attr.enDispRange = DPU_DISP_RANGE_64;
	grp_attr.u16DispStartPos = 0;
	grp_attr.u32Rshift1 = 0;
	grp_attr.u32Rshift2 = 2;
	grp_attr.u32CaP1 = 1800;
	grp_attr.u32CaP2 = 14400;
	grp_attr.u32UniqRatio = 25;
	grp_attr.u32DispShift = 4;
	grp_attr.u32CensusShift = 1;
	grp_attr.u32FxBaseline = 864000;
	grp_attr.enDccDir = DPU_DCC_DIR_A14;
	grp_attr.u32FgsMaxCount = 19;
	grp_attr.u32FgsMaxT = 3;
	grp_attr.enDpuDepthUnit = DPU_DEPTH_UNIT_MM;
	grp_attr.bIsBtcostOut = false;

	grp_attr.stLeftImageSize.u32Width = 448;
	grp_attr.stLeftImageSize.u32Height = 368;

	grp_attr.stRightImageSize.u32Width = 448;
	grp_attr.stRightImageSize.u32Height = 368;

	chn_attr.stImgSize.u32Width =448;
	chn_attr.stImgSize.u32Height =368;

	dpuGrp =0;
	dpuChn =0;
}

void setFlipBin(CVI_CHAR *infilename,CVI_CHAR *outfilename,CVI_S32 width,CVI_S32 height,CVI_S32 flag)
{
	FILE *infile;
	FILE *outfile;
	infile =fopen(infilename,"rb");
	outfile = fopen(outfilename,"wb");
	CVI_U8 * buffer=malloc(width*height);
	fread(buffer,1,width*height,infile);
	if(flag == 1){
		for(int i=0;i<height;++i){
			for(int j=(width-1);j>=0;--j){
				fwrite(&buffer[i*width+j],1,1,outfile);
			}
		}
	}else if(flag ==2){
		for(int i=(height-1);i>=0;--i){
			for(int j=0;j<width;++j){
				fwrite(&buffer[i*width+j],1,1,outfile);
			}
		}
	}else if(flag ==3){
		for(int i=(height-1);i>=0;--i){
			for(int j=(width-1);j>=0;--j){
				fwrite(&buffer[i*width+j],1,1,outfile);
			}
		}
	}else{
		strcpy(outfilename,infilename);
	}
	free(buffer);
	fclose(infile);
	fclose(outfile);
}


void dpu_ut_HandleSig(CVI_S32 signo)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	if (SIGINT == signo || SIGTERM == signo) {
		CVI_SYS_Exit();
		CVI_VB_Exit();
		DPU_UT_PRT("Program termination abnormally\n");
	}
	exit(-1);
}

CVI_S32 CompareGrpAttr(const DPU_GRP_ATTR_S *pstGrpAttrSrc,const DPU_GRP_ATTR_S *pstGrpAttrDst)
{
	CVI_S32 failCnt = 0;
	if(pstGrpAttrSrc == NULL){
		DPU_UT_PRT("pstGrpAttrSrc is null!\n");
		return CVI_FAILURE;
	}

	if(pstGrpAttrDst == NULL){
		DPU_UT_PRT("pstGrpAttrDst is null!\n");
		return CVI_FAILURE;
	}

	if(pstGrpAttrSrc->enDccDir != pstGrpAttrDst->enDccDir ){
		DPU_UT_PRT("enDccDir is diff!\n");
		failCnt += 1;
	}

	if(pstGrpAttrSrc->enDispRange != pstGrpAttrDst->enDispRange ){
		DPU_UT_PRT("enDispRange is diff!\n");
		failCnt += 1;
	}

	if(pstGrpAttrSrc->enDpuDepthUnit != pstGrpAttrDst->enDpuDepthUnit ){
		DPU_UT_PRT("enDpuDepthUnit is diff!\n");
		failCnt += 1;
	}

	if(pstGrpAttrSrc->enDpuMode != pstGrpAttrDst->enDpuMode ){
		DPU_UT_PRT("enDpuMode is diff!\n");
		failCnt += 1;
	}

	if(pstGrpAttrSrc->enMaskMode != pstGrpAttrDst->enMaskMode ){
		DPU_UT_PRT("enMaskMode is diff!\n");
		failCnt += 1;
	}

	if(pstGrpAttrSrc->u16DispStartPos != pstGrpAttrDst->u16DispStartPos ){
		DPU_UT_PRT("u16DispStartPos is diff!\n");
		failCnt += 1;
	}

	if(pstGrpAttrSrc->u32CaP1 != pstGrpAttrDst->u32CaP1 ){
		DPU_UT_PRT("u32CaP1 is diff!\n");
		failCnt += 1;
	}

	if(pstGrpAttrSrc->u32CaP2 != pstGrpAttrDst->u32CaP2 ){
		DPU_UT_PRT("u32CaP2 is diff!\n");
		failCnt += 1;
	}

	if(pstGrpAttrSrc->u32CensusShift != pstGrpAttrDst->u32CensusShift ){
		DPU_UT_PRT("u32CensusShift is diff!\n");
		failCnt += 1;
	}

	if(pstGrpAttrSrc->u32DispShift != pstGrpAttrDst->u32DispShift ){
		DPU_UT_PRT("u32DispShift is diff!\n");
		failCnt += 1;
	}

	if(pstGrpAttrSrc->u32FgsMaxCount != pstGrpAttrDst->u32FgsMaxCount ){
		DPU_UT_PRT("u32FgsMaxCount is diff!\n");
		failCnt += 1;
	}

	if(pstGrpAttrSrc->u32FgsMaxT != pstGrpAttrDst->u32FgsMaxT ){
		DPU_UT_PRT("u32FgsMaxT is diff!\n");
		failCnt += 1;
	}

	if(pstGrpAttrSrc->u32FxBaseline != pstGrpAttrDst->u32FxBaseline ){
		DPU_UT_PRT("u32FxBaseline is diff!\n");
		failCnt += 1;
	}

	if(pstGrpAttrSrc->u32Rshift1 != pstGrpAttrDst->u32Rshift1 ){
		DPU_UT_PRT("u32Rshift1 is diff!\n");
		failCnt += 1;
	}

	if(pstGrpAttrSrc->u32Rshift2 != pstGrpAttrDst->u32Rshift2 ){
		DPU_UT_PRT("u32Rshift2 is diff!\n");
		failCnt += 1;
	}

	if(pstGrpAttrSrc->u32UniqRatio != pstGrpAttrDst->u32UniqRatio ){
		DPU_UT_PRT("u32UniqRatio is diff!\n");
		failCnt += 1;
	}

	if(failCnt != 0){
		DPU_UT_PRT("grp compare fail!!!\n");
		return CVI_FAILURE;
	}else{
		DPU_UT_PRT("grp compare Pass!!!\n");
		return CVI_SUCCESS;
	}
}

CVI_S32 CompareChnAttr(const DPU_CHN_ATTR_S *pstChnAttrSrc,const DPU_CHN_ATTR_S *pstChnAttrDst)
{
	if(pstChnAttrSrc->stImgSize.u32Width != pstChnAttrDst->stImgSize.u32Width){
		DPU_UT_PRT("chn compare fail!!! src width(%d) dst width(%d)\n",pstChnAttrSrc->stImgSize.u32Width,\
						pstChnAttrDst->stImgSize.u32Width);
		return CVI_FAILURE;
	}
	if(pstChnAttrSrc->stImgSize.u32Height != pstChnAttrDst->stImgSize.u32Height){
		DPU_UT_PRT("chn compare fail!!! src Height(%d) dst Height(%d)\n",pstChnAttrSrc->stImgSize.u32Height,\
						pstChnAttrDst->stImgSize.u32Height);
		return CVI_FAILURE;
	}
	DPU_UT_PRT("chn compare Pass!!! \n");
	return CVI_SUCCESS;
}

static CVI_S32 basic()
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	DPU_GRP DpuGrp = dpuGrp;
	DPU_CHN DpuChn = dpuChn;
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSizeIn_left,u32BlkSizeOut;
	CVI_U32 u32BlkSizeOut16,u32BlkSizeOut_btcost;
	VIDEO_FRAME_INFO_S stVideoFrameInLeft;
	VIDEO_FRAME_INFO_S stVideoFrameInRight;
	VIDEO_FRAME_INFO_S stVideoFrameOut;
	VIDEO_FRAME_INFO_S stVideoFrameOutBtcost;
	VB_BLK vb_left;
	VB_BLK vb_right;
	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	memset(&grp_attr_zero,0,sizeof(DPU_GRP_ATTR_S));
	memset(&chn_attr_zero,0,sizeof(DPU_CHN_ATTR_S));
	//CVI_DPU_Reset();
	u32BlkSizeIn_left = COMMON_GetPicBufferSize(grp_attr.stLeftImageSize.u32Width, grp_attr.stLeftImageSize.u32Height,
		PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_16);
	if(	grp_attr.enDpuMode == DPU_MODE_DEFAULT ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_MUX0 ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_MUX2 ){

		stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn_left;
		stVbConf.astCommPool[0].u32BlkCnt	= 3;
		stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;

		u32BlkSizeOut = COMMON_GetPicBufferSize(grp_attr.stLeftImageSize.u32Width, grp_attr.stLeftImageSize.u32Height,
		PIXEL_FORMAT_YUV_PLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_16);
		stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut;
		stVbConf.astCommPool[1].u32BlkCnt	= 1;
		stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
		DPU_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn_left);
		DPU_UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut);
		if(grp_attr.bIsBtcostOut){
			u32BlkSizeOut_btcost = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width*128, chn_attr.stImgSize.u32Height,
			PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_16, COMPRESS_MODE_NONE, ALIGN_16);
			stVbConf.astCommPool[2].u32BlkSize	= u32BlkSizeOut_btcost;
			stVbConf.astCommPool[2].u32BlkCnt	= 1;
			stVbConf.astCommPool[2].enRemapMode	= VB_REMAP_MODE_CACHED;
			DPU_UT_PRT("common pool[2] BlkSize %d\n", u32BlkSizeOut_btcost);
			stVbConf.u32MaxPoolCnt              = 3;
		}else{
			stVbConf.u32MaxPoolCnt              = 2;
		}

	}else if(grp_attr.enDpuMode == DPU_MODE_FGS_MUX0 ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_FGS_ONLINE_MUX0  ){
		u32BlkSizeOut = COMMON_GetPicBufferSize(grp_attr.stLeftImageSize.u32Width, grp_attr.stLeftImageSize.u32Height,
							PIXEL_FORMAT_YUV_PLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_32);

		if(grp_attr.stLeftImageSize.u32Width % 32 == 0){
			stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn_left;
			stVbConf.astCommPool[0].u32BlkCnt	= 3;
			stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;

			stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut;
			stVbConf.astCommPool[1].u32BlkCnt	= 1;
			stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
			DPU_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn_left);
			DPU_UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut);
			if(grp_attr.bIsBtcostOut){
				u32BlkSizeOut_btcost = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width*128, chn_attr.stImgSize.u32Height,
				PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_16, COMPRESS_MODE_NONE, ALIGN_16);
				stVbConf.astCommPool[2].u32BlkSize	= u32BlkSizeOut_btcost;
				stVbConf.astCommPool[2].u32BlkCnt	= 1;
				stVbConf.astCommPool[2].enRemapMode	= VB_REMAP_MODE_CACHED;
				DPU_UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut_btcost);
				stVbConf.u32MaxPoolCnt              = 3;
			}else{
				stVbConf.u32MaxPoolCnt              = 2;
			}
		}else{
			stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn_left;
			stVbConf.astCommPool[0].u32BlkCnt	= 2;
			stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;

			stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut;
			stVbConf.astCommPool[1].u32BlkCnt	= 1;
			stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
			DPU_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn_left);
			DPU_UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut);
			if(grp_attr.bIsBtcostOut){
				u32BlkSizeOut_btcost = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width*128, chn_attr.stImgSize.u32Height,
				PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_16, COMPRESS_MODE_NONE, ALIGN_16);
				stVbConf.astCommPool[2].u32BlkSize	= u32BlkSizeOut_btcost;
				stVbConf.astCommPool[2].u32BlkCnt	= 1;
				stVbConf.astCommPool[2].enRemapMode	= VB_REMAP_MODE_CACHED;
				DPU_UT_PRT("common pool[2] BlkSize %d\n", u32BlkSizeOut_btcost);
				stVbConf.u32MaxPoolCnt              = 3;
			}else{
				stVbConf.u32MaxPoolCnt              = 2;
			}
		}

	}else if(grp_attr.enDpuMode == DPU_MODE_SGBM_MUX1 ){

		stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn_left;
		stVbConf.astCommPool[0].u32BlkCnt	= 2;
		stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
		DPU_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn_left);

		u32BlkSizeOut16 = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width, chn_attr.stImgSize.u32Height,
		PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_16);
		stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut16*2;
		stVbConf.astCommPool[1].u32BlkCnt	= 1;
		stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
		DPU_UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut16*2);
		if(grp_attr.bIsBtcostOut){
			u32BlkSizeOut_btcost = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width*128, chn_attr.stImgSize.u32Height,
			PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_16, COMPRESS_MODE_NONE, ALIGN_16);
			stVbConf.astCommPool[2].u32BlkSize	= u32BlkSizeOut_btcost;
			stVbConf.astCommPool[2].u32BlkCnt	= 1;
			stVbConf.astCommPool[2].enRemapMode	= VB_REMAP_MODE_CACHED;
			DPU_UT_PRT("common pool[2] BlkSize %d\n", u32BlkSizeOut_btcost);
			stVbConf.u32MaxPoolCnt              = 3;
		}else{
			stVbConf.u32MaxPoolCnt              = 2;
		}

	}else if(grp_attr.enDpuMode == DPU_MODE_FGS_MUX1 ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_FGS_ONLINE_MUX1 ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_FGS_ONLINE_MUX2 ){
		stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn_left;
		stVbConf.astCommPool[0].u32BlkCnt	= 2;
		stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
		DPU_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn_left);

		// u32BlkSizeChfh = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width, chn_attr.stImgSize.u32Height,
		// PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_16);
		// stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeChfh*4;
		// stVbConf.astCommPool[1].u32BlkCnt	= 1;
		// stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
		// DPU_UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeChfh*4);

		u32BlkSizeOut16 = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width, chn_attr.stImgSize.u32Height,
		PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_32);
		stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut16*2;
		stVbConf.astCommPool[1].u32BlkCnt	= 1;
		stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
		DPU_UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut16*2);
		if(grp_attr.bIsBtcostOut){
			u32BlkSizeOut_btcost = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width*128, chn_attr.stImgSize.u32Height,
			PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_16, COMPRESS_MODE_NONE, ALIGN_16);
			stVbConf.astCommPool[2].u32BlkSize	= u32BlkSizeOut_btcost;
			stVbConf.astCommPool[2].u32BlkCnt	= 1;
			stVbConf.astCommPool[2].enRemapMode	= VB_REMAP_MODE_CACHED;
			DPU_UT_PRT("common pool[2] BlkSize %d\n", u32BlkSizeOut_btcost);
			stVbConf.u32MaxPoolCnt              = 3;
		}else{
			stVbConf.u32MaxPoolCnt              = 2;
		}
	}
	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		DPU_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		DPU_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		DPU_UT_PRT("CVI_SYS_Init failed!\n");
		CVI_VB_Exit();
		return s32Ret;
	}


	/************************************************
	 * step2:  Init DPU
	 ************************************************/

	s32Ret = CVI_DPU_CreateGrp(DpuGrp, &grp_attr_zero);
	if (s32Ret != CVI_SUCCESS) {
		DPU_UT_PRT("CVI_DPU_CreateGrp(grp:%d) failed with %#x!\n", DpuGrp, s32Ret);
		CVI_DPU_DestroyGrp(DpuGrp);
		CVI_SYS_Exit();
		CVI_VB_Exit();
		return s32Ret;
	}

	s32Ret = CVI_DPU_SetGrpAttr(DpuGrp, &grp_attr);
	if (s32Ret != CVI_SUCCESS) {
		DPU_UT_PRT("CVI_DPU_SetGrpAttr(grp:%d) failed with %#x!\n", DpuGrp, s32Ret);
		CVI_DPU_DestroyGrp(DpuGrp);
		CVI_SYS_Exit();
		CVI_VB_Exit();
		return s32Ret;
	}

	s32Ret = CVI_DPU_GetGrpAttr(DpuGrp, &grp_attr_res);
	if (s32Ret != CVI_SUCCESS) {
		DPU_UT_PRT("CVI_DPU_GetGrpAttr(grp:%d) failed with %#x!\n", DpuGrp, s32Ret);
		CVI_DPU_DestroyGrp(DpuGrp);
		CVI_SYS_Exit();
		CVI_VB_Exit();
		return s32Ret;
	}

	s32Ret = CompareGrpAttr(&grp_attr_res,&grp_attr);
	if (s32Ret != CVI_SUCCESS) {
		DPU_UT_PRT("CompareGrpAttr(grp:%d) failed with %#x!\n", DpuGrp, s32Ret);
		CVI_DPU_DestroyGrp(DpuGrp);
		CVI_SYS_Exit();
		CVI_VB_Exit();
		return s32Ret;
	}

	s32Ret = CVI_DPU_SetChnAttr(DpuGrp, 0, &chn_attr);
	if (s32Ret != CVI_SUCCESS) {
		DPU_UT_PRT("CVI_DPU_SetChnAttr failed with %#x\n", s32Ret);
		CVI_DPU_DestroyGrp(DpuGrp);
		CVI_SYS_Exit();
		CVI_VB_Exit();
		return s32Ret;
	}

	s32Ret = CVI_DPU_GetChnAttr(DpuGrp, 0, &chn_attr_zero);
	if (s32Ret != CVI_SUCCESS) {
		DPU_UT_PRT("CVI_DPU_GetChnAttr failed with %#x\n", s32Ret);
		CVI_DPU_DestroyGrp(DpuGrp);
		CVI_SYS_Exit();
		CVI_VB_Exit();
		return s32Ret;
	}
	s32Ret = CompareChnAttr(&chn_attr,&chn_attr_zero);
	if (s32Ret != CVI_SUCCESS) {
		CVI_DPU_DestroyGrp(DpuGrp);
		CVI_SYS_Exit();
		CVI_VB_Exit();
		return s32Ret;
	}

	s32Ret = CVI_DPU_EnableChn(DpuGrp, 0);
	if (s32Ret != CVI_SUCCESS) {
		DPU_UT_PRT("CVI_DPU_EnableChn failed with %#x\n", s32Ret);
		CVI_DPU_DestroyGrp(DpuGrp);
		CVI_SYS_Exit();
		CVI_VB_Exit();
		return s32Ret;
	}

	if(grp_attr.bIsBtcostOut){
		s32Ret = CVI_DPU_SetChnAttr(DpuGrp, 1, &chn_attr);
		if (s32Ret != CVI_SUCCESS) {
			DPU_UT_PRT("CVI_DPU_SetChnAttr failed with %#x\n", s32Ret);
			CVI_DPU_DisableChn(DpuGrp, 0);
			CVI_DPU_DestroyGrp(DpuGrp);
			CVI_SYS_Exit();
			CVI_VB_Exit();
			return s32Ret;
		}

	s32Ret = CVI_DPU_EnableChn(DpuGrp,1);
		if (s32Ret != CVI_SUCCESS) {
			DPU_UT_PRT("CVI_DPU_EnableChn failed with %#x\n", s32Ret);
			CVI_DPU_DisableChn(DpuGrp, 0);
			CVI_DPU_DestroyGrp(DpuGrp);
			CVI_SYS_Exit();
			CVI_VB_Exit();
			return s32Ret;
		}
	}

	/*start dpu*/
	s32Ret = CVI_DPU_StartGrp(DpuGrp);
	if (s32Ret != CVI_SUCCESS) {
		DPU_UT_PRT("CVI_DPU_StartGrp failed with %#x\n", s32Ret);
		CVI_DPU_DisableChn(DpuGrp, 0);
		if(grp_attr.bIsBtcostOut)
			CVI_DPU_DisableChn(DpuGrp, 1);
		CVI_DPU_DestroyGrp(DpuGrp);
		CVI_SYS_Exit();
		CVI_VB_Exit();
		return s32Ret;
	}

	//send frame5
	s32Ret = FileToFrame(&grp_attr.stLeftImageSize,PIXEL_FORMAT_YUV_400,fileNameInLeft,&stVideoFrameInLeft);
	if (s32Ret != CVI_SUCCESS) {
		DPU_UT_PRT("[left]FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		CVI_DPU_DisableChn(DpuGrp, 0);
		if(grp_attr.bIsBtcostOut)
			CVI_DPU_DisableChn(DpuGrp, 1);
		CVI_DPU_DestroyGrp(DpuGrp);
		CVI_SYS_Exit();
		CVI_VB_Exit();
		return s32Ret;
	}
	s32Ret = FileToFrame(&grp_attr.stRightImageSize,PIXEL_FORMAT_YUV_400,fileNameInRight,&stVideoFrameInRight);
	if (s32Ret != CVI_SUCCESS) {
		DPU_UT_PRT("[lRight]FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		vb_left = CVI_VB_PhysAddr2Handle(stVideoFrameInLeft.stVFrame.u64PhyAddr[0]);
		if(CVI_VB_ReleaseBlock(vb_left)!=CVI_SUCCESS)
			DPU_UT_PRT("ReleaseBlock blk_left fail !\n");
		CVI_DPU_DisableChn(DpuGrp, 0);
		if(grp_attr.bIsBtcostOut)
			CVI_DPU_DisableChn(DpuGrp, 1);
		CVI_DPU_DestroyGrp(DpuGrp);
		CVI_SYS_Exit();
		CVI_VB_Exit();
		return s32Ret;
	}

	s32Ret = CVI_DPU_SendFrame(DpuGrp,&stVideoFrameInLeft,&stVideoFrameInRight,TIMEOUT_GET_FRAME);
	if(s32Ret != CVI_SUCCESS){
		DPU_UT_PRT("[lRight]SendFrame fail, s32Ret: 0x%x !\n", s32Ret);
		vb_left = CVI_VB_PhysAddr2Handle(stVideoFrameInLeft.stVFrame.u64PhyAddr[0]);
		if(CVI_VB_ReleaseBlock(vb_left)!=CVI_SUCCESS)
			DPU_UT_PRT("ReleaseBlock blk_left fail !\n");
		vb_right = CVI_VB_PhysAddr2Handle(stVideoFrameInRight.stVFrame.u64PhyAddr[0]);
		if(CVI_VB_ReleaseBlock(vb_right)!=CVI_SUCCESS)
			DPU_UT_PRT("ReleaseBlock blk_right fail !\n");
		CVI_DPU_DisableChn(DpuGrp, 0);
		if(grp_attr.bIsBtcostOut)
			CVI_DPU_DisableChn(DpuGrp, 1);
		CVI_DPU_DestroyGrp(DpuGrp);
		CVI_SYS_Exit();
		CVI_VB_Exit();
		return s32Ret;
	}
	// BLK_left =CVI_VB_PhysAddr2Handle(stVideoFrameInLeft.stVFrame.u64PhyAddr[0]);
	// BLK_right =CVI_VB_PhysAddr2Handle(stVideoFrameInRight.stVFrame.u64PhyAddr[0]);
	// CVI_VB_ReleaseBlock(BLK_left);
	// CVI_VB_ReleaseBlock(BLK_right);

	//get frame
	if(grp_attr.bIsBtcostOut){
		DpuChn =1;
		s32Ret = CVI_DPU_GetFrame(DpuGrp,DpuChn,&stVideoFrameOutBtcost,TIMEOUT_GET_FRAME);
		if (s32Ret != CVI_SUCCESS) {
			DPU_UT_PRT("CVI_DPU_GetFrame btcost fail. s32Ret: 0x%x !\n", s32Ret);
			CVI_DPU_DisableChn(DpuGrp, 0);
			CVI_DPU_DisableChn(DpuGrp, 1);
			CVI_DPU_DestroyGrp(DpuGrp);
			CVI_SYS_Exit();
			CVI_VB_Exit();
			return s32Ret;
		}
		// DpuChn =0;
		// s32Ret = CVI_DPU_GetFrame(DpuGrp,DpuChn,&stVideoFrameOut,TIMEOUT_GET_FRAME);
		// if (s32Ret != CVI_SUCCESS) {
		// 	DPU_UT_PRT("CVI_DPU_GetFrame out fail. s32Ret: 0x%x !\n", s32Ret);
		// 	CVI_DPU_ReleaseFrame(DpuGrp, 1, &stVideoFrameOutBtcost);
		// 	CVI_DPU_DisableChn(DpuGrp, 0);
		// 	CVI_DPU_DisableChn(DpuGrp, 1);
		// 	CVI_DPU_DestroyGrp(DpuGrp);
		// 	CVI_SYS_Exit();
		// 	CVI_VB_Exit();
		// 	return s32Ret;
		// }
	}else {
		DpuChn =0;
		s32Ret = CVI_DPU_GetFrame(DpuGrp,DpuChn,&stVideoFrameOut,TIMEOUT_GET_FRAME);
		if (s32Ret != CVI_SUCCESS) {
			DPU_UT_PRT("CVI_DPU_GetFrame out fail. s32Ret: 0x%x !\n", s32Ret);
			CVI_DPU_DisableChn(DpuGrp, 0);
			CVI_DPU_DisableChn(DpuGrp, 1);
			CVI_DPU_DestroyGrp(DpuGrp);
			CVI_SYS_Exit();
			CVI_VB_Exit();
			return s32Ret;
		}
	}

	DPU_UT_PRT("***CVI_DPU_GetChnFrame Success***\n");

	if (grp_attr.bIsBtcostOut) {
		// DpuChn=0;

		// s32Ret = FrameSaveToFile(fileNameOut, &stVideoFrameOut);
		// if (s32Ret != CVI_SUCCESS) {
		// 	DPU_UT_PRT("FrameSaveToFile. out s32Ret: 0x%x !\n", s32Ret);
		// }

		DpuChn=1;
		s32Ret = FrameSaveToFile(fileNameOutBtcost, &stVideoFrameOutBtcost);
		if (s32Ret != CVI_SUCCESS) {
			DPU_UT_PRT("FrameSaveToFile.btcost s32Ret: 0x%x !\n", s32Ret);
		}
		// CVI_DPU_ReleaseFrame(DpuGrp,0,&stVideoFrameOut);
		CVI_DPU_ReleaseFrame(DpuGrp, 1, &stVideoFrameOutBtcost);
		CVI_DPU_DisableChn(DpuGrp, 0);
		CVI_DPU_DisableChn(DpuGrp, 1);
		CVI_DPU_StopGrp(DpuGrp);
		CVI_DPU_DestroyGrp(DpuGrp);
		CVI_SYS_Exit();
		CVI_VB_Exit();
		return s32Ret;

	}else{
		DpuChn=0;
		s32Ret = FrameSaveToFile(fileNameOut, &stVideoFrameOut);
		if (s32Ret != CVI_SUCCESS) {
			DPU_UT_PRT("FrameSaveToFile. out s32Ret: 0x%x !\n", s32Ret);
		}
		CVI_DPU_ReleaseFrame(DpuGrp,0,&stVideoFrameOut);
		CVI_DPU_DisableChn(DpuGrp, 0);
		CVI_DPU_StopGrp(DpuGrp);
		CVI_DPU_DestroyGrp(DpuGrp);
		CVI_SYS_Exit();
		CVI_VB_Exit();
		return s32Ret;
	}

}

static CVI_S32 basic_loop()
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	DPU_GRP DpuGrp = dpuGrp;
	DPU_CHN DpuChn = dpuChn;
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSizeIn_left,u32BlkSizeOut;
	CVI_U32 u32BlkSizeOut16,u32BlkSizeOut_btcost;
	VIDEO_FRAME_INFO_S stVideoFrameInLeft;
	VIDEO_FRAME_INFO_S stVideoFrameInRight;
	VIDEO_FRAME_INFO_S stVideoFrameOut;
	VIDEO_FRAME_INFO_S stVideoFrameOutBtcost;
	VB_BLK vb_left;
	VB_BLK vb_right;
	CVI_S32 times=0;
	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	memset(&grp_attr_zero,0,sizeof(DPU_GRP_ATTR_S));
	//CVI_DPU_Reset();
	u32BlkSizeIn_left = COMMON_GetPicBufferSize(grp_attr.stLeftImageSize.u32Width, grp_attr.stLeftImageSize.u32Height,
		PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_16);

	if(	grp_attr.enDpuMode == DPU_MODE_DEFAULT ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_MUX0 ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_MUX2 ){

		stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn_left;
		stVbConf.astCommPool[0].u32BlkCnt	= 3;
		stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;

		u32BlkSizeOut = COMMON_GetPicBufferSize(grp_attr.stLeftImageSize.u32Width, grp_attr.stLeftImageSize.u32Height,
		PIXEL_FORMAT_YUV_PLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_16);
		stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut;
		stVbConf.astCommPool[1].u32BlkCnt	= 1;
		stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
		DPU_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn_left);
		DPU_UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut);
		if(grp_attr.bIsBtcostOut){
			u32BlkSizeOut_btcost = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width*128, chn_attr.stImgSize.u32Height,
			PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_16, COMPRESS_MODE_NONE, ALIGN_16);
			stVbConf.astCommPool[2].u32BlkSize	= u32BlkSizeOut_btcost;
			stVbConf.astCommPool[2].u32BlkCnt	= 1;
			stVbConf.astCommPool[2].enRemapMode	= VB_REMAP_MODE_CACHED;
			DPU_UT_PRT("common pool[2] BlkSize %d\n", u32BlkSizeOut_btcost);
			stVbConf.u32MaxPoolCnt              = 3;
		}else{
			stVbConf.u32MaxPoolCnt              = 2;
		}

	}else if(grp_attr.enDpuMode == DPU_MODE_FGS_MUX0 ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_FGS_ONLINE_MUX0  ){
		u32BlkSizeOut = COMMON_GetPicBufferSize(grp_attr.stLeftImageSize.u32Width, grp_attr.stLeftImageSize.u32Height,
							PIXEL_FORMAT_YUV_PLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_32);

		if(grp_attr.stLeftImageSize.u32Width % 32 == 0){
			stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn_left;
			stVbConf.astCommPool[0].u32BlkCnt	= 3;
			stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;

			stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut;
			stVbConf.astCommPool[1].u32BlkCnt	= 1;
			stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
			DPU_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn_left);
			DPU_UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut);
			if(grp_attr.bIsBtcostOut){
				u32BlkSizeOut_btcost = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width*128, chn_attr.stImgSize.u32Height,
				PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_16, COMPRESS_MODE_NONE, ALIGN_16);
				stVbConf.astCommPool[2].u32BlkSize	= u32BlkSizeOut_btcost;
				stVbConf.astCommPool[2].u32BlkCnt	= 1;
				stVbConf.astCommPool[2].enRemapMode	= VB_REMAP_MODE_CACHED;
				DPU_UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut_btcost);
				stVbConf.u32MaxPoolCnt              = 3;
			}else{
				stVbConf.u32MaxPoolCnt              = 2;
			}
		}else{
			stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn_left;
			stVbConf.astCommPool[0].u32BlkCnt	= 2;
			stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;

			stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut;
			stVbConf.astCommPool[1].u32BlkCnt	= 1;
			stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
			DPU_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn_left);
			DPU_UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut);
			if(grp_attr.bIsBtcostOut){
				u32BlkSizeOut_btcost = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width*128, chn_attr.stImgSize.u32Height,
				PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_16, COMPRESS_MODE_NONE, ALIGN_16);
				stVbConf.astCommPool[2].u32BlkSize	= u32BlkSizeOut_btcost;
				stVbConf.astCommPool[2].u32BlkCnt	= 1;
				stVbConf.astCommPool[2].enRemapMode	= VB_REMAP_MODE_CACHED;
				DPU_UT_PRT("common pool[2] BlkSize %d\n", u32BlkSizeOut_btcost);
				stVbConf.u32MaxPoolCnt              = 3;
			}else{
				stVbConf.u32MaxPoolCnt              = 2;
			}
		}

	}else if(grp_attr.enDpuMode == DPU_MODE_SGBM_MUX1 ){

		stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn_left;
		stVbConf.astCommPool[0].u32BlkCnt	= 2;
		stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
		DPU_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn_left);

		u32BlkSizeOut16 = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width, chn_attr.stImgSize.u32Height,
		PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_16);
		stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut16*2;
		stVbConf.astCommPool[1].u32BlkCnt	= 1;
		stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
		DPU_UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut16*2);
		if(grp_attr.bIsBtcostOut){
			u32BlkSizeOut_btcost = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width*128, chn_attr.stImgSize.u32Height,
			PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_16, COMPRESS_MODE_NONE, ALIGN_16);
			stVbConf.astCommPool[2].u32BlkSize	= u32BlkSizeOut_btcost;
			stVbConf.astCommPool[2].u32BlkCnt	= 1;
			stVbConf.astCommPool[2].enRemapMode	= VB_REMAP_MODE_CACHED;
			DPU_UT_PRT("common pool[2] BlkSize %d\n", u32BlkSizeOut_btcost);
			stVbConf.u32MaxPoolCnt              = 3;
		}else{
			stVbConf.u32MaxPoolCnt              = 2;
		}

	}else if(grp_attr.enDpuMode == DPU_MODE_FGS_MUX1 ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_FGS_ONLINE_MUX1 ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_FGS_ONLINE_MUX2 ){
		stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn_left;
		stVbConf.astCommPool[0].u32BlkCnt	= 2;
		stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
		DPU_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn_left);

		// u32BlkSizeChfh = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width, chn_attr.stImgSize.u32Height,
		// PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_16);
		// stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeChfh*4;
		// stVbConf.astCommPool[1].u32BlkCnt	= 1;
		// stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
		// DPU_UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeChfh*4);

		u32BlkSizeOut16 = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width, chn_attr.stImgSize.u32Height,
		PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_32);
		stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut16*2;
		stVbConf.astCommPool[1].u32BlkCnt	= 1;
		stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
		DPU_UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut16*2);
		if(grp_attr.bIsBtcostOut){
			u32BlkSizeOut_btcost = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width*128, chn_attr.stImgSize.u32Height,
			PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_16, COMPRESS_MODE_NONE, ALIGN_16);
			stVbConf.astCommPool[2].u32BlkSize	= u32BlkSizeOut_btcost;
			stVbConf.astCommPool[2].u32BlkCnt	= 1;
			stVbConf.astCommPool[2].enRemapMode	= VB_REMAP_MODE_CACHED;
			DPU_UT_PRT("common pool[2] BlkSize %d\n", u32BlkSizeOut_btcost);
			stVbConf.u32MaxPoolCnt              = 3;
		}else{
			stVbConf.u32MaxPoolCnt              = 2;
		}
	}

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		DPU_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		DPU_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		DPU_UT_PRT("CVI_SYS_Init failed!\n");
		CVI_VB_Exit();
		return s32Ret;
	}


	/************************************************
	 * step2:  Init DPU
	 ************************************************/

	s32Ret = CVI_DPU_CreateGrp(DpuGrp, &grp_attr_zero);
	if (s32Ret != CVI_SUCCESS) {
		DPU_UT_PRT("CVI_DPU_CreateGrp(grp:%d) failed with %#x!\n", DpuGrp, s32Ret);
		CVI_DPU_DestroyGrp(DpuGrp);
		CVI_SYS_Exit();
		CVI_VB_Exit();
		return s32Ret;
	}

	s32Ret = CVI_DPU_SetGrpAttr(DpuGrp, &grp_attr);
	if (s32Ret != CVI_SUCCESS) {
		DPU_UT_PRT("CVI_DPU_SetGrpAttr(grp:%d) failed with %#x!\n", DpuGrp, s32Ret);
		CVI_DPU_DestroyGrp(DpuGrp);
		CVI_SYS_Exit();
		CVI_VB_Exit();
		return s32Ret;
	}

	s32Ret = CVI_DPU_GetGrpAttr(DpuGrp, &grp_attr_res);
	if (s32Ret != CVI_SUCCESS) {
		DPU_UT_PRT("CVI_DPU_GetGrpAttr(grp:%d) failed with %#x!\n", DpuGrp, s32Ret);
		CVI_DPU_DestroyGrp(DpuGrp);
		CVI_SYS_Exit();
		CVI_VB_Exit();
		return s32Ret;
	}

	s32Ret = CompareGrpAttr(&grp_attr_res,&grp_attr);
	if (s32Ret != CVI_SUCCESS) {
		DPU_UT_PRT("CompareGrpAttr(grp:%d) failed with %#x!\n", DpuGrp, s32Ret);
		CVI_DPU_DestroyGrp(DpuGrp);
		CVI_SYS_Exit();
		CVI_VB_Exit();
		return s32Ret;
	}

	s32Ret = CVI_DPU_SetChnAttr(DpuGrp, 0, &chn_attr);
	if (s32Ret != CVI_SUCCESS) {
		DPU_UT_PRT("CVI_DPU_SetChnAttr failed with %#x\n", s32Ret);
		CVI_DPU_DestroyGrp(DpuGrp);
		CVI_SYS_Exit();
		CVI_VB_Exit();
		return s32Ret;
	}

	s32Ret = CVI_DPU_EnableChn(DpuGrp, 0);
	if (s32Ret != CVI_SUCCESS) {
		DPU_UT_PRT("CVI_DPU_EnableChn failed with %#x\n", s32Ret);
		CVI_DPU_DestroyGrp(DpuGrp);
		CVI_SYS_Exit();
		CVI_VB_Exit();
		return s32Ret;
	}

	if(grp_attr.bIsBtcostOut){
		s32Ret = CVI_DPU_SetChnAttr(DpuGrp, 1, &chn_attr);
		if (s32Ret != CVI_SUCCESS) {
			DPU_UT_PRT("CVI_DPU_SetChnAttr failed with %#x\n", s32Ret);
			CVI_DPU_DisableChn(DpuGrp, 0);
			CVI_DPU_DestroyGrp(DpuGrp);
			CVI_SYS_Exit();
			CVI_VB_Exit();
			return s32Ret;
		}

	s32Ret = CVI_DPU_EnableChn(DpuGrp,1);
		if (s32Ret != CVI_SUCCESS) {
			DPU_UT_PRT("CVI_DPU_EnableChn failed with %#x\n", s32Ret);
			CVI_DPU_DisableChn(DpuGrp, 0);
			CVI_DPU_DestroyGrp(DpuGrp);
			CVI_SYS_Exit();
			CVI_VB_Exit();
			return s32Ret;
		}
	}

	/*start dpu*/
	s32Ret = CVI_DPU_StartGrp(DpuGrp);
	if (s32Ret != CVI_SUCCESS) {
		DPU_UT_PRT("CVI_DPU_StartGrp failed with %#x\n", s32Ret);
		CVI_DPU_DisableChn(DpuGrp, 0);
		if(grp_attr.bIsBtcostOut)
			CVI_DPU_DisableChn(DpuGrp, 1);
		CVI_DPU_DestroyGrp(DpuGrp);
		CVI_SYS_Exit();
		CVI_VB_Exit();
		return s32Ret;
	}

	//send frame5
	s32Ret = FileToFrame(&grp_attr.stLeftImageSize,PIXEL_FORMAT_YUV_400,fileNameInLeft,&stVideoFrameInLeft);
	if (s32Ret != CVI_SUCCESS) {
		DPU_UT_PRT("[left]FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		CVI_DPU_DisableChn(DpuGrp, 0);
		if(grp_attr.bIsBtcostOut)
			CVI_DPU_DisableChn(DpuGrp, 1);
		CVI_DPU_DestroyGrp(DpuGrp);
		CVI_SYS_Exit();
		CVI_VB_Exit();
		return s32Ret;
	}
	s32Ret = FileToFrame(&grp_attr.stRightImageSize,PIXEL_FORMAT_YUV_400,fileNameInRight,&stVideoFrameInRight);
	if (s32Ret != CVI_SUCCESS) {
		DPU_UT_PRT("[lRight]FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		vb_left = CVI_VB_PhysAddr2Handle(stVideoFrameInLeft.stVFrame.u64PhyAddr[0]);
		if(CVI_VB_ReleaseBlock(vb_left)!=CVI_SUCCESS)
			DPU_UT_PRT("ReleaseBlock blk_left fail !\n");
		CVI_DPU_DisableChn(DpuGrp, 0);
		if(grp_attr.bIsBtcostOut)
			CVI_DPU_DisableChn(DpuGrp, 1);
		CVI_DPU_DestroyGrp(DpuGrp);
		CVI_SYS_Exit();
		CVI_VB_Exit();
		return s32Ret;
	}
	while (1)
	{
		s32Ret = CVI_DPU_SendFrame(DpuGrp,&stVideoFrameInLeft,&stVideoFrameInRight,TIMEOUT_GET_FRAME);
		if(s32Ret != CVI_SUCCESS){
			DPU_UT_PRT("[lRight]SendFrame fail, s32Ret: 0x%x !\n", s32Ret);
			vb_left = CVI_VB_PhysAddr2Handle(stVideoFrameInLeft.stVFrame.u64PhyAddr[0]);
			if(CVI_VB_ReleaseBlock(vb_left)!=CVI_SUCCESS)
				DPU_UT_PRT("ReleaseBlock blk_left fail !\n");
			vb_right = CVI_VB_PhysAddr2Handle(stVideoFrameInRight.stVFrame.u64PhyAddr[0]);
			if(CVI_VB_ReleaseBlock(vb_right)!=CVI_SUCCESS)
				DPU_UT_PRT("ReleaseBlock blk_right fail !\n");
			CVI_DPU_DisableChn(DpuGrp, 0);
			if(grp_attr.bIsBtcostOut)
				CVI_DPU_DisableChn(DpuGrp, 1);
			CVI_DPU_DestroyGrp(DpuGrp);
			CVI_SYS_Exit();
			CVI_VB_Exit();
			return s32Ret;
		}
		// BLK_left =CVI_VB_PhysAddr2Handle(stVideoFrameInLeft.stVFrame.u64PhyAddr[0]);
		// BLK_right =CVI_VB_PhysAddr2Handle(stVideoFrameInRight.stVFrame.u64PhyAddr[0]);
		// CVI_VB_ReleaseBlock(BLK_left);
		// CVI_VB_ReleaseBlock(BLK_right);

		//get frame
		if(grp_attr.bIsBtcostOut){
			DpuChn =1;
			s32Ret = CVI_DPU_GetFrame(DpuGrp,DpuChn,&stVideoFrameOutBtcost,TIMEOUT_GET_FRAME);
			if (s32Ret != CVI_SUCCESS) {
				DPU_UT_PRT("CVI_DPU_GetFrame btcost fail. s32Ret: 0x%x !\n", s32Ret);
				CVI_DPU_DisableChn(DpuGrp, 0);
				CVI_DPU_DisableChn(DpuGrp, 1);
				CVI_DPU_DestroyGrp(DpuGrp);
				CVI_SYS_Exit();
				CVI_VB_Exit();
				return s32Ret;
			}
			CVI_DPU_ReleaseFrame(DpuGrp, 1, &stVideoFrameOutBtcost);
		}else {
			DpuChn =0;
			s32Ret = CVI_DPU_GetFrame(DpuGrp,DpuChn,&stVideoFrameOut,TIMEOUT_GET_FRAME);
			if (s32Ret != CVI_SUCCESS) {
				DPU_UT_PRT("CVI_DPU_GetFrame out fail. s32Ret: 0x%x !\n", s32Ret);
				CVI_DPU_DisableChn(DpuGrp, 0);
				CVI_DPU_DisableChn(DpuGrp, 1);
				CVI_DPU_DestroyGrp(DpuGrp);
				CVI_SYS_Exit();
				CVI_VB_Exit();
				return s32Ret;
			}
			CVI_DPU_ReleaseFrame(DpuGrp,0,&stVideoFrameOut);
		}
		times++;
		DPU_UT_PRT("***CVI_DPU_GetChnFrame Success(%d)***\n",times);
	}

	if (grp_attr.bIsBtcostOut) {
		DpuChn=0;

		s32Ret = FrameSaveToFile(fileNameOut, &stVideoFrameOut);
		if (s32Ret != CVI_SUCCESS) {
			DPU_UT_PRT("FrameSaveToFile. out s32Ret: 0x%x !\n", s32Ret);
		}

		DpuChn=1;
		s32Ret = FrameSaveToFile(fileNameOutBtcost, &stVideoFrameOutBtcost);
		if (s32Ret != CVI_SUCCESS) {
			DPU_UT_PRT("FrameSaveToFile.btcost s32Ret: 0x%x !\n", s32Ret);
		}
		CVI_DPU_ReleaseFrame(DpuGrp,0,&stVideoFrameOut);
		CVI_DPU_ReleaseFrame(DpuGrp, 1, &stVideoFrameOutBtcost);
		CVI_DPU_DisableChn(DpuGrp, 0);
		CVI_DPU_DisableChn(DpuGrp, 1);
		CVI_DPU_StopGrp(DpuGrp);
		CVI_DPU_DestroyGrp(DpuGrp);
		CVI_SYS_Exit();
		CVI_VB_Exit();
		return s32Ret;

	}else{
		DpuChn=0;
		s32Ret = FrameSaveToFile(fileNameOut, &stVideoFrameOut);
		if (s32Ret != CVI_SUCCESS) {
			DPU_UT_PRT("FrameSaveToFile. out s32Ret: 0x%x !\n", s32Ret);
		}
		CVI_DPU_ReleaseFrame(DpuGrp,0,&stVideoFrameOut);
		CVI_DPU_DisableChn(DpuGrp, 0);
		CVI_DPU_StopGrp(DpuGrp);
		CVI_DPU_DestroyGrp(DpuGrp);
		CVI_SYS_Exit();
		CVI_VB_Exit();
		return s32Ret;
	}

}

void dpu_Usage()
{
	DPU_UT_PRT("\t 0)DPU_CHECK_REG_READ.\n");
	DPU_UT_PRT("\t 1)DPU_CHECK_REG_WRITE.\n");
	DPU_UT_PRT("\t 2)DPU_CHECK_SGBM_STATUS.\n");
	DPU_UT_PRT("\t 3)DPU_CHECK_FGS_STATUS.\n");
	DPU_UT_PRT("\t 4)DPU_MODE_SGBM_MUX0.\n");
	DPU_UT_PRT("\t 5)DPU_MODE_SGBM_MUX1.\n");
	DPU_UT_PRT("\t 6)DPU_MODE_SGBM_MUX2.\n");
	DPU_UT_PRT("\t 7)DPU_MODE_SGBM_FGS_ONLINE_MUX0.\n");
	DPU_UT_PRT("\t 8)DPU_MODE_SGBM_FGS_ONLINE_MUX1.\n");
	DPU_UT_PRT("\t 9)DPU_MODE_SGBM_FGS_ONLINE_MUX2.\n");
	DPU_UT_PRT("\t 10)DPU_MODE_FGS_MUX0.\n");
	DPU_UT_PRT("\t 11)DPU_MODE_FGS_MUX1.\n");
	DPU_UT_PRT("\t 100)DPU_AUTO_TEST.\n");
	DPU_UT_PRT("\t 255)exit.\n");
}
static CVI_S32 dpu_mode_test(CVI_CHAR stFileName[128],CVI_CHAR reFileName[128],DPU_MODE_E dpu_mode){
	CVI_S32 s32Ret;
	CVI_S32 size;
	strncpy(fileNameInLeft, DPU_SOFA_L, sizeof(DPU_SOFA_L));
	strncpy(fileNameInRight, DPU_SOFA_R, sizeof(DPU_SOFA_R));
	strncpy(fileNameOut, stFileName, 128);
	if(reFileName !=NULL){
		strncpy(fileNameResult, reFileName, 128);
	}else{
		memset(fileNameResult,0,128);
	}
	grp_attr.stLeftImageSize.u32Height = 284;
	grp_attr.stLeftImageSize.u32Width = 512;
	grp_attr.stRightImageSize.u32Height = 284;
	grp_attr.stRightImageSize.u32Width = 512;
	chn_attr.stImgSize.u32Width = 512;
	chn_attr.stImgSize.u32Height = 284;
	set_attr_default();
	grp_attr.enDpuMode = dpu_mode;
	s32Ret = basic();
	if(grp_attr.enDpuMode == DPU_MODE_SGBM_MUX0 ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_MUX2 ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_FGS_ONLINE_MUX0 ||
		grp_attr.enDpuMode == DPU_MODE_FGS_MUX0)
			size = grp_attr.stLeftImageSize.u32Height * grp_attr.stLeftImageSize.u32Width;
	else if(grp_attr.enDpuMode == DPU_MODE_SGBM_MUX1 ||
			grp_attr.enDpuMode == DPU_MODE_SGBM_FGS_ONLINE_MUX1 ||
			grp_attr.enDpuMode == DPU_MODE_SGBM_FGS_ONLINE_MUX2 ||
			grp_attr.enDpuMode == DPU_MODE_FGS_MUX1)
			size = grp_attr.stLeftImageSize.u32Height * grp_attr.stLeftImageSize.u32Width*2;
	else{
		DPU_UT_PRT("No this dpu mode! \n");
		return CVI_FAILURE;
	}

	s32Ret = CompareWithFile(fileNameOut,fileNameResult,size);
	DPU_UT_PRT("dpu output mode(%d) test %s \n", dpu_mode,s32Ret == CVI_SUCCESS ? "pass" : "fail");
	return s32Ret;
}

static CVI_S32 dpu_fgs_mode_test(CVI_CHAR stFileName[128],CVI_CHAR reFileName[128], DPU_MODE_E dpu_mode){
	CVI_S32 s32Ret;
	CVI_S32 size;
	strncpy(fileNameInLeft, DPU_SOFA_L, sizeof(DPU_SOFA_L));
	strncpy(fileNameInRight, DPU_SGBM_R, sizeof(DPU_SGBM_R));
	strncpy(fileNameOut, stFileName, 128);
	if(reFileName !=NULL){
		strncpy(fileNameResult, reFileName, 128);
	}else{
		memset(fileNameResult,0,128);
	}
	grp_attr.stLeftImageSize.u32Height = 284;
	grp_attr.stLeftImageSize.u32Width = 512;
	grp_attr.stRightImageSize.u32Height = 284;
	grp_attr.stRightImageSize.u32Width = 512;
	chn_attr.stImgSize.u32Width = 512;
	chn_attr.stImgSize.u32Height = 284;
	set_attr_default();
	if(dpu_mode < 7 || dpu_mode > 8){
		DPU_UT_PRT("dpu not supported! \n");
		return CVI_FAILURE;
	}
	grp_attr.enDpuMode = dpu_mode;
	s32Ret = basic();
	if(grp_attr.enDpuMode == DPU_MODE_SGBM_MUX0 ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_MUX2 ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_FGS_ONLINE_MUX0 ||
		grp_attr.enDpuMode == DPU_MODE_FGS_MUX0)
			size = grp_attr.stLeftImageSize.u32Height * grp_attr.stLeftImageSize.u32Width;
	else if(grp_attr.enDpuMode == DPU_MODE_SGBM_MUX1 ||
			grp_attr.enDpuMode == DPU_MODE_SGBM_FGS_ONLINE_MUX1 ||
			grp_attr.enDpuMode == DPU_MODE_SGBM_FGS_ONLINE_MUX2 ||
			grp_attr.enDpuMode == DPU_MODE_FGS_MUX1)
			size = grp_attr.stLeftImageSize.u32Height * grp_attr.stLeftImageSize.u32Width*2;
	else{
		DPU_UT_PRT("No this dpu mode! \n");
		return CVI_FAILURE;
	}
	s32Ret = CompareWithFile(fileNameOut,fileNameResult,size);
	DPU_UT_PRT("dpu output mode(%d)  test %s \n", dpu_mode, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	return s32Ret;
}

static CVI_S32 dpu_size_test(CVI_CHAR fileNameInL[128],
								CVI_CHAR fileNameInR[128],
								CVI_CHAR stFileName[128],
								CVI_CHAR reFileName[128],
								CVI_S32 width,CVI_S32 height,
								DPU_MODE_E dpu_mode)
{
	CVI_S32 s32Ret;
	CVI_S32 size;
	strncpy(fileNameInLeft, fileNameInL, 128);
	strncpy(fileNameInRight, fileNameInR, 128);
	strncpy(fileNameOut, stFileName, 128);
	if(reFileName !=NULL){
		strncpy(fileNameResult, reFileName, 128);
	}else{
		memset(fileNameResult,0,128);
	}
	grp_attr.stLeftImageSize.u32Height = height;
	grp_attr.stLeftImageSize.u32Width = width;
	grp_attr.stRightImageSize.u32Height = height;
	grp_attr.stRightImageSize.u32Width = width;
	set_attr_default();
	chn_attr.stImgSize.u32Width = width;
	chn_attr.stImgSize.u32Height = height;
	grp_attr.enDpuMode = dpu_mode;
	s32Ret = basic();
	if(grp_attr.enDpuMode == DPU_MODE_SGBM_MUX0 ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_MUX2 ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_FGS_ONLINE_MUX0 ||
		grp_attr.enDpuMode == DPU_MODE_FGS_MUX0)
			size = grp_attr.stLeftImageSize.u32Height * DPU_ALIGN(grp_attr.stLeftImageSize.u32Width,16);
	else if(grp_attr.enDpuMode == DPU_MODE_SGBM_MUX1)
			size = grp_attr.stLeftImageSize.u32Height * DPU_ALIGN(grp_attr.stLeftImageSize.u32Width,16)*2;
	else if(grp_attr.enDpuMode == DPU_MODE_SGBM_FGS_ONLINE_MUX1 ||
			grp_attr.enDpuMode == DPU_MODE_SGBM_FGS_ONLINE_MUX2 ||
			grp_attr.enDpuMode == DPU_MODE_FGS_MUX1)
			size = grp_attr.stLeftImageSize.u32Height * DPU_ALIGN(grp_attr.stLeftImageSize.u32Width,32)*2;
	else{
		DPU_UT_PRT("No this dpu mode! \n");
		return CVI_FAILURE;
	}
	s32Ret = CompareWithFile(fileNameOut,fileNameResult,size);
	DPU_UT_PRT("width(%d) height(%d) dpuMode(%d) dpu size test %s \n", width,height,(CVI_S32)dpu_mode ,s32Ret == CVI_SUCCESS ? "pass" : "fail");
	return s32Ret;
}

static CVI_S32 dpu_press_test(CVI_CHAR fileNameInL[128],
								CVI_CHAR fileNameInR[128],
								CVI_S32 width,CVI_S32 height,
								DPU_MODE_E dpu_mode)
{
	CVI_S32 s32Ret;
	strncpy(fileNameInLeft, fileNameInL, 128);
	strncpy(fileNameInRight, fileNameInR, 128);
	grp_attr.stLeftImageSize.u32Height = height;
	grp_attr.stLeftImageSize.u32Width = width;
	grp_attr.stRightImageSize.u32Height = height;
	grp_attr.stRightImageSize.u32Width = width;
	set_attr_default();
	chn_attr.stImgSize.u32Width = width;
	chn_attr.stImgSize.u32Height = height;
	grp_attr.enDpuMode = dpu_mode;
	s32Ret = basic_loop();
	return s32Ret;
}

static CVI_S32 dpu_golden_test(CVI_CHAR fileNameInL[128],
								CVI_CHAR fileNameInR[128],
								CVI_CHAR stFileName[128],
								CVI_CHAR reFileName[128],
								CVI_S32 width,
								CVI_S32 height,
								DPU_GOLDEN_MODE_E enDpuGolden)
{
	CVI_S32 s32Ret;
	CVI_S32 size;
	strncpy(fileNameInLeft, fileNameInL, 128);
	strncpy(fileNameInRight, fileNameInR, 128);
	strncpy(fileNameOut, stFileName, 128);
	if(reFileName !=NULL){
		strncpy(fileNameResult, reFileName, 128);
	}else{
		memset(fileNameResult,0,128);
	}
	if(enDpuGolden == DPU_GOLDEN_JADEPENT)
		set_attr_golden_jadepent();
	else if(enDpuGolden == DPU_GOLDEN_PIANO)
		set_attr_golden_piano();
	else if(enDpuGolden == DPU_GOLDEN_TEDDY)
		set_attr_golden_teddy();
	else{
		DPU_UT_PRT("No this dpu golden mode! \n");
		s32Ret = CVI_FAILURE;
		return s32Ret;
	}
	grp_attr.stLeftImageSize.u32Height = height;
	grp_attr.stLeftImageSize.u32Width = width;
	grp_attr.stRightImageSize.u32Height = height;
	grp_attr.stRightImageSize.u32Width = width;
	chn_attr.stImgSize.u32Width = width;
	chn_attr.stImgSize.u32Height = height;
	s32Ret = basic();
	if(grp_attr.enDpuMode == DPU_MODE_SGBM_MUX0 ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_MUX2 ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_FGS_ONLINE_MUX0 ||
		grp_attr.enDpuMode == DPU_MODE_FGS_MUX0)
			size = grp_attr.stLeftImageSize.u32Height * DPU_ALIGN(grp_attr.stLeftImageSize.u32Width,16);
	else if(grp_attr.enDpuMode == DPU_MODE_SGBM_MUX1)
			size = grp_attr.stLeftImageSize.u32Height * DPU_ALIGN(grp_attr.stLeftImageSize.u32Width,16)*2;
	else if(grp_attr.enDpuMode == DPU_MODE_SGBM_FGS_ONLINE_MUX1 ||
			grp_attr.enDpuMode == DPU_MODE_SGBM_FGS_ONLINE_MUX2 ||
			grp_attr.enDpuMode == DPU_MODE_FGS_MUX1)
			size = grp_attr.stLeftImageSize.u32Height * DPU_ALIGN(grp_attr.stLeftImageSize.u32Width,32)*2;
	else{
		DPU_UT_PRT("No this dpu mode! \n");
		return CVI_FAILURE;
	}
	s32Ret = CompareWithFile(fileNameOut,fileNameResult,size);
	DPU_UT_PRT("width(%d) height(%d) goldenMode(%d) dpu size test %s \n", width,height,(CVI_S32)enDpuGolden ,s32Ret == CVI_SUCCESS ? "pass" : "fail");
	return s32Ret;
}

static CVI_S32 dpu_disp_range_test(CVI_CHAR fileNameInL[128],
								   CVI_CHAR fileNameInR[128],
								   CVI_CHAR stFileName[128],
								   CVI_CHAR reFileName[128],
								   CVI_U32 width,
								   CVI_U32 height,
								   CVI_U16 dispStartPos,
								   DPU_DISP_RANGE_E dispRange){
	CVI_S32 s32Ret;
	CVI_S32 size;
	strncpy(fileNameInLeft, fileNameInL, 128);
	strncpy(fileNameInRight, fileNameInR, 128);
	strncpy(fileNameOut, stFileName, 128);
	if(reFileName !=NULL){
		strncpy(fileNameResult, reFileName, 128);
	}else{
		memset(fileNameResult,0,128);
	}
	grp_attr.stLeftImageSize.u32Height = height;
	grp_attr.stLeftImageSize.u32Width = width;
	grp_attr.stRightImageSize.u32Height = height;
	grp_attr.stRightImageSize.u32Width = width;
	chn_attr.stImgSize.u32Width = width;
	chn_attr.stImgSize.u32Height = height;
	set_attr_default();
	grp_attr.enDpuMode = DPU_MODE_SGBM_MUX1;
	grp_attr.u16DispStartPos = dispStartPos;
	grp_attr.enDispRange = dispRange;
	s32Ret = basic();
	size = grp_attr.stLeftImageSize.u32Height * grp_attr.stLeftImageSize.u32Width*2;
	s32Ret = CompareWithFile(fileNameOut,fileNameResult,size);
	DPU_UT_PRT("diapStartPos(%d) dispRange(%d) dpu disp range test %s \n", dispStartPos,dispRange,s32Ret == CVI_SUCCESS ? "pass" : "fail");
	return s32Ret;
}

static CVI_S32 dpu_census_test(CVI_CHAR stFileName[128], CVI_CHAR reFileName[128], CVI_U32 censusShift){
	CVI_S32 s32Ret;
	CVI_S32 size;
	strncpy(fileNameInLeft, DPU_SOFA_L, sizeof(DPU_SOFA_L));
	strncpy(fileNameInRight, DPU_SOFA_R, sizeof(DPU_SOFA_R));
	strncpy(fileNameOut, stFileName, 128);
	if(reFileName !=NULL){
		strncpy(fileNameResult, reFileName, 128);
	}else{
		memset(fileNameResult,0,128);
	}
	grp_attr.stLeftImageSize.u32Height = 284;
	grp_attr.stLeftImageSize.u32Width = 512;
	grp_attr.stRightImageSize.u32Height = 284;
	grp_attr.stRightImageSize.u32Width = 512;
	chn_attr.stImgSize.u32Width = 512;
	chn_attr.stImgSize.u32Height = 284;
	set_attr_default();
	grp_attr.enDpuMode = DPU_MODE_SGBM_MUX1;
	grp_attr.u32CensusShift = censusShift;
	s32Ret = basic();
	size = grp_attr.stLeftImageSize.u32Height * grp_attr.stLeftImageSize.u32Width*2;
	s32Ret = CompareWithFile(fileNameOut,fileNameResult,size);
	DPU_UT_PRT("censusShift(%d)dpu census test %s \n", censusShift, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	return s32Ret;
}

static CVI_S32 dpu_add_test(CVI_CHAR stFileName[128], CVI_CHAR reFileName[128], CVI_U32 rshift1, CVI_U32 rshift2){
	CVI_S32 s32Ret;
	CVI_S32 size;
	strncpy(fileNameInLeft, DPU_SOFA_L, sizeof(DPU_SOFA_L));
	strncpy(fileNameInRight, DPU_SOFA_R, sizeof(DPU_SOFA_R));
	strncpy(fileNameOut, stFileName, 128);
	if(reFileName !=NULL){
		strncpy(fileNameResult, reFileName, 128);
	}else{
		memset(fileNameResult,0,128);
	}
	grp_attr.stLeftImageSize.u32Height = 284;
	grp_attr.stLeftImageSize.u32Width = 512;
	grp_attr.stRightImageSize.u32Height = 284;
	grp_attr.stRightImageSize.u32Width = 512;
	chn_attr.stImgSize.u32Width = 512;
	chn_attr.stImgSize.u32Height = 284;
	set_attr_default();
	grp_attr.enDpuMode = DPU_MODE_SGBM_MUX1;
	grp_attr.u32Rshift1 = rshift1;
	grp_attr.u32Rshift2 = rshift2;
	s32Ret = basic();
	size = grp_attr.stLeftImageSize.u32Height * grp_attr.stLeftImageSize.u32Width*2;
	s32Ret = CompareWithFile(fileNameOut,fileNameResult,size);
	DPU_UT_PRT("rshift1(%d) rshift2(%d) dpu add test %s \n", rshift1, rshift2, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	return s32Ret;
}

static CVI_S32 dpu_bfw_test(CVI_CHAR stFileName[128], CVI_CHAR reFileName[128], DPU_MASK_MODE_E maskMode){
	CVI_S32 s32Ret;
	CVI_S32 size;
	strncpy(fileNameInLeft, DPU_SOFA_L, sizeof(DPU_SOFA_L));
	strncpy(fileNameInRight, DPU_SOFA_R, sizeof(DPU_SOFA_R));
	strncpy(fileNameOut, stFileName, 128);
	if(reFileName !=NULL){
		strncpy(fileNameResult, reFileName, 128);
	}else{
		memset(fileNameResult,0,128);
	}
	grp_attr.stLeftImageSize.u32Height = 284;
	grp_attr.stLeftImageSize.u32Width = 512;
	grp_attr.stRightImageSize.u32Height = 284;
	grp_attr.stRightImageSize.u32Width = 512;
	chn_attr.stImgSize.u32Width = 512;
	chn_attr.stImgSize.u32Height = 284;
	set_attr_default();
	grp_attr.enDpuMode = DPU_MODE_SGBM_MUX1;
	grp_attr.enMaskMode = maskMode;
	s32Ret = basic();
	size = grp_attr.stLeftImageSize.u32Height * grp_attr.stLeftImageSize.u32Width*2;
	s32Ret = CompareWithFile(fileNameOut,fileNameResult,size);
	DPU_UT_PRT("maskMode(%d) dpu bfw test %s \n", maskMode, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	return s32Ret;
}

static CVI_S32 dpu_dcc_test(CVI_CHAR stFileName[128], CVI_CHAR reFileName[128], CVI_U32 caP1, CVI_U32 caP2, DPU_DCC_DIR_E dccDir){
	CVI_S32 s32Ret;
	CVI_S32 size;
	strncpy(fileNameInLeft, DPU_SOFA_L, sizeof(DPU_SOFA_L));
	strncpy(fileNameInRight, DPU_SOFA_R, sizeof(DPU_SOFA_R));
	strncpy(fileNameOut, stFileName, 128);
	if(reFileName !=NULL){
		strncpy(fileNameResult, reFileName, 128);
	}else{
		memset(fileNameResult,0,128);
	}
	grp_attr.stLeftImageSize.u32Height = 284;
	grp_attr.stLeftImageSize.u32Width = 512;
	grp_attr.stRightImageSize.u32Height = 284;
	grp_attr.stRightImageSize.u32Width = 512;
	chn_attr.stImgSize.u32Width = 512;
	chn_attr.stImgSize.u32Height = 284;
	set_attr_default();
	grp_attr.enDpuMode = DPU_MODE_SGBM_MUX1;
	grp_attr.u32CaP1 = caP1;
	grp_attr.u32CaP2 = caP2;
	grp_attr.enDccDir = dccDir;
	s32Ret = basic();
	size = grp_attr.stLeftImageSize.u32Height * grp_attr.stLeftImageSize.u32Width*2;
	s32Ret = CompareWithFile(fileNameOut,fileNameResult,size);
	DPU_UT_PRT("cap1(%d) cap2(%d) dccDir(%d) dpu dcc test %s \n", caP1, caP2, dccDir, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	return s32Ret;
}

static CVI_S32 dpu_uniq_check_test(CVI_CHAR stFileName[128], CVI_CHAR reFileName[128],CVI_U32 uniqRatio){
	CVI_S32 s32Ret;
	CVI_S32 size;
	strncpy(fileNameInLeft, DPU_SOFA_L, sizeof(DPU_SOFA_L));
	strncpy(fileNameInRight, DPU_SOFA_R, sizeof(DPU_SOFA_R));
	strncpy(fileNameOut, stFileName, 128);
	if(reFileName !=NULL){
		strncpy(fileNameResult, reFileName, 128);
	}else{
		memset(fileNameResult,0,128);
	}
	grp_attr.stLeftImageSize.u32Height = 284;
	grp_attr.stLeftImageSize.u32Width = 512;
	grp_attr.stRightImageSize.u32Height = 284;
	grp_attr.stRightImageSize.u32Width = 512;
	chn_attr.stImgSize.u32Width = 512;
	chn_attr.stImgSize.u32Height = 284;
	set_attr_default();
	grp_attr.enDpuMode = DPU_MODE_SGBM_MUX1;
	grp_attr.u32UniqRatio = uniqRatio;
	s32Ret = basic();
	size = grp_attr.stLeftImageSize.u32Height * grp_attr.stLeftImageSize.u32Width*2;
	s32Ret = CompareWithFile(fileNameOut,fileNameResult,size);
	DPU_UT_PRT("uniqRatio(%d) dpu uniq check test %s \n",uniqRatio, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	return s32Ret;
}

static CVI_S32 dpu_dispShift_test(CVI_CHAR stFileName[128], CVI_CHAR reFileName[128], CVI_U32 dispShift){
	CVI_S32 s32Ret;
	CVI_S32 size;
	strncpy(fileNameInLeft, DPU_SOFA_L, sizeof(DPU_SOFA_L));
	strncpy(fileNameInRight, DPU_SOFA_R, sizeof(DPU_SOFA_R));
	strncpy(fileNameOut, stFileName, 128);
	if(reFileName !=NULL){
		strncpy(fileNameResult, reFileName, 128);
	}else{
		memset(fileNameResult,0,128);
	}
	grp_attr.stLeftImageSize.u32Height = 284;
	grp_attr.stLeftImageSize.u32Width = 512;
	grp_attr.stRightImageSize.u32Height = 284;
	grp_attr.stRightImageSize.u32Width = 512;
	chn_attr.stImgSize.u32Width = 512;
	chn_attr.stImgSize.u32Height = 284;
	set_attr_default();
	grp_attr.enDpuMode = DPU_MODE_SGBM_MUX1;
	grp_attr.u32DispShift = dispShift;
	s32Ret = basic();
	size = grp_attr.stLeftImageSize.u32Height * grp_attr.stLeftImageSize.u32Width*2;
	s32Ret = CompareWithFile(fileNameOut,fileNameResult,size);
	DPU_UT_PRT("dispShift(%d) dpu dispShift test %s \n", dispShift, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	return s32Ret;
}

static CVI_S32 dpu_u16tou8_test(CVI_CHAR stFileName[128], CVI_CHAR reFileName[128], CVI_U32 dispShift, DPU_DISP_RANGE_E enDispRange){
	CVI_S32 s32Ret;
	CVI_S32 size;
	strncpy(fileNameInLeft, DPU_SOFA_L, sizeof(DPU_SOFA_L));
	strncpy(fileNameInRight, DPU_SOFA_R, sizeof(DPU_SOFA_R));
	strncpy(fileNameOut, stFileName, 128);
	if(reFileName !=NULL){
		strncpy(fileNameResult, reFileName, 128);
	}else{
		memset(fileNameResult,0,128);
	}
	grp_attr.stLeftImageSize.u32Height = 284;
	grp_attr.stLeftImageSize.u32Width = 512;
	grp_attr.stRightImageSize.u32Height = 284;
	grp_attr.stRightImageSize.u32Width = 512;
	chn_attr.stImgSize.u32Width = 512;
	chn_attr.stImgSize.u32Height = 284;
	set_attr_default();
	grp_attr.enDpuMode = DPU_MODE_SGBM_MUX2;
	grp_attr.u32DispShift = dispShift;
	grp_attr.enDispRange = enDispRange;
	s32Ret = basic();
	size = grp_attr.stLeftImageSize.u32Height * grp_attr.stLeftImageSize.u32Width;
	s32Ret = CompareWithFile(fileNameOut,fileNameResult,size);
	DPU_UT_PRT("dispShift(%d) enDispRange(%d) dpu u16tou8 test %s \n", dispShift, enDispRange, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	return s32Ret;
}

static CVI_S32 dpu_fgsMaxCount_test(CVI_CHAR stFileName[128], CVI_CHAR reFileName[128], CVI_U32 maxCount){
	CVI_S32 s32Ret;
	CVI_S32 size;
	strncpy(fileNameInLeft, DPU_SOFA_L, sizeof(DPU_SOFA_L));
	strncpy(fileNameInRight, DPU_SGBM_R, sizeof(DPU_SGBM_R));
	strncpy(fileNameOut, stFileName, 128);
	if(reFileName !=NULL){
		strncpy(fileNameResult, reFileName, 128);
	}else{
		memset(fileNameResult,0,128);
	}
	grp_attr.stLeftImageSize.u32Height = 284;
	grp_attr.stLeftImageSize.u32Width = 512;
	grp_attr.stRightImageSize.u32Height = 284;
	grp_attr.stRightImageSize.u32Width = 512;
	chn_attr.stImgSize.u32Width = 512;
	chn_attr.stImgSize.u32Height = 284;
	set_attr_default();
	grp_attr.enDpuMode = DPU_MODE_FGS_MUX0;
	grp_attr.u32FgsMaxCount = maxCount;
	s32Ret = basic();
	size = grp_attr.stLeftImageSize.u32Height * grp_attr.stLeftImageSize.u32Width;
	s32Ret = CompareWithFile(fileNameOut,fileNameResult,size);
	DPU_UT_PRT("maxCount(%d) dpu fgs maxCount test %s \n", maxCount, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	return s32Ret;
}

static CVI_S32 dpu_fgsMaxT_test(CVI_CHAR stFileName[128], CVI_CHAR reFileName[128], CVI_U32 maxT){
	CVI_S32 s32Ret;
	CVI_S32 size;
	strncpy(fileNameInLeft, DPU_SOFA_L, sizeof(DPU_SOFA_L));
	strncpy(fileNameInRight, DPU_SGBM_R, sizeof(DPU_SGBM_R));
	strncpy(fileNameOut, stFileName, 128);
	if(reFileName !=NULL){
		strncpy(fileNameResult, reFileName, 128);
	}else{
		memset(fileNameResult,0,128);
	}
	grp_attr.stLeftImageSize.u32Height = 284;
	grp_attr.stLeftImageSize.u32Width = 512;
	grp_attr.stRightImageSize.u32Height = 284;
	grp_attr.stRightImageSize.u32Width = 512;
	chn_attr.stImgSize.u32Width = 512;
	chn_attr.stImgSize.u32Height = 284;
	set_attr_default();
	grp_attr.enDpuMode = DPU_MODE_FGS_MUX0;
	grp_attr.u32FgsMaxT = maxT;
	s32Ret = basic();
	size = grp_attr.stLeftImageSize.u32Height * grp_attr.stLeftImageSize.u32Width;
	s32Ret = CompareWithFile(fileNameOut,fileNameResult,size);
	DPU_UT_PRT("maxT(%d) dpu fgs maxT test %s \n", maxT, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	return s32Ret;
}

static CVI_S32 dpu_depth_test(CVI_CHAR stFileName[128], CVI_CHAR reFileName[128], DPU_DISP_RANGE_E enDispRange, CVI_U32 fxBaseline){
	CVI_S32 s32Ret;
	CVI_S32 size;
	strncpy(fileNameInLeft, DPU_SOFA_L, sizeof(DPU_SOFA_L));
	strncpy(fileNameInRight, DPU_SGBM_R, sizeof(DPU_SGBM_R));
	strncpy(fileNameOut, stFileName, 128);
	if(reFileName !=NULL){
		strncpy(fileNameResult, reFileName, 128);
	}else{
		memset(fileNameResult,0,128);
	}
	grp_attr.stLeftImageSize.u32Height = 284;
	grp_attr.stLeftImageSize.u32Width = 512;
	grp_attr.stRightImageSize.u32Height = 284;
	grp_attr.stRightImageSize.u32Width = 512;
	chn_attr.stImgSize.u32Width = 512;
	chn_attr.stImgSize.u32Height = 284;
	set_attr_default();
	grp_attr.enDpuMode = DPU_MODE_FGS_MUX1;
	grp_attr.enDispRange = enDispRange;
	grp_attr.u32FxBaseline = fxBaseline;
	s32Ret = basic();
	size = grp_attr.stLeftImageSize.u32Height * grp_attr.stLeftImageSize.u32Width*2;
	s32Ret = CompareWithFile(fileNameOut,fileNameResult,size);
	DPU_UT_PRT("enDispRange(%d) fxBaseline(%d) dpu depth test %s \n", enDispRange, fxBaseline, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	return s32Ret;
}

static CVI_S32 dpu_unitChoose_test(CVI_CHAR stFileName[128], CVI_CHAR reFileName[128], CVI_U32 depthUnit){
	CVI_S32 s32Ret;
	CVI_S32 size;
	strncpy(fileNameInLeft, DPU_SOFA_L, sizeof(DPU_SOFA_L));
	strncpy(fileNameInRight, DPU_SGBM_R, sizeof(DPU_SGBM_R));
	strncpy(fileNameOut, stFileName, 128);
	if(reFileName !=NULL){
		strncpy(fileNameResult, reFileName, 128);
	}else{
		memset(fileNameResult,0,128);
	}
	grp_attr.stLeftImageSize.u32Height = 284;
	grp_attr.stLeftImageSize.u32Width = 512;
	grp_attr.stRightImageSize.u32Height = 284;
	grp_attr.stRightImageSize.u32Width = 512;
	chn_attr.stImgSize.u32Width = 512;
	chn_attr.stImgSize.u32Height = 284;
	set_attr_default();
	grp_attr.enDpuMode = DPU_MODE_FGS_MUX1;
	grp_attr.enDpuDepthUnit = depthUnit;
	s32Ret = basic();
	size = grp_attr.stLeftImageSize.u32Height * grp_attr.stLeftImageSize.u32Width*2;
	s32Ret = CompareWithFile(fileNameOut,fileNameResult,size);
	DPU_UT_PRT("depthUnit(%d) dpu unitChoose test %s \n",depthUnit, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	return s32Ret;
}

static CVI_S32 dpu_btcost_test(CVI_CHAR stFileName[128],CVI_CHAR reFileName[128])
{
	CVI_S32 s32Ret;
	CVI_S32 size;
	strncpy(fileNameInLeft, DPU_BTCOST_L, sizeof(DPU_BTCOST_L));
	strncpy(fileNameInRight, DPU_BTCOST_R, sizeof(DPU_BTCOST_R));
	strncpy(fileNameOutBtcost, stFileName, 128);
	if(reFileName !=NULL){
		strncpy(fileNameResult, reFileName, 128);
	}else{
		memset(fileNameResult,0,128);
	}
	grp_attr.stLeftImageSize.u32Height = 64;
	grp_attr.stLeftImageSize.u32Width = 64;
	grp_attr.stRightImageSize.u32Height = 64;
	grp_attr.stRightImageSize.u32Width = 64;
	set_attr_default();
	grp_attr.bIsBtcostOut = 1;
	chn_attr.stImgSize.u32Width = 64;
	chn_attr.stImgSize.u32Height = 64;
	s32Ret = basic();
	size = grp_attr.stLeftImageSize.u32Height * grp_attr.stLeftImageSize.u32Width*128*2;
	s32Ret = CompareWithFile(fileNameOutBtcost,fileNameResult,size);
	DPU_UT_PRT("dpu btcost test %s \n", s32Ret == CVI_SUCCESS ? "pass" : "fail");
	return s32Ret;
}

static CVI_S32 dpu_auto_regression(CVI_VOID){
	CVI_S32 failCnt =0;
	CVI_S32 sumCnt = 0;
	CVI_S32 info[100][2];
	//online mode test case00-02
	DPU_UT_PRT("case 0 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_size_test(DPU_SOFA_L, DPU_SOFA_R,"res/online_fgsDisp_res1.bin", "ref/online_fgsDisp_ref1.bin",512, 284,DPU_MODE_SGBM_FGS_ONLINE_MUX0);
	sumCnt +=1 ;
	info[sumCnt][0] =0;
	DPU_UT_PRT("case 0 end -------------------------------! \n");

	// DPU_UT_PRT("case 1 start +++++++++++++++++++++++++++++! \n");
	// info[sumCnt][0] =dpu_size_test(DPU_804_L, DPU_804_R,"res/online_fgsDisp_res2.bin", "ref/online_fgsDisp_ref2.bin",804, 540,DPU_MODE_SGBM_FGS_ONLINE_MUX0);
	// info[sumCnt][1] =1;
	// sumCnt +=1 ;
	// DPU_UT_PRT("case 1 end -------------------------------! \n");

	// DPU_UT_PRT("case 2 start +++++++++++++++++++++++++++++! \n");
	// info[sumCnt][0] =dpu_size_test(DPU_1608_L, DPU_1608_R,"res/online_fgsDisp_res3.bin", "ref/online_fgsDisp_ref3.bin",1608, 1080,DPU_MODE_SGBM_FGS_ONLINE_MUX0);
	// info[sumCnt][1] =2;
	// sumCnt +=1 ;
	// DPU_UT_PRT("case 2 end -------------------------------! \n");

	DPU_UT_PRT("case 11 start +++++++++++++++++++++++++++++! \n");
	//SGBM&FGS mode test case11-12
	info[sumCnt][0] =dpu_mode_test("res/sgbm_medianDispu16_res.bin", "ref/sgbm_medianDispu16_ref.bin",DPU_MODE_SGBM_MUX1);
	info[sumCnt][1] =11;
	sumCnt +=1 ;
	DPU_UT_PRT("case 11 end -------------------------------! \n");

	DPU_UT_PRT("case 12 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_fgs_mode_test("res/fgs_disp_res.bin", "ref/fgs_disp_ref.bin", DPU_MODE_FGS_MUX0);
	info[sumCnt][1] =12;
	sumCnt +=1 ;
	DPU_UT_PRT("case 12 end -------------------------------! \n");

	DPU_UT_PRT("case 31 start +++++++++++++++++++++++++++++! \n");
	//size test case31-38
	info[sumCnt][0] =dpu_size_test(DPU_PENDULUM_L, DPU_PENDULUM_R,"res/sgbm_size_res1.bin", "ref/sgbm_size_ref1.bin",1920, 1080,DPU_MODE_SGBM_MUX1);
	info[sumCnt][1] =31;
	sumCnt +=1 ;
	DPU_UT_PRT("case 31 end -------------------------------! \n");

	DPU_UT_PRT("case 32 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_size_test(DPU_OHTA_L, DPU_OHTA_R,"res/sgbm_size_res2.bin", "ref/sgbm_size_ref2.bin",384, 288,DPU_MODE_SGBM_MUX1);
	info[sumCnt][1] =32;
	sumCnt +=1 ;
	DPU_UT_PRT("case 32 end -------------------------------! \n");

	DPU_UT_PRT("case 33 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_size_test(DPU_804_L, DPU_804_R,"res/sgbm_size_res3.bin", "ref/sgbm_size_ref3.bin",804, 540,DPU_MODE_SGBM_MUX1);
	info[sumCnt][1] =33;
	sumCnt +=1 ;
	DPU_UT_PRT("case 33 end -------------------------------! \n");

	DPU_UT_PRT("case 34 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_size_test(DPU_1608_L, DPU_1608_R,"res/sgbm_size_res4.bin", "ref/sgbm_size_ref4.bin",1608, 1080,DPU_MODE_SGBM_MUX1);
	info[sumCnt][1] =34;
	sumCnt +=1 ;
	DPU_UT_PRT("case 34 end -------------------------------! \n");

	DPU_UT_PRT("case 35 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_size_test(DPU_PENDULUM_L, DPU_PENDULUM_SGBM_R,"res/sgbm_size_res5.bin", "ref/sgbm_size_ref5.bin",1920, 1080,DPU_MODE_FGS_MUX0);
	info[sumCnt][1] =35;
	sumCnt +=1 ;
	DPU_UT_PRT("case 35 end -------------------------------! \n");

	DPU_UT_PRT("case 36 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_size_test(DPU_OHTA_L, DPU_OHTA_SGBM_R,"res/sgbm_size_res6.bin", "ref/sgbm_size_ref6.bin",384, 288,DPU_MODE_FGS_MUX0);
	info[sumCnt][1] =36;
	sumCnt +=1 ;
	DPU_UT_PRT("case 36 end -------------------------------! \n");

	DPU_UT_PRT("case 37 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_size_test(DPU_804_L, DPU_804_SGBM_R,"res/sgbm_size_res7.bin", "ref/sgbm_size_ref7.bin",804, 540,DPU_MODE_FGS_MUX0);
	info[sumCnt][1] =37;
	sumCnt +=1 ;
	DPU_UT_PRT("case 37 end -------------------------------! \n");

	DPU_UT_PRT("case 38 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_size_test(DPU_1608_L, DPU_1608_SGBM_R,"res/sgbm_size_res8.bin", "ref/sgbm_size_ref8.bin",1608, 1080,DPU_MODE_FGS_MUX0);
	info[sumCnt][1] =38;
	sumCnt +=1 ;
	DPU_UT_PRT("case 38 end -------------------------------! \n");

	DPU_UT_PRT("case 41 start +++++++++++++++++++++++++++++! \n");
	//disp range test case41-47
	info[sumCnt][0] =dpu_disp_range_test(DPU_SOFA_L, DPU_SOFA_R, "res/sgbm_dispRange_res1.bin", "ref/sgbm_dispRange_ref1.bin",512, 284, 16, 3);
	info[sumCnt][1] =41;
	sumCnt +=1 ;
	DPU_UT_PRT("case 41 end -------------------------------! \n");

	DPU_UT_PRT("case 42 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_disp_range_test(DPU_OCTOGONS_L, DPU_OCTOGONS_R, "res/sgbm_dispRange_res2.bin", "ref/sgbm_dispRange_ref2.bin",1920, 1080, 32, 4);
	info[sumCnt][1] =42;
	sumCnt +=1 ;
	DPU_UT_PRT("case 42 end -------------------------------! \n");

	DPU_UT_PRT("case 43 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_disp_range_test(DPU_VINTAGE_L, DPU_VINTAGE_L, "res/sgbm_dispRange_res3.bin", "ref/sgbm_dispRange_ref3.bin",720, 480, 48, 5);
	info[sumCnt][1] =43;
	sumCnt +=1 ;
	DPU_UT_PRT("case 43 end -------------------------------! \n");

	DPU_UT_PRT("case 44 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_disp_range_test(DPU_RANDOM_L, DPU_RANDOM_R, "res/sgbm_dispRange_res4.bin", "ref/sgbm_dispRange_ref4.bin",1920, 1080, 64, 6);
	info[sumCnt][1] =44;
	sumCnt +=1 ;
	DPU_UT_PRT("case 44 end -------------------------------! \n");

	DPU_UT_PRT("case 45 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_disp_range_test(DPU_PENDULUM_L, DPU_PENDULUM_R, "res/sgbm_dispRange_res5.bin", "ref/sgbm_dispRange_ref5.bin",1920, 1080, 96, 7);
	info[sumCnt][1] =45;
	sumCnt +=1 ;
	DPU_UT_PRT("case 45 end -------------------------------! \n");

	DPU_UT_PRT("case 46 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_disp_range_test(DPU_RECYCLE_L, DPU_RECYCLE_R, "res/sgbm_dispRange_res6.bin", "ref/sgbm_dispRange_ref6.bin",720, 480, 127, 8);
	info[sumCnt][1] =46;
	sumCnt +=1 ;
	DPU_UT_PRT("case 46 end -------------------------------! \n");

	DPU_UT_PRT("case 47 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_disp_range_test(DPU_PENDULUM_L, DPU_PENDULUM_R, "res/sgbm_dispRange_res7.bin", "ref/sgbm_dispRange_ref7.bin",1920, 1080, 112, 2);
	info[sumCnt][1] =47;
	sumCnt +=1 ;
	DPU_UT_PRT("case 47 end -------------------------------! \n");

	DPU_UT_PRT("case 51 start +++++++++++++++++++++++++++++! \n");
	//census test case51-52
	info[sumCnt][0] =dpu_census_test("res/sgbm_census_res1.bin", "ref/sgbm_census_ref1.bin",0);
	info[sumCnt][1] =51;
	sumCnt +=1 ;
	DPU_UT_PRT("case 51 end -------------------------------! \n");

	DPU_UT_PRT("case 52 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_census_test("res/sgbm_census_res2.bin", "ref/sgbm_census_ref2.bin",7);
	info[sumCnt][1] =52;
	sumCnt +=1 ;
	DPU_UT_PRT("case 52 end -------------------------------! \n");

	DPU_UT_PRT("case 61 start +++++++++++++++++++++++++++++! \n");
	//add test case61-63
	info[sumCnt][0] =dpu_add_test("res/sgbm_add_res1.bin", "ref/sgbm_add_ref1.bin", 7, 7);
	info[sumCnt][1] =61;
	sumCnt +=1 ;
	DPU_UT_PRT("case 61 end -------------------------------! \n");

	DPU_UT_PRT("case 62 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_add_test("res/sgbm_add_res2.bin", "ref/sgbm_add_ref2.bin", 4, 6);
	info[sumCnt][1] =62;
	sumCnt +=1 ;
	DPU_UT_PRT("case 62 end -------------------------------! \n");

	DPU_UT_PRT("case 63 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_add_test("res/sgbm_add_res3.bin", "ref/sgbm_add_ref3.bin", 0, 0);
	info[sumCnt][1] =63;
	sumCnt +=1 ;
	DPU_UT_PRT("case 63 end -------------------------------! \n");

	DPU_UT_PRT("case 71 start +++++++++++++++++++++++++++++! \n");
	//box filter window size test case71-73
	info[sumCnt][0] =dpu_bfw_test("res/sgbm_bfw_res1.bin", "ref/sgbm_bfw_ref1.bin", 1);
	info[sumCnt][1] =71;
	sumCnt +=1 ;
	DPU_UT_PRT("case 71 end -------------------------------! \n");

	DPU_UT_PRT("case 72 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_bfw_test("res/sgbm_bfw_res2.bin", "ref/sgbm_bfw_ref2.bin", 2);
	info[sumCnt][1] =72;
	sumCnt +=1 ;
	DPU_UT_PRT("case 72 end -------------------------------! \n");

	DPU_UT_PRT("case 73 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_bfw_test("res/sgbm_bfw_res3.bin", "ref/sgbm_bfw_ref3.bin", 3);
	info[sumCnt][1] =73;
	sumCnt +=1 ;
	DPU_UT_PRT("case 73 end -------------------------------! \n");

	DPU_UT_PRT("case 81 start +++++++++++++++++++++++++++++! \n");
	//dcc(p1 p2) test case81-84
	info[sumCnt][0] =dpu_dcc_test("res/sgbm_dcc_res1.bin", "ref/sgbm_dcc_ref1.bin", 0, 32768, 1);
	info[sumCnt][1] =81;
	sumCnt +=1 ;
	DPU_UT_PRT("case 81 end -------------------------------! \n");

	DPU_UT_PRT("case 82 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_dcc_test("res/sgbm_dcc_res2.bin", "ref/sgbm_dcc_ref2.bin", 32768, 65535, 1);
	info[sumCnt][1] =82;
	sumCnt +=1 ;
	DPU_UT_PRT("case 82 end -------------------------------! \n");

	DPU_UT_PRT("case 83 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_dcc_test("res/sgbm_dcc_res3.bin", "ref/sgbm_dcc_ref3.bin", 1, 8, 1);
	info[sumCnt][1] =83;
	sumCnt +=1 ;
	DPU_UT_PRT("case 83 end -------------------------------! \n");

	DPU_UT_PRT("case 84 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_dcc_test("res/sgbm_dcc_res4.bin", "ref/sgbm_dcc_ref4.bin", 49, 392, 1);
	info[sumCnt][1] =84;
	sumCnt +=1 ;
	DPU_UT_PRT("case 84 end -------------------------------! \n");

	DPU_UT_PRT("case 91 start +++++++++++++++++++++++++++++! \n");
	//uniqueness check test case91-93
	info[sumCnt][0] =dpu_uniq_check_test("res/sgbm_uniq_check_res1.bin", "ref/sgbm_uniq_check_ref1.bin", 75);
	info[sumCnt][1] =91;
	sumCnt +=1 ;
	DPU_UT_PRT("case 91 end -------------------------------! \n");

	DPU_UT_PRT("case 92 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_uniq_check_test("res/sgbm_uniq_check_res2.bin", "ref/sgbm_uniq_check_ref2.bin",10);
	info[sumCnt][1] =92;
	sumCnt +=1 ;
	DPU_UT_PRT("case 92 end -------------------------------! \n");

	DPU_UT_PRT("case 93 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_uniq_check_test("res/sgbm_uniq_check_res3.bin", "ref/sgbm_uniq_check_ref3.bin", 5);
	info[sumCnt][1] =93;
	sumCnt +=1 ;
	DPU_UT_PRT("case 93 end -------------------------------! \n");

	DPU_UT_PRT("case 101 start +++++++++++++++++++++++++++++! \n");
	//dispInterp shift test case101-102
	info[sumCnt][0] =dpu_dispShift_test("res/sgbm_dispShift_res1.bin", "ref/sgbm_dispShift_ref1.bin",2);
	info[sumCnt][1] =101;
	sumCnt +=1 ;
	DPU_UT_PRT("case 101 end -------------------------------! \n");

	DPU_UT_PRT("case 102 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_dispShift_test("res/sgbm_dispShift_res2.bin", "ref/sgbm_dispShift_ref2.bin",7);
	info[sumCnt][1] =102;
	sumCnt +=1 ;
	DPU_UT_PRT("case 102 end -------------------------------! \n");

	DPU_UT_PRT("case 111 start +++++++++++++++++++++++++++++! \n");
	//fgs max count test case111-112
	info[sumCnt][0] =dpu_fgsMaxCount_test("res/fgs_MaxCount_res1.bin", "ref/fgs_MaxCount_ref1.bin",20);
	info[sumCnt][1] =111;
	sumCnt +=1 ;
	DPU_UT_PRT("case 111 end -------------------------------! \n");

	DPU_UT_PRT("case 112 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_fgsMaxCount_test("res/fgs_MaxCount_res2.bin", "ref/fgs_MaxCount_ref2.bin",31);
	info[sumCnt][1] =112;
	sumCnt +=1 ;
	DPU_UT_PRT("case 112 end -------------------------------! \n");

	DPU_UT_PRT("case 121 start +++++++++++++++++++++++++++++! \n");
	//fgs max T test case121-122
	info[sumCnt][0] =dpu_fgsMaxT_test("res/fgs_MaxT_res1.bin", "ref/fgs_MaxT_ref1.bin", 4);
	info[sumCnt][1] =121;
	sumCnt +=1 ;
	DPU_UT_PRT("case 121 end -------------------------------! \n");

	DPU_UT_PRT("case 122 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_fgsMaxT_test("res/fgs_MaxT_res2.bin", "ref/fgs_MaxT_ref2.bin", 2);
	info[sumCnt][1] =122;
	sumCnt +=1 ;
	DPU_UT_PRT("case 122 end -------------------------------! \n");

	DPU_UT_PRT("case 131 start +++++++++++++++++++++++++++++! \n");
	//fgs depth test case131-132
	info[sumCnt][0] =dpu_depth_test("res/dpu_depth_res1.bin","ref/dpu_depth_ref1.bin", 5, 351960);
	info[sumCnt][1] =131;
	sumCnt +=1 ;
	DPU_UT_PRT("case 131 end -------------------------------! \n");

	DPU_UT_PRT("case 132 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_depth_test("res/dpu_depth_res2.bin","ref/dpu_depth_ref2.bin",6, 230400);
	info[sumCnt][1] =132;
	sumCnt +=1 ;
	DPU_UT_PRT("case 132 end -------------------------------! \n");

	DPU_UT_PRT("case 141 start +++++++++++++++++++++++++++++! \n");
	//dcc dir test case141-142
	info[sumCnt][0] =dpu_dcc_test("res/sgbm_a13_res.bin", "ref/sgbm_a13_ref.bin", 1800, 14400, 2);
	info[sumCnt][1] =141;
	sumCnt +=1 ;
	DPU_UT_PRT("case 141 end -------------------------------! \n");

	DPU_UT_PRT("case 142 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_dcc_test("res/sgbm_a14_res.bin", "ref/sgbm_a14_ref.bin", 1800, 14400, 3);
	info[sumCnt][1] =142;
	sumCnt +=1 ;
	DPU_UT_PRT("case 142 end -------------------------------! \n");

	DPU_UT_PRT("case 161 start +++++++++++++++++++++++++++++! \n");
	//disp mux test case161-162
	info[sumCnt][0] =dpu_mode_test("res/online_sgbmDepth_res.bin", "ref/online_sgbmDepth_ref.bin",DPU_MODE_SGBM_FGS_ONLINE_MUX2);
	info[sumCnt][1] =161;
	sumCnt +=1 ;
	DPU_UT_PRT("case 161 end -------------------------------! \n");

	DPU_UT_PRT("case 162 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_mode_test("res/online_fgsDepth_res.bin", "ref/online_fgsDepth_ref.bin", DPU_MODE_SGBM_FGS_ONLINE_MUX1);
	info[sumCnt][1] =162;
	sumCnt +=1 ;
	DPU_UT_PRT("case 162 end -------------------------------! \n");

	DPU_UT_PRT("case 171 start +++++++++++++++++++++++++++++! \n");
	//depth unit test case171-173
	info[sumCnt][0] =dpu_unitChoose_test("res/unitChoose_res1.bin","ref/unitChoose_ref1.bin", 1);
	info[sumCnt][1] =171;
	sumCnt +=1 ;
	DPU_UT_PRT("case 171 end -------------------------------! \n");

	DPU_UT_PRT("case 172 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_unitChoose_test("res/unitChoose_res2.bin","ref/unitChoose_ref2.bin", 2);
	info[sumCnt][1] =172;
	sumCnt +=1 ;
	DPU_UT_PRT("case 172 end -------------------------------! \n");

	DPU_UT_PRT("case 173 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_unitChoose_test("res/unitChoose_res3.bin","ref/unitChoose_ref3.bin", 3);
	info[sumCnt][1] =173;
	sumCnt +=1 ;
	DPU_UT_PRT("case 173 end -------------------------------! \n");

	// DPU_UT_PRT("case 181 start +++++++++++++++++++++++++++++! \n");
	//btcost test case181
	info[sumCnt][0] =dpu_btcost_test("res/btcost_res.bin","ref/btcost_ref.bin");
	info[sumCnt][1] =181;
	sumCnt +=1 ;
	DPU_UT_PRT("case 181 end -------------------------------! \n");

	DPU_UT_PRT("case 191 start +++++++++++++++++++++++++++++! \n");
	//u16tou8 test case191-192
	info[sumCnt][0] =dpu_u16tou8_test("res/sgbm_u16tou8_res1.bin", "ref/sgbm_u16tou8_ref1.bin",3, 4);
	info[sumCnt][1] =191;
	sumCnt +=1 ;
	DPU_UT_PRT("case 191 end -------------------------------! \n");

	DPU_UT_PRT("case 192 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_u16tou8_test("res/sgbm_u16tou8_res2.bin", "ref/sgbm_u16tou8_ref2.bin", 7, 8);
	info[sumCnt][1] =192;
	sumCnt +=1 ;
	DPU_UT_PRT("case 192 end -------------------------------! \n");

	DPU_UT_PRT("case 201 start +++++++++++++++++++++++++++++! \n");
	//data sel test case201-208
	info[sumCnt][0] =dpu_size_test(DPU_PENDULUM_L, DPU_PENDULUM_R,"res/sgbm_sel_res1.bin", "ref/sgbm_sel_ref1.bin",1920, 1080,DPU_MODE_SGBM_MUX0);
	info[sumCnt][1] =201;
	sumCnt +=1 ;
	DPU_UT_PRT("case 201 end -------------------------------! \n");

	DPU_UT_PRT("case 202 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_size_test(DPU_OHTA_L, DPU_OHTA_R,"res/sgbm_sel_res2.bin", "ref/sgbm_sel_ref2.bin",384, 288,DPU_MODE_SGBM_MUX0);
	info[sumCnt][1] =202;
	sumCnt +=1 ;
	DPU_UT_PRT("case 202 end -------------------------------! \n");

	DPU_UT_PRT("case 203 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_size_test(DPU_804_L, DPU_804_R,"res/sgbm_sel_res3.bin", "ref/sgbm_sel_ref3.bin",804, 540,DPU_MODE_SGBM_MUX0);
	info[sumCnt][1] =203;
	sumCnt +=1 ;
	DPU_UT_PRT("case 203 end -------------------------------! \n");

	DPU_UT_PRT("case 204 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_size_test(DPU_1608_L, DPU_1608_R,"res/sgbm_sel_res4.bin", "ref/sgbm_sel_ref4.bin",1608, 1080,DPU_MODE_SGBM_MUX0);
	info[sumCnt][1] =204;
	sumCnt +=1 ;
	DPU_UT_PRT("case 204 end -------------------------------! \n");

	DPU_UT_PRT("case 205 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_size_test(DPU_PENDULUM_L, DPU_PENDULUM_R,"res/sgbm_sel_res5.bin", "ref/sgbm_sel_ref5.bin",1920, 1080,DPU_MODE_SGBM_MUX2);
	info[sumCnt][1] =205;
	sumCnt +=1 ;
	DPU_UT_PRT("case 205 end -------------------------------! \n");

	DPU_UT_PRT("case 206 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_size_test(DPU_OHTA_L, DPU_OHTA_R,"res/sgbm_sel_res6.bin", "ref/sgbm_sel_ref6.bin",384, 288,DPU_MODE_SGBM_MUX2);
	info[sumCnt][1] =206;
	sumCnt +=1 ;
	DPU_UT_PRT("case 206 end -------------------------------! \n");

	DPU_UT_PRT("case 207 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_size_test(DPU_804_L, DPU_804_R,"res/sgbm_sel_res7.bin", "ref/sgbm_sel_ref7.bin",804, 540,DPU_MODE_SGBM_MUX2);
	info[sumCnt][1] =207;
	sumCnt +=1 ;
	DPU_UT_PRT("case 207 end -------------------------------! \n");

	DPU_UT_PRT("case 208 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_size_test(DPU_1608_L, DPU_1608_R,"res/sgbm_sel_res8.bin", "ref/sgbm_sel_ref8.bin",1608, 1080,DPU_MODE_SGBM_MUX2);
	info[sumCnt][1] =208;
	sumCnt +=1 ;
	DPU_UT_PRT("case 208 end -------------------------------! \n");

	DPU_UT_PRT("case 211 start +++++++++++++++++++++++++++++! \n");
	//golden test case211-213
	info[sumCnt][0] =dpu_golden_test(DPU_JADEPLANT_L,DPU_JADEPLANT_R,"res/sgbm_golden_jadeplant_res.bin", "ref/sgbm_golden_jadeplant_ref.bin",656,496,DPU_GOLDEN_JADEPENT);
	info[sumCnt][1] =211;
	sumCnt +=1 ;
	DPU_UT_PRT("case 211 end -------------------------------! \n");

	DPU_UT_PRT("case 212 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_golden_test(DPU_PIANO_L,DPU_PIANO_R,"res/sgbm_golden_piano_res.bin", "ref/sgbm_golden_piano_ref.bin",704,480,DPU_GOLDEN_PIANO);
	info[sumCnt][1] =212;
	sumCnt +=1 ;
	DPU_UT_PRT("case 212 end -------------------------------! \n");

	DPU_UT_PRT("case 213 start +++++++++++++++++++++++++++++! \n");
	info[sumCnt][0] =dpu_golden_test(DPU_TEDDY_L,DPU_TEDDY_R,"res/sgbm_golden_teddy_res.bin", "ref/sgbm_golden_teddy_ref.bin",448,368,DPU_GOLDEN_TEDDY);
	info[sumCnt][1] =213;
	sumCnt +=1 ;
	DPU_UT_PRT("case 213 end -------------------------------! \n");

	for(int i=0;i<sumCnt;++i){
		DPU_UT_PRT("case(%d) %s !\n", info[i][1] ,info[i][0] == CVI_SUCCESS ? "pass" : "fail");
		if(info[i][0] != CVI_SUCCESS)
			failCnt += 1;
	}

	DPU_UT_PRT("dpu auto regression %s !! sumCnt(%d) failcnt(%d)\n",failCnt == 0 ? "pass" : "fail",sumCnt,failCnt);

	return failCnt == 0 ? CVI_SUCCESS: CVI_FAILURE;

}

static CVI_S32 _dpu_handle_bmtest_op(CVI_S32 op)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	// CVI_S32 resCnt =0;
	// CVI_S32 size;
	// size =chn_attr.stImgSize.u32Width*chn_attr.stImgSize.u32Height;

	// CVI_U32 mode = 0;
	// CVI_U32 tmp1 = 0;
	// CVI_U32 tmp2 = 0;
	// CVI_CHAR resFileName[128] = "0";
	// CVI_CHAR leftFileName[128] = "0";
	// CVI_CHAR rightFileName[128] = "0";
	// CVI_U32 width;
	// CVI_U32 height;

	switch (op) {
	case 0:
		s32Ret = dpu_size_test(DPU_SOFA_L, DPU_SOFA_R,"res/online_fgsDisp_res1.bin", "ref/online_fgsDisp_ref1.bin",512, 284,DPU_MODE_SGBM_FGS_ONLINE_MUX0);
		DPU_UT_PRT("dpu_case0 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 1:
		s32Ret = dpu_size_test(DPU_804_L, DPU_804_R,"res/online_fgsDisp_res2.bin", "ref/online_fgsDisp_ref2.bin",804, 540,DPU_MODE_SGBM_FGS_ONLINE_MUX0);
		DPU_UT_PRT("dpu_case1 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 2:
		s32Ret = dpu_size_test(DPU_1608_L, DPU_1608_R,"res/online_fgsDisp_res3.bin", "ref/online_fgsDisp_ref3.bin",1608, 1080,DPU_MODE_SGBM_FGS_ONLINE_MUX0);
		DPU_UT_PRT("dpu_case2 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;

	case 11:
		s32Ret = dpu_mode_test("res/sgbm_medianDispu16_res.bin", "ref/sgbm_medianDispu16_ref.bin",DPU_MODE_SGBM_MUX1);
		DPU_UT_PRT("dpu_case11 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 12:
		s32Ret = dpu_fgs_mode_test("res/fgs_disp_res.bin", "ref/fgs_disp_ref.bin", DPU_MODE_FGS_MUX0);
		DPU_UT_PRT("dpu_case12 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 31:
		s32Ret = dpu_size_test(DPU_PENDULUM_L, DPU_PENDULUM_R,"res/sgbm_size_res1.bin", "ref/sgbm_size_ref1.bin",1920, 1080,DPU_MODE_SGBM_MUX1);
		DPU_UT_PRT("dpu_case31 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 32:
		s32Ret = dpu_size_test(DPU_OHTA_L, DPU_OHTA_R,"res/sgbm_size_res2.bin", "ref/sgbm_size_ref2.bin",384, 288,DPU_MODE_SGBM_MUX1);
		DPU_UT_PRT("dpu_case32 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 33:
		s32Ret = dpu_size_test(DPU_804_L, DPU_804_R,"res/sgbm_size_res3.bin", "ref/sgbm_size_ref3.bin",804, 540,DPU_MODE_SGBM_MUX1);
		DPU_UT_PRT("dpu_case33 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 34:
		s32Ret = dpu_size_test(DPU_1608_L, DPU_1608_R,"res/sgbm_size_res4.bin", "ref/sgbm_size_ref4.bin",1608, 1080,DPU_MODE_SGBM_MUX1);
		DPU_UT_PRT("dpu_case34 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 35:
		s32Ret = dpu_size_test(DPU_PENDULUM_L, DPU_PENDULUM_SGBM_R,"res/sgbm_size_res5.bin", "ref/sgbm_size_ref5.bin",1920, 1080,DPU_MODE_FGS_MUX0);
		DPU_UT_PRT("dpu_case35 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 36:
		s32Ret = dpu_size_test(DPU_OHTA_L, DPU_OHTA_SGBM_R,"res/sgbm_size_res6.bin", "ref/sgbm_size_ref6.bin",384, 288,DPU_MODE_FGS_MUX0);
		DPU_UT_PRT("dpu_case36 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 37:
		s32Ret = dpu_size_test(DPU_804_L, DPU_804_SGBM_R,"res/sgbm_size_res7.bin", "ref/sgbm_size_ref7.bin",804, 540,DPU_MODE_FGS_MUX0);
		DPU_UT_PRT("dpu_case37 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 38:
		s32Ret = dpu_size_test(DPU_1608_L, DPU_1608_SGBM_R,"res/sgbm_size_res8.bin", "ref/sgbm_size_ref8.bin",1608, 1080,DPU_MODE_FGS_MUX0);
		DPU_UT_PRT("dpu_case38 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 41:
		s32Ret = dpu_disp_range_test(DPU_SOFA_L, DPU_SOFA_R, "res/sgbm_dispRange_res1.bin", "ref/sgbm_dispRange_ref1.bin",512, 284, 16, 3);
		DPU_UT_PRT("dpu_case41 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 42:
		s32Ret = dpu_disp_range_test(DPU_OCTOGONS_L, DPU_OCTOGONS_R, "res/sgbm_dispRange_res2.bin", "ref/sgbm_dispRange_ref2.bin",1920, 1080, 32, 4);
		DPU_UT_PRT("dpu_case42 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 43:
		s32Ret = dpu_disp_range_test(DPU_VINTAGE_L, DPU_VINTAGE_L, "res/sgbm_dispRange_res3.bin", "ref/sgbm_dispRange_ref3.bin",720, 480, 48, 5);
		DPU_UT_PRT("dpu_case43 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 44:
		s32Ret = dpu_disp_range_test(DPU_RANDOM_L, DPU_RANDOM_R, "res/sgbm_dispRange_res4.bin", "ref/sgbm_dispRange_ref4.bin",1920, 1080, 64, 6);
		DPU_UT_PRT("dpu_case44 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 45:
		s32Ret = dpu_disp_range_test(DPU_PENDULUM_L, DPU_PENDULUM_R, "res/sgbm_dispRange_res5.bin", "ref/sgbm_dispRange_ref5.bin",1920, 1080, 96, 7);
		DPU_UT_PRT("dpu_case45 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 46:
		s32Ret = dpu_disp_range_test(DPU_RECYCLE_L, DPU_RECYCLE_R, "res/sgbm_dispRange_res6.bin", "ref/sgbm_dispRange_ref6.bin",720, 480, 127, 8);
		DPU_UT_PRT("dpu_case46 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 47:
	    s32Ret = dpu_disp_range_test(DPU_PENDULUM_L, DPU_PENDULUM_R, "res/sgbm_dispRange_res7.bin", "ref/sgbm_dispRange_ref7.bin",1920, 1080, 112, 2);
		DPU_UT_PRT("dpu_case47 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 51:
	    s32Ret = dpu_census_test("res/sgbm_census_res1.bin", "ref/sgbm_census_ref1.bin",0);
	    DPU_UT_PRT("dpu_case51 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 52:
	    s32Ret = dpu_census_test("res/sgbm_census_res2.bin", "ref/sgbm_census_ref2.bin",7);
	    DPU_UT_PRT("dpu_case52 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 61:
		s32Ret = dpu_add_test("res/sgbm_add_res1.bin", "ref/sgbm_add_ref1.bin", 7, 7);
		DPU_UT_PRT("dpu_case61 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 62:
		s32Ret = dpu_add_test("res/sgbm_add_res2.bin", "ref/sgbm_add_ref2.bin", 4, 6);
		DPU_UT_PRT("dpu_case62 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 63:
		s32Ret = dpu_add_test("res/sgbm_add_res3.bin", "ref/sgbm_add_ref3.bin", 0, 0);
		DPU_UT_PRT("dpu_case63 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 71:
		s32Ret = dpu_bfw_test("res/sgbm_bfw_res1.bin", "ref/sgbm_bfw_ref1.bin", 1);
		DPU_UT_PRT("dpu_case71 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 72:
		s32Ret = dpu_bfw_test("res/sgbm_bfw_res2.bin", "ref/sgbm_bfw_ref2.bin", 2);
		DPU_UT_PRT("dpu_case72 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 73:
		s32Ret = dpu_bfw_test("res/sgbm_bfw_res3.bin", "ref/sgbm_bfw_ref3.bin", 3);
		DPU_UT_PRT("dpu_case73 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 81:
		s32Ret = dpu_dcc_test("res/sgbm_dcc_res1.bin", "ref/sgbm_dcc_ref1.bin", 0, 32768, 1);
		DPU_UT_PRT("dpu_case81 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 82:
		s32Ret = dpu_dcc_test("res/sgbm_dcc_res2.bin", "ref/sgbm_dcc_ref2.bin", 32768, 65535, 1);
		DPU_UT_PRT("dpu_case82 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 83:
		s32Ret = dpu_dcc_test("res/sgbm_dcc_res3.bin", "ref/sgbm_dcc_ref3.bin", 1, 8, 1);
		DPU_UT_PRT("dpu_case83 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 84:
		s32Ret = dpu_dcc_test("res/sgbm_dcc_res4.bin", "ref/sgbm_dcc_ref4.bin", 49, 392, 1);
		DPU_UT_PRT("dpu_case84 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;

	case 91:
		s32Ret = dpu_uniq_check_test("res/sgbm_uniq_check_res1.bin", "ref/sgbm_uniq_check_ref1.bin", 75);
		DPU_UT_PRT("dpu_case91 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 92:
		s32Ret = dpu_uniq_check_test("res/sgbm_uniq_check_res2.bin", "ref/sgbm_uniq_check_ref2.bin",10);
		DPU_UT_PRT("dpu_case92 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 93:
		s32Ret = dpu_uniq_check_test("res/sgbm_uniq_check_res3.bin", "ref/sgbm_uniq_check_ref3.bin", 5);
		DPU_UT_PRT("dpu_case93 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;

	case 101:
		s32Ret = dpu_dispShift_test("res/sgbm_dispShift_res1.bin", "ref/sgbm_dispShift_ref1.bin",2);
		DPU_UT_PRT("dpu_case101 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 102:
		s32Ret = dpu_dispShift_test("res/sgbm_dispShift_res2.bin", "ref/sgbm_dispShift_ref2.bin",7);
		DPU_UT_PRT("dpu_case102 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;

	case 111:
		s32Ret = dpu_fgsMaxCount_test("res/fgs_MaxCount_res1.bin", "ref/fgs_MaxCount_ref1.bin",20);
		DPU_UT_PRT("dpu_case111 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 112:
		s32Ret = dpu_fgsMaxCount_test("res/fgs_MaxCount_res2.bin", "ref/fgs_MaxCount_ref2.bin",31);
		DPU_UT_PRT("dpu_case112 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;

	case 121:
		s32Ret = dpu_fgsMaxT_test("res/fgs_MaxT_res1.bin", "ref/fgs_MaxT_ref1.bin", 4);
		DPU_UT_PRT("dpu_case121 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 122:
		s32Ret = dpu_fgsMaxT_test("res/fgs_MaxT_res2.bin", "ref/fgs_MaxT_ref2.bin", 2);
		DPU_UT_PRT("dpu_case122 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;

	case 131:
		s32Ret = dpu_depth_test("res/dpu_depth_res1.bin","ref/dpu_depth_ref1.bin", 5, 351960);
		DPU_UT_PRT("dpu_case131 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 132:
		s32Ret = dpu_depth_test("res/dpu_depth_res2.bin","ref/dpu_depth_ref2.bin",6, 230400);
		DPU_UT_PRT("dpu_case132 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;

	case 141:
		s32Ret = dpu_dcc_test("res/sgbm_a13_res.bin", "ref/sgbm_a13_ref.bin", 1800, 14400, 2);
		DPU_UT_PRT("dpu_case141 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 142:
		s32Ret = dpu_dcc_test("res/sgbm_a14_res.bin", "ref/sgbm_a14_ref.bin", 1800, 14400, 3);
		DPU_UT_PRT("dpu_case142 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;

	case 161:
		s32Ret = dpu_mode_test("res/online_sgbmDepth_res.bin", "ref/online_sgbmDepth_ref.bin",DPU_MODE_SGBM_FGS_ONLINE_MUX2);
		DPU_UT_PRT("dpu_case161 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 162:
		s32Ret = dpu_mode_test("res/online_fgsDepth_res.bin", "ref/online_fgsDepth_ref.bin", DPU_MODE_SGBM_FGS_ONLINE_MUX1);
		DPU_UT_PRT("dpu_case162 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;

	case 171:
		s32Ret = dpu_unitChoose_test("res/unitChoose_res1.bin","ref/unitChoose_ref1.bin", 1);
		DPU_UT_PRT("dpu_case171 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 172:
		s32Ret = dpu_unitChoose_test("res/unitChoose_res2.bin","ref/unitChoose_ref2.bin", 2);
		DPU_UT_PRT("dpu_case172 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 173:
		s32Ret = dpu_unitChoose_test("res/unitChoose_res3.bin","ref/unitChoose_ref3.bin", 3);
		DPU_UT_PRT("dpu_case173 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 181:
		s32Ret = dpu_btcost_test("res/btcost_res.bin","ref/btcost_ref.bin");
		DPU_UT_PRT("dpu_case181 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;

	case 191:
		s32Ret = dpu_u16tou8_test("res/sgbm_u16tou8_res1.bin", "ref/sgbm_u16tou8_ref1.bin",3, 4);
		DPU_UT_PRT("dpu_case191 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 192:
		s32Ret = dpu_u16tou8_test("res/sgbm_u16tou8_res2.bin", "ref/sgbm_u16tou8_ref2.bin", 7, 8);
		DPU_UT_PRT("dpu_case192 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;

	case 201:
		s32Ret = dpu_size_test(DPU_PENDULUM_L, DPU_PENDULUM_R,"res/sgbm_sel_res1.bin", "ref/sgbm_sel_ref1.bin",1920, 1080,DPU_MODE_SGBM_MUX0);
		DPU_UT_PRT("dpu_case201 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 202:
		s32Ret = dpu_size_test(DPU_OHTA_L, DPU_OHTA_R,"res/sgbm_sel_res2.bin", "ref/sgbm_sel_ref2.bin",384, 288,DPU_MODE_SGBM_MUX0);
		DPU_UT_PRT("dpu_case202 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 203:
		s32Ret = dpu_size_test(DPU_804_L, DPU_804_R,"res/sgbm_sel_res3.bin", "ref/sgbm_sel_ref3.bin",804, 540,DPU_MODE_SGBM_MUX0);
		DPU_UT_PRT("dpu_case203 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 204:
		s32Ret = dpu_size_test(DPU_1608_L, DPU_1608_R,"res/sgbm_sel_res4.bin", "ref/sgbm_sel_ref4.bin",1608, 1080,DPU_MODE_SGBM_MUX0);
		DPU_UT_PRT("dpu_case204 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 205:
		s32Ret = dpu_size_test(DPU_PENDULUM_L, DPU_PENDULUM_R,"res/sgbm_sel_res5.bin", "ref/sgbm_sel_ref5.bin",1920, 1080,DPU_MODE_SGBM_MUX2);
		DPU_UT_PRT("dpu_case205 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 206:
		s32Ret = dpu_size_test(DPU_OHTA_L, DPU_OHTA_R,"res/sgbm_sel_res6.bin", "ref/sgbm_sel_ref6.bin",384, 288,DPU_MODE_SGBM_MUX2);
		DPU_UT_PRT("dpu_case206 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 207:
		s32Ret = dpu_size_test(DPU_804_L, DPU_804_R,"res/sgbm_sel_res7.bin", "ref/sgbm_sel_ref7.bin",804, 540,DPU_MODE_SGBM_MUX2);
		DPU_UT_PRT("dpu_case207 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 208:
		s32Ret = dpu_size_test(DPU_1608_L, DPU_1608_R,"res/sgbm_sel_res8.bin", "ref/sgbm_sel_ref8.bin",1608, 1080,DPU_MODE_SGBM_MUX2);
		DPU_UT_PRT("dpu_case208 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;

	case 211:
		s32Ret = dpu_golden_test(DPU_JADEPLANT_L,DPU_JADEPLANT_R,"res/sgbm_golden_jadeplant_res.bin", "ref/sgbm_golden_jadeplant_ref.bin",656,496,DPU_GOLDEN_JADEPENT);
		DPU_UT_PRT("dpu_case211 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 212:
		s32Ret = dpu_golden_test(DPU_PIANO_L,DPU_PIANO_R,"res/sgbm_golden_piano_res.bin", "ref/sgbm_golden_piano_ref.bin",704,480,DPU_GOLDEN_PIANO);
		DPU_UT_PRT("dpu_case212 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 213:
		s32Ret = dpu_golden_test(DPU_TEDDY_L,DPU_TEDDY_R,"res/sgbm_golden_teddy_res.bin", "ref/sgbm_golden_teddy_ref.bin",448,368,DPU_GOLDEN_TEDDY);
		DPU_UT_PRT("dpu_case213 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 221:
		s32Ret = dpu_size_test(DPU_PENDULUM_L, DPU_PENDULUM_SGBM_R,"res/sgbm_U16time.bin", "ref/sgbm_size_ref5.bin",1920, 1080,DPU_MODE_SGBM_MUX1);
		DPU_UT_PRT("dpu_case221 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 222:
		s32Ret = dpu_size_test(DPU_PENDULUM_L, DPU_PENDULUM_SGBM_R,"res/fgs_U16time.bin", "ref/sgbm_size_ref5.bin",1920, 1080,DPU_MODE_FGS_MUX1);
		DPU_UT_PRT("dpu_case222 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	case 223:
		s32Ret = dpu_size_test(DPU_PENDULUM_L, DPU_PENDULUM_SGBM_R,"res/online_U16time.bin", "ref/sgbm_size_ref5.bin",1920, 1080,DPU_MODE_SGBM_FGS_ONLINE_MUX1);
		DPU_UT_PRT("dpu_case223 %s!\n",s32Ret ==CVI_SUCCESS?"PASS":"FAIL");
		break;
	default:
	DPU_UT_PRT("the test case not find!!!\n");
	s32Ret = CVI_FAILURE;
	break;
	}

	return s32Ret;
}

static CVI_S32 _dpu_handle_op(CVI_S32 op)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CVI_S32 resCnt =0;
	CVI_S32 size;
	size =chn_attr.stImgSize.u32Width*chn_attr.stImgSize.u32Height;

	CVI_U32 mode = 0;
	CVI_U32 tmp1 = 0;
	CVI_U32 tmp2 = 0;
	CVI_CHAR resFileName[128] = "0";
	CVI_CHAR leftFileName[128] = "0";
	CVI_CHAR rightFileName[128] = "0";
	CVI_CHAR leftFileNameflip[128] = "flip_";
	CVI_CHAR rightFileNameflip[128] = "flip_";
	CVI_S32 width;
	CVI_S32 height;
	CVI_S32 flag;
	CVI_S32 bConvert;

	switch (op) {
	case DPU_CHECK_REG_READ:
		CVI_DPU_CheckRegRead();
		break;
	case DPU_CHECK_REG_WRITE:
		CVI_DPU_CheckRegWrite();
		break;
	case DPU_CHECK_SGBM_STATUS:
		CVI_DPU_CheckSgbmStatus();
		break;
	case DPU_CHECK_FGS_STATUS:
		CVI_DPU_CheckFgsStatus();
		break;
	case DPU_MODE_SGBM_MUX0_TEST:
		set_attr_default();
		grp_attr.enDpuMode =DPU_MODE_SGBM_MUX0;
		strcpy(fileNameInLeft,FILE_IN_LEFT);
		strcpy(fileNameInRight,FILE_IN_RIGHT);
		strcpy(fileNameOut,FILE_OUT);
		strcat(fileNameOut,"sgbm_mux0.bin");
		strcpy(fileNameResult,FILE_RESULT_SGBM_MUX0);
		s32Ret = basic();
		if(s32Ret != CVI_SUCCESS){
			DPU_UT_PRT("sgbm_mux0 case fail\n");
			return CVI_FAILURE;
		}
		s32Ret = CompareWithFile(fileNameOut,fileNameResult,size);
		if(s32Ret != CVI_SUCCESS){
			DPU_UT_PRT("sgbm_mux0 CompareWithFile fail\n");
			return CVI_FAILURE;
		}
		DPU_UT_PRT("sgbm_mux0 case PASS!\n");
		break;
	case DPU_MODE_SGBM_MUX1_TEST:
		set_attr_default();
		grp_attr.enDpuMode =DPU_MODE_SGBM_MUX1;
		strcpy(fileNameInLeft,FILE_IN_LEFT);
		strcpy(fileNameInRight,FILE_IN_RIGHT);
		strcpy(fileNameOut,FILE_OUT);
		strcat(fileNameOut,"sgbm_mux1.bin");
		strcpy(fileNameResult,FILE_RESULT_SGBM_MUX1);
		s32Ret = basic();
		if(s32Ret != CVI_SUCCESS){
			DPU_UT_PRT("sgbm_mux1 case fail\n");
			return CVI_FAILURE;
		}
		s32Ret = CompareWithFile(fileNameOut,fileNameResult,size);
		if(s32Ret != CVI_SUCCESS){
			DPU_UT_PRT("sgbm_mux1 CompareWithFile fail\n");
			return CVI_FAILURE;
		}
		DPU_UT_PRT("sgbm_mux1 case PASS!\n");
		break;
	case DPU_MODE_SGBM_MUX2_TEST:
		set_attr_default();
		grp_attr.enDpuMode =DPU_MODE_SGBM_MUX2;
		strcpy(fileNameInLeft,FILE_IN_LEFT);
		strcpy(fileNameInRight,FILE_IN_RIGHT);
		strcpy(fileNameOut,FILE_OUT);
		strcat(fileNameOut,"sgbm_mux2.bin");
		strcpy(fileNameResult,FILE_RESULT_SGBM_MUX2);
		s32Ret = basic();
		if(s32Ret != CVI_SUCCESS){
			DPU_UT_PRT("sgbm_mux2 case fail\n");
			return CVI_FAILURE;
		}
		s32Ret = CompareWithFile(fileNameOut,fileNameResult,size);
		if(s32Ret != CVI_SUCCESS){
			DPU_UT_PRT("sgbm_mux2 CompareWithFile fail\n");
			return CVI_FAILURE;
		}
		DPU_UT_PRT("sgbm_mux2 case PASS!\n");
		break;
	case DPU_MODE_ONLINE_MUX0_TEST:
		set_attr_default();
		grp_attr.enDpuMode =DPU_MODE_SGBM_FGS_ONLINE_MUX0;
		strcpy(fileNameInLeft,FILE_IN_LEFT);
		strcpy(fileNameInRight,FILE_IN_RIGHT);
		strcpy(fileNameOut,FILE_OUT);
		strcat(fileNameOut,"online_mux0.bin");
		strcpy(fileNameResult,FILE_RESULT_ONLINE_MUX0);
		s32Ret = basic();
		if(s32Ret != CVI_SUCCESS){
			DPU_UT_PRT("online_mux0 case fail\n");
			return CVI_FAILURE;
		}
		s32Ret = CompareWithFile(fileNameOut,fileNameResult,size);
		if(s32Ret != CVI_SUCCESS){
			DPU_UT_PRT("online_mux0 CompareWithFile fail\n");
			return CVI_FAILURE;
		}
		DPU_UT_PRT("online_mux0 case PASS!\n");
		break;
	case DPU_MODE_ONLINE_MUX1_TEST:
		set_attr_default();
		grp_attr.enDpuMode =DPU_MODE_SGBM_FGS_ONLINE_MUX1;
		strcpy(fileNameInLeft,FILE_IN_LEFT);
		strcpy(fileNameInRight,FILE_IN_RIGHT);
		strcpy(fileNameOut,FILE_OUT);
		strcat(fileNameOut,"online_mux1.bin");
		strcpy(fileNameResult,FILE_RESULT_ONLINE_MUX1);
		s32Ret = basic();
		if(s32Ret != CVI_SUCCESS){
			DPU_UT_PRT("online_mux1 case fail\n");
			return CVI_FAILURE;
		}
		s32Ret = CompareWithFile(fileNameOut,fileNameResult,size);
		if(s32Ret != CVI_SUCCESS){
			DPU_UT_PRT("online_mux1 CompareWithFile fail\n");
			return CVI_FAILURE;
		}
		DPU_UT_PRT("online_mux1 case PASS!\n");
		break;
	case DPU_MODE_ONLINE_MUX2_TEST:
		set_attr_default();
		grp_attr.enDpuMode =DPU_MODE_SGBM_FGS_ONLINE_MUX2;
		strcpy(fileNameInLeft,FILE_IN_LEFT);
		strcpy(fileNameInRight,FILE_IN_RIGHT);
		strcpy(fileNameOut,FILE_OUT);
		strcat(fileNameOut,"online_mux2.bin");
		strcpy(fileNameResult,FILE_RESULT_ONLINE_MUX2);
		s32Ret = basic();
		if(s32Ret != CVI_SUCCESS){
			DPU_UT_PRT("online_mux2 case fail\n");
			return CVI_FAILURE;
		}
		s32Ret = CompareWithFile(fileNameOut,fileNameResult,size);
		if(s32Ret != CVI_SUCCESS){
			DPU_UT_PRT("online_mux2 CompareWithFile fail\n");
			return CVI_FAILURE;
		}
		DPU_UT_PRT("online_mux2 case PASS!\n");
		break;
	case DPU_MODE_FGS_MUX0_TEST:
		set_attr_default();
		grp_attr.enDpuMode =DPU_MODE_FGS_MUX0;
		strcpy(fileNameInLeft,FILE_IN_LEFT);
		strcpy(fileNameInRight,FILE_IN_SGBM_RESULT);
		strcpy(fileNameOut,FILE_OUT);
		strcat(fileNameOut,"fgs_mux0.bin");
		strcpy(fileNameResult,FILE_RESULT_FGS_MUX0);
		s32Ret = basic();
		if(s32Ret != CVI_SUCCESS){
			DPU_UT_PRT("fgs_mux0 case fail\n");
			return CVI_FAILURE;
		}
		s32Ret = CompareWithFile(fileNameOut,fileNameResult,size);
		if(s32Ret != CVI_SUCCESS){
			DPU_UT_PRT("fgs_mux0 CompareWithFile fail\n");
			return CVI_FAILURE;
		}
		DPU_UT_PRT("fgs_mux0 case PASS!\n");
		break;
	case DPU_MODE_FGS_MUX1_TEST:
		set_attr_default();
		grp_attr.enDpuMode =DPU_MODE_FGS_MUX1;
		strcpy(fileNameInLeft,FILE_IN_LEFT);
		strcpy(fileNameInRight,FILE_IN_SGBM_RESULT);
		strcpy(fileNameOut,FILE_OUT);
		strcat(fileNameOut,"fgs_mux1.bin");
		strcpy(fileNameResult,FILE_RESULT_FGS_MUX1);
		s32Ret = basic();
		if(s32Ret != CVI_SUCCESS){
			DPU_UT_PRT("fgs_mux1 case fail\n");
			return CVI_FAILURE;
		}
		s32Ret = CompareWithFile(fileNameOut,fileNameResult,size);
		if(s32Ret != CVI_SUCCESS){
			DPU_UT_PRT("fgs_mux1 CompareWithFile fail\n");
			return CVI_FAILURE;
		}
		DPU_UT_PRT("fgs_mux1 case PASS!\n");
		break;
	case DPU_MODE_BTCOST_TEST:
		set_attr_default();
		grp_attr.enDpuMode =DPU_MODE_SGBM_MUX2;
		grp_attr.bIsBtcostOut = 1;
		grp_attr.stLeftImageSize.u32Width=64;
		grp_attr.stLeftImageSize.u32Height=64;
		grp_attr.stRightImageSize.u32Width=64;
		grp_attr.stRightImageSize.u32Height=64;

		chn_attr.stImgSize.u32Width=64;
		chn_attr.stImgSize.u32Height=64;
		strcpy(fileNameInLeft,FILE_COSTMAP_IN_LEFT);
		strcpy(fileNameInRight,FILE_COSTMAP_IN_RIGHT);
		strcpy(fileNameOutBtcost,FILE_COSTMAP_OUT);
		strcpy(fileNameResult, FILE_RESULT_SGBM_COSTMAP );
		s32Ret = basic();
		if(s32Ret != CVI_SUCCESS){
			DPU_UT_PRT("sgbm_costmap case fail\n");
			return CVI_FAILURE;
		}
		s32Ret = CompareWithFile(fileNameOutBtcost,fileNameResult,size);
		if(s32Ret != CVI_SUCCESS){
			DPU_UT_PRT("sgbm_costmap CompareWithFile fail\n");
			return CVI_FAILURE;
		}
		DPU_UT_PRT("sgbm_costmap case PASS!\n");
		break;
	case DPU_MODE_AUTO_TEST:
		set_attr_default();
		for(int i=1; i<DPU_MODE_BUTT ;++i){
			strcpy(fileNameOut,FILE_OUT);
			grp_attr.enDpuMode = i;
			switch (i)
			{
			case 1:
				strcat(fileNameOut,"sgbm_mux0.bin");
				strcpy(fileNameResult,FILE_RESULT_SGBM_MUX0);
				strcpy(fileNameInLeft,FILE_IN_LEFT);
				strcpy(fileNameInRight,FILE_IN_RIGHT);
				s32Ret = basic();
				if(s32Ret != CVI_SUCCESS){
					DPU_UT_PRT("sgbm_mux0 case fail\n");
					return CVI_FAILURE;
				}
				break;

			case 2:
				strcat(fileNameOut,"sgbm_mux1.bin");
				strcpy(fileNameResult,FILE_RESULT_SGBM_MUX1);
				strcpy(fileNameInLeft,FILE_IN_LEFT);
				strcpy(fileNameInRight,FILE_IN_RIGHT);
				s32Ret = basic();
				if(s32Ret != CVI_SUCCESS){
					DPU_UT_PRT("sgbm_mux1 case fail\n");
					return CVI_FAILURE;
				}
				break;

			case 3:
				strcat(fileNameOut,"sgbm_mux2.bin");
				strcpy(fileNameResult,FILE_RESULT_SGBM_MUX2);
				strcpy(fileNameInLeft,FILE_IN_LEFT);
				strcpy(fileNameInRight,FILE_IN_RIGHT);
				s32Ret = basic();
				if(s32Ret != CVI_SUCCESS){
					DPU_UT_PRT("sgbm_mux2 case fail\n");
					return CVI_FAILURE;
				}
				break;
			case 4:
				strcat(fileNameOut,"online_mux0.bin");
				strcpy(fileNameResult,FILE_RESULT_ONLINE_MUX0);
				strcpy(fileNameInLeft,FILE_IN_LEFT);
				strcpy(fileNameInRight,FILE_IN_RIGHT);
				s32Ret = basic();
				if(s32Ret != CVI_SUCCESS){
					DPU_UT_PRT("online_mux0 case fail\n");
					return CVI_FAILURE;
				}
				break;
			case 5:
				strcat(fileNameOut,"online_mux1.bin");
				strcpy(fileNameResult,FILE_RESULT_ONLINE_MUX1);
				strcpy(fileNameInLeft,FILE_IN_LEFT);
				strcpy(fileNameInRight,FILE_IN_RIGHT);
				s32Ret = basic();
				if(s32Ret != CVI_SUCCESS){
					DPU_UT_PRT("online_mux1 case fail\n");
					return CVI_FAILURE;
				}
				break;
			case 6:
				strcat(fileNameOut,"online_mux2.bin");
				strcpy(fileNameResult,FILE_RESULT_ONLINE_MUX2);
				strcpy(fileNameInLeft,FILE_IN_LEFT);
				strcpy(fileNameInRight,FILE_IN_RIGHT);
				s32Ret = basic();
				if(s32Ret != CVI_SUCCESS){
					DPU_UT_PRT("online_mux2 case fail\n");
					return CVI_FAILURE;
				}
				break;
			case 7:
				strcat(fileNameOut,"fgs_mux0.bin");
				strcpy(fileNameResult,FILE_RESULT_FGS_MUX0);
				strcpy(fileNameInLeft,FILE_IN_LEFT);
				strcpy(fileNameInRight,FILE_IN_SGBM_RESULT);
				s32Ret = basic();
				if(s32Ret != CVI_SUCCESS){
					DPU_UT_PRT("fgs_mux0 case fail\n");
					return CVI_FAILURE;
				}
				break;
			case 8:
				strcat(fileNameOut,"fgs_mux2.bin");
				strcpy(fileNameResult,FILE_RESULT_FGS_MUX1);
				strcpy(fileNameInLeft,FILE_IN_LEFT);
				strcpy(fileNameInRight,FILE_IN_SGBM_RESULT);
				s32Ret = basic();
				if(s32Ret != CVI_SUCCESS){
					DPU_UT_PRT("fgs_mux1 case fail\n");
					return CVI_FAILURE;
				}
				break;

			default:
				DPU_UT_PRT("op out of the range\n");
				break;
			}
			s32Ret = CompareWithFile(fileNameOut,fileNameResult,size);
			if(s32Ret != CVI_SUCCESS){
				DPU_UT_PRT("case (%d) CompareWithFile fail\n",i);
				return CVI_FAILURE;
			}
				resCnt +=1;
		}
		//costmap test
		set_attr_default();
		grp_attr.enDpuMode =DPU_MODE_SGBM_MUX2;
		grp_attr.bIsBtcostOut = 1;
		grp_attr.stLeftImageSize.u32Width=64;
		grp_attr.stLeftImageSize.u32Height=64;
		grp_attr.stRightImageSize.u32Width=64;
		grp_attr.stRightImageSize.u32Height=64;

		chn_attr.stImgSize.u32Width=64;
		chn_attr.stImgSize.u32Height=64;
		strcpy(fileNameInLeft,FILE_COSTMAP_IN_LEFT);
		strcpy(fileNameInRight,FILE_COSTMAP_IN_RIGHT);
		strcpy(fileNameOutBtcost,FILE_COSTMAP_OUT);
		strcpy(fileNameResult, FILE_RESULT_SGBM_COSTMAP );
		s32Ret = basic();
		if(s32Ret != CVI_SUCCESS){
			DPU_UT_PRT("sgbm_costmap case fail\n");
			return CVI_FAILURE;
		}
		s32Ret = CompareWithFile(fileNameOutBtcost,fileNameResult,size);
		if(s32Ret != CVI_SUCCESS){
			DPU_UT_PRT("sgbm_costmap CompareWithFile fail\n");
			return CVI_FAILURE;
		}
		resCnt +=1;
		DPU_UT_PRT("sgbm_costmap case PASS!\n");

		if(resCnt == DPU_MODE_BUTT){
			DPU_UT_PRT("DPU AUTO TEST PASS!!!!!!!!!!!!\n");
		}else{
			DPU_UT_PRT("DPU AUTO TEST FAILED!!!!!!!!!!!!\n");
			return CVI_FAILURE;

		}
		break;
	case DPU_MODE_PRESS_TEST1:
		s32Ret = dpu_size_test(DPU_PENDULUM_L, DPU_PENDULUM_R,"res/sgbm_size_res5.bin", "ref/sgbm_size_ref5.bin",1920, 1080,DPU_MODE_SGBM_FGS_ONLINE_MUX0);
		break;
	case DPU_MODE_PRESS_TEST2:
		s32Ret = dpu_press_test(DPU_PENDULUM_L, DPU_PENDULUM_R,1920, 1080,DPU_MODE_SGBM_FGS_ONLINE_MUX0);
		break;
	case DPU_MODE_TEST:
	    cvi_get_filename("resFileName:  ", resFileName);
	    cvi_get_value("dpu mode = ", mode);
		s32Ret = dpu_mode_test(resFileName, NULL,(DPU_MODE_E)mode);
		break;
	case DPU_MODE_FGS_TEST:
	    cvi_get_filename("resFileName:  ", resFileName);
	    cvi_get_value("dpu mode = ", mode);
		s32Ret = dpu_fgs_mode_test(resFileName, NULL,(DPU_MODE_E)mode);
	    break;
	case DPU_DISP_RANGE_TEST:
	    cvi_get_filename("resFileName:  ", resFileName);
		cvi_get_filename("leftFileName:  ", leftFileName);
		cvi_get_filename("rightFileName:  ", rightFileName);
		cvi_get_value("width = ", width);
		cvi_get_value("height = ", height);
		cvi_get_value("dispStartPos = ", tmp1);
		cvi_get_value("dispRange = ", mode);
		s32Ret = dpu_disp_range_test(leftFileName, rightFileName,
		         resFileName,NULL, width, height, tmp1, (DPU_DISP_RANGE_E)mode);
	    break;
	case DPU_CENSUS_TEST:
	    cvi_get_filename("resFileName:  ", resFileName);
		cvi_get_value("u32CensusShift = ", tmp1);
		s32Ret = dpu_census_test(resFileName, NULL,tmp1);
		break;
	case DPU_ADD_TEST:
	    cvi_get_filename("resFileName:  ", resFileName);
		cvi_get_value("u32Rshift1 =  ", tmp1);
		cvi_get_value("u32Rshift2 =  ", tmp2);
		s32Ret = dpu_add_test(resFileName, NULL,tmp1, tmp2);
		break;
	case DPU_BOXFILTER_TEST:
	    cvi_get_filename("resFileName:  ", resFileName);
		cvi_get_value("boxfileter mode (1:boxfilter size 1x1  2:boxfilter size 3x3 \
		               3:boxfilter size 5x5  4:boxfilter size 7x7)", mode);
		s32Ret = dpu_bfw_test(resFileName, NULL,(DPU_MASK_MODE_E)mode);
		break;
	case DPU_DCC_TEST:
	    cvi_get_filename("resFileName:  ", resFileName);
		cvi_get_value("u32CaP1 = ", tmp1);
		cvi_get_value("u32CaP2 = ", tmp2);
		cvi_get_value("enDccDir mode (1:DPU_DCC_A12  2:	DPU_DCC_A13  3:DPU_DCC_A14)", mode);
		s32Ret = dpu_dcc_test(resFileName, NULL,tmp1, tmp2, (DPU_DCC_DIR_E )mode);
		break;
	case DPU_UNIQ_CHECK_TEST:
	    cvi_get_filename("resFileName:  ", resFileName);
		cvi_get_value("uniqRatio = ", tmp1);
		s32Ret = dpu_uniq_check_test(resFileName, NULL,tmp1);
	    break;
	case DPU_DISPINTERP_TEST:
	    cvi_get_filename("resFileName:  ", resFileName);
		cvi_get_value("dispShift = ", tmp1);
		s32Ret = dpu_dispShift_test(resFileName, NULL,tmp1);
		break;
	case DPU_U16TOU8_TEST:
	    cvi_get_filename("resFileName:  ", resFileName);
		cvi_get_value("dispShift = ", tmp1);
		cvi_get_value("dispRange = ", tmp2);
		s32Ret = dpu_u16tou8_test(resFileName, NULL,tmp1, (DPU_DISP_RANGE_E)tmp2);
		break;
	case DPU_FGS_COUNT_TEST:
	    cvi_get_filename("resFileName:  ", resFileName);
		cvi_get_value("fgsMaxCount = ", tmp1);
		s32Ret = dpu_fgsMaxCount_test(resFileName, NULL,tmp1);
		break;
	case DPU_FGS_MAX_T_TEST:
	    cvi_get_filename("resFileName:  ", resFileName);
		cvi_get_value("fgsMaxT = ", tmp1);
		s32Ret = dpu_fgsMaxT_test(resFileName,NULL, tmp1);
		break;
	case DPU_DEPTH_TEST:
	    cvi_get_filename("resFileName:  ", resFileName);
		cvi_get_value("dispRange = ", mode);
		cvi_get_value("fxBaseline = ", tmp1);
		s32Ret = dpu_depth_test(resFileName, NULL,(DPU_DISP_RANGE_E)mode, tmp1);
		break;
	case DPU_UNIT_CHOOSE_TEST:
	    cvi_get_filename("resFileName:  ", resFileName);
		cvi_get_value("dpuDepthUnit = (1:MM 2:CM 3:DM 4:M)", tmp1);
	    s32Ret = dpu_unitChoose_test(resFileName,NULL, tmp1);
		break;
	case DPU_BTCOST_TEST:
	    cvi_get_filename("resFileName:  ", resFileName);
		s32Ret = dpu_btcost_test(resFileName,NULL);
		break;
	case DPU_SIZE_TEST:
	    cvi_get_filename("resFileName:  ", resFileName);
		cvi_get_filename("leftFileName:  ", leftFileName);
		cvi_get_filename("rightFileName:  ", rightFileName);
	    cvi_get_value("dpu mode = ", mode);
		cvi_get_value("width = ", width);
		cvi_get_value("height = ", height);
		s32Ret = dpu_size_test(leftFileName,rightFileName,resFileName, NULL,width,height,(DPU_MODE_E)mode);
		break;
	case DPU_GOLDEN_TEST:
	    cvi_get_filename("resFileName:  ", resFileName);
		cvi_get_filename("leftFileName:  ", leftFileName);
		cvi_get_filename("rightFileName:  ", rightFileName);
	    cvi_get_value("dpu mode = ", mode);
		cvi_get_value("width = ", width);
		cvi_get_value("height = ", height);
		s32Ret = dpu_golden_test(leftFileName,rightFileName,resFileName, NULL,width,height,(DPU_GOLDEN_MODE_E)mode);
		break;
	case DPU_AUTO_REGRESSION_TEST:
	    s32Ret = dpu_auto_regression();
		break;

	case DPU_FILE_TEST:
		cvi_get_filename("leftFileName:  ", leftFileName);
		cvi_get_filename("rightFileName:  ", fileNameInRight);
		cvi_get_filename("outFileName:  ", fileNameOut);
		cvi_get_value("width = ", width);
		cvi_get_value("height = ", height);
		cvi_get_value("flag = ", flag);
		cvi_get_value("bConvert = ", bConvert);
		strcat(leftFileNameflip,leftFileName);
		strcat(rightFileNameflip,rightFileName);
		setFlipBin(leftFileName,leftFileNameflip,width,height,flag);
		setFlipBin(fileNameInRight,rightFileNameflip,width,height,flag);
		if(bConvert){
			strcpy(fileNameInLeft,rightFileNameflip);
			strcpy(fileNameInRight,leftFileNameflip);
		}else{
			strcpy(fileNameInLeft,leftFileNameflip);
			strcpy(fileNameInRight,rightFileNameflip);
		}
		FILE *file=fopen(FILE_DPU_INI,"r");
		if(file==NULL){
			printf("read dpu ini failed!\n");
			break;
		}
		int value;
		long long int lvalue;
	    grp_attr.stLeftImageSize.u32Height = height;
		grp_attr.stLeftImageSize.u32Width = width;
		grp_attr.stRightImageSize.u32Height = height;
		grp_attr.stRightImageSize.u32Width = width;
		chn_attr.stImgSize.u32Width = width;
		chn_attr.stImgSize.u32Height = height;
		set_attr_default();
		fscanf(file,"%d",&value);
		grp_attr.enDpuMode = (DPU_MODE_E)value;

		fscanf(file,"%d",&value);
		grp_attr.enMaskMode= (DPU_MASK_MODE_E)value;

		fscanf(file,"%d",&value);
		grp_attr.enDispRange = (DPU_DISP_RANGE_E)value;

		fscanf(file,"%d",&value);
		grp_attr.u16DispStartPos = value;

		fscanf(file,"%d",&value);
		grp_attr.u32Rshift1 = value;

		fscanf(file,"%d",&value);
		grp_attr.u32Rshift2 = value;

		fscanf(file,"%d",&value);
		grp_attr.u32CaP1 = value;

		fscanf(file,"%d",&value);
		grp_attr.u32CaP2 = value;

		fscanf(file,"%d",&value);
		grp_attr.u32UniqRatio = value;

		fscanf(file,"%d",&value);
		grp_attr.u32DispShift = value;

		fscanf(file,"%d",&value);
		grp_attr.u32CensusShift = value;

		fscanf(file,"%lld",&lvalue);
		grp_attr.u32FxBaseline = lvalue;

		fscanf(file,"%d",&value);
		grp_attr.enDccDir = (DPU_DCC_DIR_E)value;

		fscanf(file,"%d",&value);
		grp_attr.u32FgsMaxCount = value;

		fscanf(file,"%d",&value);
		grp_attr.u32FgsMaxT = value;

		fscanf(file,"%d",&value);
		grp_attr.enDpuDepthUnit = (DPU_DEPTH_UNIT_E)value;

		fscanf(file,"%d",&value);
		grp_attr.bIsBtcostOut = value;

		if(grp_attr.bIsBtcostOut){
			strncpy(fileNameOutBtcost, fileNameOut, 128);
		}
		s32Ret = basic();
		fclose(file);
		break;

	default:
	DPU_UT_PRT("the test case not find!!!\n");
	s32Ret = CVI_FAILURE;
	break;
	}

	return s32Ret;
}

static void dpu_bmtest_help(void){
	DPU_UT_PRT("0: dpu_case0  [ online mode ]\n");
	DPU_UT_PRT("1: dpu_case0  [ online mode 4 align]\n");
	DPU_UT_PRT("2: dpu_case0  [ online mode 8 align]\n");
	DPU_UT_PRT("11: dpu_case11  [SGBM base]\n");
    DPU_UT_PRT("12: dpu_case12  [fgs base]\n");
	DPU_UT_PRT("21: dpu_case21  [check read reg]\n");
	DPU_UT_PRT("22: dpu_case22  [check write reg]\n");
	DPU_UT_PRT("31: dpu_case31  [sgbm img size] 1080p\n");
	DPU_UT_PRT("32: dpu_case32  [sgbm img size] 288p\n");
	DPU_UT_PRT("33: dpu_case33  [sgbm img size] 804 540p\n");
	DPU_UT_PRT("34: dpu_case34  [sgbm img size] 1608 1080p\n");
	DPU_UT_PRT("35: dpu_case35  [fgs img size] 1080p\n");
	DPU_UT_PRT("36: dpu_case36  [fgs img size] 288p\n");
	DPU_UT_PRT("37: dpu_case37  [fgs img size] 804 540p\n");
	DPU_UT_PRT("38: dpu_case38  [fgs img size] 1608 1080p\n");
	DPU_UT_PRT("41: dpu_case41  [disp test] 16-2\n");
	DPU_UT_PRT("42: dpu_case42  [disp test] 32-3\n");
	DPU_UT_PRT("43: dpu_case43  [disp test] 48-4\n");
	DPU_UT_PRT("44: dpu_case44  [disp test] 64-5\n");
	DPU_UT_PRT("45: dpu_case45  [disp test] 96-6\n");
	DPU_UT_PRT("46: dpu_case46  [disp test] 127-7\n");
	DPU_UT_PRT("47: dpu_case47  [disp test] 112-1\n");
	DPU_UT_PRT("51: dpu_case51  [census shift] 0\n");
	DPU_UT_PRT("52: dpu_case52  [census shift] 7\n");
	DPU_UT_PRT("61: dpu_case61  [add test] 7 7\n");
	DPU_UT_PRT("62: dpu_case62  [add test] 4 6\n");
	DPU_UT_PRT("63: dpu_case63  [add test] 0 0\n");
	DPU_UT_PRT("71: dpu_case71  [bfw size] 1x1\n");
	DPU_UT_PRT("72: dpu_case72  [bfw size] 3x3\n");
	DPU_UT_PRT("73: dpu_case73  [bfw size] 5x5\n");
	DPU_UT_PRT("81: dpu_case81  [dcc test] 0 32768\n");
	DPU_UT_PRT("82: dpu_case82  [dcc test] 32768 65535\n");
	DPU_UT_PRT("83: dpu_case83  [dcc test] 1 8 \n");
	DPU_UT_PRT("84: dpu_case84  [dcc test] 49 392\n");
	DPU_UT_PRT("91: dpu_case91  [uniq check] 75\n");
	DPU_UT_PRT("92: dpu_case92  [uniq check] 10\n");
	DPU_UT_PRT("93: dpu_case93  [uniq check] 5\n");
	DPU_UT_PRT("101: dpu_case101 [DispInterp] 2\n");
	DPU_UT_PRT("102: dpu_case102 [DispInterp] 7\n");
	DPU_UT_PRT("111: dpu_case111 [Max count] 20\n");
	DPU_UT_PRT("112: dpu_case112 [Max count] 31\n");
	DPU_UT_PRT("121: dpu_case121 [Max t] 4\n");
	DPU_UT_PRT("122: dpu_case122 [Max t] 2\n");
	DPU_UT_PRT("131: dpu_case131 [Depth test] 351960 0\n");
	DPU_UT_PRT("132: dpu_case132 [Depth test] 230400 1\n");
	DPU_UT_PRT("141: dpu_case141 [A234] 1\n");
	DPU_UT_PRT("142: dpu_case142 [A234] 2\n");
	DPU_UT_PRT("161: dpu_case161 [disp mux] 1\n");
	DPU_UT_PRT("162: dpu_case162 [disp mux] 0\n");
	DPU_UT_PRT("171: dpu_case171 [unit choose] 1\n");
	DPU_UT_PRT("172: dpu_case172 [unit choose] 2\n");
	DPU_UT_PRT("173: dpu_case173 [unit choose] 3\n");
	DPU_UT_PRT("181: dpu_case181 [bf dma test]\n");
	DPU_UT_PRT("191: dpu_case191 [U16ToU8 test] 3 3 6\n");
	DPU_UT_PRT("192: dpu_case192 [U16ToU8 test] 7 7 14\n");
	DPU_UT_PRT("201: dpu_case201 [Median DMA sel 0] 512 284\n");
	DPU_UT_PRT("202: dpu_case202 [Median DMA sel 0] 1920 1080\n");
	DPU_UT_PRT("203: dpu_case203 [Median DMA sel 0] 804 540\n");
	DPU_UT_PRT("204: dpu_case204 [Median DMA sel 0] 1608 1080\n");
	DPU_UT_PRT("205: dpu_case205 [Median DMA sel 0] 512 284\n");
	DPU_UT_PRT("206: dpu_case206 [Median DMA sel 0] 1920 1080\n");
	DPU_UT_PRT("207: dpu_case207 [Median DMA sel 0] 804 540\n");
	DPU_UT_PRT("201: dpu_case208 [Median DMA sel 0] 1608 1080\n");
	DPU_UT_PRT("211: dpu_case211 [Golden Sample Jadeplant] 1608 1080\n");
	DPU_UT_PRT("212: dpu_case212 [Golden Sample Piano] 720 480\n");
	DPU_UT_PRT("213: dpu_case213 [Golden Sample Teedy] 720 480\n");
	DPU_UT_PRT("221: dpu_case221 [TimeProf sgbmU16disp] 1080P\n");
	DPU_UT_PRT("222: dpu_case222 [TimeProf fgsU16Depth] 1080P\n");
	DPU_UT_PRT("223: dpu_case223 [TimeProf onlineU16Depth] 1080P\n");
	DPU_UT_PRT(" 255: exit \n");

};

static void dpu_show_help(void){
	DPU_UT_PRT("%4d: dpu check reg read \n", DPU_CHECK_REG_READ);
	DPU_UT_PRT("%4d: dpu check reg write \n", DPU_CHECK_REG_WRITE);
	DPU_UT_PRT("%4d: dpu check SGBM status \n", DPU_CHECK_SGBM_STATUS);
	DPU_UT_PRT("%4d: dpu check FGS status \n", DPU_CHECK_FGS_STATUS);
	DPU_UT_PRT("%4d: dpu sgbm mux0 \n", DPU_MODE_SGBM_MUX0_TEST);
	DPU_UT_PRT("%4d: dpu sgbm mux1 \n", DPU_MODE_SGBM_MUX1_TEST);
	DPU_UT_PRT("%4d: dpu sgbm mux2 \n", DPU_MODE_SGBM_MUX2_TEST);
	DPU_UT_PRT("%4d: dpu online mux0 \n", DPU_MODE_ONLINE_MUX0_TEST);
	DPU_UT_PRT("%4d: dpu online mux1 \n", DPU_MODE_ONLINE_MUX1_TEST);
	DPU_UT_PRT("%4d: dpu online mux2 \n", DPU_MODE_ONLINE_MUX2_TEST);
	DPU_UT_PRT("%4d: dpu fgs mux0 \n", DPU_MODE_FGS_MUX0_TEST);
	DPU_UT_PRT("%4d: dpu fgs mux1 \n", DPU_MODE_FGS_MUX1_TEST);
	DPU_UT_PRT("%4d: dpu sgbm costmap \n", DPU_MODE_SGBM_COSTMAP_TEST);
	DPU_UT_PRT("%4d: dpu auto test \n", DPU_MODE_AUTO_TEST);
	DPU_UT_PRT("%4d: dpu press test1 \n", DPU_MODE_PRESS_TEST1);
	DPU_UT_PRT("%4d: dpu press test2(loop) \n", DPU_MODE_PRESS_TEST2);
	DPU_UT_PRT("%4d: dpu mode test (SGBM、ONLINE)\n", DPU_MODE_TEST);
	DPU_UT_PRT("%4d: only fgs mode test \n", DPU_MODE_FGS_TEST);
	DPU_UT_PRT("%4d: only sgbm, disp range test \n", DPU_DISP_RANGE_TEST);
	DPU_UT_PRT("%4d: only sgbm, census test \n", DPU_CENSUS_TEST);
	DPU_UT_PRT("%4d: only sgbm, add test \n", DPU_ADD_TEST);
	DPU_UT_PRT("%4d: only sgbm, boxfilter test \n", DPU_BOXFILTER_TEST);
	DPU_UT_PRT("%4d: only sgbm, dcc test \n", DPU_DCC_TEST);
	DPU_UT_PRT("%4d: only sgbm, uniq check test \n", DPU_UNIQ_CHECK_TEST);
	DPU_UT_PRT("%4d: only sgbm, dispinterp test \n", DPU_DISPINTERP_TEST);
	DPU_UT_PRT("%4d: only sgbm, median output(u16 to u8) test \n", DPU_U16TOU8_TEST);
	DPU_UT_PRT("%4d: only fgs, fgs count test \n", DPU_FGS_COUNT_TEST);
	DPU_UT_PRT("%4d: only fgs, fgs max t test \n", DPU_FGS_MAX_T_TEST);
	DPU_UT_PRT("%4d: only fgs, fgs depth test \n", DPU_DEPTH_TEST);
	DPU_UT_PRT("%4d: only fgs, fgs unit choose test \n", DPU_UNIT_CHOOSE_TEST);
	DPU_UT_PRT("%4d: debug, btcost test \n", DPU_BTCOST_TEST);
	DPU_UT_PRT("%4d: dpu size test  \n", DPU_SIZE_TEST);
	DPU_UT_PRT("%4d: dpu golden test  \n", DPU_GOLDEN_TEST);
	DPU_UT_PRT("%4d: dpu auto regression \n", DPU_AUTO_REGRESSION_TEST);
	DPU_UT_PRT("%4d: dpu load param file \n", DPU_FILE_TEST);
	DPU_UT_PRT(" 255: exit \n");
}

int main(int argc, char **argv)
{
	CVI_S32 s32Ret;
	CVI_S32 op = 255;
	CVI_S32 op_ut = 0;
	CVI_U32 width;
	CVI_U32 height;
	system("stty erase ^H");

	signal(SIGINT, dpu_ut_HandleSig);
	signal(SIGTERM, dpu_ut_HandleSig);
	strcpy(fileNameInLeft,FILE_IN_LEFT);
	strcpy(fileNameInRight,FILE_IN_RIGHT);
	width =512;
	height =284;
	grp_attr.stLeftImageSize.u32Width=width;
	grp_attr.stLeftImageSize.u32Height=height;
	grp_attr.stRightImageSize.u32Width=width;
	grp_attr.stRightImageSize.u32Height=height;

	chn_attr.stImgSize.u32Width=width;
	chn_attr.stImgSize.u32Height=height;
	memset(&grp_attr_zero,0,sizeof(grp_attr_zero));
	// printf("select ut(0) or bmtest(1): \n");
	// scanf("%d", &op_ut);
	op_ut = UT_MODE ;
	if(argc >= 2){
		op = (CVI_S32)atoi(argv[1]);
		if(op_ut == 0)
			s32Ret = _dpu_handle_op(op);
		else if(op_ut == 1)
			s32Ret = _dpu_handle_bmtest_op(op);
		else{
			printf("no this op \n");
			return 0;
		}
		DPU_UT_PRT("dpu ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	}else{
		do{
			if(op_ut == 0){
				dpu_show_help();
				scanf("%d", &op);
				s32Ret = _dpu_handle_op(op);
			}
			else if(op_ut == 1){
				dpu_bmtest_help();
				scanf("%d", &op);
				s32Ret = _dpu_handle_bmtest_op(op);
			}
			else{
				printf("no this op \n");
				return 0;
			}

			if(s32Ret != CVI_SUCCESS){
				DPU_UT_PRT("dpu ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
				break;
			}
		}while(op!=255);
	}
	return s32Ret;
}
