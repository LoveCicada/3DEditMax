# 3DEditMax UI 重构需求规格

日期：2026-08-25  
状态：需求已定稿（待实现）；实现进度见下方需求表 **状态** 列  
设计参考：[`docs/ui/design/`](../../ui/design/)（`DESIGN.md`、`mockup.html`、`mockup.svg`、`index.html`）  
上位架构：[`2026-08-16-dx11-lab-teach-design.md`](./2026-08-16-dx11-lab-teach-design.md)  
壳布局：[`docs/ui/01-lab-shell-layout.md`](../../ui/01-lab-shell-layout.md)  
实现计划：[`docs/superpowers/plans/2026-08-25-ui-redesign.md`](../plans/2026-08-25-ui-redesign.md)

---

## 1. 目标与非目标

### 1.1 目标

在**不推翻**现行 `QMainWindow` + `QDockWidget` + HWND DX11 视口架构的前提下，按 `docs/ui/design` 的视觉语言升级实验台 UI，并分两期落地：

- **P1：** 全局换肤、面板色键、轴色贯穿、矩阵 `demoHot` 按色发光、工具栏只读演示条；已有教学能力保持可操作。
- **P2：** 按 mockup 增加新控件的 **UI 空壳**（可展示、可点但不要求业务闭环）；真功能另开任务接线。

成功标准：

- 打开 `mockup.html` 对照：主色板、矩阵四色、World/View/Proj 色头、轴色在左栏与视口图例一致。
- 演示播放时底栏教程条仍是主控；工具栏 `demo-bar` 同步只读文案。
- 规格表中每条 `REQ-*` 可被标为「已完成」或「不做」，实现过程可追踪。

### 1.2 非目标（本规格不要求实现）

| 项 | 说明 |
|----|------|
| 替换为 mockup 固定三列 HTML 栅格 | 保留 Qt dock；视觉对齐，结构不强制 1:1 |
| 自定义 Win32 非客户区标题栏 | mockup 标题栏仅作视觉参考；继续用系统标题栏 + Qt 菜单 |
| 录制 / 截图 / 性能采样真数据 | P2 可空壳按钮；无编码器、无 GPU 计数器接线 |
| 真 Navigator 相机小窗 | P2 空壳装饰；不另开第二 Swapchain |
| 把 Qt 控件叠在 Swapchain 上 | 硬禁止；HUD 仅非 native 层 |
| 改教学数学 / JSON / 渲染线程模型 | 除非某 REQ 明确要求接线现有 `LabState`/`TeachingState` |

---

## 2. 约束与决策

| ID | 决策 |
|----|------|
| D-01 | **色值源：** 以 [`mockup.html`](../../ui/design/mockup.html) `:root` 为准；`DESIGN.md` 叙述冲突时服从 mockup。 |
| D-02 | **壳路径：** 保留现有停靠壳 + 中央视口下教程条（非底 dock）。 |
| D-03 | **演示入口：** 底栏 `TutorialPanel` 为主控（上一步/下一步/演示\|停止）；工具栏 `demo-bar` 为 **只读同步** 或静态空壳。 |
| D-04 | **实现深度：** `接线` = 真行为；`空壳` = 外观到位，点击无业务或仅 `qDebug`/状态栏占位文案。 |
| D-05 | **HWND 边界：** 新视口装饰（Navigator、浮动工具、性能条、Toast、pills）必须在 `Dx11ViewportWidget` 非 native 区域或中央列其它 Qt 层，不得覆盖 `Dx11NativeSurface`。 |

---

## 3. 锁定色板（mockup `:root`）

### 3.1 表面与文字

| Token | Hex | 用途 |
|-------|-----|------|
| bg-base / bg-1 | `#0E1320` | 窗体最深底 |
| bg-canvas | `#0A0E18` | 视口画布感 |
| bg-panel / bg-2 | `#161B2B` | 停靠面板 |
| bg-elev / bg-3 | `#1D2335` | 抬升条、输入周边 |
| bg-input | `#0C1019` | 输入框 |
| border | `#272E44` | 默认边 |
| border-light | `#323A55` | 浅边 |
| text-1 | `#EEF1F9` | 主文字 |
| text-2 | `#B3BBD1` | 次文字 |
| text-3 | `#7C8499` | 弱文字 |
| accent | `#5B8DEF` | 强调蓝 |
| accent-2 | `#7AA1FF` | 强调亮 |

### 3.2 域色（World / View / Projection 面板头）

| Token | Hex | 语义 |
|-------|-----|------|
| world | `#EF476F` | World 面板色条 |
| view | `#06D6A0` | View 面板色条 |
| proj | `#FFB703` | Projection 面板色条 |

### 3.3 轴与追踪

| Token | Hex |
|-------|-----|
| x | `#EF476F` |
| y | `#06D6A0` |
| z | `#4CC9F0` |
| tracker | `#F72585` |

视口 3D 轴锥 / DebugDraw / HUD 色点应与上表对齐（允许渲染侧用归一化 float，但视觉等价）。

### 3.4 矩阵色键

| Token | Hex | 看板 |
|-------|-----|------|
| mw | `#4CC9F0` | M_W |
| mv | `#B388FF` | M_V |
| mp | `#FFB703` | M_P |
| mmvp | `#F72585` | MVP |

`demoHot` 聚焦时：左侧色条 + 与该矩阵色键同色的边框/微 glow（不再用统一黄洗）。

### 3.5 形态

- 圆角档：约 `6px / 10px / 14px`（QSS 可近似）。
- 背景三层：`#0E1320` → `#161B2B` → `#1D2335`。

---

## 4. 与现行壳的差异

| mockup | 现行实现 | 本规格策略 |
|--------|----------|------------|
| 自定义标题栏 + Logo | 系统标题栏 | 不做替换；菜单/工具栏用新色 |
| 固定 312 / 1fr / 320 栅格 | 左右 dock 可拖 | 默认宽度接近即可 |
| 演示在工具栏可点 | 教程在中央底条 | 底条主控；工具栏只读 demo-bar |
| Lab 四卡片横排 | Lab 垂直 GroupBox | P1 卡片化样式；布局可仍垂直 |
| 底状态栏 | 无 | P2 空壳 `QStatusBar` |
| Navigator / 浮动工具 / 性能 / Toast / pills | 仅轴图例 + 演示徽章 | P2 空壳 HUD |

---

## 5. 完成状态约定

每条需求使用固定字段：

| 字段 | 取值 |
|------|------|
| 分期 | `P1` / `P2` |
| 深度 | `接线` / `空壳` |
| 状态 | `未开始` / `进行中` / `已完成` / `不做` |

**更新规则（实现阶段）：**

1. 开始某 `REQ-*` → 状态改为 `进行中`。
2. 验收通过 → `已完成`，并在实现计划对应 Task 勾选。
3. 产品决定砍掉 → `不做`，并在计划中注明原因。
4. 本文件为进度单一事实来源；计划 todos 与之对齐。

初始状态：除非另有说明，全部为 **未开始**。

---

## 6. 需求目录

### 6.1 全局视觉

| ID | 描述 | 分期 | 深度 | 状态 | 对照 |
|----|------|------|------|------|------|
| REQ-VIS-001 | 应用 §3 表面/文字/强调色到 `app.qss` 与 Fusion 壳 | P1 | 接线 | 已完成 | mockup `:root` surfaces |
| REQ-VIS-002 | GroupBox / Dock / ToolBar / Menu / 输入控件圆角与边框对齐 mockup 密度 | P1 | 接线 | 已完成 | `.panel` / `.tool-group` |
| REQ-VIS-003 | `#tutorialStrip` 使用新表面色与顶部分割线 | P1 | 接线 | 已完成 | 现行 strip + mockup panel |
| REQ-VIS-004 | 文档注明色值以 mockup 为准；废弃旧抛光表中 W红/V青/P金/MVP紫 作为目标色 | P1 | 接线 | 已完成 | 旧 `viewport-polish` 色表 |

### 6.2 面板头

| ID | 描述 | 分期 | 深度 | 状态 | 对照 |
|----|------|------|------|------|------|
| REQ-HDR-001 | World / View / Projection 左侧 3–4px 色条 + 标题色（§3.2） | P1 | 接线 | 已完成 | `.panel-head .accent` |
| REQ-HDR-002 | 矩阵看板总头可用渐变色条；子块 M_W/V/P/MVP 各自色键（§3.4） | P1 | 接线 | 已完成 | `.panel.matrices` + matrix cards |
| REQ-HDR-003 | 面板头徽章样式（如 `TRS` / `MV·MVP`）出现在对应 GroupBox 标题区 | P1 | 接线 | 不做 | `.badge`（本期用色条替代徽章） |
| REQ-HDR-004 | 折叠 / 重置图标按钮外观 | P2 | 空壳 | 未开始 | `.icon-btn` |
| REQ-HDR-005 | 折叠真实收起面板内容 | P2 | 接线 | 未开始 | （后补，非空壳必做） |

### 6.3 工具栏

| ID | 描述 | 分期 | 深度 | 状态 | 对照 |
|----|------|------|------|------|------|
| REQ-TB-001 | 保留「列/行主序」「重置」并套用新工具按钮样式 | P1 | 接线 | 已完成 | `.tool` / 现行 Teach toolbar |
| REQ-TB-002 | 增加只读 `demo-bar`：步数 pill、标题、简述（单行省略）；随 `tutorialStep`/`demoPlaying` 更新 | P1 | 接线 | 已完成 | `.demo-bar` |
| REQ-TB-003 | `demo-bar` 内上一步/下一步按钮不作为主控（禁用或隐藏）；主控仍在 `TutorialPanel` | P1 | 接线 | 已完成 | D-03 |
| REQ-TB-004 | 工具组空壳：选择/平移/轨道/推拉 | P2 | 空壳 | 未开始 | toolbar 第一组 |
| REQ-TB-005 | 工具组空壳：实心/选择/线框/追踪点（与已有着色能力接线可后做） | P2 | 空壳 | 未开始 | toolbar 第二组 |
| REQ-TB-006 | 工具组空壳：立方体/球/柱 | P2 | 空壳 | 未开始 | toolbar 第三组 |
| REQ-TB-007 | 工具组空壳：录制/性能分析/截图 | P2 | 空壳 | 未开始 | toolbar 右侧 |

### 6.4 左栏 Transform / Object

| ID | 描述 | 分期 | 深度 | 状态 | 对照 |
|----|------|------|------|------|------|
| REQ-L-001 | World/View/Projection 分组应用 REQ-HDR-001 | P1 | 接线 | 已完成 | TransformPanel objectNames |
| REQ-L-002 | Pos/Scale 等 X/Y/Z 标签或前缀使用轴色 §3.3 | P1 | 接线 | 已完成 | mockup 左栏轴色字段 |
| REQ-L-003 | Object 面板视觉纳入新 QSS（无强制新布局） | P1 | 接线 | 已完成 | dockObject |
| REQ-L-004 | View 预设按钮样式对齐 mockup 小按钮 | P1 | 接线 | 已完成 | Front/Side/Top/Iso |

### 6.5 视口 HUD

| ID | 描述 | 分期 | 深度 | 状态 | 对照 |
|----|------|------|------|------|------|
| REQ-VP-001 | 轴图例色点/文字改为 §3.3；可保留中文「左右/上下/前后」或 mockup 英式短标签（实现时二选一写进计划，默认保留中文） | P1 | 接线 | 已完成 | `.ax-legend` / 现行 HUD |
| REQ-VP-002 | 演示中顶栏仅短徽章 `演示中 N/8`（已有行为）；色调对齐 accent | P1 | 接线 | 已完成 | 现行 `m_demoCaption` |
| REQ-VP-003 | Mini Navigator 空壳（右上装饰控件，无交互相机） | P2 | 空壳 | 未开始 | `.mini-axis` |
| REQ-VP-004 | 浮动竖工具条空壳：实体/线框/材质/网格/轴向/追踪点 | P2 | 空壳 | 未开始 | `.vp-tools` |
| REQ-VP-005 | 性能条空壳：FPS/MS/GPU/VRAM/Draws 占位数字 | P2 | 空壳 | 未开始 | `.vp-perf` |
| REQ-VP-006 | Toast 空壳：演示步进时可显示静态或简单文案（可选接线一步提示） | P2 | 空壳 | 未开始 | `.toast` |
| REQ-VP-007 | 视口底 pills 空壳：MSAA/Vsync/材质/剔除/Depth/分辨率/演示模式 | P2 | 空壳 | 未开始 | `.vp-bottom-strip` |
| REQ-VP-008 | 光标信息条空壳 | P2 | 空壳 | 未开始 | `.vp-cursor-info` |

### 6.6 右栏矩阵

| ID | 描述 | 分期 | 深度 | 状态 | 对照 |
|----|------|------|------|------|------|
| REQ-MX-001 | M_W/M_V/M_P/MVP 边框或色条使用 §3.4 | P1 | 接线 | 已完成 | matrix cards |
| REQ-MX-002 | `demoHot` 使用对应矩阵色 glow/边框，替代统一黄洗 | P1 | 接线 | 已完成 | `demoHot` property |
| REQ-MX-003 | 主序切换控件样式纳入新工具按钮语言 | P1 | 接线 | 已完成 | 看板头 actions |

### 6.7 Lab / Debug / Tracker

| ID | 描述 | 分期 | 深度 | 状态 | 对照 |
|----|------|------|------|------|------|
| REQ-LAB-001 | Device/Shader/Rasterizer/CB 视觉拆成卡片式 GroupBox（仍绑定现有 LabPanel 控件） | P1 | 接线 | 已完成 | `.card` 四块 |
| REQ-LAB-002 | Debug 日志区边框/背景对齐新表面色 | P1 | 接线 | 已完成 | DebugLogPanel |
| REQ-LAB-003 | CB 十六进制展示区空壳（可假数据或只读占位） | P2 | 空壳 | 未开始 | mockup CB hex |
| REQ-TR-001 | Tracker 面板头 + X/Y/Z 标签轴色 | P1 | 接线 | 已完成 | `.panel.tracker` |

### 6.8 教程条

| ID | 描述 | 分期 | 深度 | 状态 | 对照 |
|----|------|------|------|------|------|
| REQ-TU-001 | 保持中央底条 `TutorialPanel`：上一步/下一步/演示\|停止 + 标题/正文 | P1 | 接线 | 已完成 | 现行 strip（D-03） |
| REQ-TU-002 | 演示中禁用步进；停止恢复（已有则核对样式） | P1 | 接线 | 已完成 | TutorialPanel |
| REQ-TU-003 | 教程按钮/正文排版纳入新 QSS | P1 | 接线 | 已完成 | `#btnDemo` 等 |
| REQ-TU-004 | 与 REQ-TB-002 文案同步（同一步标题/简述） | P1 | 接线 | 已完成 | D-03 |

### 6.9 状态栏

| ID | 描述 | 分期 | 深度 | 状态 | 对照 |
|----|------|------|------|------|------|
| REQ-SB-001 | 主窗口 `QStatusBar` 空壳：Device OK、SwapChain、Backbuffer、Tick、Polling、CB Upload、Render thread、DPI 等占位段 | P2 | 空壳 | 未开始 | `.statusbar` |
| REQ-SB-002 | 将反馈队列中已有 Device/FPS 等信息接到状态栏（可选后补） | P2 | 接线 | 未开始 | FeedbackQueue |

### 6.10 明确「不做」或仅参考

| ID | 描述 | 分期 | 深度 | 状态 | 对照 |
|----|------|------|------|------|------|
| REQ-OUT-001 | 自定义标题栏/窗口按钮 | — | — | 不做 | `.titlebar` |
| REQ-OUT-002 | 第二套完整 HTML 壳替换 Qt | — | — | 不做 | mockup 整页 |
| REQ-OUT-003 | 录制/截图编码器与文件导出 | — | — | 不做 | REQ-TB-007 仅空壳 |

---

## 7. 验收检查清单

### P1

- [ ] 主窗口/停靠/输入在新色板下可读，无大面积旧紫青残留。
- [ ] World/View/Proj 色头与矩阵四色符合 §3。
- [ ] 左栏 X/Y/Z 与视口图例轴色一致。
- [ ] 演示时矩阵聚焦色随 `demoMatrixFocus` 变化。
- [ ] 底栏教程可完整走完 8 步与「停止」；工具栏 demo-bar 只读同步。
- [ ] 无控件叠在 Swapchain 上；无回归黑闪/崩溃。

### P2（空壳）

- [ ] 工具栏新工具组、状态栏、视口 Navigator/浮动条/性能/Toast/pills 可见且不破坏布局。
- [ ] 空壳点击不崩溃、不误改 TeachingState（除非该控件后续已接线）。

---

## 8. 建议实现落点（供计划引用）

| 区域 | 主要文件 |
|------|----------|
| 全局 QSS | `src/app/app.qss` |
| 壳/工具栏/状态栏 | `src/app/MainWindow.cpp` / `.h` |
| 变换/物体 | `src/ui/TransformPanel.*`、`ObjectPanel.*` |
| 矩阵 | `src/ui/MatrixBoardPanel.*` |
| Lab/日志 | `src/ui/LabPanel.*`、`DebugLogPanel.*` |
| Tracker | `src/ui/TrackerPanel.*` |
| 教程 | `src/ui/TutorialPanel.*` |
| 视口 HUD | `src/ui/Dx11ViewportWidget.*` |
| 轴色渲染对齐 | `src/render/DebugDraw.*`、`D3D11Renderer.*`、`AxisLabels.*`（仅当 REQ-VP/轴要求） |

---

## 9. 修订记录

| 日期 | 说明 |
|------|------|
| 2026-08-25 | 初版：自 mockup/DESIGN 拆解 REQ；两期 + 空壳策略；演示底栏主控 |
