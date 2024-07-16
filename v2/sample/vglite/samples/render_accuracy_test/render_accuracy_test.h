
#ifndef __ERRORS_H__
#define __ERRORS_H__


#define TRANSPARENT_VGLITE_COLOUR(a, r, g, b) \
	((uint32_t)(a) << 24) | ((uint32_t)(b) << 16) | ((uint32_t)(g) << 8) | \
	(uint32_t)r

#define OPAQUE_VGLITE_COLOUR(r, g, b)	TRANSPARENT_VGLITE_COLOUR(0xff, r, g, b)

 /** No error */
#define ERR_SUCCESS                             0
/** Invalid argument */
#define ERR_INVALID_ARG                         1
/** Unspecified failure */
#define ERR_FAIL                                2
/** Input buffer empty */
#define ERR_INPUT_EMPTY                         3
/** Output buffer full */
#define ERR_OUTPUT_FULL                         4
/** Not enough memory to serve the request */
#define ERR_OUT_OF_MEM                          5
/** An object does not exist or it was not found */
#define ERR_NOT_FOUND                           6
/** Operation not supported */
#define ERR_NOT_SUPPORTED                       7
/** Operation has timed out */
#define ERR_TIMEOUT                             8
/** Operation was cancelled */
#define ERR_CANCELLED                           9
/** Parameter out of range */
#define ERR_OUT_OF_RANGE                        10
/** Error code range terminator */
#define ERR_CODE_MAX                            11

#define TARGET_WIDTH_DEFAULT                            720  /* pixels */
#define TARGET_HEIGHT_DEFAULT                           1280  /* pixels */
#define APP_NUM_PATHS_IDENTITY_PATH			6
#define APP_NUM_PATHS					12

#define PATH1_DATA_SIZE					10  /* items */
#define PATH2_DATA_SIZE					16  /* items */
#define PATH3_DATA_SIZE					25  /* items */
#define PATH4_DATA_SIZE					25  /* items */
#define PATH5_DATA_SIZE					13  /* items */
#define PATH6_DATA_SIZE					52  /* items */
#define PATH7_DATA_SIZE					7   /* items */
#define PATH8_DATA_SIZE					13  /* items */
#define PATH9_DATA_SIZE					22  /* items */
#define PATH10_DATA_SIZE				22  /* items */
#define PATH11_DATA_SIZE				10  /* items */
#define PATH12_DATA_SIZE				49  /* items */

#define APP_RECTANGLE_PATH_SIZE                 31 /* items */
#define APP_NUM_PATHS_ROTATE_PATH_INC			3
#define APP_NUM_PATHS_ROTATE_RASTER_INC			3

#define APP_NUM_PATHS_SCALEDOWN_PATH			3

#define ERR_SUCCESS                             0
#define ERR_OUT_OF_MEM                          5
#define RASTER_IMAGE_WIDTH_MASK				0xf
#define MAX_RASTER_IMAGE_SIZE				0x80000 /* bytes */

#define CAPTURE_WINDOW			            40, 15, 700, 1250

typedef struct test_path {
	int32_t* pathData;
	unsigned		sizeInBytes;
	int32_t			xMin, yMin, xMax, yMax;
	vg_lite_matrix_t	m;
	vg_lite_color_t		colour;
	vg_lite_fill_t		fillMode;
} test_path_t;

typedef struct obj_transform {
	/**
	 * opcodes:
	 * 0 for blit external source
	 * 1 for blit object image
	 * 2 for draw object path
	 * 3 for draw object pattern
	 **/
	vg_lite_int32_t  opcode;
	vg_lite_point_t  pos;
	vg_lite_float_t  rotation;
	vg_lite_float_t  scaling;
	vg_lite_buffer_t* src;
	vg_lite_path_t* path;
	vg_lite_color_t  path_fill_color;
} obj_transform_t;

static int32_t path1_data[PATH1_DATA_SIZE] = {
	VLC_OP_MOVE,		490,	210,
	VLC_OP_LINE_REL,	170,	  0,
	VLC_OP_LINE_REL,	-85,	 85,
	VLC_OP_CLOSE
};

/* An hourglass shape */
static int32_t path2_data[PATH2_DATA_SIZE] = {
	VLC_OP_MOVE,		350,	500,
	VLC_OP_LINE_REL,	100,	  0,
	VLC_OP_LINE_REL,	-50,	100,
	VLC_OP_LINE_REL,	-50,	100,
	VLC_OP_LINE_REL,	100,	  0,
	VLC_OP_CLOSE
};

/* A chessboard-like shape */
static int32_t path3_data[PATH3_DATA_SIZE] = {
	VLC_OP_MOVE,		 390,    900,
	VLC_OP_LINE_REL,	   0,	 100,
	VLC_OP_LINE_REL,	 200,	   0,
	VLC_OP_LINE_REL,	   0,	  90,
	VLC_OP_LINE_REL,	-180,	   0,
	VLC_OP_LINE_REL,	   0,	 120,
	VLC_OP_LINE_REL,	  80,	   0,
	VLC_OP_LINE_REL,	   0,	-310,
	VLC_OP_CLOSE
};

/* A rectangular shape with a hole */
static int32_t path4_data[PATH4_DATA_SIZE] = {
	VLC_OP_MOVE,		200,	900,
	VLC_OP_LINE_REL,	 45,	  0,
	VLC_OP_LINE_REL,	  0,	 53,
	VLC_OP_LINE_REL,	-30,	  0,
	VLC_OP_LINE_REL,	  0,	-35,
	VLC_OP_LINE_REL,	 20,	  0,
	VLC_OP_LINE_REL,	  0,	 22,
	VLC_OP_LINE_REL,	-35,	  0,
	VLC_OP_CLOSE
};

/* A simple square */
static int32_t path5_data[PATH5_DATA_SIZE] = {
	VLC_OP_MOVE,		 100,	200,
	VLC_OP_LINE_REL,	 100,	  0,
	VLC_OP_LINE_REL,	   0,	100,
	VLC_OP_LINE_REL,	-100,	  0,
	VLC_OP_CLOSE
};

/* An "R" shaped figure */
static int32_t path6_data[PATH6_DATA_SIZE] = {
	VLC_OP_MOVE,		 310,	 100,
	VLC_OP_LINE_REL,	   0,	 100,
	VLC_OP_LINE_REL,	   0,	 100,
	VLC_OP_LINE_REL,	  15,	   0,
	VLC_OP_LINE_REL,	   0,	 -85,
	VLC_OP_LINE_REL,	  70,	  85,
	VLC_OP_LINE_REL,	  15,	 -15,
	VLC_OP_LINE_REL,	 -70,	 -85,
	VLC_OP_LINE_REL,	  70,	   0,
	VLC_OP_LINE_REL,	   0,	-100,
	VLC_OP_LINE_REL,	-100,	   0,
	VLC_OP_LINE_REL,	  15,	  15,
	VLC_OP_LINE_REL,	   0,	  70,
	VLC_OP_LINE_REL,	  70,	   0,
	VLC_OP_LINE_REL,	   0,	 -70,
	VLC_OP_LINE_REL,	 -70,	   0,
	VLC_OP_LINE_REL,	 -15,	  15,
	VLC_OP_CLOSE
};

/* A simple triangle */
static int32_t path7_data[PATH7_DATA_SIZE] = {
	VLC_OP_LINE_REL,	170,	 0,
	VLC_OP_LINE_REL,	-85,	85,
	VLC_OP_CLOSE
};

/* An hourglass shape */
static int32_t path8_data[PATH8_DATA_SIZE] = {
	VLC_OP_LINE_REL,	100,	  0,
	VLC_OP_LINE_REL,	-50,	100,
	VLC_OP_LINE_REL,	-50,	100,
	VLC_OP_LINE_REL,	100,	  0,
	VLC_OP_CLOSE
};

/* A chessboard-like shape */
static int32_t path9_data[PATH9_DATA_SIZE] = {
	VLC_OP_LINE_REL,	   0,	 100,
	VLC_OP_LINE_REL,	 200,	   0,
	VLC_OP_LINE_REL,	   0,	  90,
	VLC_OP_LINE_REL,	-180,	   0,
	VLC_OP_LINE_REL,	   0,	 120,
	VLC_OP_LINE_REL,	  80,	   0,
	VLC_OP_LINE_REL,	   0,	-310,
	VLC_OP_CLOSE
};

/* A rectangular shape with a hole */
static int32_t path10_data[PATH10_DATA_SIZE] = {
	VLC_OP_LINE_REL,	 45,	  0,
	VLC_OP_LINE_REL,	  0,	 53,
	VLC_OP_LINE_REL,	-30,	  0,
	VLC_OP_LINE_REL,	  0,	-35,
	VLC_OP_LINE_REL,	 20,	  0,
	VLC_OP_LINE_REL,	  0,	 22,
	VLC_OP_LINE_REL,	-35,	  0,
	VLC_OP_CLOSE
};

/* A simple square */
static int32_t path11_data[PATH11_DATA_SIZE] = {
	VLC_OP_LINE_REL,	 100,	  0,
	VLC_OP_LINE_REL,	   0,	100,
	VLC_OP_LINE_REL,	-100,	  0,
	VLC_OP_CLOSE
};

/* An "R" shaped figure */
static int32_t path12_data[PATH12_DATA_SIZE] = {
	VLC_OP_LINE_REL,	   0,	 100,
	VLC_OP_LINE_REL,	   0,	 100,
	VLC_OP_LINE_REL,	  15,	   0,
	VLC_OP_LINE_REL,	   0,	 -85,
	VLC_OP_LINE_REL,	  70,	  85,
	VLC_OP_LINE_REL,	  15,	 -15,
	VLC_OP_LINE_REL,	 -70,	 -85,
	VLC_OP_LINE_REL,	  70,	   0,
	VLC_OP_LINE_REL,	   0,	-100,
	VLC_OP_LINE_REL,	-100,	   0,
	VLC_OP_LINE_REL,	  15,	  15,
	VLC_OP_LINE_REL,	   0,	  70,
	VLC_OP_LINE_REL,	  70,	   0,
	VLC_OP_LINE_REL,	   0,	 -70,
	VLC_OP_LINE_REL,	 -70,	   0,
	VLC_OP_LINE_REL,	 -15,	  15,
	VLC_OP_CLOSE
};

obj_transform_t obj_xfrm[] = {
	{
		.opcode = 0,
		.pos = {
			.x = 0,
			.y = 0
		},
		.rotation = 0.0,
		.scaling = 1.0,
		.src = NULL
	},
	{
		.opcode = 0,
		.pos = {
			.x = 240,
			.y = 0
		},
		.rotation = 0.0,
		.scaling = 240.0f / 128.0f,
		.src = NULL
	},
	{
		.opcode = 0,
		.pos = {
			.x = 500,
			.y = 0
		},
		.rotation = 0.0,
		.scaling = 120.0f / 128.0f,
		.src = NULL
	},
	{
		.opcode = 0,
		.pos = {
			.x = 150,
			.y = 320
		},
		.rotation = 45.0,
		.scaling = 1.0,
		.src = NULL
	},
	{
		.opcode = 0,
		.pos = {
			.x = 390,
			.y = 320
		},
		.rotation = 45.0,
		.scaling = 181.0f / 128.0f,
		.src = NULL
	},
	{
		.opcode = 0,
		.pos = {
			.x = 630,
			.y = 320
		},
		.rotation = 45.0,
		.scaling = 90.5f / 128.0f,
		.src = NULL
	},
	{
		.opcode = 0,
		.pos = {
			.x = 150,
			.y = 640
		},
		.rotation = 22.5,
		.scaling = 1.0,
		.src = NULL
	},
	{
		.opcode = 0,
		.pos = {
			.x = 390,
			.y = 640
		},
		.rotation = 22.5,
		.scaling = 181.0f / 128.0f,
		.src = NULL
	},
	{
		.opcode = 0,
		.pos = {
			.x = 630,
			.y = 640
		},
		.rotation = 22.5,
		.scaling = 90.5f / 128.0f,
		.src = NULL
	},
	{
		.opcode = 0,
		.pos = {
			.x = 150,
			.y = 960
		},
		.rotation = 77.5,
		.scaling = 1.0,
		.src = NULL
	},
	{
		.opcode = 0,
		.pos = {
			.x = 390,
			.y = 960
		},
		.rotation = 77.5,
		.scaling = 181.0f / 128.0f,
		.src = NULL
	},
	{
		.opcode = 0,
		.pos = {
			.x = 630,
			.y = 960
		},
		.rotation = 77.5,
		.scaling = 90.5f / 128.0f,
		.src = NULL
	}
};

static test_path_t testPath[APP_NUM_PATHS] = {
	{ /* 0 - A simple triangle */
		.pathData = path1_data,
		.sizeInBytes = sizeof(path1_data),
		.xMin = 490,
		.yMin = 210,
		.xMax = 660,
		.yMax = 295,
		.colour = OPAQUE_VGLITE_COLOUR(/*red=*/0xff, /*green=*/0x33, /*blue=*/0),
		.fillMode = VG_LITE_FILL_EVEN_ODD,
	},
	{ /* 1 - An hourglass shape */
		.pathData = path2_data,
		.sizeInBytes = sizeof(path2_data),
		.xMin = 350,
		.yMin = 500,
		.xMax = 550,
		.yMax = 700,
		.colour = OPAQUE_VGLITE_COLOUR(/*red=*/0xff, /*green=*/0xff, /*blue=*/0),
		.fillMode = VG_LITE_FILL_EVEN_ODD,
	},
	{ /* 2 - A chessboard-like shape */
		.pathData = path3_data,
		.sizeInBytes = sizeof(path3_data),
		.xMin = 390,
		.yMin = 900,
		.xMax = 700,
		.yMax = 1210,
		.colour = OPAQUE_VGLITE_COLOUR(/*red=*/0xff, /*green=*/0x99, /*blue=*/0xff),
		.fillMode = VG_LITE_FILL_NON_ZERO,
	},
	{ /* 3 - A rectangular shape with a hole */
		.pathData = path4_data,
		.sizeInBytes = sizeof(path4_data),
		.xMin = 200,
		.yMin = 900,
		.xMax = 245,
		.yMax = 953,
		.colour = OPAQUE_VGLITE_COLOUR(/*red=*/0x66, /*green=*/0xff, /*blue=*/0xff),
		.fillMode = VG_LITE_FILL_NON_ZERO,
	},
		{ /* 4 - A simple square */
		.pathData = path5_data,
		.sizeInBytes = sizeof(path5_data),
		.xMin = 0,
		.yMin = 0,
		.xMax = 200,
		.yMax = 300,
		.colour = OPAQUE_VGLITE_COLOUR(/*red=*/0, /*green=*/0xff, /*blue=*/0),
		.fillMode = VG_LITE_FILL_EVEN_ODD,
	},
		{ /* 5 - An "R" shaped figure */
		.pathData = path6_data,
		.sizeInBytes = sizeof(path6_data),
		.xMin = 0,
		.yMin = 0,
		.xMax = 410,
		.yMax = 300,
		.colour = OPAQUE_VGLITE_COLOUR(/*red=*/0, /*green=*/0xff, /*blue=*/0),
		.fillMode = VG_LITE_FILL_EVEN_ODD,
	},
	{ /* 6 - A simple triangle */
		.pathData = path7_data,
		.sizeInBytes = sizeof(path7_data),
		.xMin = 0,
		.yMin = 0,
		.xMax = 170,
		.yMax = 85,
		.colour = OPAQUE_VGLITE_COLOUR(/*red=*/0xff, /*green=*/0x33, /*blue=*/0),
		.fillMode = VG_LITE_FILL_EVEN_ODD,
	},
	{ /* 7 - An hourglass shape */
		.pathData = path8_data,
		.sizeInBytes = sizeof(path8_data),
		.xMin = 0,
		.yMin = 0,
		.xMax = 100,
		.yMax = 200,
		.colour = OPAQUE_VGLITE_COLOUR(/*red=*/0xff, /*green=*/0xff, /*blue=*/0),
		.fillMode = VG_LITE_FILL_EVEN_ODD,
	},
	{ /* 8 - A chessboard-like shape */
		.pathData = path9_data,
		.sizeInBytes = sizeof(path9_data),
		.xMin = 0,
		.yMin = 0,
		.xMax = 200,
		.yMax = 310,
		.colour = OPAQUE_VGLITE_COLOUR(/*red=*/0xff, /*green=*/0x99, /*blue=*/0xff),
		.fillMode = VG_LITE_FILL_NON_ZERO,
	},
	{ /* 9 - A rectangular shape with a hole */
		.pathData = path10_data,
		.sizeInBytes = sizeof(path10_data),
		.xMin = 0,
		.yMin = 0,
		.xMax = 45,
		.yMax = 53,
		.colour = OPAQUE_VGLITE_COLOUR(/*red=*/0x66, /*green=*/0xff, /*blue=*/0xff),
		.fillMode = VG_LITE_FILL_NON_ZERO,
	},
		{ /* 10 - A simple square */
		.pathData = path11_data,
		.sizeInBytes = sizeof(path11_data),
		.xMin = 0,
		.yMin = 0,
		.xMax = 100,
		.yMax = 100,
		.colour = OPAQUE_VGLITE_COLOUR(/*red=*/0, /*green=*/0xff, /*blue=*/0),
		.fillMode = VG_LITE_FILL_EVEN_ODD,
	},
		{ /* 11 - An "R" shaped figure */
		.pathData = path12_data,
		.sizeInBytes = sizeof(path12_data),
		.xMin = 0,
		.yMin = 0,
		.xMax = 100,
		.yMax = 200,
		.colour = OPAQUE_VGLITE_COLOUR(/*red=*/0, /*green=*/0xff, /*blue=*/0),
		.fillMode = VG_LITE_FILL_EVEN_ODD,
	}
};

/* Ids of the test paths to use for the downscaling test */
static unsigned testPathDownscalePath[APP_NUM_PATHS_SCALEDOWN_PATH] = {
	6, 8, 10
};

/* Ids of the test paths to use for the incremental rotation test */
static unsigned testPathRotatePathInc[APP_NUM_PATHS_ROTATE_PATH_INC] = {
	6, 8, 9
};

static unsigned testPathRotateRasterInc[APP_NUM_PATHS_ROTATE_RASTER_INC] = {
	6, 8, 9
};

#endif /* __ERRORS_H__ */
