#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sample_ive.h"

int gWriteImg;
int gInstant;
int gCount;
char VerifyList[100][64];

void check_pass(int ret, char *op)
{
	if (ret == 0) {
		sprintf(VerifyList[gCount], "TEST-PASS %s\n", op);
	} else {
		sprintf(VerifyList[gCount], "TEST-FAILED %s\n", op);
	}
	gCount++;
}

void base_test(void)
{
	check_pass(test_add(SMALL_PIC_TEST, gWriteImg, gInstant),
			"Add");
	check_pass(test_sub(SMALL_PIC_TEST, gWriteImg, gInstant),
			"Sub");
	check_pass(test_and(SMALL_PIC_TEST, gWriteImg, gInstant),
			"And");
	check_pass(test_xor(SMALL_PIC_TEST, gWriteImg, gInstant),
			"Xor");
	check_pass(test_or(SMALL_PIC_TEST, gWriteImg, gInstant), "Or");
	check_pass(test_erode(SMALL_PIC_TEST, gWriteImg, gInstant),
			"Erode");
	check_pass(test_dilate(SMALL_PIC_TEST, gWriteImg, gInstant),
			"Dilate");
	check_pass(test_lbp(SMALL_PIC_TEST, gWriteImg, gInstant), "LBP");
	check_pass(test_sobel(SMALL_PIC_TEST, gWriteImg, gInstant),
			"Sobel");
	check_pass(test_integ(gWriteImg, gInstant), "Integ");
	check_pass(test_hist(gWriteImg, gInstant), "Hist");
	check_pass(test_map(SMALL_PIC_TEST, gWriteImg, gInstant), "Map");
	check_pass(test_normgrad(SMALL_PIC_TEST, gWriteImg, gInstant),
			"NormGrad");
	check_pass(test_bernsen(SMALL_PIC_TEST, gWriteImg, gInstant), "Bernsen");
	check_pass(test_imgtoodma(SMALL_PIC_TEST, gWriteImg, gInstant),
			"ImageToOdma");
	check_pass(test_cannyhysedge(SMALL_PIC_TEST, gWriteImg,
						gInstant),
			"CannyHysEdge");
	check_pass(test_cannyedge(SMALL_PIC_TEST, gWriteImg, gInstant),
			"CannyEdge");
	check_pass(test_magandang(SMALL_PIC_TEST, gWriteImg, gInstant),
			"MagAndAng");
}

void tile_test(void)
{
	check_pass(test_add(TILE_MODE_TEST, gWriteImg, gInstant),
			"(Tile Mode) Add");
	check_pass(test_sub(TILE_MODE_TEST, gWriteImg, gInstant),
			"(Tile Mode) Sub");
	check_pass(test_and(TILE_MODE_TEST, gWriteImg, gInstant),
			"(Tile Mode) And");
	check_pass(test_xor(TILE_MODE_TEST, gWriteImg, gInstant),
			"(Tile Mode) Xor");
	check_pass(test_or(TILE_MODE_TEST, gWriteImg, gInstant),
			"(Tile Mode) Or");
	check_pass(test_erode(TILE_MODE_TEST, gWriteImg, gInstant),
			"(Tile Mode) Erode");
	check_pass(test_dilate(TILE_MODE_TEST, gWriteImg, gInstant),
			"(Tile Mode) Dilate");
	check_pass(test_imgtoodma(TILE_MODE_TEST, gWriteImg, gInstant),
			"(Tile Mode) ImgToOdma");
	check_pass(test_sobel(TILE_MODE_TEST, gWriteImg, gInstant),
			"(Tile Mode) Sobel");
	check_pass(test_normgrad(TILE_MODE_TEST, gWriteImg, gInstant),
			"(Tile Mode) NormGrad");
	check_pass(test_cannyhysedge(TILE_MODE_TEST, gWriteImg,
						gInstant),
			"(Tile Mode) CannyHysEdge");
	check_pass(test_cannyedge(TILE_MODE_TEST, gWriteImg, gInstant),
			"(Tile Mode) CannyEdge");
	check_pass(test_magandang(TILE_MODE_TEST, gWriteImg, gInstant),
			"(Tile Mode) MagAndAng");
	//check_pass(test_lbp(TILE_MODE_TEST, gWriteImg, gInstant),
	//	   "(Tile Mode) LBP");
	//check_pass(test_ordstatfilter(TILE_MODE_TEST, gWriteImg, gInstant),
	//	   "(Tile Mode) OrdStatFilter");
	//check_pass(test_filter(TILE_MODE_TEST, gWriteImg, gInstant),
	//	   "(Tile Mode) Filter");
	//check_pass(test_bernsen(TILE_MODE_TEST, gWriteImg, gInstant),
	//	   "(Tile Mode) Bernsen");
	//check_pass(test_map(TILE_MODE_TEST, gWriteImg, gInstant),
	//	   "(Tile Mode) Map");
	//check_pass(test_filterandcsc(TILE_MODE_TEST, gWriteImg, gInstant),
	//	   "(Tile Mode) FilterAndCSC");
	//check_pass(test_thresh_u16(TILE_MODE_TEST, gWriteImg, gInstant),
	//	   "(Tile Mode) ThresholdU16");
	//check_pass(test_thresh_s16(TILE_MODE_TEST, gWriteImg, gInstant),
	//	   "(Tile Mode) ThresholdS16");
	//check_pass(test_16botto8bit(TILE_MODE_TEST, gWriteImg, gInstant),
	//	   "(Tile Mode) 16BitTo8Bit");
}

int main(int argc, char **argv)
{
	int sel = 0, i = 0;

	gWriteImg = 0;
	gInstant = 1;
	gCount = 0;
	if (argc < 2) {
		printf("Incorrect testcase id. Usage: %s <id>\n", argv[0]);
		printf("		-w: write result to file\n");
		printf("		-i: use interrupt mode\n");
		printf("Id Table:\n\t%d: Run All Test\n\t%d: Add\n\t%d: Sub\n\t%d: And\n\t%d: Xor\n\t"
			   "%d: Or\n\t%d: Erode\n\t%d: Dilate\n\t%d: LBP\n\t%d: Sobel\n\t%d: MagAngAng\n\t"
			   "%d: Integ\n\t%d: Hist\n\t%d: Map\n\t%d: NormGrad\n\t%d: Bernsen\n\t"
			   "%d: CannyHysEdge\n\t%d: CannyEdge\n\t%d: Tile Mode Add\n\t%d: Tile Mode Sub\n\t"
			   "%d: Tile Mode And\n\t%d: Tile Mode Xor\n\t%d: Tile Mode Or\n\t%d: Tile Mode Erode\n\t"
			   "%d: Tile Mode Dilate\n\t%d: Tile Mode Sobel\n\t%d: Tile Mode MagAndAng\n\t"
			   "%d: Tile Mode NormGrad\n\t%d: Tile Mode CannyHysEdge\n\t%d: Tile Mode CannyEdge\n\t",
			   TESTCASE_All, TESTCASE_Add, TESTCASE_Sub, TESTCASE_And, TESTCASE_Xor, TESTCASE_Or,
			   TESTCASE_Erode, TESTCASE_Dilate, TESTCASE_LBP, TESTCASE_Sobel, TESTCASE_MagAndAng,
			   TESTCASE_Integ, TESTCASE_HIST, TESTCASE_MAP, TESTCASE_NormGrad, TESTCASE_Bernsen,
			   TESTCASE_CannyHysEdge, TESTCASE_CannyEdge, TESTCASE_TILE_Add,
			   TESTCASE_TILE_Sub, TESTCASE_TILE_And, TESTCASE_TILE_Xor, TESTCASE_TILE_Or,
			   TESTCASE_TILE_Erode, TESTCASE_TILE_Dilate, TESTCASE_TILE_Sobel, TESTCASE_TILE_MagAndAng,
			   TESTCASE_TILE_NormGrad, TESTCASE_TILE_CannyHysEdge, TESTCASE_TILE_CannyEdge);
		return 0;
	}
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
			case 't':
				printf("Set to [tile mode]\n");
				break;
			case 'w':
				printf("Set to [Save file]\n");
				gWriteImg = 1;
				break;
			case 'i':
				printf("Set to [Interrupt mode]\n");
				gInstant = 0;
				break;
			default:
				printf("Error: bad option \'%c\'\n", argv[i][1]);
			break;
			}
		} else {
			if (argv[i][0] >= '0' && argv[i][0] <= '9') {
				sel = atoi(argv[i]);
			}
		}
	}

	printf("Select Algo Id %d\n", sel);
	switch (sel) {
	case TESTCASE_All:
		base_test();
		tile_test();
		break;
	case TESTCASE_Base_All:
		base_test();
		break;
	case TESTCASE_Tile_All:
		tile_test();
		break;
	case TESTCASE_Add:
		check_pass(test_add(SMALL_PIC_TEST, gWriteImg, gInstant),
			   "Add");
		break;
	case TESTCASE_Sub:
		check_pass(test_sub(SMALL_PIC_TEST, gWriteImg, gInstant),
			   "Sub");
		break;
	case TESTCASE_And:
		check_pass(test_and(SMALL_PIC_TEST, gWriteImg, gInstant),
			   "And");
		break;
	case TESTCASE_Xor:
		check_pass(test_xor(SMALL_PIC_TEST, gWriteImg, gInstant),
			   "Xor");
		break;
	case TESTCASE_Or:
		check_pass(test_or(SMALL_PIC_TEST, gWriteImg, gInstant), "Or");
		break;
	case TESTCASE_Erode:
		check_pass(test_erode(SMALL_PIC_TEST, gWriteImg, gInstant),
			   "Erode");
		break;
	case TESTCASE_Dilate:
		check_pass(test_dilate(SMALL_PIC_TEST, gWriteImg, gInstant),
			   "Dilate");
		break;
	case TESTCASE_LBP:
		check_pass(test_lbp(SMALL_PIC_TEST, gWriteImg, gInstant), "LBP");
		break;
	case TESTCASE_Sobel:
		check_pass(test_sobel(SMALL_PIC_TEST, gWriteImg, gInstant),
			   "Sobel");
		break;
	case TESTCASE_MagAndAng:
		check_pass(test_magandang(SMALL_PIC_TEST, gWriteImg, gInstant),
			   "MagAndAng");
		break;
	case TESTCASE_Integ:
		check_pass(test_integ(gWriteImg, gInstant), "Integ");
		break;
	case TESTCASE_HIST:
		check_pass(test_hist(gWriteImg, gInstant), "Hist");
		break;
	case TESTCASE_MAP:
		check_pass(test_map(SMALL_PIC_TEST, gWriteImg, gInstant), "Map");
		break;
	case TESTCASE_NormGrad:
		check_pass(test_normgrad(SMALL_PIC_TEST, gWriteImg, gInstant),
			   "NormGrad");
		break;
	case TESTCASE_Bernsen:
		check_pass(test_bernsen(SMALL_PIC_TEST, gWriteImg, gInstant), "Bernsen");
		break;
	case TESTCASE_CannyHysEdge:
		check_pass(test_cannyhysedge(SMALL_PIC_TEST, gWriteImg,
						 gInstant),
			   "CannyHysEdge");
		break;
	case TESTCASE_CannyEdge:
		check_pass(test_cannyedge(SMALL_PIC_TEST, gWriteImg, gInstant),
			   "CannyEdge");
		break;
	case TESTCASE_ImgToOdma:
		check_pass(test_imgtoodma(SMALL_PIC_TEST, gWriteImg, gInstant),
			   "ImageToOdma");
		break;
	case TESTCASE_TILE_Add:
		check_pass(test_add(TILE_MODE_TEST, gWriteImg, gInstant),
			   "(Tile Mode) Add");
		break;
	case TESTCASE_TILE_Sub:
		check_pass(test_sub(TILE_MODE_TEST, gWriteImg, gInstant),
			   "(Tile Mode) Sub");
		break;
	case TESTCASE_TILE_And:
		check_pass(test_and(TILE_MODE_TEST, gWriteImg, gInstant),
			   "(Tile Mode) And");
		break;
	case TESTCASE_TILE_Xor:
		check_pass(test_xor(TILE_MODE_TEST, gWriteImg, gInstant),
			   "(Tile Mode) Xor");
		break;
	case TESTCASE_TILE_Or:
		check_pass(test_or(TILE_MODE_TEST, gWriteImg, gInstant),
			   "(Tile Mode) Or");
		break;
	case TESTCASE_TILE_Erode:
		check_pass(test_erode(TILE_MODE_TEST, gWriteImg, gInstant),
			   "(Tile Mode) Erode");
		break;
	case TESTCASE_TILE_Dilate:
		check_pass(test_dilate(TILE_MODE_TEST, gWriteImg, gInstant),
			   "(Tile Mode) Dilate");
		break;
	case TESTCASE_TILE_Sobel:
		check_pass(test_sobel(TILE_MODE_TEST, gWriteImg, gInstant),
			   "(Tile Mode) Sobel");
		break;
	case TESTCASE_TILE_MagAndAng:
		check_pass(test_magandang(TILE_MODE_TEST, gWriteImg, gInstant),
			   "(Tile Mode) MagAndAng");
		break;
	case TESTCASE_TILE_NormGrad:
		check_pass(test_normgrad(TILE_MODE_TEST, gWriteImg, gInstant),
			   "(Tile Mode) NormGrad");
		break;
	case TESTCASE_TILE_CannyHysEdge:
		check_pass(test_cannyhysedge(TILE_MODE_TEST, gWriteImg,
						 gInstant),
			   "(Tile Mode) CannyHysEdge");
		break;
	case TESTCASE_TILE_CannyEdge:
		check_pass(test_cannyedge(TILE_MODE_TEST, gWriteImg, gInstant),
			   "(Tile Mode) CannyEdge");
		break;
	case TESTCASE_TILE_ImgToOdma:
		check_pass(test_imgtoodma(TILE_MODE_TEST, gWriteImg, gInstant),
			   "(Tile Mode) ImageToOdma");
		break;
	case TESTCASE_TILE_LBP:
		check_pass(test_lbp(TILE_MODE_TEST, gWriteImg, gInstant),
			   "(Tile Mode) LBP");
		break;
	case TESTCASE_TILE_Bernsen:
		check_pass(test_bernsen(TILE_MODE_TEST, gWriteImg, gInstant),
			   "(Tile Mode) Bernsen");
		break;
	case TESTCASE_TILE_Map:
		check_pass(test_map(TILE_MODE_TEST, gWriteImg, gInstant),
			   "(Tile Mode) Map");
		break;
	case TESTCASE_IVE_RESET:
		reset();
		return 0;
	case TESTCASE_PRINT_REG:
		dump();
		return 0;
	}
	for (int i = 0; i < gCount; i++)
		printf("%s", VerifyList[i]);
	return 0;
}
