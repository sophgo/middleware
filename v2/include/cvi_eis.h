#ifndef CVI_EIS_H
#define CVI_EIS_H

#ifdef __cplusplus
extern "C" {
#endif


#define MAX_MESH_NUM (10000) /* Maximum number of mesh points for EIS warping */

typedef CVI_S32 MeshCoordArray[MAX_MESH_NUM][4]; /* Define a type for mesh coordinates */

typedef struct EIS_MOTION_DATA_S {  // EIS motion data structure
	CVI_U64 pts;   /* pts: in microseconds */
	CVI_DOUBLE x;  /* x: represented in rad/s aligned with camera frame */
	CVI_DOUBLE y;  /* y: represented in rad/s aligned with camera Frame */
	CVI_DOUBLE z;  /* z: represented in rad/s aligned with camera Frame */
} EIS_MOTION_DATA_S;

typedef struct EIS_MOTION_BUF_S {
	EIS_MOTION_DATA_S *pData;  /* Pointer to motion data array */
	CVI_U32 len;               /* Number of motion data samples in the buffer */
} EIS_MOTION_BUF_S;

typedef struct EIS_ALGO_CFG_S { // EIS algorithm configuration

	CVI_BOOL bypassModeEn;      /* Enable bypass mode (disable module) */

	CVI_U32 gyroODR;            /* Gyroscope output data rate (Hz) */
	CVI_DOUBLE lineReadoutTime; /* Line readout time (microseconds) */

	CVI_U32 stabThr;            /* Stabilization threshold */
	CVI_DOUBLE kalmanQ;         /* Kalman Q, increasing value for faster response */
	CVI_DOUBLE kalmanR;         /* Kalman R, increasing value for more smoothing */

	CVI_DOUBLE fx;              /* Camera focal length X (pixels) */
	CVI_DOUBLE fy;              /* Camera focal length Y (pixels) */

	CVI_DOUBLE cx;              /* Camera principal point X (pixels) */
	CVI_DOUBLE cy;              /* Camera principal point Y (pixels) */

	CVI_DOUBLE k1;              /* Radial distortion coefficient k1 */
	CVI_DOUBLE k2;              /* Radial distortion coefficient k2 */
	CVI_DOUBLE k3;              /* Radial distortion coefficient k3 */
	CVI_DOUBLE p1;              /* Tangential distortion coefficient p1 */
	CVI_DOUBLE p2;              /* Tangential distortion coefficient p2 */

} EIS_ALGO_CFG_S;

typedef struct EIS_CFG_S {  // Params that should be set according to the ISP pipeline

	SIZE_S inSize;                   /* Input frame size */
	SIZE_S meshSize;                 /* Output mesh grid size */

	EIS_ALGO_CFG_S algoCfg;          /* EIS algorithm configuration */

} EIS_CFG_S;

typedef struct EIS_INPUT_INFO_S {

	CVI_U64 pts;       /* in microseconds */
	CVI_U32 expTime;   /* in microseconds */
	CVI_VOID *pFrame;  /* pointer to input frame */

	EIS_MOTION_BUF_S GyroData;       /* Gyroscope data buffer */

} EIS_INPUT_INFO_S;

typedef struct EIS_OUTPUT_INFO_S {

	CVI_VOID *pFrame;            /* Input frame information */

	MeshCoordArray *pMeshX;      /* Pointer to X mesh coordinates (4 corners per mesh) */
	MeshCoordArray *pMeshY;      /* Pointer to Y mesh coordinates (4 corners per mesh) */
	CVI_U32 meshNum;             /* Number of mesh */

} EIS_OUTPUT_INFO_S;

typedef struct _EIS_HANDLE_S EIS_HANDLE_S;

CVI_S32 CVI_EIS_Init(EIS_CFG_S *pEisCfg, EIS_HANDLE_S **ppeis);

CVI_S32 CVI_EIS_DeInit(EIS_HANDLE_S **ppeis);

CVI_S32 CVI_EIS_Process(EIS_HANDLE_S *peis, EIS_INPUT_INFO_S *stInput, EIS_OUTPUT_INFO_S *stOutput);  // only output the frame mesh


#ifdef __cplusplus
}
#endif

#endif /* CVI_EIS_H */
