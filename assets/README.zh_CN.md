<p align="right">
  <strong>简体中文</strong> · <a href="README.md">English</a>
</p>

# 资源目录（Assets）

本目录集中存放可复用的资源（字库、图片、音乐等），按资源类型分子目录管理。每个资源放在其类型对应的子目录，并记录放置路径、命名方式、集成方式与来源/许可。二进制资源（字体、图片、音频）不属于纯 markdown 文档，请勿与文档混放。涉及版权/授权的资源需注明来源与许可。

## 字库（fonts）

可复用的字库文件与生成的字库源码放在 `fonts/`。

- 命名要能反映字族、字重、字级与格式。
- 记录来源、许可、字符范围、转换命令与目标放置路径。
- 添加字库前评估 Flash 与内部 RAM 影响；ESP32-C3 无 PSRAM。
- 不提交许可不允许分发的字库。

## 图片（images）

可复用的源图与生成的显示资产放在 `images/`。

| 文件 | 尺寸与格式 | 用途与来源 |
| --- | --- | --- |
| [`images/azmi-cartoon-v2.png`](images/azmi-cartoon-v2.png) | 方形 PNG；设备版本为 144 × 144 RGB565 | 最终卡通头像。于 2026-10-21 使用内置 imagegen 根据制作礼物时提供的参考图生成，保留本人特征、自然配色与温和微笑。执行 `python tools/gen_avatar.py` 生成 `main/avatar_data.h`（占用 41,472 字节 Flash，无额外头像 RAM 缓冲）。原始参考照片和废弃方案不随仓库分发。 |
| [`images/azmi-cartoon-prompt.txt`](images/azmi-cartoon-prompt.txt) | UTF-8 文本 | 最终生成提示词和参考图路径。插画由 imagegen 制作；Python 资源工具仅缩放并打包 RGB565。 |
| [`screenshots/overview.png`](screenshots/overview.png) 和 [`screenshots/sections.gif`](screenshots/sections.gif) | 1080 × 764，PNG / GIF | 根目录 README 的实际主机渲染，由 `tools/simulate.py` 导出；电量 87% 是示例值，不是真机测量。 |
| [`images/home.jpg`](images/home.jpg) | 3840 × 2160，JPEG | 嵌入中英文项目 README 的产品主图，突出 AI Passport 产品形象与开放、人人可创作的理念。 |
| [`images/readme-hardware-specs.png`](images/readme-hardware-specs.png) | 2172 × 724，PNG RGBA | 保留为可选技术参考图，不再用于首页主视觉。于 2026-09-17 使用内置图像生成工具为本仓库生成；已根据文档中的硬件能力契约核对图中的六项标签与参数。 |
| [`images/logo-wordmark.png`](images/logo-wordmark.png) | 1648 × 336，PNG RGBA | 从仓库原始 `images/logo.png` 中精确裁切并去除背景的黑色字标；用于中英文项目 README 的浅色主题。 |
| [`images/logo-wordmark-dark.png`](images/logo-wordmark-dark.png) | 1648 × 336，PNG RGBA | 提取字标的白色版本；README 使用 `<picture>` 在 GitHub 深色主题下显示。 |

- 使用描述性命名，并记录尺寸、像素格式、转换步骤与目标路径。
- 优先采用适合 240 × 320 RGB565 显示的格式，并纳入 Flash 与内部 RAM 考量。
- 许可允许时保留可编辑源文件，并记录来源与许可。
- 图片中不得包含设备二维码秘密、凭证或个人数据。

### 礼物界面预览

执行 `cmake -S tests/ui_preview -B build/ui-host` 和
`cmake --build build/ui-host` 构建真实 LVGL 渲染器；需要已下载固定版本的
LVGL Managed Component。运行 `python tools/simulate.py` 检查全部 20 个视图、
四条庆祝文案、500 次快速切换和动画回收，随后导出页面 PNG、
`build/preview-v3/overview.png` 总览和 `build/preview-v3/motion.gif` 动效预览。
这些是 `main/passport_ui.c` 的主机渲染结果，并非真机截图。
预览中的 87% 电量是示例值，实际固件读取 BSP 电量计。

八个章节各有一张 40 × 42 的原生插画贴纸，实现在 `main/passport_art.c`：
微笑太阳、探索者证件、旅程路线、VR 眼镜、金色奖杯、爱心涟漪、知识书本、
打开的信封。每张使用八帧动作循环，进入页面时短暂播放，3.2 秒后停稳。
贴纸直接使用 LVGL 图形绘制，无需额外位图或图片缓冲，放在预留的标题区域，
不占正文和二维码空间。预览测试会验证每个章节至少有三个不同的实际渲染帧，
并将八页动画一起导出到 `build/preview-v3/sections.gif`。

## 音乐与音效（music）

可复用的音乐与音效源码放在 `music/`。

`music/startup.mid` 和 `music/startup.wav`：**A Little Wonder for Azmi**，为礼物
原创的 C 大调／add9 上行合唱式短音效，按本仓库 MIT 许可证分发；没有使用录音、
采样或音色库。编辑 `data/startup-score.json` 后运行 `python tools/gen_startup_sound.py`。
MIDI 为 type 0、480 PPQN、120 BPM，使用 GM Voice Oohs（从零计数的 program 53）。
WAV 为 2.2 秒、16 kHz、16 位单声道，峰值约 -9 dBFS，带柔和起落、轻微失谐的
谐波声部及两次轻混响。外部 MIDI 乐器不一定与这套自制合成音色一致。
相同 PCM 嵌入 `main/startup_sound_data.h`（70,400 字节 Flash）；3 KB 的一次性任务
以 256 个采样（512 字节 RAM）分块播放。BSP 另保留 I2S 通道及 DMA；播放后挂起
codec ADC/DAC 和 I2S，任务栈自动回收。不读取或保存麦克风采样。音量、音色和
爆音需真机扬声器试听确认，主机测试不能证明听感；默认 codec 音量为 55%，不是
经过校准的声压级。

- 记录来源、许可、采样率、位深、声道、转换命令与目标路径。
- 与当前 BSP 音频路径匹配时优先采用 16 kHz、16 位单声道 PCM。
- 嵌入音频前评估 Flash 与内部 RAM 成本；长录音应流式或分块。
- 无再分发许可不提交媒体文件。
