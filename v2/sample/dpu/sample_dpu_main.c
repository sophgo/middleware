#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "sample_comm.h"
#include "sample_dpu.h"
#include "cvi_sys.h"
#include <linux/cvi_type.h>

// #define GRID_INFO_FIEL_L  "meshdatagrid_info_63_31_1953_128_40_2048x800.dat"
// #define GRID_INFO_FIEL_R  "meshdatagrid_info_62_31_1922_128_40_2048x800.dat"

// #define GRID_INFO_FIEL_L  "grid_info_79_43_3397_80_45_1280x720.dat"
// #define GRID_INFO_FIEL_R  "grid_info_79_44_3476_80_45_1280x720.dat"

// #define GRID_INFO_FIEL_L "grid_info_42_20_840_80_45_1280x720_L.dat"
// #define GRID_INFO_FIEL_R "grid_info_42_20_840_80_45_1280x720_R.dat"

// #define GRID_INFO_FIEL_L  "fileGridInfoL.dat"
// #define GRID_INFO_FIEL_R  "fileGridInfoR.dat"

#define GRID_INFO_FIEL_L "grid_info_27_22_594_28_23_448x368_L.dat"
#define GRID_INFO_FIEL_R "grid_info_27_22_594_28_23_448x368_R.dat"
#define WIDTH 448
#define HEIGHT 368

static char fileNameInLeft[50] ="0";
static char fileNameInRight[50]="0";
static char fileNameOut[50]="0";
void SAMPLE_DPU_HandleSig(CVI_S32 signo)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	if (SIGINT == signo || SIGTERM == signo) {
		//todo for release
		SAMPLE_PRT("Program termination abnormally\n");
	}
	exit(-1);
}

void SAMPLE_DPU_Usage(char *sPrgNm)
{
	printf("Usage : %s <index>\n", sPrgNm);
	printf("index:\n");
	printf("\t 0)DPU_MODE_SGBM_MUX0.\n");
	printf("\t 1)DPU_MODE_SGBM_MUX1.\n");
	printf("\t 2)DPU_MODE_SGBM_MUX2.\n");
	printf("\t 4)DPU_MODE_SGBM_FGS_ONLINE_MUX0.\n");
	printf("\t 5)DPU_MODE_SGBM_FGS_ONLINE_MUX1.\n");
	printf("\t 6)DPU_MODE_SGBM_FGS_ONLINE_MUX2.\n");
	printf("\t 7)DPU_MODE_FGS_MUX0.\n");
	printf("\t 8)DPU_MODE_FGS_MUX1.\n");
	printf("\t 9)DPU_DEMO.\n");
}

int main(int argc, char *argv[])
{
	CVI_S32 s32Ret = CVI_FAILURE;
	CVI_S32 s32Index;
	SIZE_S stSize,stSizeIn,stSizeOut;

	if (argc < 2) {
		SAMPLE_DPU_Usage(argv[0]);
		printf("para not enough!\n");
		return CVI_FAILURE;
	}

	// if (!strncmp(argv[1], "-h", 2)) {
	// 	SAMPLE_DPU_Usage(argv[0]);
	// 	return CVI_SUCCESS;
	// }

	signal(SIGINT, SAMPLE_DPU_HandleSig);
	signal(SIGTERM, SAMPLE_DPU_HandleSig);
	printf("0)---------------case mode\n");
	printf("1)---------------left image file name\n");
	printf("2)---------------right image file name\n");
	printf("3)---------------output image file name\n");
	printf("4)---------------image width\n");
	printf("5)---------------image height\n");
	s32Index = atoi(argv[1]);
	// strcpy(fileNameInLeft,argv[2]);
	// strcpy(fileNameInRight,argv[3]);
	// strcpy(fileNameOut,argv[4]);
	memset(&stSize,0,sizeof(stSize));
	memset(&stSizeIn,0,sizeof(stSizeIn));
	memset(&stSizeOut,0,sizeof(stSizeOut));
	// stSize.u32Width = atoi(argv[5]);
	// stSize.u32Height = atoi(argv[6]);

	strcpy(fileNameOut,"dpu_demo.bin");
	// strcpy(fileNameInLeft,"imgL.bin");
	// strcpy(fileNameInRight,"imgR.bin");
	strcpy(fileNameInLeft,"Teddy_left_img.bin");
	strcpy(fileNameInRight,"Teddy_right_img.bin");
	// strcpy(fileNameInLeft,"image_left1024X672.yuv");
	// strcpy(fileNameInRight,"image_right1024X672.yuv");
	// stSize.u32Width = 1280;
	// stSize.u32Height = 720;
	// stSizeIn.u32Width = 1280;
	// stSizeIn.u32Height = 720;
	// stSizeOut.u32Width = 1280;
	// stSizeOut.u32Height = 720;

	stSize.u32Width = WIDTH;
	stSize.u32Height = HEIGHT;
	stSizeIn.u32Width = WIDTH;
	stSizeIn.u32Height = HEIGHT;
	stSizeOut.u32Width = WIDTH;
	stSizeOut.u32Height = HEIGHT;
	switch (s32Index) {
	case 0:
		s32Ret = SAMPLE_DPU_SGBM_MUX0(stSize,fileNameInLeft,fileNameInRight,fileNameOut);
		break;

	case 1:
		s32Ret = SAMPLE_DPU_SGBM_MUX1(stSize,fileNameInLeft,fileNameInRight,fileNameOut);
		break;

	case 2:
		s32Ret = SAMPLE_DPU_SGBM_MUX2(stSize,fileNameInLeft,fileNameInRight,fileNameOut);
		break;

	case 3:
		s32Ret = SAMPLE_DPU_SGBM_MUX3(stSize,fileNameInLeft,fileNameInRight,fileNameOut);
		break;

	case 4:
		s32Ret = SAMPLE_DPU_ONLINE_MUX0(stSize,fileNameInLeft,fileNameInRight,fileNameOut);
		break;

	case 5:
		s32Ret = SAMPLE_DPU_ONLINE_MUX1(stSize,fileNameInLeft,fileNameInRight,fileNameOut);
		break;

	case 6:
		s32Ret = SAMPLE_DPU_ONLINE_MUX2(stSize,fileNameInLeft,fileNameInRight,fileNameOut);
		break;
	case 7:
		s32Ret = SAMPLE_DPU_FGS_MUX0(stSize,fileNameInLeft,fileNameInRight,fileNameOut);
		break;
	case 8:
		s32Ret = SAMPLE_DPU_FGS_MUX1(stSize,fileNameInLeft,fileNameInRight,fileNameOut);
		break;
	case 9:
		// char * grid_info_l ="fileGridInfoL.dat" ;
		// char * grid_info_r ="fileGridInfoR.dat" ;
		s32Ret = SAMPLE_DPU_DWA_TEST(stSizeIn ,stSizeOut, fileNameInLeft,fileNameInRight,fileNameOut,
							GRID_INFO_FIEL_L,GRID_INFO_FIEL_R);

		break;
	default:
		printf("invaild index!!!\n");
		break;
	}
	return s32Ret;

}

