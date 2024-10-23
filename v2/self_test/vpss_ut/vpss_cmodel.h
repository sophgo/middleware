#ifndef __VPSS_CMODEL_H__
#define __VPSS_CMODEL_H__


void vpss_csc_rgb2yuv(CVI_U8 rgbData[3], CVI_U8 yuvData[3]);
void vpss_csc_yuv2rgb(CVI_U8 yuvData[3], CVI_U8 rgbData[3]);

#endif
