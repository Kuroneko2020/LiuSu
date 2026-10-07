#pragma once

#include "domain/ProjectDocument.h"

#include <QImage>
#include <QSizeF>

namespace liusu::render {

// 域类型简写（渲染层大量使用）。
using liusu::domain::ProjectPage;
using liusu::domain::SolidBackground;

// 页面合成渲染入口（渲染管线第一片，子方案 04）。
// 契约（尺寸与渲染规则）：
// - 只认领域模型输入（页布局 + 槽位状态 + 背景色 + 页面毫米尺寸），
//   不接触 QML item 尺寸 / 屏幕缩放；
// - 预览与导出共用本函数，只有 ppi（输出像素密度）不同；
// - 绘制顺序：背景 → 槽位照片（页面级边框/裁切线层随 ADR-0005 立项时插入）。
//
// 像素变换顺序（工程规则·注释底线重点位，不得随意调换）：
//   EXIF 方向（ImageLoader 内完成）→ 用户旋转 → 镜像 → 填充缩放 → 裁切偏移取景
namespace PageRenderer {

// ppi 决定输出像素密度：px = mm / 25.4 * ppi（换算唯一入口在 domain::Units）。
QImage renderPage(const ProjectPage& page, const SolidBackground& background,
                  qreal pageWidthMm, qreal pageHeightMm, int ppi);

} // namespace PageRenderer
} // namespace liusu::render
