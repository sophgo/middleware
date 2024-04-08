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
	check_pass(test_gmm(SMALL_PIC_TEST, gWriteImg, gInstant),
		"GMM");
	check_pass(test_gmm2(SMALL_PIC_TEST, gWriteImg, gInstant),
		"GMM2");
	check_pass(test_bgmodel(SMALL_PIC_TEST, gWriteImg, gInstant),
		"BgModel");
}

void tile_test(void)
{
	check_pass(test_gmm(TILE_MODE_TEST, gWriteImg, gInstant),
		"(Tile Mode) GMM");
	check_pass(test_gmm2(TILE_MODE_TEST, gWriteImg, gInstant),
		"(Tile Mode) GMM2");
	check_pass(test_bgmodel(TILE_MODE_TEST, gWriteImg, gInstant),
		"(Tile Mode) BgModel");
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
		printf("Id Table:\n\t%d: Run All Test\n\t%d: GMM\n\t%d: GMM2\n\t%d: BgModel\n\t"
			   "%d: Tile Mode GMM\n\t%d: Tile Mode GMM2\n\t%d: Tile Mode BgModel\n",
			   TESTCASE_All, TESTCASE_GMM, TESTCASE_GMM2, TESTCASE_BgModel,
			   TESTCASE_TILE_GMM, TESTCASE_TILE_GMM2, TESTCASE_TILE_BgModel);
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
	case TESTCASE_GMM:
		check_pass(test_gmm(SMALL_PIC_TEST, gWriteImg, gInstant),
			   "GMM");
		break;
	case TESTCASE_GMM2:
		check_pass(test_gmm2(SMALL_PIC_TEST, gWriteImg, gInstant),
			   "GMM2");
		break;
	case TESTCASE_BgModel:
		check_pass(test_bgmodel(SMALL_PIC_TEST, gWriteImg, gInstant),
			   "BgModel");
		break;
	case TESTCASE_TILE_GMM:
		check_pass(test_gmm(TILE_MODE_TEST, gWriteImg, gInstant),
			   "(Tile Mode) GMM");
		break;
	case TESTCASE_TILE_GMM2:
		check_pass(test_gmm2(TILE_MODE_TEST, gWriteImg, gInstant),
			   "(Tile Mode) GMM2");
		break;
	case TESTCASE_TILE_BgModel:
		check_pass(test_bgmodel(TILE_MODE_TEST, gWriteImg, gInstant),
			   "(Tile Mode) BgModel");
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
