# 可选完整固件验证

固件、NAND 和五个游戏资源须由使用者自行准备，不放在仓库或 CI 中。公共构建不需要它们。
先完成 README 的本地资源导入、`requirements-test.txt` 和带 `--assets` 的宿主测试。

在自有模拟器中离线安装当前 `dist/Yugioh9588.bda` 与兼容原资源，然后正常启动到游戏主菜单。
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
