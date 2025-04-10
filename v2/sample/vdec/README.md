# Sample_vdec User Guide

## Sample_vdec 使用方法

sample_vdec --numChn=num-all-channels --chn=currChnId -c codec -i bitstream -o out.yuv

EX.

```sh
sample_vdec --numChn=1 --chn=0 -c 264 -i in.264 -o out.yuv

sample_vdec --numChn=2 --chn=0 -c 264 -i in0.264 -o out0.yuv --chn=1 -c 264 -i in1.264 -o out1.yuv
```

## 参数及其使用说明

### 短参数

短参数是长参数的别名，短参数与参数值之间使用**空格**进行分隔

|参数|描述|
|-----------------|---------------------------------------------------------|
| -c              | 264 = h.264, 265 = h.265, jpg = jpeg, mjp = motion jpeg |
| -i              | input bitstream file path                               |
| -o              | output yuv file path                                    |
| -d              | whether dump yuv, 0: don't dump, calc md5; 1: dump yuv  |
| -h              | print usage info                                        |

### 长参数

长参数与参数值之间使用**等号**进行连接

|参数|描述|
|--------------------|---------------------------------------------------------|
| --numChn             | number of channels to decode                            |
| --chn                | set channel-id to configure the following parameters    |
| --bufwidth           | set max width for alloc frame buffer                    |
| --bufheight          | set max height for alloc frame buffer                   |
| --maxframe           | set max frame buffer number                             |
| --vbMode             | vb mode for alloc frame buffer, 0: COMMON, 3: USER      |
| --bitStreamFolder    | bitstream files folder                                  |
| --getframe-timeout   | samele_vdec getframe-timeout    -1:block mode, 0:try_once, >0 timeout in ms  |
| --sendstream-timeout | samele_vdec sendstream-timeout  -1:block mode, 0:try_once, >0 timeout in ms  |
| --bindmode           | bind mode. 0 = VDEC_BIND_DISABLE, 1 = VDEC_BIND_VPSS_VO, 2 = VDEC_BIND_VPSS_VENC |
| --venc_num_frames    | encode frame number when vd_vpss_ve bind mode           |
| --venc_rc_mode       | venc RC strategy when vd_vpss_ve bind mode              |
| --pixel_format       | output pixel format 0: do not specify, 1: NV12, 2: NV21 |
| --circle_send        | loop read input bitstream to decode 0: no loop, 1: loop |
