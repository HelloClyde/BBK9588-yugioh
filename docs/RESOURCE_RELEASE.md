# Release 资源与完整安装包

`v0.1.0-alpha.1` 在原 CI 产物之外增补两个 ZIP：

- `BBK9588-yugioh-v0.1.0-alpha.1-full.zip`：推荐给使用者，包含已发布的同一份 BDA 和全部五个资源，解压到 A 盘根目录即可启动。
- `BBK9588-yugioh-v0.1.0-alpha.1-resources.zip`：只含配套资源及说明，安装目录为 `YUGIOH/`，用于搭配单独 BDA。

原 `BBK9588-yugioh.zip` 继续保留为不带资源的 CI 程序包，原 `BUILD.json` / `SHA256SUMS.txt` 的含义不变。增补文件另附 `RESOURCE_BUILD.json` 和 `RESOURCES-SHA256SUMS.txt`；不移动公开标签，不重编译游戏，不把资源写入 Git。

资源采用固定上游 H1 `v0.1.1` 配套文件，逐个对照 `upstream/assets/reference-manifest.json`。来源记录不等于第三方授权，许可范围见 [SOURCE_NOTICE.md](../SOURCE_NOTICE.md)。

本地打包（程序目录必须是已下载并验证的四个原 CI 资产）：

```powershell
python tools/package_resources.py --program-dir build/github/release-v0.1.0-alpha.1 --assets .local-assets --output-dir dist/resource-release
```

脚本先验证程序、版本、依赖、清单和对应 BDA 的成功固件记录，再验证全部五个资源。输出目录需没有同名文件。ZIP 使用白名单、固定时间与顺序，不递归收集输入目录。额外图片、资源原压缩包、存档、固件和 NAND 均不会打包。

完整包保留原程序 `BUILD.json`，增补清单分别记录程序源提交与打包工具源提交。检查 ZIP 内每个文件与输入逐字节一致；上传后下载并核对增补校验清单及 GitHub 资产摘要。安装说明见 [完整包安装](INSTALL-FULL.md) 和 [独立资源包安装](INSTALL-RESOURCES.md)。
