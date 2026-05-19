# 留素

把几张照片拼成一张可打印的 6 英寸页面，就这点事。

---

## 适合谁用

- 想把多张照片快速拼成 2 宫格、4 宫格或 9 宫格
- 需要批量处理，统一导出，不用每次去设计软件里手动摆
- 最终输出是打印或数字发布

---

## 功能一览

**首页** — 选择模板（2/4/9 格），手动创建任务或自动导入图片按规则分配槽位

**编辑页** — 给每个槽位选图、替换、删除；支持旋转 90°、镜像、填充模式切换（完整放入 / 铺满裁切）；可以设页面背景（纯色或纹理）和构图偏移；自定义取色器

**胶片栏** — 页面缩略图横向滚动浏览，多页时拖动切换

**导出页** — 导出当前页或全部；可设置命名规则、格式（JPG/PNG）、PPI（300/600/自定义）、裁切标记；原图导出模式不按 PPI 预设计算像素尺寸，但文件会写入实际物理尺寸对应的分辨率元数据

**设置页** — 自动拼版默认参数（填充策略、方向、PPI）、缓存目录与保留时间、纹理目录管理

---

## 技术栈

| 层级 | 技术 |
|------|------|
| UI | QML / Qt Quick / Qt Quick Controls 2 |
| 核心逻辑 | C++17 |
| 构建工具 | CMake |
| 核心模块 | AppController · ProjectState · ImageService · ExportService · AutoLayoutPolicy · TemplateLayout |

---

## 构建

### 依赖

Qt 6.5+，包含以下模块：

Core · Gui · Qml · Quick · QuickControls2

### 编译运行

```bash
cmake -S . -B build
cmake --build build
./build/liusu
```

如果 CMake 报找不到 `Qt6Config.cmake`，检查本机 Qt SDK 是否装全，以及 `CMAKE_PREFIX_PATH` 或 `Qt6_DIR` 环境变量是否指向了正确的 Qt 安装路径。

---

## 图片格式支持

导入时支持：JPG / JPEG · PNG · WebP · BMP · TIFF · HEIC / HEIF

HEIC / HEIF 依赖系统 Qt 图像插件，无插件时可能导入失败。

EXIF 方向信息会在预览和导出尺寸计算中参与自动旋转。

---

## 项目结构

```
LiuSu/
├── src/
│   ├── app/          # AppController 应用入口与流程编排
│   ├── models/      # ProjectState 项目状态模型
│   ├── services/    # ImageService / ExportService / AutoLayoutPolicy / TemplateLayout
│   └── main.cpp
├── qml/
│   ├── pages/       # HomePage / EditorPage / ExportPage / SettingsPage
│   └── components/  # 各类可复用 QML 组件
├── res/             # SVG 图标等资源
└── docs/            # 产品需求文档与各阶段进度记录
```

---

## 已知限制（第一版不做）

- 自定义模板编辑器
- 修图 / 滤镜 / 调色
- 文字、贴纸、边框素材
- 多图层
- 账号系统 / 云同步
- AI 选图 / 人脸识别
- RAW / PSD / PDF 支持