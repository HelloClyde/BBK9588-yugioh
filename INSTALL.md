# 安装与运行

适用目标：BBK 9588 / C200，240×320 竖屏。尚未进行 9588 真机测试。

1. 解压 `BBK9588-yugioh.zip` 到设备 A 盘根目录，得到 `A:\应用\程序\游戏王.bda`。
2. 自行提供有权使用的兼容原资源，放到：

   ```text
   A:\YUGIOH\FONT.BIN
   A:\YUGIOH\H1UI.PAK
   A:\YUGIOH\H1ART.PAK
   A:\YUGIOH\H1SCN.PAK
   A:\YUGIOH\H1SND.PAK
   ```

   资源也支持 `B:\YUGIOH\`，优先 A 盘；大小与 SHA-256 见 ZIP 内 `YUGIOH/resources-manifest.json`。公共 ZIP 不携带这些资源，仅复制 BDA 不能运行。
3. 刷新桌面，在“娱乐天地”启动“游戏王”。

方向键选择、确定执行、返回关闭页面；决斗中返回打开菜单。触摸卡片先选中，再点同一卡片执行操作。选牌弹窗提供“返回 / 详情 / 确认”；右方向键也能打开当前高亮牌的详情，返回保留原选项与页码。

存档 `JNAT0.SAV`、`JNAT1.SAV` 写在实际加载 H1UI.PAK 的资源目录。安装包不携带存档。与固定上游 H1 版本的存档格式相同，迁移已有存档前自行保留备份。

原素材、字库和音频不适用本项目的代码许可证；来源与许可缺项见 `SOURCE_NOTICE.md`。
