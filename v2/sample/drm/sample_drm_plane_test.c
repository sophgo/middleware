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

//NV21 64 beytes ALIGN PIC for test
#define PIC1_NV21 "pic1_nv21.yuv"
//argb8888 PIC for test vgop
#define PIC2_ARGB8888 "pic2_argb8888.bmp"
#define PIC2_WIDTH 64
#define PIC2_HEIGHT 64

struct bo **bo_obj;

void print_usage(char *arg)
{
	printf("usage:%s <crtc_id> <connector_id>\n", arg);
}

int main(int argc, char *argv[])
{
	int fd;
	drmModeConnector *conn;
	drmModeRes *res;
	drmModePlaneRes *plane_res;
	uint32_t crtc_id;
	uint32_t conn_id;
	uint32_t plane_id;
	uint32_t disp_id = 0;
	int plane_num_each_crtc = 0;

	if (argc < 3) {
		print_usage(argv[0]);
		return 0;
	} else {
		sscanf(argv[1], "%u", &crtc_id);
		sscanf(argv[2], "%u", &conn_id);
	}

	fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);

	res = drmModeGetResources(fd);

	for (int i = 0; i < res->count_crtcs; i++) {
		if(crtc_id == res->crtcs[i]) {
			disp_id = i;
			break;
		}
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

	drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
	plane_res = drmModeGetPlaneResources(fd);
	//disp0/1 has same num planes
	plane_num_each_crtc = plane_res->count_planes / res->count_crtcs;
	plane_id = plane_res->planes[disp_id * plane_num_each_crtc];

	bo_obj = (struct bo **)malloc(plane_num_each_crtc * sizeof(struct bo *));

	conn = drmModeGetConnector(fd, conn_id);

	for (int i = 0; i < plane_num_each_crtc; i++) {
		bo_obj[i] = bo_create(fd, (i == 0) ? DRM_FORMAT_NV21 : DRM_FORMAT_ARGB8888,
		   (i == 0) ? conn->modes[0].hdisplay : PIC2_WIDTH,
		   (i == 0) ? conn->modes[0].vdisplay : PIC2_HEIGHT,
		   (i == 0) ? PIC1_NV21 : PIC2_ARGB8888);
	}

	for (int i = 0; i < plane_num_each_crtc; i++) {
		if (i == 0) {
			drmModeSetCrtc(fd, crtc_id, bo_obj[i]->fb_id,
				0, 0, &conn_id, 1, &conn->modes[0]);
		} else {
			plane_id = plane_res->planes[disp_id * plane_num_each_crtc + i];
			drmModeSetPlane(fd, plane_id, crtc_id, bo_obj[i]->fb_id, 0,
				0, i * 200, PIC2_WIDTH, PIC2_HEIGHT,
				0 << 16, 0 << 16, PIC2_WIDTH << 16, PIC2_HEIGHT << 16);
		}
	}

	printf("drmModeSetPlane\n");
	getchar();

	for (int i = 0; i < plane_num_each_crtc; i++) {
		bo_destroy(bo_obj[i]);
	}

	free(bo_obj);
	drmModeFreeConnector(conn);
	drmModeFreePlaneResources(plane_res);
	drmModeFreeResources(res);

	close(fd);

	return 0;
}
