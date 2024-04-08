#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>
#include "buffers.h"

//64 beytes ALIGN PIC for test
#define PIC_RGB888 "pic_rgb888.bmp"
#define BIT8_TO_BIT16(x) ((float)x/255.0 * 65535.0)

struct cvitek_format {
	uint32_t pixel_format;
	char filename[32];
};

struct cvitek_format disp_formats[] = {
	{ DRM_FORMAT_RGB888, PIC_RGB888 },
};

static uint16_t disp_gamma_data_8bit[5][65] = {
    {0, 176, 189, 196, 202, 206, 209,
    212, 215, 217, 219, 221, 222, 224, 225, 227,
    228, 229, 230, 231, 232, 233, 234, 235, 236,
    236, 237, 238, 239, 239, 240, 241, 241, 242,
    242, 243, 243, 244, 244, 245, 245, 246, 246,
    247, 247, 248, 248, 249, 249, 249, 250, 250,
    250, 251, 251, 252, 252, 252, 253, 253, 253,
    254, 254, 254, 255},
    {0, 12, 24, 36, 48, 60,72, 83, 94, 104, 112, 120,
    127, 133, 139, 144, 149, 154, 159, 163, 167, 170,
    174, 177, 181, 184, 187, 190, 192, 195, 198, 200,
    202, 205, 207, 209, 211, 213, 215, 217, 219, 221,
    223, 225, 226, 228, 230, 231, 233, 234, 236, 237,
    239, 240, 242, 243, 244, 246, 247, 248, 250, 251,
    252, 253, 255},
    {0, 3, 7, 11, 15, 19, 23, 27,
    31, 35, 39, 43, 47, 51, 55, 59,
    63, 67, 71, 75, 79, 83, 87, 91,
    95, 99, 103, 107, 111, 115, 119, 123,
    127, 131, 135, 139, 143, 147, 151, 155,
    159, 163, 167, 171, 175, 179, 183, 187,
    191, 195, 199, 203, 207, 211, 215, 219,
    223, 227, 231, 235, 239, 243, 247, 251,
    255},
    {0, 0, 0, 1, 2, 3, 5, 6, 8, 10, 11, 13,
    16, 18, 20, 23, 25, 28, 31, 34, 37, 40,
    43, 47, 50, 54, 57, 61, 65, 69, 73, 77,
    81, 85, 89, 94, 98, 103, 107, 112, 117,
    122, 127, 132, 137, 142, 147, 153, 158,
    164, 169, 175, 181, 186, 192, 198, 204,
    210, 216, 222, 229, 235, 241, 248, 255},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2,
    4, 5, 8, 12, 17, 25, 35, 50, 70, 97, 135,
    186, 255 }};

static uint16_t disp_gamma_data[5][65] = {0};

struct bo *bo_obj;
struct crtc_data {
  uint16_t *r, *g, *b; // gammas
  int fd;
  uint32_t crtc_id;
  drmModeCrtc *crtc;
};

void print_usage(char *arg)
{
	printf("usage:%s <crtc_id> <connector_id>\n", arg);
}

int main(int argc, char *argv[])
{
	int fd;
	drmModeConnector *conn;
	drmModeRes *res;
	uint32_t crtc_id;
	uint32_t conn_id;
	uint32_t gamma_size = 65;
	if (argc < 3) {
		print_usage(argv[0]);
		return 0;
	} else {
		sscanf(argv[1], "%u", &crtc_id);
		sscanf(argv[2], "%u", &conn_id);
		// sscanf(argv[3], "%u", &gamma_id); // 0 ~ 4
	}

	fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);

	res = drmModeGetResources(fd);
	for (int i = 0; i < res->count_crtcs; i++) {
		if(crtc_id == res->crtcs[i])
			break;
		if(i == res->count_crtcs -1) {
			printf("not find crtc-%u\n", crtc_id);
			close(fd);
			return 0;
		}
	}

	for (int i = 0; i < res->count_connectors; i++) {
		if(conn_id == res->connectors[i])
			break;
		if(i == res->count_connectors -1) {
			printf("not find connector-%u\n", conn_id);
			close(fd);
			return 0;
		}
	}

	for (uint32_t i = 0; i < 5; i++) {
		for (uint32_t j = 0; j < 65; j++) {
			disp_gamma_data[i][j] = BIT8_TO_BIT16(disp_gamma_data_8bit[i][j]);
		}
	}

	conn = drmModeGetConnector(fd, conn_id);

	for (uint32_t gamma_id = 0; gamma_id < 5; gamma_id++) {
		bo_obj = bo_create(fd, disp_formats[0].pixel_format,
			conn->modes[0].hdisplay, conn->modes[0].vdisplay,
			disp_formats[0].filename);

		drmModeSetCrtc(fd, crtc_id, bo_obj->fb_id,
				0, 0, &conn_id, 1, &conn->modes[0]);

		drmModeCrtcSetGamma(fd, crtc_id, gamma_size,
				disp_gamma_data[gamma_id], disp_gamma_data[gamma_id],
				disp_gamma_data[gamma_id]);


		printf("drmModeSetCrtc\n");
		getchar();

		bo_destroy(bo_obj);
	}

	drmModeFreeConnector(conn);
	drmModeFreeResources(res);

	close(fd);

	return 0;
}
