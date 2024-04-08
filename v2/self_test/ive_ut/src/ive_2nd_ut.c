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
	check_pass(test_ordstatfilter(SMALL_PIC_TEST, gWriteImg, gInstant),
			"OrdStatFilter");
	check_pass(test_filter(SMALL_PIC_TEST, gWriteImg, gInstant), "Filter");
	check_pass(test_csc(SMALL_PIC_TEST, gWriteImg, gInstant), "CSC");
	check_pass(test_filterandcsc(SMALL_PIC_TEST, gWriteImg, gInstant),
			"FilterAndCSC");
	check_pass(test_ncc(gWriteImg, gInstant), "NCC");
	check_pass(test_threshold(SMALL_PIC_TEST, gWriteImg, gInstant),
			"Threshold");
	check_pass(test_dma(gWriteImg, gInstant), "DMA");
	check_pass(test_thresh_s16(SMALL_PIC_TEST, gWriteImg, gInstant),
			"ThresholdS16");
	check_pass(test_thresh_u16(SMALL_PIC_TEST, gWriteImg, gInstant),
			"ThresholdU16");
	check_pass(test_16botto8bit(SMALL_PIC_TEST, gWriteImg, gInstant),
			"16BitTo8Bit");
	check_pass(test_resize(gWriteImg, gInstant), "Resize");
	check_pass(test_gradfg(SMALL_PIC_TEST, gWriteImg, gInstant),
			"GradFg");
	check_pass(test_stcandicorner(SMALL_PIC_TEST, gWriteImg,
						gInstant),
			"StCandiCorner");
	check_pass(test_sad(gWriteImg, gInstant), "Sad");
	check_pass(test_framediffmotion(SMALL_PIC_TEST, gWriteImg,
					gInstant),
			"Frame Diff Motion");
	check_pass(test_ccl(gWriteImg, gInstant), "CCL");
	check_pass(test_multi_thread(SMALL_PIC_TEST, gWriteImg, gInstant), "multi_thread");
}

void tile_test(void)
{
	check_pass(test_gradfg(TILE_MODE_TEST, gWriteImg, gInstant),
			"(Tile Mode) GradFg");
	check_pass(test_stcandicorner(TILE_MODE_TEST, gWriteImg,
						gInstant),
			"(Tile Mode) StCandiCorner");
	check_pass(test_threshold(TILE_MODE_TEST, gWriteImg, gInstant),
			"(Tile Mode) Threshold");
	check_pass(test_framediffmotion(TILE_MODE_TEST, gWriteImg,
					gInstant),
			"(Tile Mode) Frame Diff Motion");
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
		printf("Id Table:\n\t%d: Run All Test\n\t%d: Sad\n\t%d: Resize\n\t%d: 16BitTo8Bit\n\t"
			   "%d: Threshold\n\t%d: Thresh_S16\n\t%d: Thresh_U16\n\t%d: NCC\n\t%d: Filter\n\t"
			   "%d: CSC\n\t%d: FilterAndCSC\n\t%d: DMA\n\t%d: OrdStatFilter\n\t"
			   "%d: GradFg\n\t%d: StCandiCorner\n\t%d: FrameDiffMotion\n\t%d: CCL\n\t%d: Multi_thread\n\t"
			   "%d: Tile Mode GradFg\n\t"
			   "%d: Tile Mode StCandiCorner\n\t%d: Tile Mode Threshold\n\t%d: Tile Mode FrameDiffMotion\n",
			   TESTCASE_All, TESTCASE_SAD, TESTCASE_Resize, TESTCASE_16BitTo8Bit, TESTCASE_Threshold,
			   TESTCASE_Thresh_S16, TESTCASE_Thresh_U16, TESTCASE_NCC, TESTCASE_Filter, TESTCASE_CSC,
			   TESTCASE_FilterAndCSC, TESTCASE_DMA, TESTCASE_OrdStatFilter, TESTCASE_GradFg,
			   TESTCASE_StCandiCorner, TESTCASE_FrameDiff, TESTCASE_CCL, TESTCASE_Multi_thread,
			   TESTCASE_TILE_GradFg, TESTCASE_TILE_StCandiCorner, TESTCASE_TILE_Threshold,
			   TESTCASE_TILE_FrameDiff);
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

	if (argc == 2)
		sel = atoi(argv[1]);
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
	case TESTCASE_OrdStatFilter:
		check_pass(test_ordstatfilter(SMALL_PIC_TEST, gWriteImg, gInstant),
			   "OrdStatFilter");
		break;
	case TESTCASE_Filter:
		check_pass(test_filter(SMALL_PIC_TEST, gWriteImg, gInstant), "Filter");
		break;
	case TESTCASE_GradFg:
		check_pass(test_gradfg(SMALL_PIC_TEST, gWriteImg, gInstant),
			   "GradFg");
		break;
	case TESTCASE_StCandiCorner:
		check_pass(test_stcandicorner(SMALL_PIC_TEST, gWriteImg,
						  gInstant),
			   "StCandiCorner");
		break;
	case TESTCASE_CSC:
		check_pass(test_csc(SMALL_PIC_TEST, gWriteImg, gInstant), "CSC");
		break;
	case TESTCASE_DMA:
		check_pass(test_dma(gWriteImg, gInstant), "DMA");
		break;
	case TESTCASE_FilterAndCSC:
		check_pass(test_filterandcsc(SMALL_PIC_TEST, gWriteImg, gInstant),
			   "FilterAndCSC");
		break;
	case TESTCASE_Threshold:
		check_pass(test_threshold(SMALL_PIC_TEST, gWriteImg, gInstant),
			   "Threshold");
		break;
	case TESTCASE_NCC:
		check_pass(test_ncc(gWriteImg, gInstant), "NCC");
		break;
	case TESTCASE_Thresh_S16:
		check_pass(test_thresh_s16(SMALL_PIC_TEST, gWriteImg, gInstant),
			   "ThresholdS16");
		break;
	case TESTCASE_Thresh_U16:
		check_pass(test_thresh_u16(SMALL_PIC_TEST, gWriteImg, gInstant),
			   "ThresholdU16");
		break;
	case TESTCASE_16BitTo8Bit:
		check_pass(test_16botto8bit(SMALL_PIC_TEST, gWriteImg, gInstant),
			   "16BitTo8Bit");
		break;
	case TESTCASE_Resize:
		check_pass(test_resize(gWriteImg, gInstant), "Resize");
		break;
	case TESTCASE_SAD:
		check_pass(test_sad(gWriteImg, gInstant), "Sad");
		break;
	case TESTCASE_FrameDiff:
		check_pass(test_framediffmotion(SMALL_PIC_TEST, gWriteImg,
						gInstant),
			   "Frame Diff Motion");
		break;
	case TESTCASE_CCL:
		check_pass(test_ccl(gWriteImg, gInstant), "CCL");
		break;
	case TESTCASE_Multi_thread:
		check_pass(test_multi_thread(SMALL_PIC_TEST, gWriteImg, gInstant), "multi_thread");
		break;
	case TESTCASE_TILE_GradFg:
		check_pass(test_gradfg(TILE_MODE_TEST, gWriteImg, gInstant),
			   "(Tile Mode) GradFg");
		break;
	case TESTCASE_TILE_StCandiCorner:
		check_pass(test_stcandicorner(TILE_MODE_TEST, gWriteImg,
						  gInstant),
			   "(Tile Mode) StCandiCorner");
		break;
	case TESTCASE_TILE_Threshold:
		check_pass(test_threshold(TILE_MODE_TEST, gWriteImg, gInstant),
			   "(Tile Mode) Threshold");
		break;
	case TESTCASE_TILE_FrameDiff:
		check_pass(test_framediffmotion(TILE_MODE_TEST, gWriteImg,
						gInstant),
			   "(Tile Mode) Frame Diff Motion");
		break;
	case TESTCASE_TILE_OrdStatFilter:
		check_pass(test_ordstatfilter(TILE_MODE_TEST, gWriteImg, gInstant),
			   "(Tile Mode) OrdStatFilter");
		break;
	case TESTCASE_TILE_Filter:
		check_pass(test_filter(TILE_MODE_TEST, gWriteImg, gInstant),
			   "(Tile Mode) Filter");
		break;
	case TESTCASE_TILE_FilterAndCSC:
		check_pass(test_filterandcsc(TILE_MODE_TEST, gWriteImg, gInstant),
			   "(Tile Mode) FilterAndCSC");
		break;
	case TESTCASE_TILE_THresh_U16:
		check_pass(test_thresh_u16(TILE_MODE_TEST, gWriteImg, gInstant),
			   "(Tile Mode) ThresholdU16");
		break;
	case TESTCASE_TILE_THresh_S16:
		check_pass(test_thresh_s16(TILE_MODE_TEST, gWriteImg, gInstant),
			   "(Tile Mode) ThresholdS16");
		break;
	case TESTCASE_TILE_16BitTo8Bit:
		check_pass(test_16botto8bit(TILE_MODE_TEST, gWriteImg, gInstant),
			   "(Tile Mode) 16BitTo8Bit");
		break;
	case TESTCASE_CMDQ:
		check_pass(test_cmdq(), "CmdQ");
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
