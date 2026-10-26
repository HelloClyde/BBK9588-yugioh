# 安装与运行

适用目标：BBK 9588 / C200，240×320 竖屏。尚未进行 9588 真机测试。

希望解压即玩时，请下载 Release 的 `BBK9588-yugioh-v0.1.0-alpha.2-full.zip`，按 [完整包安装](https://github.com/HelloClyde/BBK9588-yugioh/blob/main/docs/INSTALL-FULL.md) 解压到 A 盘根目录即可。本页以下步骤针对不带资源的 CI 程序包 `BBK9588-yugioh.zip`。

1. 解压 `BBK9588-yugioh.zip` 到设备 A 盘根目录，得到 `A:\应用\程序\游戏王.bda`。
2. 将 Release 的 `-resources.zip` 配套资源包也解压到 A 盘根目录，或使用自己的兼容原资源，放到：

   ```text
   A:\应用\数据\游戏\YUGIOH\FONT.BIN
   A:\应用\数据\游戏\YUGIOH\H1UI.PAK
   A:\应用\数据\游戏\YUGIOH\H1ART.PAK
   A:\应用\数据\游戏\YUGIOH\H1SCN.PAK
   A:\应用\数据\游戏\YUGIOH\H1SND.PAK
   ```

   资源也支持 `B:\应用\数据\游戏\YUGIOH\`，优先 A 盘；大小与 SHA-256 见 ZIP 内 `应用/数据/游戏/YUGIOH/resources-manifest.json`。此 CI 程序 ZIP 不携带这些资源，仅复制 BDA 不能运行；`-full.zip` 已包含它们，无需重复安装。
3. 刷新桌面，在“娱乐天地”启动“游戏王”。

方向键选择、确定执行、返回关闭页面；决斗中返回打开菜单。触摸卡片先选中，再点同一卡片执行操作。选牌弹窗提供“返回 / 详情 / 确认”；右方向键也能打开当前高亮牌的详情，返回保留原选项与页码。

存档 `JNAT0.SAV`、`JNAT1.SAV` 写在实际加载 H1UI.PAK 的资源目录。安装包不携带存档。与固定上游 H1 版本的存档格式相同，迁移已有存档前自行保留备份。

从 alpha.1 升级时，可把旧盘根 `YUGIOH` 里的两个 SAV 文件复制到本版 `应用\数据\游戏\YUGIOH` 目录，保留旧文件备份；新版本使用新目录中的资源和存档。

原素材、字库和音频不适用本项目的代码许可证；来源与许可缺项见 `SOURCE_NOTICE.md`。
