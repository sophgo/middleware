# Sample_venc User Guide

## Sample_venc 使用方法

sample_venc -c codec -w width -h height -i src.yuv -o enc

EX.

```sh
sample_venc -c 265 -w 1920 -h 1080 -i ReadySteadyGo_1920x1080_600.yuv -o enc

sample_venc --numChn=1 --chn=0 -c 264 --getBsMode=1 --statTime=2 --gop=50 --srcFramerate=25 --framerate=25 \
--initQp=32 --minIqp=28 --maxIqp=46 --minQp=28 --maxQp=46 --ipQpDelta=0 -w 720 -h 480 -i input_yuv -o output_stream \
--rcMode=0 -n 100 --bitrate=10000 --initialDelay=100 --thrdLv=4 --maxIprop=10
```

## 参数及其使用说明

### 短参数

短参数是长参数的别名，短参数与参数值之间使用**空格**进行分隔

|参数|描述|
|-----------------|---------------------------------------------------------|
| -c              | 265 = h.265, jpg = jpeg, mjp = motion jpeg              |
| -w              | yuv width                                               |
| -h              | yuv height                                              |
| -i              | input yuv file                                          |
| -o              | output bitstream file                                   |
| -n              | number of frame to be encoded                           |

### 长参数

长参数与参数值之间使用**等号**进行连接

|参数|描述|
|--------------------|---------------------------------------------------------|
| --getBsMode        | get-bitstream mode, 0 = loop query, 1 = poll-select     |
| --profile          | 0 = h264 baseline, 1 = h264 main, 2 = h264 high, Default = 2 |
| --rcMode           | rate control mode, 0 = CBR, 1 = VBR, 2 = AVBR, 4 = FIXQP, 6 = UBR (User BR), default = 4 |
| --iqp              | I frame QP                                              |
| --pqp              | P frame QP                                              |
| --ipQpDelta        | QP Delta between P frame and I frame                    |
| --bgQpDelta        | Smart-P QP delta between P frame and BG (background) frame. [-10, 30], default = 0  |
| --viQpDelta        | Smart-P QP delta between P frame and VI (virtual I)  frame. [-10, 30], default = 0  |
| --gop              | The period of one gop                                   |
| --gopMode          | GOP mode. 0: Normal P, 2: Smart P, Default: 0           |
| --bitrate          | The average target bitrate (kbits)                      |
| --initQp           | The Start Qp of 1st frame, 63 = default                 |
| --minQp            | Minimum Qp for one frame                                |
| --maxQp            | Maximum Qp for one frame                                |
| --minIqp           | Minimum Qp for I frame                                  |
| --maxIqp           | Maximum Qp for I frame                                  |
| --srcFramerate     | source frame rate                                       |
| --framerate        | destination frame rate                                  |
| --vfps             | whether to enable variable FPS                          |
| --quality          | jpeg encode quality factor                              |
| --maxbitrate       | Maximum output bit rate (kbits)                         |
| --changePos        | Ratio to change Qp                                      |
| --minStillPercent  | Percentage of target bitrate in low motion              |
| --maxStillQp       | Maximum Qp in low motion                                |
| --motionSense      | Motion sensitivity                                      |
| --avbrPureStillThr | AVBR still pic threshold                                |
| --bgEnhanceEn      | whether to enable background enhancement                |
| --statTime         | statistics time in seconds                              |
| --testMode         | samele_venc test mode                                   |
| --getstream-timeout | getstream-timeout  -1:block mode, 0:try_once, >0 timeout in ms |
| --sendframe-timeout | sendframe-timeout  -1:block mode, 0:try_once, >0 timeout in ms |
| --vbMode           | VB pool mode. 0 = common, 1 = module, 2 = private, 3 = user     |
| --bindmode         | 0: BIND_DISABLE, 1: BIND_VI, 2:BIND_VPSS, 3: BIND_DPU, 4: BIND_STITCH |
| --pixel_format     | 0: 420 planar, 1: 422 planar, 2: NV12, 3: NV21          |
| --posX             | x axis of start position, need to be multiple of 16 (used for crop) |
| --posY             | y axis of start position, need to be multiple of 16 (used for crop) |
| --inWidth          | width of input frame (used for crop)                    |
| --inHeight         | height of input frame (used for crop)                   |
| --bufSize          | bitstream Buffer size                                   |
| --forceIdr         | 0: disable, > 0: set force idr at number of frame       |
| --chgNum           | frame num to change venc attr                           |
| --chgBitrate       | change bitrate (kbits)                                  |
| --chgFramerate     | change dstframerate                                     |
| --tempLayer        | scalable video coding(SVC) layer number                 |
| --roiCfgFile       | ROI configuration file                                  |
| --jpegQTableCfgFile | jpeg/mjpeg Quality Table config file                   |
| --numChn           | number of channels to encode                            |
| --chn              | channel-id to configure parameters                      |
| --user_data1       | user data binary file 1                                 |
| --user_data2       | user data binary file 2                                 |
| --user_data3       | user data binary file 3                                 |
| --user_data4       | user data binary file 4                                 |
| --h265RefreshType  | 0: IDR, 1: CRA, default = 0                             |
| --initialDelay     | rc initial delay in ms, default = 1000                  |
| --jpegMarkerOrder  | 0: Cvitek, 1: SOI-JFIF-DQT_MERGE-SOF0-DHT_MERGE-DRI, 2: Cvitek w/ JFIF, default = 0 |
| --intraCost        | intraCost, the extra cost of intra mode                 |
| --thrdLv           | threshold to control block qp                           |
| --h264EntropyMode  | 0: CAVLC, 1: CABAC, default = 1                         |
| --h264ChromaQpOffset | H264 Chroma QP offset [-12, 12], default = 0          |
| --h265CbQpOffset   | H265 Cb QP offset [-12, 12], default = 0                |
| --h265CrQpOffset   | H265 Cr QP offset [-12, 12], default = 0                |
| --maxIprop         | max I frame bitrate ratio to P frame, default = 100     |
| --rowQpDelta       | rowQpDelta [0, 10], default = 1                         |
| --superFrmMode     | superFrmMode, 0 = disable, 3 = encode to IDR, default = 0 |
| --superIBitsThr    | superIBitsThreshold [1000, 33554432], default = 4000000 |
| --superPBitsThr    | superPBitsThreshold [1000, 33554432], default = 4000000 |
| --maxReEnc         | max number of re-encode times, [0, 3], default = 0      |
| --aspectRatioInfoPresentFlag   | vui_parameters::aspect_ratio_info_present_flag, [0, 1],     default = 0 |
| --aspectRatioIdc               | vui_parameters::aspect_ratio_idc                [0, 255],   default = 1 |
| --overscanInfoPresentFlag      | vui_parameters::overscan_info_present_flag      [0, 1],     default = 0 |
| --overscanAppropriateFlag      | vui_parameters::overscan_appropriate_flag       [0, 1],     default = 0 |
| --sarWidth                     | vui_parameters::sar_width                       [0, 65535], default = 1 |
| --sarHeight                    | vui_parameters::sar_height                      [0, 65535], default = 1 |
| --timingInfoPresentFlag        | vui_parameters::timing_info_present_flag        [0, 1],     default = 0 |
| --fixedFrameRateFlag           | vui_parameters::fixed_frame_rate_flag           [0, 1],     default = 0 |
| --numUnitsInTick               | vui_parameters::num_units_in_tick,                          default = 1 |
| --timeScale                    | vui_parameters::time_scale,                                 default = 60 |
| --videoSignalTypePresentFlag   | vui_parameters::video_signal_type_present_flag, [0, 1],     default = 0 |
| --videoFormat                  | vui_parameters::video_format                    [0, 7],     default = 5 |
| --videoFullRangeFlag           | vui_parameters::video_full_range_flag           [0, 1],     default = 0 |
| --colourDescriptionPresentFlag | vui_parameters::colour_description_present_flag [0, 1],     default = 0 |
| --colourPrimaries              | vui_parameters::colour_primaries                [0, 255],   default = 2 |
| --transferCharacteristics      | vui_parameters::transfer_characteristics        [0, 255],   default = 2 |
| --matrixCoefficients           | vui_parameters::matrix_coefficients             [0, 255],   default = 2 |
| --testUbrEn        | whether to enable to test ubr [0, 1], default = 0       |
| --frameQp          | frameQp [0, 51], default = 38                           |
| --esBufQueueEn     | whether to use es buffer queue, default = 1             |
| --isoSendFrmEn     | whether to isolate SendFrame/GetStream pairing, default = 1 |
| --sliceSplitCnt    | sliceSplitCnt [1, 5], default = 1                       |
| --disabledblk      | disable deblock filter  [0, 1], default = 0             |
| --betaOffset       | slice_beta_offset  [-6, 6], default = 0                 |
| --alphaoffset      | slice_alpha_offset [-6, 6], default = 0                 |
| --goppreset        | gop preset for h264/h265 [0, 9], default = 6            |
| --sao              | sample adaptive offset (SAO) enable flag [0, 1], default = 1 |
| --rotate           | rotation angel option   [0, 3], default = 0             |
| --mirr             | mirror direction option [0, 3], default = 0             |
| --cmdqueue         | cmdqueue depth [1, 16], default = 0                     |
| --setPredUnit      | h26x pred unit set enable [0, 1], default = 0           |
| --intraPredFlag    | h26x intraPredFlag  [0, 1], default = 0                 |
| --smoothingEnableFlag | h265 strong_intra_smoothing_enabled_flag  [0, 1], default = 0  |
| --disableIDRCnt    | when send frame count equal disableIDRCnt, disable IDR, default = 0 |
| --enableIDRCnt     | when send frame count equal enableIDRCnt,  enable IDR, default = 0  |
| --SearchVer        | set vertical search window, default = 0                 |
| --SearchHor        | set horizontal search window, default = 0               |
| --useExternbuf     | use extern physical buffer [0, 1], default = 0          |
