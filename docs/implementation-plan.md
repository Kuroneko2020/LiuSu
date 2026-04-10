# 第一阶段实现计划（Qt 6 + C++ + QML）

## 1. 目录结构与分层

```text
src/
  app/        # 应用入口、路由、控制器
  models/     # 状态模型（项目、页面、槽位、导出、设置）
  services/   # 图像导入处理、导出服务（接口先行）
  ui/         # QML 暴露桥接对象（ViewModel / Controller）
qml/
  pages/      # Home/Edit/Export/Settings 页面
  components/ # 复用组件（模板卡、槽位、队列等）
docs/
assets/
```

设计理由：
- QML 专注展示与交互编排。
- C++ 持有核心状态，便于后续单元测试与性能优化。
- `services` 将图像/导出引擎边界前置，避免后期重构成本。

## 2. 第一阶段开发清单

### 2.1 工程初始化
- CMake 配置 Qt 6（Core/Gui/Qml/Quick/QuickControls2）。
- 创建主程序入口 `main.cpp`，加载 `qml/Main.qml`。
- 注册 C++ 控制器到 QML（`AppController`）。

### 2.2 状态模型
- `TemplateType`（Two/Four/Six）。
- `SlotState`：槽位图片状态、选中、旋转、镜像、填充模式、拖拽占位接口。
- `PageState`：模板类型、槽位列表、有效页判定。
- `ProjectState`：页面队列、当前页索引。
- `ExportSettings` 与 `AppSettings`：导出与应用设置参数。

### 2.3 服务边界
- `ImageService`：
  - 单图导入接口（槽位）
  - 批量导入接口（编辑页入口）
  - 未来扩展：格式识别、EXIF 纠正、内部位图标准化
- `ExportService`：
  - 导出当前页/全队列接口
  - 第一阶段返回占位结果并打通调用路径

### 2.4 QML 页面
- `Main.qml`：顶部导航 + `StackLayout` 页面切换。
- `HomePage.qml`：模板卡片展开/收起交互。
- `EditorPage.qml`：预览区、队列栏、工具按钮、批量导入入口。
- `ExportPage.qml`：导出参数表单。
- `SettingsPage.qml`：设置参数表单占位。

### 2.5 编辑器组件
- `TemplateCanvas.qml`：根据模板渲染槽位网格。
- `SlotItem.qml`：空槽位加号、有图槽位选中态、操作条、拖拽入口占位。
- `FilmStrip.qml`：底部页面队列、当前页高亮、导出全队列按钮。

## 3. 平台与依赖说明
- 依赖：Qt 6.5+（建议），需要 `qtdeclarative` 与 `qtquickcontrols2`。
- HEIC/HEIF 实际解码可能依赖系统插件（后续阶段接入）。
- 若本地缺少 Qt 开发包，`cmake` 配置会失败（环境问题，不影响代码结构）。

## 4. 第二阶段建议
1. 接入真实文件选择器与图片解码管线。
2. 完成槽位交换拖拽与图像偏移拖拽。
3. 落地导出引擎（PPI、裁切标记、命名规则）。
4. 增加项目持久化（自动保存/恢复）。
5. 补充模型单测与关键交互自动化测试。
