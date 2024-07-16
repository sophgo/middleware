/*
 Resolution: 720 x 1280
 Format: VG_LITE_RGB565
 Transformation: Scale/Translate
 Alpha Blending: VG_LITE_BLEND_SRC_OVER
 Related APIs: vg_lite_clear/vg_lite_translate/vg_lite_scale/vg_lite_blit/vg_lite_draw
 Description: This case contains three sub-tests: 1.Draw several simple paths and scale them up; 2.Render raster images using matrices
 that accumulate incremental rotations; 3.Blits a test image multiple times on the screen with different transformations.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vg_lite.h"
#include "vg_lite_util.h"
#include "render_accuracy_test.h"

#define __func__ __FUNCTION__
static char* error_type[] =
{
	"VG_LITE_SUCCESS",
	"VG_LITE_INVALID_ARGUMENT",
	"VG_LITE_OUT_OF_MEMORY",
	"VG_LITE_NO_CONTEXT",
	"VG_LITE_TIMEOUT",
	"VG_LITE_OUT_OF_RESOURCES",
	"VG_LITE_GENERIC_IO",
	"VG_LITE_NOT_SUPPORT",
};
#define IS_ERROR(status)         (status > 0)
#define CHECK_ERROR(Function) \
    err = Function; \
    if (IS_ERROR(err)) \
    { \
        printf("[%s: %d] failed.error type is %s\n", __func__, __LINE__,error_type[err]);\
        goto ErrorHandler; \
    }

static vg_lite_buffer_t buffer, src, ras_fb;
static vg_lite_buffer_t* fb;
static vg_lite_path_t p, p1;

static void cleanup(void)
{
	if (buffer.handle != NULL) {
		vg_lite_free(&buffer);
	}
	if (src.handle != NULL) {
		vg_lite_free(&src);
	}
	if (fb->handle != NULL) {
		vg_lite_free(fb);
	}
	if (ras_fb.handle != NULL) {
		vg_lite_free(&ras_fb);
	}
	vg_lite_clear_path(&p);
	vg_lite_clear_path(&p1);

	vg_lite_close();
}

/* Generates a BGR565 raster image from a test path */
static int APP_GenerateRasterImage(vg_lite_buffer_t* fb1,
	test_path_t* tp)
{
	vg_lite_error_t err = VG_LITE_SUCCESS;
	vg_lite_color_t bkColour = OPAQUE_VGLITE_COLOUR(/*red  =*/ 0,
		/*green=*/ 0x99,
		/*blue =*/ 0);
	vg_lite_color_t fgColour;
	vg_lite_matrix_t m;

	/* Release the test image if it already exists */
	if (fb1->address) {
		vg_lite_free(fb1);
		memset(fb1, 0, sizeof(vg_lite_buffer_t));
	}

	fb1->format = VG_LITE_BGR565;
	fb1->width = (tp->xMax + 1 + RASTER_IMAGE_WIDTH_MASK) &
		((uint32_t)~RASTER_IMAGE_WIDTH_MASK);
	fb1->stride = fb1->width << 1;
	fb1->height = tp->yMax + 1;
	fb1->image_mode = VG_LITE_NORMAL_IMAGE_MODE;
	fb1->transparency_mode = VG_LITE_IMAGE_OPAQUE;
	vg_lite_allocate(fb1);

	/* Check if the selected test path size fits into the test image size */
	if (fb1->stride * fb1->height > MAX_RASTER_IMAGE_SIZE) {
		printf("ERROR: Path size %u x %u requires a test raster image of %u bytes.\r\n",
			fb1->width, fb1->height, fb1->stride * fb1->height);
		printf("       Only %u bytes are available (MAX_RASTER_IMAGE_SIZE).\r\n",
			MAX_RASTER_IMAGE_SIZE);

		return 0;
	}

	/* Clear the target buffer using the predefined background colour */
	err = vg_lite_clear(fb1, NULL, bkColour);
	if (err) {
		printf("ERROR: vg_lite_clear failed (err=%d)!\r\n", err);
		return 0;
	}

	/* Initialize path */
	vg_lite_init_path(&p1, VG_LITE_S32, VG_LITE_LOW, tp->sizeInBytes,
		tp->pathData, tp->xMin, tp->yMin, tp->xMax, tp->yMax);

	/* Update foreground colour */
	fgColour = tp->colour;

	/* Always use the identity matrix when drawing the path */
	vg_lite_identity(&m);

	/* Draw the path onto the target buffer */
	err = vg_lite_draw(fb1, &p1, tp->fillMode, &m, VG_LITE_BLEND_SRC_OVER,
		fgColour);
	if (err) {
		printf("ERROR: vg_lite_draw has failed (err=%d)!\r\n", err);
		return 0;
	}

	/* Flush commands to GPU */
	vg_lite_finish();

	return ERR_SUCCESS;
}

static int APP_RunMatrixRasterTest(vg_lite_buffer_t* target,
	const unsigned* testPathIdx,
	unsigned numPaths)
{
	vg_lite_error_t err;
	vg_lite_color_t bkColour = OPAQUE_VGLITE_COLOUR(/*red  =*/ 0,
		/*green=*/ 0x99,
		/*blue =*/ 0);
	unsigned i;
	int status;

	/* Clear the target buffer using the predefined background colour */
	err = vg_lite_clear(target, NULL, bkColour);
	if (err) {
		printf("ERROR: vg_lite_clear failed (err=%d)!\r\n", err);
		return ERR_FAIL;
	}

	printf(" Generating and displaying raster images...\r\n");
	for (i = 0; i < numPaths; i++) {
		printf("    - Path #%d, %u bytes\r\n", testPathIdx[i] + 1,
			testPath[testPathIdx[i]].sizeInBytes);

		status = APP_GenerateRasterImage(&ras_fb,
			&testPath[testPathIdx[i]]);
		if (status != ERR_SUCCESS)
			/*
			 * Assuming that "APP_GenerateRasterImage" has already
			 * displayed information about the error.
			 */
			return status;

		/* Blit the test image onto the target */
		err = vg_lite_blit(target, &ras_fb,
			&testPath[testPathIdx[i]].m, VG_LITE_BLEND_SRC_OVER,
			testPath[testPathIdx[i]].colour, VG_LITE_FILTER_POINT);

		vg_lite_finish();
		if (err) {
			printf("ERROR: vg_lite_blit has failed (err=%d)!\r\n",
				err);
			return ERR_FAIL;
		}
	}
	return ERR_SUCCESS;;
}

/* Draws the paths described in a path descriptors array */
static int APP_RunMatrixPathTest(vg_lite_buffer_t* target,
	const unsigned* testPathIdx,
	unsigned numPaths)
{
	vg_lite_error_t err;
	vg_lite_color_t bkColour = OPAQUE_VGLITE_COLOUR(/*red  =*/ 0,
		/*green=*/ 0x66,
		/*blue =*/ 0xcc);
	unsigned i;

	/* Clear the target buffer using the predefined background colour */
	err = vg_lite_clear(target, NULL, bkColour);
	if (err) {
		printf("ERROR: vg_lite_clear failed (err=%d)!\r\n", err);
		return ERR_FAIL;
	}

	printf(" Drawing paths...\r\n");
	for (i = 0; i < numPaths; i++) {
		printf("    - Path #%d, %u bytes\r\n", testPathIdx[i] + 1,
			testPath[testPathIdx[i]].sizeInBytes);

		/* Initialize path */
		vg_lite_init_path(&p, VG_LITE_S32, VG_LITE_LOW,
			testPath[testPathIdx[i]].sizeInBytes,
			testPath[testPathIdx[i]].pathData,
			testPath[testPathIdx[i]].xMin,
			testPath[testPathIdx[i]].yMin,
			testPath[testPathIdx[i]].xMax,
			testPath[testPathIdx[i]].yMax);

		/* Draw the path onto the target buffer */
		err = vg_lite_draw(target, &p, testPath[testPathIdx[i]].fillMode,
			&testPath[testPathIdx[i]].m, VG_LITE_BLEND_SRC_OVER,
			testPath[testPathIdx[i]].colour);
		if (err) {
			printf("ERROR: vg_lite_draw has failed (err=%d)!\r\n", err);
			return ERR_FAIL;
		}
	}

	return ERR_SUCCESS;;
}


/* Draws a thin, empty rectangle in a frame buffer using a specified colour */
void APP_DrawBoundingRect(vg_lite_buffer_t* target,
	int16_t xMin,
	int16_t yMin,
	int16_t xMax,
	int16_t yMax,
	vg_lite_color_t rectColour)
{
	vg_lite_matrix_t m;
	static int16_t rectPathData[APP_RECTANGLE_PATH_SIZE] = {
		VLC_OP_MOVE, 0, 0,
		VLC_OP_LINE, 0, 0,
		VLC_OP_LINE, 0, 0,
		VLC_OP_LINE, 0, 0,
		VLC_OP_LINE, 0, 0,
		VLC_OP_LINE, 0, 0,
		VLC_OP_LINE, 0, 0,
		VLC_OP_LINE, 0, 0,
		VLC_OP_LINE, 0, 0,
		VLC_OP_LINE, 0, 0,
		VLC_OP_END
	};

	rectPathData[1] = xMin - 1; rectPathData[2] = yMin - 1;
	/* Draw outer line */
	/* ...down... */
	rectPathData[4] = xMin - 1; rectPathData[5] = yMax + 2;
	/* ...right... */
	rectPathData[7] = xMax + 2; rectPathData[8] = yMax + 2;
	/* ...up... */
	rectPathData[10] = xMax + 2; rectPathData[11] = yMin - 1;
	/* ...left... */
	rectPathData[13] = xMin; rectPathData[14] = yMin - 1;
	/* Switch to inner line */
	rectPathData[16] = xMin; rectPathData[17] = yMin;
	/* Draw inner line */
	/* ...right... */
	rectPathData[19] = xMax + 1; rectPathData[20] = yMin;
	/* ...down... */
	rectPathData[22] = xMax + 1; rectPathData[23] = yMax + 1;
	/* ...left... */
	rectPathData[25] = xMin; rectPathData[26] = yMax + 1;
	/* ...up and close. */
	rectPathData[28] = xMin; rectPathData[29] = yMin - 1;

	vg_lite_identity(&m);

	vg_lite_init_path(&p, VG_LITE_S16, VG_LITE_LOW,
		APP_RECTANGLE_PATH_SIZE * sizeof(int16_t), rectPathData, xMin - 1,
		yMin - 1, xMax + 2, yMax + 2);

	vg_lite_draw(target, &p, VG_LITE_FILL_EVEN_ODD, &m, VG_LITE_BLEND_SRC_OVER,
		rectColour);
}

static int APP_InitChessBoard(vg_lite_buffer_t* img,
	unsigned width,
	unsigned height)
{
	vg_lite_error_t vg_err;
	int err = ERR_SUCCESS;
	unsigned h, w;
	uint32_t color_idx, colors[] = {
		0xffffffff, // white
		0x0000ffff  // red
	};

	memset(img, 0, sizeof(vg_lite_buffer_t));

	img->width = width;
	img->height = height;
	img->format = VG_LITE_ARGB8888;
	img->tiled = VG_LITE_LINEAR;
	img->image_mode = VG_LITE_NORMAL_IMAGE_MODE;
	img->transparency_mode = VG_LITE_IMAGE_OPAQUE;

	vg_err = vg_lite_allocate(img);
	if (vg_err != VG_LITE_SUCCESS) {
		printf("ERROR: Failed to allocate test image (err=%d)!\r\n",
			vg_err);
		err = ERR_OUT_OF_MEM;
	}

	for (h = 0; h < img->height; ++h)
	{
		for (w = 0; w < img->width; ++w)
		{
			color_idx = (w / 2 + h / 2) % 2;
			*((uint32_t*)img->memory + h * img->width + w) =
				colors[color_idx];
		}
	}

	return err;
}

static int APP_RenderTexture(vg_lite_buffer_t* src,
	vg_lite_buffer_t* target,
	unsigned n,
	const obj_transform_t* transform,
	vg_lite_matrix_t* mat)
{
	int err = ERR_SUCCESS;
	int i;
	vg_lite_matrix_t matrix;
	vg_lite_error_t vg_err;

	for (i = 0; i < n; i++) {
		vg_lite_identity(&matrix);
		vg_lite_translate(transform[i].pos.x, transform[i].pos.y, &matrix);
		vg_lite_rotate(transform[i].rotation, &matrix);
		vg_lite_scale(transform[i].scaling, transform[i].scaling, &matrix);
		/* Save the image transformation matrix; provide it to user. */
		if (mat != NULL)
			memcpy(mat, &matrix, sizeof(vg_lite_matrix_t));
		if (transform[i].opcode == 0)
		{
			/**
			 * The user provided an external source,
			 * so this object is a set of blit matrix transformations.
			 **/
			vg_err = vg_lite_blit(target,
				src,
				&matrix,
				VG_LITE_BLEND_SRC_OVER,
				0,
				VG_LITE_FILTER_POINT);
		}
		else if (transform[i].opcode == 1)
		{
			/* This object is an image */
			vg_err = vg_lite_blit(target,
				transform[i].src,
				&matrix,
				VG_LITE_BLEND_SRC_OVER,
				0,
				VG_LITE_FILTER_POINT);
		}
		else if (transform[i].opcode == 2)
		{
			/* This object is a path */
			vg_err = vg_lite_draw(target,
				transform[i].path,
				VG_LITE_FILL_EVEN_ODD,
				&matrix,
				VG_LITE_BLEND_SRC_OVER,
				transform[i].path_fill_color);
		}
		else if (transform[i].opcode == 3)
		{
			/* This object is a pattern */
			vg_err = vg_lite_draw_pattern(target,
				transform[i].path,
				VG_LITE_FILL_EVEN_ODD,
				&matrix,
				transform[i].src,
				&matrix,
				VG_LITE_BLEND_SRC_OVER,
				VG_LITE_PATTERN_COLOR,
				transform[i].path_fill_color,
				0xffffffff,
				VG_LITE_FILTER_POINT);
		}
		if (vg_err != VG_LITE_SUCCESS) {
			printf("ERROR: VGLite operation failed at coordinates x=%d, y=%d (err=%d)!\r\n",
				transform[i].pos.x, transform[i].pos.y, vg_err);
			err = ERR_FAIL;
			break;
		}
	}

	return err;
}

//blit_rect test
int main(int argc, const char* argv[])
{
	int err = ERR_SUCCESS;
	vg_lite_error_t vg_err = VG_LITE_SUCCESS;


	vg_lite_init(TARGET_WIDTH_DEFAULT, TARGET_HEIGHT_DEFAULT);
	buffer.width = TARGET_WIDTH_DEFAULT;
	buffer.height = TARGET_HEIGHT_DEFAULT;
	buffer.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
	buffer.transparency_mode = VG_LITE_IMAGE_OPAQUE;
	buffer.format = VG_LITE_BGR565;
	vg_lite_allocate(&buffer);
	fb = &buffer;


	if (vg_err != VG_LITE_SUCCESS) {
		printf("ERROR: Failed to allocate test target buffer (err = %d)!\r\n",
			vg_err);
		return ERR_OUT_OF_MEM;
	}

	///* Initialize paths transformation matrices to identity */
	for (int i = 0; i < APP_NUM_PATHS_SCALEDOWN_PATH; i++)
		vg_lite_identity(&testPath[testPathDownscalePath[i]].m);

	/* Update matrices with the parameters that are specific for this test */
	vg_lite_translate(100, 100, &testPath[testPathDownscalePath[0]].m);
	vg_lite_scale(0.5, 0.5, &testPath[testPathDownscalePath[0]].m);

	vg_lite_translate(300, 300, &testPath[testPathDownscalePath[1]].m);
	vg_lite_scale(0.25, 0.25, &testPath[testPathDownscalePath[1]].m);

	vg_lite_translate(500, 500, &testPath[testPathDownscalePath[2]].m);
	vg_lite_scale(0.75, 0.75, &testPath[testPathDownscalePath[2]].m);

	/* Draw the graphic artefacts */
	err = APP_RunMatrixPathTest(fb, testPathDownscalePath,
		APP_NUM_PATHS_SCALEDOWN_PATH);

	/* Draw a bounding rectangle to highlight the captured zone for this test */
	APP_DrawBoundingRect(fb, CAPTURE_WINDOW,
		OPAQUE_VGLITE_COLOUR(/*red=*/0xff, /*green=*/0xff, /*blue=*/0));

	vg_lite_finish();
	vg_lite_save_png("matrix_downscale_path_bgr565.png", fb);

	/* Initialize paths transformation matrices to identity */
	for (int i = 0; i < APP_NUM_PATHS_ROTATE_PATH_INC; i++)
		vg_lite_identity(&testPath[testPathRotatePathInc[i]].m);

	/* Update matrices with the parameters that are specific for this test */
	vg_lite_translate(360, 640, &testPath[testPathRotatePathInc[0]].m);
	vg_lite_rotate(45, &testPath[testPathRotatePathInc[0]].m);
	vg_lite_rotate(22.5, &testPath[testPathRotatePathInc[0]].m);
	vg_lite_rotate(22.5, &testPath[testPathRotatePathInc[0]].m);

	vg_lite_translate(360, 640, &testPath[testPathRotatePathInc[1]].m);
	vg_lite_rotate(10, &testPath[testPathRotatePathInc[1]].m);
	vg_lite_rotate(140, &testPath[testPathRotatePathInc[1]].m);
	vg_lite_rotate(30, &testPath[testPathRotatePathInc[1]].m);

	vg_lite_translate(360, 640, &testPath[testPathRotatePathInc[2]].m);
	vg_lite_rotate(90, &testPath[testPathRotatePathInc[2]].m);
	vg_lite_rotate(180, &testPath[testPathRotatePathInc[2]].m);
	vg_lite_rotate(45, &testPath[testPathRotatePathInc[2]].m);

	/* Draw the graphic artefacts */
	err = APP_RunMatrixRasterTest(fb, testPathRotateRasterInc,
		APP_NUM_PATHS_ROTATE_RASTER_INC);

	/* Draw a bounding rectangle to highlight the captured zone for this test */
	APP_DrawBoundingRect(fb, CAPTURE_WINDOW,
		OPAQUE_VGLITE_COLOUR(/*red=*/0xff, /*green=*/0xff, /*blue=*/0));

	/* End drawing */
	vg_lite_finish();

	vg_lite_save_png("matrix_rotate_raster_inc_bgr565.png", fb);

	err = APP_InitChessBoard(&src, 128, 128);
	if (err != ERR_SUCCESS)
		return err;
	/* Create a black background */
	vg_lite_clear(fb, NULL, 0xff000000);

	err = APP_RenderTexture(&src, fb, 12, obj_xfrm, NULL);

	/* End drawing */
	vg_lite_finish();

	vg_lite_save_png("matrix_blit_chessboard_test.png", fb);

ErrorHandler:
	// Cleanup.
	cleanup();
	return 0;
}
