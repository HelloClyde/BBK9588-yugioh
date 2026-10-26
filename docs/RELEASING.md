# 发布约定

拟用独立仓库名：`HelloClyde/BBK9588-yugioh`。当前只做本地准备，没有创建远端仓库、推送标签或创建 Release。

1. 在干净检出中执行 README 的子模块初始化、工具链安装、源码测试、构建和 `tools/check_release.py`。
2. 查看 `VERSION` 与 `docs/releases/v<VERSION>.md`。首次候选版本为 `v0.1.0-alpha.1`，未做真机验证，因此标为预发布。
3. 对计划发布的确切 BDA 做模拟器 / 真机检查，在验证记录中记录完整 BDA 与载入代码 SHA-256、SDK 提交和验证范围。文档或图标变化不能把旧的完整 BDA 校验值直接替换成新值。
4. 获得明确的远程发布授权后，创建目标仓库并推送审查过的 main 提交；先核对远端标签 / Release，再创建并推送匹配 VERSION 的新标签。不要强推已有公共标签。
5. 等待该标签对应的 Actions 成功。Release 只取构建 job 产生的同一组 artifacts，不重新构建；下载动作执行校验，发布脚本再按 SHA256SUMS、版本、源提交和固定依赖复核。
6. 检查 Release 的实际文件名、大小并下载核对 SHA-256。远程动作没有发生时，不把本地 workflow 准备写成已发布。

公开资产清单：

- `Yugioh9588.bda`
- `BBK9588-yugioh.zip`（BDA、说明、原资源哈希；不含原资源）
- `BUILD.json`
- `SHA256SUMS.txt`

不公开五个 BIN/PAK、商业字体、原游戏程序、NAND/固件、个人存档、工具链或测试可执行文件。模拟器补丁和截图的许可边界见 SOURCE_NOTICE。

工作流接口已参照官方定义核对：
[checkout v7](https://github.com/actions/checkout/blob/v7/action.yml)、
[setup-python v6](https://github.com/actions/setup-python/blob/v6/action.yml)、
[cache v6](https://github.com/actions/cache/blob/v6/action.yml)、
[upload-artifact v7](https://github.com/actions/upload-artifact/blob/v7/action.yml)、
[download-artifact v8](https://github.com/actions/download-artifact/blob/v8/action.yml)。
本项目采用普通归档 artifact，再由 download-artifact 解包；不把 artifact 容器摘要当成 BDA 文件摘要。
