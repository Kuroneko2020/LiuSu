# Qt / QML 实践陷阱（留素实录）

本文记录留素开发中实际踩到并已解决的 Qt/QML 工程陷阱，含症状、根因与处置。
每条都对应仓库中的修复位置，供后续会话避免重蹈或快速定位。

## 1. QML 文件单例运行时解析失败 → 主题全黑

**症状**：界面所有颜色变成黑块；运行日志出现大量
`Unable to assign [undefined] to QColor`，随后 `ReferenceError: AppTheme is not defined`。

**根因**：`pragma Singleton` 的 QML 文件（`AppTheme.qml`）经 `qt_add_qml_module`
打包后，运行时无法解析该单例；属性读取全部返回 `undefined`，Qt 对颜色属性
回退为黑色。

**诊断方法**：
- 在 `main.cpp` 用 `engine.singletonInstance<QObject*>("LiuSu", "AppTheme")` 打探针，
  返回空即单例不存在；
- 用独立 `qml.exe` 加载一个引用该单例的小文件，报 `No such file or directory`，
  证明单例文件未按预期进入运行时。

**处置**：主题常量改用 **C++ 单例**（`src/app/AppTheme.h` +
`qmlRegisterSingletonInstance("LiuSu", 1, 0, "AppTheme", &theme)`），
QML 侧用法不变（`AppTheme.ink` 等），并在每个使用处 `import LiuSu`。
注意：C++ 注册的单例必须 `import` 所属模块才能被模块内 QML 文件看见。

**顺带教训**：`set_source_files_properties(... QT_QML_SINGLETON_TYPE TRUE)`
必须放在 `qt_add_qml_module` **之前**；但即便位置正确，本项目最终仍放弃
QML 文件单例方案（见上）。

## 2. ShaderEffectSource 抓背景做毛玻璃：本布局中不稳定

**目标**：按钮毛玻璃想模糊"按钮背后的内容"（真 backdrop blur）。

**实测**：以 `ShaderEffectSource { sourceItem: 板内容; hideSource: ... }` +
`MultiEffect` 实现后出现两种坏结果——`hideSource: true` 时整块板内容消失
（四张卡片变灰块）；`hideSource: false` 时按钮渲染出整卡缩放内容并错位。

**处置**：回退为**材质模拟**（半透明填充 + `MultiEffect` 对按钮本体轻度 blur
+ 顶缘高光 + 内斜微光），观感接近且完全稳定。真 backdrop blur 留作未来
自定义 shader 专项（在界面设计基准第九节已记录为已知限制）。

## 3. 高 DPI 缩放：物理分辨率 ≠ 逻辑可用区

**环境**：显示器 2560×1440，系统缩放 175% → 应用可见逻辑区约 1463×775。

**症状**：固定 1280×800 的窗口超出屏幕（底部被裁）。截图脚本另有一坑：
不调用 `SetProcessDPIAware()` 时拿到的窗口坐标与截图按虚拟化坐标解析，
画面看似被裁（实际是工具坐标错位，不是应用 bug）。

**处置**：
- 窗口尺寸取 `Screen.desktopAvailableWidth/Height - 24`，与上限 1440×900 取小；
- 主页元素按窗口高度比例定位，展示板行按宽度等比缩放；
- 截图一律先 `SetProcessDPIAware()`。

## 4. Qt 关键字宏污染成员名

`slots` / `signals` / `emits` 是 Qt 宏，作为结构体成员名会被预处理器吞掉，
报 `declaration does not declare anything`。留素 `LayoutModel::slotRects`、
`ProjectPage::slotStates` 即因此改名（代码注释已标注）。

## 5. QtTest 宏与花括号初始化的逗号

`QVERIFY(rect{1,2,3,4}.isValid())` 失败：宏把初始化列表里的逗号当参数分隔符
（`macro "QVERIFY" passed 4 arguments`）。处置：测试里经辅助函数传参
（如 `rect(x,y,w,h)`）绕开。`tst_domain.cpp` 与 `tst_render.cpp` 均用此法。

## 6. Qt PNG 解码器不应用 eXIf 方向块

`QImageReader` 的 `setAutoTransform` 对 JPEG/TIFF 生效，对 PNG 的 `eXIf` 块
不生效（相机不产出带方向的 PNG，属罕见边角）。已在 `ImageLoader.h` 记录为
已知限制，禁止在调用侧各自修补。

## 7. 运行中的可执行文件无法重链接

构建报 `cannot open output file LiuSu.exe: Permission denied`——应用还在运行。
处置：重构建前先 `taskkill //IM LiuSu.exe //F`。
同理，Windows 下验证用的离屏启动脚本应在截图/测试后确保进程退出。

## 8. sed 全局替换命中辅助函数自身 → 无限递归

用 `sed` 把 `NormalizedRect{...}` 批量替换为 `rect(...)` 时，辅助函数自身的
`return NormalizedRect{...}` 也被替换成 `return rect(...)`，运行时栈溢出
（测试进程 SEGFAULT，无栈信息）。教训：批量文本替换后必须复查辅助函数与
同名字符串，替换范围不用过宽的全局模式。

## 9. 截图与自动化操作（Windows 实录）

- `CopyFromScreen` 在 RDP 会话/窗口被遮挡时不稳定（句柄无效或黑图）；
  改用 `PrintWindow(hwnd, hdc, 2)`（`PW_RENDERFULLCONTENT`）抓窗口自身。
- 模拟点击：`SendMessage` 合成的鼠标消息 Qt 不响应；需
  `SetCursorPos` + `mouse_event` 真实注入，并把窗口 `SetWindowPos(HWND_TOPMOST)`
  置顶（`SetForegroundWindow` 在 IDE 抢焦点时可能失败）。
- 用完后记得 `HWND_NOTOPMOST` 复位，避免应用永久置顶。

## 10. 中文输出在 Git Bash 下乱码

`echo` 中文经 Git Bash 输出可能出现乱码，判断脚本结果时改用英文标记
（如 `OK` / `BOOT-OK`），避免误判为失败。
