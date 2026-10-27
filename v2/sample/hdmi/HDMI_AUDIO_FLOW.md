# HDMI 音频：第一次播音 vs 后续 SendAudioFrame 流程对比

## 一、应用层：第一次能播音的流程（初始化 + Start）

### 1. 应用调用的 API 顺序

| 顺序 | 应用 API | 作用 |
|------|----------|------|
| 1 | `CVI_SYS_Init()` | 系统初始化 |
| 2 | `AUDIO_MAP(&setAttr, filename, &u64PhyAddr, &pVirAddr, strName)` | 读音频文件、Ion 分配、**写入 setAttr.audio_start_paddr / audio_stop_paddr** |
| 3 | `CVI_HDMI_ForceGetEdid()` | 读 EDID |
| 4 | `CVI_HDMI_GetSinkCapability()` | 读 Sink 能力 |
| 5 | **填充 setAttr**（见下表） | 包含视频、音频、采样率等 |
| 6 | `CVI_HDMI_Init()` | 驱动 Init |
| 7 | **`CVI_HDMI_SetAttr(&setAttr)`** | **把整块 attr 交给驱动（含 audio 起止地址和音频参数）** |
| 8 | （可选）`CVI_HDMI_SetInfoFrame(&infoframe)` | 设置 AVI/Audio 等 InfoFrame |
| 9 | **`CVI_HDMI_Start()`** | **启动 TX；驱动内部会做 api_configure → audio_configure → audio_ahbdma，第一次 DMA 在这里启动** |
| 10 | `CVI_HDMI_GetAttr()` | 读回属性 |
| 11 | （可选）SetAvmute / SetAudioMute | 静音控制 |
| 12 | `CVI_HDMI_RegisterCallback()` | 事件回调 |

### 2. 应用在 SetAttr 前写入的 setAttr 字段（与音频相关）

```c
// sample_hdmi.c 中填充
setAttr.hdmi_en = true;
setAttr.audio_en = audio_en;                           // 1 表示开音频
setAttr.bit_depth = CVI_HDMI_BIT_DEPTH_24;            // 24bit → 驱动 paudio.msample_size
setAttr.sample_rate = CVI_HDMI_SAMPLE_RATE_44K;       // 44.1k → 驱动 paudio.msampling_frequency
setAttr.audio_start_paddr = ...;                       // AUDIO_MAP 里填的起始物理地址
setAttr.audio_stop_paddr = ...;                        // AUDIO_MAP 里填的结束物理地址（start + file_size）
// 以及 video_format, pix_clk, deep_color_mode 等
```

### 3. 驱动侧：第一次播音走的路径

- **SetAttr** → `hdmitx_set_attr()`：  
  把 `attr` 写入 `ctx->mode.paudio`：  
  `start_addr`, `stop_addr`, `msample_size`(bit_depth), `msampling_frequency`(sample_rate), `audio_en`。

- **Start** → `hdmitx_start()` → `api_configure(..., &ctx->mode.paudio, ...)` → **`audio_configure(dev, audio)`**：
  - `audio_mute(dev, 1)`
  - `fc_audio_config(dev, audio)`（Frame Composer 音频）
  - 计算 N/CTS，写 `_audio_clock_cts`, `_audio_clock_n`
  - `audio_mute(dev, 0)`
  - `fc_audio_info_config(dev, audio)`
  - **`audio_ahbdma(dev, audio)`** ← 第一次 DMA 在这里启动，此时 **IH_AHBDMAAUD_STAT0 从未被置位，写 0 也不会误触发**

所以：第一次能播音，是因为 **SetAttr + Start** 这条路径里，**完整走了 audio_configure（静音、FC、N/CTS、unmute）再 audio_ahbdma**，且之前没有产生过 DMA 完成中断。

---

## 二、应用层：后续送帧（SendAudioFrame）流程

### 1. 应用调用的 API

| 顺序 | 应用 API | 作用 |
|------|----------|------|
| 1 | `CVI_HDMI_SendAudioFrame(&frame)` | 只传 **一帧** 的描述，不传整块 attr |

### 2. 应用填充的 frame 内容

```c
frame.u64PhyAddr = g_audio_phy;   // 与 setAttr.audio_start_paddr 相同
frame.u32Len     = g_audio_len;   // setAttr.audio_stop_paddr - setAttr.audio_start_paddr
frame.u64VbBlk   = 0;
frame.u32Seq     = seq++;
```

应用 **没有** 再调 SetAttr / SetInfoFrame / 任何配置 API，只调了 **SendAudioFrame**。

### 3. 驱动侧：SendAudioFrame 走的路径

- **SendAudioFrame** → ioctl → `hdmitx_send_audio_frame()`：  
  把 frame 入队，`schedule_work(&dev->audio_work)`。

- **audio_work** → `hdmi_audio_work_handler()`：
  - 从队列取出 node，设  
    `ctx->mode.paudio.start_addr = node->frame.phy_addr`  
    `ctx->mode.paudio.stop_addr = node->frame.phy_addr + node->frame.len - 1`
  - **只调用 `audio_ahbdma(&ctx->hdmi_tx, &ctx->mode.paudio)`**  
  - **没有** 再走 `audio_configure()`（没有 audio_mute、fc_audio_config、N/CTS、unmute 等）

所以：后续送帧时，**只改了 paudio 的 start/stop，只调了 audio_ahbdma()**，没有重新做一遍 audio_configure。

---

## 三、两种流程的差异总结

| 项目 | 第一次播音（SetAttr + Start） | 后续送帧（SendAudioFrame） |
|------|------------------------------|----------------------------|
| 应用是否调 SetAttr | 是，带完整 attr（含 audio 起止、sample_rate、bit_depth） | 否，只调 SendAudioFrame |
| 应用是否调 Start | 是 | 否（已经 Start 过了） |
| 驱动是否走 audio_configure() | 是（在 Start → api_configure → audio_configure） | 否，只走 audio_work |
| 驱动是否走 audio_ahbdma() | 是（在 audio_configure 末尾） | 是（在 audio_work 里） |
| 传入 audio_ahbdma 的 paudio 来源 | SetAttr 存的完整 paudio（含采样率、位深等） | 仅 work 里改过的 start_addr / stop_addr，其余仍是第一次 SetAttr 的值 |
| 调用 audio_ahbdma 前是否有“上一次 DMA 完成” | 否，首次启动 | 是，第一次 DMA 已完成，**IH_AHBDMAAUD_STAT0 的 INTDONE(0x04) 已被置位** |

结论：  
- **应用层**：第一次播音依赖 **SetAttr（完整音频参数 + 起止地址）+ Start**；后续只发 **SendAudioFrame（仅 phy+len）**，不再改配置，这是合理分工。  
- **驱动层**：第一次会做完整 **audio_configure + audio_ahbdma**；后续只做 **audio_ahbdma**。  
- **关键 bug**：第二次及以后进 `audio_ahbdma()` 时，**IH_AHBDMAAUD_STAT0** 里还留着上一次 DMA 完成的 **INTDONE(0x04)**。该寄存器是 **写 1 清除（W1C）**，当前代码写的是 `dev_write(IH_AHBDMAAUD_STAT0, 0x0)`，**无法清除该位**。接着 unmask 中断后，会立刻触发“完成”中断，表现为“送一帧就马上中断、后续帧异常”。  
- **修复**：在 `audio_ahbdma()` 里，**先读 IH_AHBDMAAUD_STAT0，再按 W1C 写回要清除的位（如 0x04/0x24）**，再配地址、开中断、START，这样后续送帧的中断才会在真正 DMA 完成时才触发。

---

## 四、应用层参数与 API 对照（便于核对）

- **第一次播音用到的应用参数**：  
  `audio_en`, `bit_depth`, `sample_rate`, `audio_start_paddr`, `audio_stop_paddr`（以及可选 SetInfoFrame 里的 sampling_freq / sample_size）。
- **第一次播音用到的应用 API**：  
  `CVI_SYS_Init`, `AUDIO_MAP`, `CVI_HDMI_ForceGetEdid`, `CVI_HDMI_GetSinkCapability`, `CVI_HDMI_Init`, **`CVI_HDMI_SetAttr`**, （可选）**`CVI_HDMI_SetInfoFrame`**, **`CVI_HDMI_Start`**, `CVI_HDMI_GetAttr`, `CVI_HDMI_RegisterCallback`。
- **后续送帧用到的应用 API**：  
  仅 **`CVI_HDMI_SendAudioFrame`**，参数仅一帧的 phy、len、seq、vb_blk。

差异就是：**第一次 = 完整配置 + Start（驱动里 audio_configure + audio_ahbdma）；后续 = 只送帧（驱动里只 audio_ahbdma，且必须正确清除上一次的 INTDONE）。**
