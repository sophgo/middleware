运行步骤：

1.加载fb驱动：
insmod /mnt/system/ko/cfbcopyarea.ko
insmod /mnt/system/ko/cfbfillrect.ko
insmod /mnt/system/ko/cfbimgblt.ko
insmod /mnt/system/ko/soph_fb.ko option = 0x2 fb1_mem_size=16588800
注意：fb驱动可以传入参数：option，fb0_mem_size， fb1_mem_size。
option: to control fb options
 * - bit[0]: if true, fb0 double buffer
 * - bit[1]: if true, fb1 double buffer
 * - bit[2]: if true, fb0 on vpss not vo
 * - bit[3]: if true, fb1 on vpss not vo
fb0_mem_size：设备0 显存buffer大小
fb1_mem_size：设备1 显存buffer大小
特别注意显存buffer大小要设置正确，否则应用ioctl会返回失败，计算公式如下：
mem_size = displayWidth * displayHeight * bpp * BufferMode

bpp：单个像素字节数，ARGB8888 格式为4，ARGB1555格式为2.
BufferMode: 单buffer为1， 双buffer为2.
例如：1920*1080 分辨率，ARGB8888 格式的 double buffer 模式下需要的内存：
mem_size = 1920*1080*4*2 = 16588800


2. 拷贝sample_fb和res资源文件到SD卡。
3. 运行：./sample_fb <index> <VoDev> <IntfType> <IntfSync>
    index:
		0: ARGB8888 ONE BUF mode
		1: ARGB1555 DOUBLE BUF mode
		2: ARGB8888 fb + TDE
	VoDev: 设备号，0 or 1
	IntfType:
		0: MIPI
		1: LVDS
		2: HDMI
		3: BT656
		4: BT1120
	IntfSync:
		0: 720x1280@60
		1: 1280x720@60
		2: 1920x1080@30
		3: 1920x1080@60
		4: 3840x2160@30
		5: 3840x2160@60
		6: 4096x2160@30
		7: 4096x2160@60

	注意：
	1. 只有option配置double buffer，才能运行BUF_DOUBLE mode和fb + TDE。
	2. 只有VoDev=1，才支持HDMI，当使用HDMI时，请先连接好HDMI设备后运行sample。
	例如：./sample_fb 0 1 2 3
