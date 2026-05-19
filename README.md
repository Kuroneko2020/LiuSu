# 留素 (Liusu)

留素是一款简洁高效的桌面端照片拼版工具，帮助你将多张照片快速组合成一张 6 英寸标准相纸，支持直接打印或分享传播。

---

## 功能一览

### 📋 模板选择
- 提供三种固定模板：**2 格（两张拼图）**、**4 格（四张拼图）**、**9 格（九张拼图）**
- 手动布局：逐个槽位导入，自主控制每张照片的位置
- 自动布局：选择模板后一键自动分配导入的图片

### 🖼️ 照片编辑
- **旋转**：每张照片可单独旋转 90°
- **镜像**：水平翻转
- **填充模式**：完整放入（留白）/ 铺满裁切
- **批量导入**：一次性向当前页面所有空槽位填充图片
- 多页面横向滚动浏览

### 🎨 页面样式
- **纯色背景**：自定义任意颜色
- **纹理背景**：支持自定义纹理图片

### 📤 导出设置
- 格式：JPG / PNG
- 分辨率：预设 300 PPI、600 PPI 或自定义 PPI
- 文件命名规则：组合命名 / 日期-序号
- 裁切标记开关
- **原图导出模式**：保留原始像素尺寸，不压缩

### ⚙️ 设置选项
- 默认填充策略、方向策略
- 默认导出路径、格式、分辨率
- 缓存目录与自动清理
- 纹理目录管理
- 预览质量调节

---

## 运行方法

### 环境要求
- Qt 6.5 或更高版本（包含以下模块）：
  - Core / Gui / Qml / Quick / QuickControls2
- CMake 3.21+

### 编译构建

```bash
cmake -S . -B build
cmake --build build
```

如果 CMake 无法找到 Qt，请通过以下方式指定 Qt 安装路径：

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/qt6
# 或设置 Qt6_DIR 环境变量
```

### 启动

```bash
./build/liusu.exe
```

---

## 支持的图片格式

| 格式 | 说明 |
|------|------|
| JPG / JPEG | ✅ 完全支持 |
| PNG | ✅ 完全支持 |
| WebP | ✅ 完全支持 |
| BMP | ✅ 完全支持 |
| TIFF | ✅ 完全支持 |
| HEIC / HEIF | ⚠️ 依赖系统 Qt 插件，部分系统可能无法读取 |

> EXIF 方向信息会自动应用。

---

## 项目结构

```
LiuSu/
├── CMakeLists.txt          # CMake 构建配置
├── README.md               # 项目说明文档
├── .gitignore              # Git 忽略文件
│
├── docs/                   # 产品需求与开发进度文档
│   ├── product-requirements.md
│   └── implementation-plan.md
│
├── src/                    # C++ 核心代码
│   ├── main.cpp            # 程序入口
│   ├── app/                # 应用控制器（流程管理）
│   │   ├── AppController.cpp
│   │   └── AppController.h
│   ├── models/             # 数据模型（项目状态、类型定义）
│   │   ├── ProjectState.h / .cpp
│   │   ├── Types.h
│   │   └── ImageResource.h
│   └── services/           # 服务层（图像处理、导出、自动布局）
│       ├── ImageService.h / .cpp
│       ├── ExportService.h / .cpp
│       ├── TemplateLayout.h / .cpp
│       └── AutoLayoutPolicy.h / .cpp
│
├── qml/                    # Qt Quick QML 界面
│   ├── Main.qml            # 应用主窗口
│   ├── pages/              # 四个主要页面
│   │   ├── HomePage.qml    # 首页（模板选择）
│   │   ├── EditorPage.qml  # 编辑页
│   │   ├── ExportPage.qml  # 导出页
│   │   └── SettingsPage.qml # 设置页
│   └── components/         # 可复用 UI 组件
│       ├── TemplateCard.qml
│       ├── TemplateCanvas.qml
│       ├── SlotItem.qml
│       ├── FilmStrip.qml
│       ├── ColorPickerDialog.qml
│       ├── IconToolButton.qml
│       └── PageTopBar.qml
│
└── res/                    # 资源文件（图标）
    └── icons/
```

---

## 未来版本计划

以下功能将在后续迭代中加入：

- 自定义模板编辑器
- 修图 / 滤镜 / 调色
- 文字、贴纸、边框素材
- 多图层支持
- 账号系统与云同步
- AI 自动选图
- 人脸识别自动构图
- RAW / PSD / PDF 文件支持

---

## 技术栈

- **语言**：C++ (C++17)
- **GUI 框架**：Qt Quick / QML
- **构建系统**：CMake
- **Qt 版本**：6.5+

---

> 💡 本项目**留素 (Liusu)** 由 **ChatGPT** 全权策划并通过 **Codex** 开发完成。