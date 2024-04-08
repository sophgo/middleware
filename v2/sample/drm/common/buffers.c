#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>
#include "buffers.h"

/* -----------------------------------------------------------------------------
 * Buffers management
 */
static int fill_img_to_fb(unsigned int format, void *planes[3], unsigned int plane_num, unsigned int length[3], char *filename)
{
	//open file & fread into the mmap address
	FILE *fp;
	int s32len;
	unsigned int i, bmp_offset;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("open file error\n");
		return -1;
	}

	switch (format) {
	case DRM_FORMAT_NV12:
	case DRM_FORMAT_NV21:
	case DRM_FORMAT_NV16:
	case DRM_FORMAT_NV61:
	case DRM_FORMAT_YUV420:
	case DRM_FORMAT_YUV422:
	case DRM_FORMAT_UYVY:
	case DRM_FORMAT_VYUY:
	case DRM_FORMAT_YUYV:
	case DRM_FORMAT_YVYU:
		for(i = 0; i < plane_num; i++) {
			s32len = fread(planes[i], 1, length[i], fp);
			if (s32len <= 0) {
				printf("fread error\n");
				fclose(fp);
				return -1;
			}
		}
		break;
	case DRM_FORMAT_ARGB4444:
	case DRM_FORMAT_ARGB1555:
	case DRM_FORMAT_BGR888:
	case DRM_FORMAT_RGB888:
	case DRM_FORMAT_ARGB8888:
		//bmp data offset
		fseek(fp, 10, SEEK_SET);
		fread(&bmp_offset, 1, 4, fp);
		printf("bmp_offset = %u.\n", bmp_offset);
		fseek(fp, bmp_offset, SEEK_SET);
		for(i = 0; i < plane_num; i++) {
			s32len = fread(planes[i], 1, length[i], fp);
			if (s32len <= 0) {
				printf("fread error\n");
				fclose(fp);
				return -1;
			}
		}
		break;
	default:
		fprintf(stderr, "unsupported format 0x%08x\n",  format);
		fclose(fp);
		return -1;
	}

	fclose(fp);

	return 0;
}

static struct bo *
bo_create_dumb(int fd, unsigned int width, unsigned int height, unsigned int bpp)
{
	struct drm_mode_create_dumb arg;
	struct bo *bo;
	int ret;

	bo = calloc(1, sizeof(*bo));
	if (bo == NULL) {
		fprintf(stderr, "failed to allocate buffer object\n");
		return NULL;
	}

	memset(&arg, 0, sizeof(arg));
	arg.bpp = bpp;
	arg.width = width;
	arg.height = height;

	ret = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &arg);
	if (ret) {
		fprintf(stderr, "failed to create dumb buffer: %s\n",
			strerror(errno));
		free(bo);
		return NULL;
	}

	bo->fd = fd;
	bo->handle = arg.handle;
	bo->size = arg.size;
	bo->pitch = arg.pitch;

	return bo;
}

static int bo_map(struct bo *bo, void **out)
{
	struct drm_mode_map_dumb arg;
	void *map;
	int ret;

	memset(&arg, 0, sizeof(arg));
	arg.handle = bo->handle;

	ret = drmIoctl(bo->fd, DRM_IOCTL_MODE_MAP_DUMB, &arg);
	if (ret)
		return ret;

	map = mmap(0, bo->size, PROT_READ | PROT_WRITE, MAP_SHARED,
		       bo->fd, arg.offset);
	if (map == MAP_FAILED)
		return -EINVAL;

	bo->ptr = map;
	*out = map;

	return 0;
}

static void bo_unmap(struct bo *bo)
{
	if (!bo->ptr)
		return;
	munmap(bo->ptr, bo->size);
	bo->ptr = NULL;
}

struct bo *
bo_create(int fd, unsigned int format,
	  unsigned int width, unsigned int height,
	  char *filename)
{
	unsigned int handles[4];
	unsigned int pitches[4];
	unsigned int offsets[4];
	unsigned int length[3];
	unsigned int virtual_height;
	unsigned int plane_num = 0;
	struct bo *bo;
	unsigned int bpp;
	void *planes[3] = { 0, };
	void *virtual;
	int ret;

	switch (format) {
	case DRM_FORMAT_NV12:
	case DRM_FORMAT_NV21:
	case DRM_FORMAT_NV16:
	case DRM_FORMAT_NV61:
	case DRM_FORMAT_YUV420:
	case DRM_FORMAT_YUV422:
		bpp = 8;
		break;

	case DRM_FORMAT_ARGB4444:
	case DRM_FORMAT_ARGB1555:
	case DRM_FORMAT_UYVY:
	case DRM_FORMAT_VYUY:
	case DRM_FORMAT_YUYV:
	case DRM_FORMAT_YVYU:
		bpp = 16;
		break;

	case DRM_FORMAT_BGR888:
	case DRM_FORMAT_RGB888:
		bpp = 24;
		break;

	case DRM_FORMAT_ARGB8888:
		bpp = 32;
		break;

	default:
		fprintf(stderr, "unsupported format 0x%08x\n",  format);
		return NULL;
	}

	switch (format) {
	case DRM_FORMAT_NV12:
	case DRM_FORMAT_NV21:
	case DRM_FORMAT_YUV420:
		virtual_height = height * 3 / 2;
		break;

	case DRM_FORMAT_NV16:
	case DRM_FORMAT_NV61:
	case DRM_FORMAT_YUV422:
		virtual_height = height * 2;
		break;

	default:
		virtual_height = height;
		break;
	}

	bo = bo_create_dumb(fd, width, virtual_height, bpp);
	if (!bo)
		return NULL;

	ret = bo_map(bo, &virtual);
	if (ret) {
		fprintf(stderr, "failed to map buffer: %s\n",
			strerror(-errno));
		bo_destroy_dumb(bo);
		return NULL;
	}

	switch (format) {
	case DRM_FORMAT_UYVY:
	case DRM_FORMAT_VYUY:
	case DRM_FORMAT_YUYV:
	case DRM_FORMAT_YVYU:
		offsets[0] = 0;
		handles[0] = bo->handle;
		pitches[0] = bo->pitch;

		planes[0] = virtual;
		length[0] = pitches[0] * height;
		plane_num = 1;
		break;

	case DRM_FORMAT_NV12:
	case DRM_FORMAT_NV21:
		offsets[0] = 0;
		handles[0] = bo->handle;
		pitches[0] = bo->pitch;
		pitches[1] = pitches[0];
		offsets[1] = pitches[0] * height;
		handles[1] = bo->handle;

		planes[0] = virtual;
		planes[1] = virtual + offsets[1];
		length[0] = offsets[1];
		length[1] = offsets[1] / 2;
		plane_num = 2;
		break;
	case DRM_FORMAT_NV16:
	case DRM_FORMAT_NV61:
		offsets[0] = 0;
		handles[0] = bo->handle;
		pitches[0] = bo->pitch;
		pitches[1] = pitches[0];
		offsets[1] = pitches[0] * height;
		handles[1] = bo->handle;

		planes[0] = virtual;
		planes[1] = virtual + offsets[1];
		length[0] = offsets[1];
		length[1] = offsets[1];
		plane_num = 2;
		break;

	case DRM_FORMAT_YUV420:
		offsets[0] = 0;
		handles[0] = bo->handle;
		pitches[0] = bo->pitch;
		pitches[1] = pitches[0] / 2;
		offsets[1] = pitches[0] * height;
		handles[1] = bo->handle;
		pitches[2] = pitches[1];
		offsets[2] = offsets[1] + pitches[1] * height / 2;
		handles[2] = bo->handle;

		planes[0] = virtual;
		planes[1] = virtual + offsets[1];
		planes[2] = virtual + offsets[2];
		length[0] = offsets[1];
		length[1] = offsets[2] - offsets[1];
		length[2] = length[1];
		plane_num = 3;
		break;

	case DRM_FORMAT_YUV422:
		offsets[0] = 0;
		handles[0] = bo->handle;
		pitches[0] = bo->pitch;
		pitches[1] = pitches[0] / 2;
		offsets[1] = pitches[0] * height;
		handles[1] = bo->handle;
		pitches[2] = pitches[1];
		offsets[2] = offsets[1] + pitches[1] * height;
		handles[2] = bo->handle;

		planes[0] = virtual;
		planes[1] = virtual + offsets[1];
		planes[2] = virtual + offsets[2];
		length[0] = offsets[1];
		length[1] = offsets[2] - offsets[1];
		length[2] = length[1];
		plane_num = 3;
		break;

	case DRM_FORMAT_ARGB4444:
	case DRM_FORMAT_ARGB1555:
	case DRM_FORMAT_BGR888:
	case DRM_FORMAT_RGB888:
	case DRM_FORMAT_ARGB8888:
		offsets[0] = 0;
		handles[0] = bo->handle;
		pitches[0] = bo->pitch;

		planes[0] = virtual;
		length[0] = pitches[0] * height;
		plane_num = 1;
		break;
	}

	ret = drmModeAddFB2(fd, width, height, format, handles, pitches,
			offsets, &bo->fb_id, 0);
	if (ret) {
		fprintf(stderr, "failed to add FB: %s\n",
			strerror(-errno));
		bo_unmap(bo);
		bo_destroy_dumb(bo);
		return NULL;
	}

	ret = fill_img_to_fb(format, planes, plane_num, length, filename);
	if (ret) {
		fprintf(stderr, "failed to load img data: %s\n",
			strerror(-errno));
		bo_destroy(bo);
		return NULL;
	}

	return bo;
}

void bo_destroy_dumb(struct bo *bo)
{
	struct drm_mode_destroy_dumb arg;
	int ret;

	memset(&arg, 0, sizeof(arg));
	arg.handle = bo->handle;

	ret = drmIoctl(bo->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &arg);
	if (ret)
		fprintf(stderr, "failed to destroy dumb buffer: %s\n",
			strerror(errno));

	free(bo);
}

void bo_destroy(struct bo *bo)
{
	struct drm_mode_destroy_dumb arg;
	int ret;

	drmModeRmFB(bo->fd, bo->fb_id);

	bo_unmap(bo);

	memset(&arg, 0, sizeof(arg));
	arg.handle = bo->handle;

	ret = drmIoctl(bo->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &arg);
	if (ret)
		fprintf(stderr, "failed to destroy dumb buffer: %s\n",
			strerror(errno));

	free(bo);
}
