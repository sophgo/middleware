#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cvi_debug.h"
#include "grid_info.h"

#define USE_OLD 0

extern MESH_DATA_ALL_S g_MeshData[MESH_DATA_MAX_NUM];

static int match_meshdata(char *bindName)
{
	if (!bindName) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "bindName is NULL.\n");
		return -1;
	}

	for (int i = 0; i < MESH_DATA_MAX_NUM; i++) {
		if (!strcmp(g_MeshData[i].grid_name, bindName))
			return i;
	}

	return -1;
}

int get_free_meshdata(const char *bindName)
{
	if (!bindName) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "bindName is NULL.\n");
		return -1;
	}

	for (int i = 0; i < MESH_DATA_MAX_NUM; i++) {
		if (!strlen(g_MeshData[i].grid_name)) {
			strcpy(g_MeshData[i].grid_name, bindName);
			return i;
		}
	}

	return -1;
}

int load_meshdata(const char *path, MESH_DATA_ALL_S *pmeshdata, const char *bindName)
{
	int info[100] = {0};
	FILE *fpGrid;

	if (path == NULL) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "file [%s] null\n", path);
		return -1;
	}

	fpGrid = fopen(path, "rb");
	if (fpGrid == NULL) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "open file fail, %s\n", path);
		return -1;
	}

	//fread(&pmeshdata->mesh_horcnt, sizeof(int), 1, fpGrid);
	//fread(&pmeshdata->mesh_vercnt, sizeof(int), 1, fpGrid);
	//fread(&pmeshdata->num_pairs, sizeof(int), 1, fpGrid);
	//fread(&pmeshdata->imgw, sizeof(int), 1, fpGrid);
	//fread(&pmeshdata->imgh, sizeof(int), 1, fpGrid);
#if USE_OLD
	if (fread(info, sizeof(int), 20, fpGrid) != 20) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "read file fail, %s\n", path);
		return -1;
	}
#else
	if (fread(info, sizeof(int), 100, fpGrid) != 100) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "read file fail, %s\n", path);
		return -1;
	}
#endif

	CVI_TRACE_GDC(CVI_DBG_DEBUG, "head %d %d %d %d %d\n", info[0], info[1], info[2], info[3], info[4]);
	pmeshdata->mesh_horcnt = info[0]; // num of mesh in roi
	pmeshdata->mesh_vercnt = info[1]; // num of mesh in roi
	pmeshdata->num_pairs = info[2];
	pmeshdata->imgw = info[3];
	pmeshdata->imgh = info[4];
	//
	pmeshdata->mesh_w = info[5];   // unit: pixel
	pmeshdata->mesh_h = info[6];   // unit: pixel
	pmeshdata->unit_rx = info[7];  // unit: mesh_w
	pmeshdata->unit_ry = info[8];  // unit: mesh_h
	pmeshdata->_nbr_mesh_x = info[9];	// total meshes in horizontal
	pmeshdata->_nbr_mesh_y = info[10];	// total meshes in vertical
	memcpy(pmeshdata->corners, info + 11, sizeof(int) * 8);

	int _nbr_mesh_y = pmeshdata->mesh_vercnt; // for roi, not for whole image
	int _nbr_mesh_x = pmeshdata->mesh_horcnt;
	int count_grid = pmeshdata->num_pairs;
	pmeshdata->node_index = (_nbr_mesh_x + 1)*(_nbr_mesh_y + 1);

	strcpy(pmeshdata->grid_name, bindName);

	pmeshdata->pgrid_src = (int *)calloc(count_grid * 2, sizeof(int));
	pmeshdata->pgrid_dst = (int *)calloc(count_grid * 2, sizeof(int));
	pmeshdata->pmesh_src = (int *)calloc(count_grid * 8, sizeof(int));
	pmeshdata->pmesh_dst = (int *)calloc(count_grid * 8, sizeof(int));
	pmeshdata->pnode_src = (int *)calloc(pmeshdata->node_index*2, sizeof(int));
	pmeshdata->pnode_dst = (int *)calloc(pmeshdata->node_index*2, sizeof(int));

	CVI_TRACE_GDC(CVI_DBG_DEBUG, "mesh_horcnt,mesh_vercnt,_nbr_mesh_x, _nbr_mesh_y, count_grid, num_nodes: %d %d %d %d %d %d \n"
		, pmeshdata->mesh_horcnt, pmeshdata->mesh_vercnt, _nbr_mesh_x, _nbr_mesh_y, count_grid, pmeshdata->node_index);
	CVI_TRACE_GDC(CVI_DBG_DEBUG, "imgw, imgh, mesh_w, mesh_h ,unit_rx,unit_ry: %d %d %d %d %d %d \n"
		, pmeshdata->imgw, pmeshdata->imgh, pmeshdata->mesh_w, pmeshdata->mesh_h, pmeshdata->unit_rx, pmeshdata->unit_ry);

	if (fread(pmeshdata->pgrid_src, sizeof(int), (count_grid * 2), fpGrid) != (size_t)(count_grid * 2)) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "read file fail, %s\n", path);
		return -1;
	}

	if (fread(pmeshdata->pgrid_dst, sizeof(int), (count_grid * 2), fpGrid) != (size_t)(count_grid * 2)) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "read file fail, %s\n", path);
		return -1;
	}
	// hw mesh
	if (fread(pmeshdata->pmesh_src, sizeof(int), (count_grid * 2 * 4), fpGrid) != (size_t)(count_grid * 2 * 4)) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "read file fail, %s\n", path);
		return -1;
	}
	if (fread(pmeshdata->pmesh_dst, sizeof(int), (count_grid * 2 * 4), fpGrid) != (size_t)(count_grid * 2 * 4)) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "read file fail, %s\n", path);
		return -1;
	}
	// hw node
	if (fread(pmeshdata->pnode_src, sizeof(int), (pmeshdata->node_index * 2), fpGrid) != (size_t)(pmeshdata->node_index * 2)) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "read file fail, %s\n", path);
		return -1;
	}
	if (fread(pmeshdata->pnode_dst, sizeof(int), (pmeshdata->node_index * 2), fpGrid) != (size_t)(pmeshdata->node_index * 2)) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "read file fail, %s\n", path);
		return -1;
	}

	fclose(fpGrid);

	pmeshdata->balloc = true;
	CVI_TRACE_GDC(CVI_DBG_DEBUG, "read succ\n");

	return 0;
}

int free_cur_meshdata(char *bindName)
{
	int grid_idx;

	grid_idx = match_meshdata(bindName);
	if (grid_idx < 0 || grid_idx >= MESH_DATA_MAX_NUM)
		return -1;

	SAFE_FREE_POINTER(g_MeshData[grid_idx].pgrid_src);
	SAFE_FREE_POINTER(g_MeshData[grid_idx].pgrid_dst);
	SAFE_FREE_POINTER(g_MeshData[grid_idx].pmesh_src);
	SAFE_FREE_POINTER(g_MeshData[grid_idx].pmesh_dst);
	SAFE_FREE_POINTER(g_MeshData[grid_idx].pnode_src);
	SAFE_FREE_POINTER(g_MeshData[grid_idx].pnode_dst);
	//SAFE_FREE_POINTER(pMeshData->_pmapx);
	//SAFE_FREE_POINTER(pMeshData->_pmapy);
	g_MeshData[grid_idx].balloc = false;
	g_MeshData[grid_idx]._bhomo = false;

	return 0;
}

int free_meshdata(MESH_DATA_ALL_S *pmeshdata)
{
	SAFE_FREE_POINTER(pmeshdata->pgrid_src);
	SAFE_FREE_POINTER(pmeshdata->pgrid_dst);
	SAFE_FREE_POINTER(pmeshdata->pmesh_src);
	SAFE_FREE_POINTER(pmeshdata->pmesh_dst);
	SAFE_FREE_POINTER(pmeshdata->pnode_src);
	SAFE_FREE_POINTER(pmeshdata->pnode_dst);

	pmeshdata->balloc = false;
	pmeshdata->_bhomo = false;

	return 0;
}

int free_all_meshdata(void)
{
	for (int i = 0; i < MESH_DATA_MAX_NUM; i++) {
		if(strlen(g_MeshData[i].grid_name) && g_MeshData[i].balloc) {
			SAFE_FREE_POINTER(g_MeshData[i].pgrid_src);
			SAFE_FREE_POINTER(g_MeshData[i].pgrid_dst);
			SAFE_FREE_POINTER(g_MeshData[i].pmesh_src);
			SAFE_FREE_POINTER(g_MeshData[i].pmesh_dst);
			SAFE_FREE_POINTER(g_MeshData[i].pnode_src);
			SAFE_FREE_POINTER(g_MeshData[i].pnode_dst);
			//SAFE_FREE_POINTER(pMeshData->_pmapx);
			//SAFE_FREE_POINTER(pMeshData->_pmapy);
			g_MeshData[i].balloc = false;
		}
	}
	return 0;
}

int save_meshdata(char *path, MESH_DATA_ALL_S *pstMeshData)
{
	if (!path) {
		CVI_TRACE_GDC(CVI_DBG_DEBUG, "path [%s]\n", path);
		return -1;
	}

	pstMeshData->_nbr_mesh_x = pstMeshData->imgw / pstMeshData->mesh_w;
	pstMeshData->_nbr_mesh_y = pstMeshData->imgh / pstMeshData->mesh_h;
	//CVI_TRACE_GDC(CVI_DBG_DEBUG, "%d %d %d %d\n", pstMeshData->imgw, pstMeshData->mesh_w, pstMeshData->imgh, pstMeshData->mesh_h );
	//CVI_TRACE_GDC(CVI_DBG_DEBUG, "%d %d\n", pstMeshData->_nbr_mesh_x, pstMeshData->_nbr_mesh_y);
	char szfilename[256];
	sprintf(szfilename, "%s/grid_info_%d_%d_%d_%d_%d_%dx%d.dat", path,
		pstMeshData->mesh_horcnt, pstMeshData->mesh_vercnt,
		pstMeshData->num_pairs, pstMeshData->_nbr_mesh_x, pstMeshData->_nbr_mesh_y,
		pstMeshData->imgw, pstMeshData->imgh);
	CVI_TRACE_GDC(CVI_DBG_DEBUG, "%s\n", szfilename);

	int count_grid = pstMeshData->num_pairs;
	int node_index = pstMeshData->node_index;
	//CVI_TRACE_GDC(CVI_DBG_DEBUG, "%d %d\n", pstMeshData->unit_rx, pstMeshData->unit_rx);
	int info[100] = {0};
	memset(info, 0, sizeof(info));

	info[0] = pstMeshData->mesh_horcnt; // in roi
	info[1] = pstMeshData->mesh_vercnt; // in roi
	info[2] = pstMeshData->num_pairs;
	info[3] = pstMeshData->imgw;		// output width
	info[4] = pstMeshData->imgh;		// output height
	info[5] = pstMeshData->mesh_w;   // unit: pixel
	info[6] = pstMeshData->mesh_h;   // unit: pixel
	info[7] = pstMeshData->unit_rx;  // unit: mesh_w
	info[8] = pstMeshData->unit_ry;  // unit: mesh_h
	info[9] = pstMeshData->_nbr_mesh_x;
	info[10] = pstMeshData->_nbr_mesh_y;
	// corner positions
	int roiw = pstMeshData->mesh_horcnt*info[5];//pstMeshData->mesh_horcnt-1
	int roih = pstMeshData->mesh_vercnt*info[6];//pstMeshData->mesh_vercnt-1
	//int roiw = pstMeshData->mesh_horcnt*info[5];
	//int roih = pstMeshData->mesh_vercnt*info[6];
	int px = pstMeshData->unit_rx*info[5];
	int py = pstMeshData->unit_ry*info[6];

	info[11] = px;
	info[12] = py;
	info[13] = px + roiw;
	info[14] = py;
	info[15] = px + roiw;
	info[16] = py + roih;
	info[17] = px;
	info[18] = py + roih;

	FILE *fpGrid = fopen(szfilename, "wb");
	//fwrite(&pstMeshData->mesh_horcnt, sizeof(int), 1, fpGrid);
	//fwrite(&pstMeshData->mesh_vercnt, sizeof(int), 1, fpGrid);
	//fwrite(&pstMeshData->num_pairs, sizeof(int), 1, fpGrid);
	//fwrite(&pstMeshData->imgw, sizeof(int), 1, fpGrid);
	//fwrite(&pstMeshData->imgh, sizeof(int), 1, fpGrid);
#if USE_OLD
	fwrite(info, sizeof(int), 20, fpGrid);
#else
	fwrite(info, sizeof(int), 100, fpGrid);
#endif
	fwrite(pstMeshData->pgrid_src, sizeof(int), (count_grid * 2), fpGrid);
	fwrite(pstMeshData->pgrid_dst, sizeof(int), (count_grid * 2), fpGrid);
	// hw mesh
	fwrite(pstMeshData->pmesh_src, sizeof(int), (count_grid * 2 * 4), fpGrid);
	fwrite(pstMeshData->pmesh_dst, sizeof(int), (count_grid * 2 * 4), fpGrid);
	// hw node
	fwrite(pstMeshData->pnode_src, sizeof(int), (node_index * 2), fpGrid);
	fwrite(pstMeshData->pnode_dst, sizeof(int), (node_index * 2), fpGrid);

	fclose(fpGrid);
	return 0;
}
