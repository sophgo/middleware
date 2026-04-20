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
#include <cvi_type.h>


#define DPU_FIEL_L "res/Teddy_left_img.bin"
#define DPU_FIEL_R "res/Teddy_right_img.bin"
#define GRID_INFO_FIEL_L "res/grid_info_27_22_594_28_23_448x368_L.dat"
#define GRID_INFO_FIEL_R "res/grid_info_27_22_594_28_23_448x368_R.dat"
#define DPU_FIEL_OUT "dpu_demo.bin"

#define DPU_WIDTH 448
#define DPU_HEIGHT 368


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
	char fileNameInLeft[128] ="0";
	char fileNameInRight[128]="0";
	char fileNameOut[128]="0";

	if (argc < 2) {
		SAMPLE_DPU_Usage(argv[0]);
		printf("para not enough!\n");
		return CVI_FAILURE;
	}
	s32Index = atoi(argv[1]);

	signal(SIGINT, SAMPLE_DPU_HandleSig);
	signal(SIGTERM, SAMPLE_DPU_HandleSig);

	strcpy(fileNameOut, DPU_FIEL_OUT);
	strcpy(fileNameInLeft, DPU_FIEL_L);
	strcpy(fileNameInRight, DPU_FIEL_R);

	stSize.u32Width = DPU_WIDTH;
	stSize.u32Height = DPU_HEIGHT;
	stSizeIn.u32Width = DPU_WIDTH;
	stSizeIn.u32Height = DPU_HEIGHT;
	stSizeOut.u32Width = DPU_WIDTH;
	stSizeOut.u32Height = DPU_HEIGHT;

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
		s32Ret = SAMPLE_DPU_DWA_TEST(stSizeIn ,stSizeOut, fileNameInLeft,fileNameInRight,fileNameOut,
							GRID_INFO_FIEL_L,GRID_INFO_FIEL_R);

		break;
	default:
		printf("invaild index!!!\n");
		break;
	}


	if (s32Ret == CVI_SUCCESS)
		printf("SAMPLE_DPU exit success!\n");
	else
		printf("SAMPLE_DPU exit abnormally!\n");

	return s32Ret;
}

