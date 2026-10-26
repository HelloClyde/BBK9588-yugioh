# 验证记录

当前版本：`0.1.0-alpha.2`。固定 SDK 和上游提交不变，见 `dependencies.json`。

本版将资源和存档根目录调整为 `A:\应用\数据\游戏\YUGIOH\`，兼容 B 盘同名目录；程序优先 A 盘文件，打不开时尝试 B 盘。新路径由同一配置生成设备 GBK 字符串和 ZIP 内的 UTF-8 目录。

## 确切程序

- BDA SHA-256：`5e97eb8b1f89ad3a17264b969e902b618ba75cdb9b350511146134089515ad7a`，1,314,316 字节。
- 载入映像 SHA-256：`0fe411a06d13bd8d5db18b3d4ab221f716a8943ec297dfb8862c45f56fb7dfa9`。
- `.text` SHA-256：`1ef1284c0b6b622a658cb3b3d3f62ebb8db395d9e33444a0ded266f4616aa365`。
- SDK：`f2fcbe020e7e94014c33d7e5c593802e4b1e3034`。
- 上游：`f1e83a84d8a9dbe686bb1eef064118e409b46274`。
- MIPS GCC 15.2.0，Pillow 12.3.0，pyelftools 0.33。

## 主机检查

固定上游规则 1,454,892 项、卡组和存档 4,691 项均零失败；带本地资源的前端 221 项断言和生产音频测试通过。见 `verification/host-ui.json`。

新增资源路径回归 93 项，检查真实生成的设备路径头文件：GBK 字节与目标中文目录一致；五个资源分别覆盖 A 盘可用、B 盘可用、两盘可用及两盘缺失；A 盘优先、B 盘回退和空驱动输出指针均符合预期。生产前端的 `profile_load` 另验证 A/B 选择后使用相应存档根目录。见 `verification/resource-paths.json`。

## 固件与安装包

当前确切 BDA 的独立官方模拟器验证见 `verification/public-build.json`。A 盘新目录实际通过启动、菜单短按/长按、触摸编辑、八次双槽保存及正常退出重开检查。每次保存后核对完整程序和内存代码；最终存档序号为 16、主卡组 42 张。离线检查程序、五个资源和两槽 CRC 全部通过，见 `verification/alpha.2-resource-integrity.json`；测试镜像中没有盘根 `YUGIOH`，避免旧目录掩盖错误。B 盘回退目前由主机回归验证，未宣称做过 B 盘固件或真机验证。

完整安装包和独立资源包使用 `应用/数据/游戏/YUGIOH/`，程序仍在 `应用/程序/游戏王.bda`。五个资源逐个保持上游原始 SHA-256。完整包中的 BDA 必须与本版本 CI 程序逐字节一致，打包及下载校验按 [资源发布说明](RESOURCE_RELEASE.md) 执行；不包含存档、固件或 NAND。

## 基础 NAND 的空闲块问题

本次使用官方 QEMU 对原基础镜像进行多次存档时，再次发现程序文件内容变化。首次候选有 45 字节变化；去掉全局 flush 的独立对照在第七次存档也出现 36 字节变化，因此该对照不能支持将异常归因于全局 flush。两份异常 NAND 和存档均在本机冻结保留，见 `verification/alpha.2-unerased-free-block-audit.json` 与 `alpha.2-no-flush-control-audit.json`。

原基础镜像的物理块 90–112 标为 FTL 空闲，用户数据全部为 FF，但空闲页 OOB 已预写 FF 数据对应的 RS 校验 `cd9d9058f48bffb76f`。NAND 编程按位 AND，不能把已清零的位写回 1。例如期望新校验 `ce39d3df100cdc40a0` 与旧空白页校验 AND 后为实测的 `cc1990581008dc0020`，导致后续分配时错误校验和纠错传播。读回数据、校验字节与 FTL 位置见 `verification/alpha.2-ecc-observation.json`。

正式对照使用全新基础镜像副本，仅将这 23 个已确认全空且 FTL 标为空闲的块恢复全 FF 擦除态，未改动固件、已有映射块、游戏程序或游戏资源。之后再部署同一候选 BDA；没有覆盖两份异常镜像来抹掉记录。处理范围见 `verification/alpha.2-free-block-normalization.json`。游戏保持原有写入、全局刷写、close 和 CRC 重读流程。

## 历史证据及边界

alpha.1 的确切程序、整局对局、选牌、音频输出、异常环境及官方模拟器对照记录保留在 [alpha.1 验证记录](VERIFICATION-alpha.1.md) 和 `verification/`，不把旧哈希改写为新哈希。本版载入映像已因路径字符串和提示变更而改变，不能声称与 alpha.1 字节相同。

README 的历史界面截图用于展示布局；本版没有调整界面布局或原规则。官方旧模拟器的 PCM 音量问题及实验 QEMU 存储异常的边界仍见历史记录；本次只使用官方发行版做固件路径验收。

尚未进行 9588 真机测试，也没有穷举所有卡片组合。Actions 状态以对应提交实际运行结果为准，CI 不做固件模拟器测试。发布脚本继续要求成功固件记录与待发布 BDA 哈希一致。
