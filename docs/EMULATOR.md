# 可选完整固件验证

固件、NAND 和五个游戏资源须由使用者自行准备，不放在仓库或 CI 中。公共构建不需要它们。
先完成 README 的本地资源导入、`requirements-test.txt` 和带 `--assets` 的宿主测试。

在自有模拟器中离线安装当前 `dist/Yugioh9588.bda` 与兼容原资源。资源目录为 `/应用/数据/游戏/YUGIOH/`，然后正常启动到游戏主菜单。
以下工具只发送正常按键/触摸，并读取 guest memory 做断言，不写游戏状态：

```powershell
$env:YUGIOH_EMULATOR_URL = 'http://127.0.0.1:8016'
$env:YUGIOH_PROGRAM_PATH = '/应用/程序/游戏王.bda'
python tools/verify_selection.py
python tools/verify_firmware.py
```

若测试镜像用其他应用作为启动入口，将 `YUGIOH_PROGRAM_PATH` 改成实际被替换的文件路径。
`tools/live.py` 可单独发送短按、重复按键和触摸，并保存到忽略的 `qa/` 目录。
测试对局和组牌流程会更改该测试镜像的存档，应始终用专用 NAND 副本。

音频验证在主菜单运行，`build/tests/audio-menu-reference.pcm` 由带原资源的宿主测试生成：

```powershell
python tools/capture_audio.py --base $env:YUGIOH_EMULATOR_URL --out qa/audio-after.wav --seconds 14 --navigate
python tools/capture_audio.py --base $env:YUGIOH_EMULATOR_URL --out qa/audio-after-idle.wav --seconds 8
python tools/analyze_audio.py
```

旧模拟器的音量乘法会把负 PCM 采样错误地转成无符号数。可在自行取得的模拟器源码应用
`emulator/signed-audio-volume.patch` 后按模拟器自身文档重建；游戏不依赖此补丁才能在设备上运行。

对镜像进行文件管理前先核对端口、进程和 NAND 路径。某些旧前端共用 FAT 快照缓存；
避免多实例并发文件管理，或停止本任务 guest 后离线校验/写入专用副本。
遇到程序或素材哈希异常保留镜像及存档，不能用反复覆盖镜像来宣称原问题已经消失。

新资源路径的存取档回归：

```powershell
python tools/verify_paths_firmware.py --base $env:YUGIOH_EMULATOR_URL --program $env:YUGIOH_PROGRAM_PATH --save-cycles 4
```

它逐次检查八次保存后的程序文件和载入代码，核对新目录中的资源与两槽 CRC，再正常退出、重开并读取同一存档。B 盘路径另由不依赖资源的主机回归验证。

本次基础 NAND ZIP 中的 23 个 FTL 空闲块虽然用户数据全 FF，OOB 却预写了 RS 校验。分配这些块时 NAND 按位 AND 会损坏新校验，连续写入可能改变其他文件。验证使用的新副本先将确认全空且标为 free 的块恢复真正全 FF 擦除态；固件和映射块未变，原 ZIP 及异常镜像均保留。已有非空块不能按此方式清理；不要拿用户镜像试验。详细证据见 [验证记录](VERIFICATION.md)。
