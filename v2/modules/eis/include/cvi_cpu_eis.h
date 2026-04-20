#ifndef CVI_CPU_EIS_H
#define CVI_CPU_EIS_H

#include <cvi_type.h>
#include <cvi_sys.h>
#include <sys/stat.h>

#include <cvi_eis.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

int cpu_eis_init(EIS_CFG_S *pcfg, CVI_VOID **phdl);
int cpu_eis_free(CVI_VOID **phdl);
int cpu_eis(CVI_VOID *phdl, EIS_MOTION_BUF_S *pGyroData, CVI_U64 framePts, CVI_U32 exposureTime, int MeshX[][4], int MeshY[][4], CVI_U32 *pMeshNum);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */


#endif