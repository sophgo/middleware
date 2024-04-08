# 说明

IPC_Turnkey 为IPCamera产品应用开发demo。

目录说明:
1. app - 主应用程序(编译生成sample 可执行档)
2. rtsp - rtsp库和头文件


# 依赖

应用程序的编译依赖SDK的header 和library.  需要先编译middleware。

# 硬件平台

目前应用的code 可以跑在1838EVB平台上面.

需要接4K sensor，不需要屏幕。

# 如何编译

修改Makefile.param，指定middleware目录MW_DIR。
（如果下载文件IPC_Turnkey、middleware处于同一目录下可以不用修改）

直接make即可生成可执行程序sample_ipc。

STATIC=0使用动态库，STATIC=1使用静态库。默认使用静态库。

# 如何运行
拷贝sample_ipc到板端，如果是链接动态库还需要拷贝库。
配置好网络。
运行:  ./sample_ipc

使用VLC打开rtsp地址：
rtsp://XXX.XXX.XXX.XXX:8554/live0		#4K
rtsp://XXX.XXX.XXX.XXX:8554/live1		#1080P
rtsp://XXX.XXX.XXX.XXX:8554/live2		#720x576
rtsp://XXX.XXX.XXX.XXX:8554/live3		#720x576
rtsp://XXX.XXX.XXX.XXX:8554/live4		#720x576



