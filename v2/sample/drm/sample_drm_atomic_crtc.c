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
#define PIC2_NV21 "pic2_nv21.yuv"

struct bo *bo_obj;

void print_usage(char *arg)
{
	printf("usage:%s <crtc_id> <connector_id>\n", arg);
}

static uint32_t get_property_id(int fd, drmModeObjectProperties *props,
				const char *name)
{
	drmModePropertyPtr property;
	uint32_t i, id = 0;

	/* find property according to the name */
	for (i = 0; i < props->count_props; i++) {
		property = drmModeGetProperty(fd, props->props[i]);
		if (!strcmp(property->name, name))
			id = property->prop_id;
		drmModeFreeProperty(property);

		if (id)
			break;
	}

	return id;
}

int main(int argc, char *argv[])
{
	int fd;
	drmModeConnector *conn;
	drmModeRes *res;
	drmModePlaneRes *plane_res;
	drmModeObjectProperties *props;
	drmModeAtomicReq *req;
	uint32_t crtc_id;
	uint32_t conn_id;
	uint32_t plane_id;
	uint32_t blob_id;
	uint32_t property_crtc_id;
	uint32_t property_mode_id;
	uint32_t property_active;
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

	conn = drmModeGetConnector(fd, conn_id);

	bo_obj = bo_create(fd, DRM_FORMAT_NV21,
		conn->modes[0].hdisplay, conn->modes[0].vdisplay,
		PIC1_NV21);

	drmSetClientCap(fd, DRM_CLIENT_CAP_ATOMIC, 1);

	/* get connector properties */
	props = drmModeObjectGetProperties(fd, conn_id,	DRM_MODE_OBJECT_CONNECTOR);
	property_crtc_id = get_property_id(fd, props, "CRTC_ID");
	drmModeFreeObjectProperties(props);

	/* get crtc properties */
	props = drmModeObjectGetProperties(fd, crtc_id, DRM_MODE_OBJECT_CRTC);
	property_active = get_property_id(fd, props, "ACTIVE");
	property_mode_id = get_property_id(fd, props, "MODE_ID");
	drmModeFreeObjectProperties(props);

	/* create blob to store current mode, and retun the blob id */
	drmModeCreatePropertyBlob(fd, &conn->modes[0],
				sizeof(conn->modes[0]), &blob_id);

	/* start modeseting */
	req = drmModeAtomicAlloc();
	drmModeAtomicAddProperty(req, crtc_id, property_active, 1);
	drmModeAtomicAddProperty(req, crtc_id, property_mode_id, blob_id);
	drmModeAtomicAddProperty(req, conn_id, property_crtc_id, crtc_id);
	drmModeAtomicCommit(fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
	drmModeAtomicFree(req);

	printf("drmModeAtomicCommit SetCrtc\n");
	getchar();

	drmModeSetPlane(fd, plane_id, crtc_id, bo_obj->fb_id, 0,
			50, 50, 320, 320,
			0, 0, 320 << 16, 320 << 16);

	printf("drmModeSetPlane\n");
	getchar();

	bo_destroy(bo_obj);

	drmModeFreeConnector(conn);
	drmModeFreePlaneResources(plane_res);
	drmModeFreeResources(res);

	close(fd);

	return 0;
}
