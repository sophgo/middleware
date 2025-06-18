#ifndef __GRID_INFO_H__
#define __GRID_INFO_H__

#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
#if __cplusplus
	extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define SLICE_W_CNT_MAX (8)
#define SLICE_H_CNT_MAX (8)
#define SLICE_MAGIC (168)

enum grid_info_mode {
	GRID_MODE_REGION_BASE = 0,
	GRID_MODE_MESH_BASE,
	GRID_MODE_MAX,
};
typedef struct _SLICE_INFO_S {
	int magic;
	int slice_h_cnt;
	int slice_v_cnt;
	int cache_hit_cnt;
	int cache_miss_cnt;
	int cache_req_cnt;
} SLICE_INFO_S;

typedef struct _MESH_DATA_ALL_S {
	char grid_name[64];
	bool balloc;
	int num_pairs, imgw, imgh, node_index;
	int *pgrid_src, *pgrid_dst;
	int *pmesh_src, *pmesh_dst;
	int *pnode_src, *pnode_dst;
	int mesh_w; 		// unit: pixel
	int mesh_h; 		// unit: pixel
	int mesh_horcnt;	// unit: mesh_w
	int mesh_vercnt;	// unit: mesh_h
	int unit_rx;		// unit: mesh_w
	int unit_ry;		// unit: mesh_h
	//int unit_ex;		// = rx + mesh_horcnt - 1
	//int unit_ey;		// = ry + mesh_vercnt - 1
	int _nbr_mesh_x, _nbr_mesh_y;
	bool _bhomo;
	float _homography[10];
	int corners[10];
	enum grid_info_mode grid_mode;
	SLICE_INFO_S slice_info;
	float *_pmapx, *_pmapy;
} MESH_DATA_ALL_S;

typedef struct _MESH_DATA_EIS_S {
	bool enable;
	uint64_t phy_addr;//mesh tbl addr
	void *vir_addr;
	uint32_t slice_start_addr[SLICE_W_CNT_MAX * SLICE_H_CNT_MAX];//id list head pos idx for per slice
	uint32_t slice_tbl_addr[SLICE_W_CNT_MAX * SLICE_H_CNT_MAX];  //tbl list head pos idx for per slice
	int *slice_tbl_lut;//id list pos idx for per mesh
	int *slice_val_pos;//tbl list pos idx for per mesh
	int nbr_mesh;//num of mesh
	int buf_length;//max mesh point * sizeof(int)
	int reorder_mesh_id_list_entry_num, reorder_mesh_tbl_entry_num;//id list idx num, tbl list idx num
	int *reorder_mesh_id_list;
	int *reorder_mesh_tbl[SLICE_W_CNT_MAX * SLICE_H_CNT_MAX];
} MESH_DATA_EIS_S;

typedef MESH_DATA_ALL_S meshdata_all;
typedef MESH_DATA_EIS_S meshdata_eis;

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#define SAFE_FREE_POINTER(ptr)	\
	do {					\
		if (ptr != NULL) {	\
			free(ptr); 		\
			ptr = NULL; 	\
		} 					\
	} while (0)

#define MESH_DATA_MAX_NUM 8

int match_meshdata(char *bindName);
int get_free_meshdata(const char *bindName);
int load_meshdata(const char *path, MESH_DATA_ALL_S *pmeshdata, const char *bindName, void *pBuf, int Len);
int free_cur_meshdata(char *bindName);
int free_all_meshdata(void);
int save_meshdata(char *path, MESH_DATA_ALL_S *pstMeshData);
int free_meshdata(MESH_DATA_ALL_S *pmeshdata);

#ifdef __cplusplus
#if __cplusplus
	}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __GRID_INFO_H__ */
