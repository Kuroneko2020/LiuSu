# 实现计划（更新到 Phase-2）

## 1. 当前分层（保持不变）

```text
src/
  app/        # 应用入口、路由、控制器（AppController）
  models/     # 状态模型（Project/Page/Slot）
  services/   # 图像导入处理、导出服务
  ui/         # 预留 ViewModel/Adapter
qml/
  pages/      # Home/Edit/Export/Settings
  components/ # TemplateCard/TemplateCanvas/SlotItem/FilmStrip
docs/
assets/
```

## 2. Phase-2 实施内容

### 2.1 图片导入（真实）
- `ImageService` 使用 `QFileDialog` 打开本地文件选择器。
- 单图导入：返回用户选定图片路径。
- 批量导入：多选后按文件名排序并返回有效列表。
- 过滤格式：JPG/JPEG/PNG/WebP/BMP/TIFF（HEIC/HEIF 放在过滤器里但依赖系统插件）。

### 2.2 EXIF 方向修正
- 在 `ImageService` 内统一通过 `QImageReader::setAutoTransform(true)` 读取校验。
- 导入前先验证可读并触发自动方向变换链路（主要覆盖 JPG/JPEG 手机照片方向问题）。

### 2.3 状态模型增强
- `ProjectState` 新增槽位图像路径、旋转、镜像、填充模式读取接口。
- 批量填充支持“当前页空槽开始、当前页满则自动建下一页、不覆盖已存在图片”。
- 删除最后一页时自动补一页空白，保证编辑器可继续使用。

### 2.4 编辑页真实显示
- `SlotItem` 使用 `Image` 组件渲染真实图片。
- “完整放入/铺满裁切”映射为 `PreserveAspectFit/PreserveAspectCrop`。
- 旋转和镜像使用 QML 变换，状态由 C++ 模型驱动。
- 空槽位保持加号和单图导入入口。

### 2.5 胶片栏联动
- 每页显示基于首张图片的简化缩略图。
- 支持当前页高亮与点击切页。
- 新建空白页与删除页流程可用。

## 3. 暂不实现（按范围控制）
- 真实导出引擎与裁切标记绘制。
- HEIC/HEIF 稳定完整支持（仅提示依赖系统插件）。
- 槽位交换拖拽落地。
- 铺满裁切模式下手动构图偏移。
- 设置页持久化。

## 4. 依赖与平台注意事项
- 需要 Qt 6.5+：Core/Gui/Qml/Quick/QuickControls2/Widgets。
- 文件选择器使用 `QFileDialog`，在不同平台样式由系统决定。
- HEIC/HEIF 能否成功读取取决于系统编解码插件，不影响常见格式导入。

## 5. Phase-3 建议
1. 图像标准化缓存（统一内部位图）与内存管理。
2. 铺满裁切手动偏移和手势交互。
3. 槽位交换拖拽。
4. 导出引擎（分辨率、命名规则、裁切标记）。
5. 项目状态持久化与恢复。
