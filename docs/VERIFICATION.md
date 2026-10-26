# 验证记录

本地检查日期：2026-10-26。固定依赖见 `dependencies.json`，应用兼容封装见 SDK_COMPATIBILITY.md。

## 当前公开候选 BDA

| 项目 | 结果 |
|---|---|
| 候选版本 | `0.1.0-alpha.1` |
| 完整 BDA SHA-256 | `ca9e8abf2ce51910488178a427dd5b37e4cc077bb94b9665fe20410e0d7a3c7d` |
| 大小 | 1,314,060 字节 |
| 完整载入映像 SHA-256 | `1f38374191da69402d819e6d9c2c1811e7775c569d5e9b4d11bfbb4809fbe57c` |
| `.text` SHA-256 | `7282b5c5075cc71de4507eab27752a8cfdbd8e9345f04f3c2243d7a86b5737f0` |
| SDK | `f2fcbe020e7e94014c33d7e5c593802e4b1e3034`，干净公开检出 |
| 上游游戏 | `f1e83a84d8a9dbe686bb1eef064118e409b46274`，未修改 |
| 工具链 | MIPS GNU GCC 15.2.0；安装归档 SHA-256 已与 SDK 安装器的 `8ba866e25c9826ee04ab4310365d264e3e73769e3738bb58ae38fd6740b7ee8d` 核对 |
| 本地主机 | Windows、Python 3.11.5、Pillow 12.3.0、pyelftools 0.33 |

已经通过标题 GBK、分类 4（娱乐天地）、入口 `0x81c00020`、四幅 VX 图标、header/checksum、静态载入窗口与硬件浮点指令扫描。公开桌面图标由几何代码生成，未把原卡片插画像素放进 BDA 图标区。

## 干净构建和主机检查

- 只从 Git 取得本仓库及固定 SDK / 上游，不取得工作区未提交的 SDK 文件或五个原素材。
- 两次独立输出目录构建的 BDA、安装 ZIP、BUILD.json 和 SHA256SUMS.txt 逐字节一致；记录见 `verification/reproducibility.json`。该清单对应其记录的源提交；文档提交会改变 BUILD.json 的 `source_commit`，实际交付始终以对应 `dist/SHA256SUMS.txt` 为准。
- 原生规则 1,454,892 项，卡组/存档 4,691 项，均零失败，见 `verification/host-rules.json`。
- 不带商业资源时选牌 UI 214 项断言通过；带使用者本地资源时 217 项及生产音频宿主测试通过，见 `verification/host-ui.json`。这些是宿主检查，不冒充固件执行。
- README 六张图片均为仓库相对路径。Git 和源码 CI 程序 ZIP 的明确文件清单检查通过，没有原资源、固件、NAND、个人存档或工具链。后续增补的完整安装和资源 ZIP 按 [资源发布说明](RESOURCE_RELEASE.md) 单独检查，仅包含白名单中的五个原资源及说明，不改变本次确切 BDA。
- Actions YAML、作业依赖、读写权限和官方 Action 接口已本地检查；远端 Actions 状态以对应提交的实际运行结果为准。

## 当前确切 BDA 的固件检查

使用 `bbk9588-emulator 0.1.5` 官方发行版 QEMU，二进制 SHA-256：
`6df16196d5acabdb96447e04150bcd4f65f34a9ae51ec5981177ca435bfe7002`。
测试使用独立 NAND 与私有 FAT 缓存，未停止或替换原有其他对局。

`verification/public-build.json` 绑定上面的完整 BDA 哈希，实际通过：

- 冷启动后进入主菜单；方向短按和菜单长按没有误退出。
- 对局中方向长按重复，手牌焦点由 1 移到 5。
- 触摸选项、返回及合成 ESC；关于页、组牌页和决斗暂停菜单正常。
- 新建双槽存档，增减卡片、保存、CRC、恢复原主卡组数量。
- 正常退出并从系统重新启动，读取同一序号、卡组数量和文件哈希。
- 启动和重开后的完整内存 `.text` 均与构建一致；已装 BDA 的 SHA-256 保持原值。

停止本任务 guest 后，离线读取 FAT 簇链并逐个核对程序及五个原资源，全部保持原值，见 `verification/public-resource-integrity.json`。本次没有重新宣称官方发行版模拟器的听感已修复。

## 可复用的前一版行为证据

`verification/reference/` 保留前一版完整 BDA：
`ad8e57161bb31d4a6c95bea03c88ebce64e30e35e68e741fdf807b68668bed2a`。
该版已完成完整对局、选牌详情、卡组/战绩保存、实际音频波形和缺样检查。

公开候选与该版在文件偏移 `0x95f8` 之后的**完整载入映像逐字节相同**，不是只比较源文件。差异仅为桌面图标区及 BDA 头校验信息。`reference/load-image.json` 记录二者对照；行为证据仍保留旧完整 BDA 哈希，不把旧摘要改写成新值。

README 中的主菜单和组牌截图来自当前公开候选；其他截图来自上述相同载入代码的参考版。`selection-firmware-*` 是正常召唤三眼怪、交由对手战斗破坏并触发搜索的真实固件画面：查看详情不改变对局和命令计数，返回保留选择，确认后实际将所选牌加入手牌。宿主合成状态图片不放入 README。

## 保留的环境异常及边界

本地重链的实验 QEMU（SHA-256 `7d0a425b7bddd3aeceb0795f3d380c5b1eb19a6e4f70e092efcb4a52c2ff3594`）曾在存档/重开后出现 BDA 45 字节变化，已装文件变成 `6886802a9e23475bb3fc003ebc0dd3a9386c32c2fff07eef26cfad33c5d9f007`，内存代码摘要也不同；五个原资源未变。见 `verification/experimental-qemu-failure.json` 和 `experimental-qemu-file-audit.json`。

异常 NAND 和存档已在本机保留，没有用覆盖同一镜像来消除失败记录。随后只用官方发行版二进制做一次独立对照，并取得前述通过结果。这个差异没有逐项定位，不能推断“音量补丁本身导致了存储异常”；实验重链环境不作为本次发布验收依据。

官方发行版旧模拟器仍有已知 PCM 负采样音量计算问题；`emulator/signed-audio-volume.patch` 是独立源码修正，应用时应针对对应模拟器版本正常重建，不把其他实验工作区的缓存对象当作官方验证环境。音频生产器的宿主和参考版实际输出证据与本次官方模拟器的存储验收分别记录。

尚未进行 9588 真机测试，也没有穷举所有卡片组合。本页记录本地验证范围；远端构建和发布状态以对应提交的 Actions 及 Release 为准。发布脚本要求与待上传 BDA 哈希匹配的成功固件记录，否则拒绝发布。
