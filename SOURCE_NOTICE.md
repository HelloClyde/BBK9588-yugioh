# 来源和许可

| 内容 | 固定来源与处理 |
|---|---|
| 原生规则、AI、卡片数值、开局、收藏、存档与解码代码 | [HelloClyde/BBKH1-yugioh](https://github.com/HelloClyde/BBKH1-yugioh)，固定 `f1e83a84d8a9dbe686bb1eef064118e409b46274`；作为 `upstream/` 子模块保留 Apache-2.0、NOTICE 和原有来源记录 |
| 9588 前端、适配及工具 | 本仓库 `src/`、`tools/`、`tests/`，Apache-2.0；未修改上游子模块中的文件 |
| 9588 SDK | [HelloClyde/bbk9588-bda-sdk](https://github.com/HelloClyde/bbk9588-bda-sdk)，固定 `f2fcbe020e7e94014c33d7e5c593802e4b1e3034`；保留其许可证和 NOTICE |
| 存档全局刷写兼容封装 | 应用自己的 `src/sdk_compat9588.h`，C200 FS `+0x074`；见 [兼容说明](docs/SDK_COMPATIBILITY.md)，不依赖 SDK 工作区未发布改动 |
| 公开桌面图标 | 改编自固定上游 `tools/make_open_assets.py` 的原创几何卡片图案；构建时生成，不读取商业图片或字体，Apache-2.0 |
| 卡图、棋盘、菜单、木纹、纸张和手势图 | 原游戏及上游适配资源，公开再分发授权未确认；五个外部 BIN/PAK 均不入库、不随公共 Release 分发；游戏运行时继续读取使用者提供的原包 |
| 汉化卡名及效果文本 | 原汉化版本署名“游戏王汉化制作组”；具体贡献名单与再分发授权未确认，文本资源包不入库 |
| 音乐和音效 | 原游戏素材，上游转换为 IMA ADPCM；公开再分发授权未确认，H1SND.PAK 不分发 |
| 字库 | 上游声明来自 Windows SimSun（宋体）；字库再分发授权未确认，FONT.BIN 和原字体文件不分发 |
| 运行截图 | 本项目的真实 9588 固件模拟器运行记录，保存在 `docs/screenshots/`；所显示的第三方内容不适用代码许可证 |
| 模拟器音量补丁 | `emulator/signed-audio-volume.patch`，沿用 QEMU 被修改文件的 GPL-2.0-or-later；只用于模拟器，不链接进游戏 BDA |
| 固件、NAND、存档、工具链和 PC 原程序 | 全部排除；CI 不取得游戏商业素材或固件 |

《Yu-Gi-Oh! Power of Chaos: Joey the Passion》及有关角色、卡片图像、音乐归原权利人，原游戏发行署名 KONAMI。本仓库为独立、非官方兼容实现。来源说明不是第三方授权书；代码许可证不会自动许可原作、汉化内容或字体。

外部资源的精确大小和 SHA-256 保留在固定上游 `assets/reference-manifest.json` 中。导入工具只校验使用者本地提供的文件，不提供下载商业资源的脚本。
