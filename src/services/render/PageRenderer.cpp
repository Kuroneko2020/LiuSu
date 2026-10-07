#include "PageRenderer.h"

#include "domain/Units.h"
#include "services/image/ImageLoader.h"

#include <QPainter>

namespace liusu::render {

using liusu::domain::FillMode;
using liusu::domain::NormalizedRect;
using liusu::domain::SlotImageState;
using liusu::domain::mmToPixels;
using liusu::services::ImageLoader;

namespace {

// 空槽位与加载失败槽位的占位：浅底 + 细边，绝不静默留白或崩溃。
void drawPlaceholder(QPainter& painter, const QRectF& target, const QString& label)
{
    painter.fillRect(target, QColor(0xec, 0xef, 0xf4));
    painter.setPen(QColor(0xa3, 0x9c, 0x8e));
    painter.drawRect(target.adjusted(0, 0, -1, -1));
    if (!label.isEmpty()) {
        painter.drawText(target, Qt::AlignCenter, label);
    }
}

// 单槽位图片绘制：按既定变换顺序将图像适配进槽位矩形。
void drawSlotImage(QPainter& painter, const QImage& source, const QRectF& target,
                   const SlotImageState& state)
{
    QImage img = source;

    // 1. 用户旋转（顺时针 90 的倍数，领域层已校验）。
    if (state.rotationDegrees != 0) {
        QTransform rotation;
        rotation.rotate(state.rotationDegrees);
        img = img.transformed(rotation, Qt::SmoothTransformation);
    }
    // 2. 水平镜像。
    if (state.mirrored)
        img = img.flipped(Qt::Horizontal);

    // 3. 填充缩放：把图缩放到"恰好覆盖（Fill）或完整放入（Fit）"槽位。
    const double slotWidth = target.width();
    const double slotHeight = target.height();
    const double scale = state.fillMode == FillMode::Fill
        ? qMax(slotWidth / img.width(), slotHeight / img.height())
        : qMin(slotWidth / img.width(), slotHeight / img.height());
    const QImage scaled = img.scaled(qRound(img.width() * scale), qRound(img.height() * scale),
                                     Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    // 4. 裁切偏移取景：Fill 时取景窗在放大后的图内移动；
    //    Fit 时图片小于槽位，居中放置。
    double offsetX = 0.0;
    double offsetY = 0.0;
    if (state.fillMode == FillMode::Fill) {
        // 裁切偏移语义（SlotImageState 注释）：-1 贴左/上，0 居中，+1 贴右/下。
        if (scaled.width() > slotWidth)
            offsetX = -(scaled.width() - slotWidth) * (state.cropOffsetX + 1.0) / 2.0;
        if (scaled.height() > slotHeight)
            offsetY = -(scaled.height() - slotHeight) * (state.cropOffsetY + 1.0) / 2.0;
    } else {
        offsetX = (slotWidth - scaled.width()) / 2.0;
        offsetY = (slotHeight - scaled.height()) / 2.0;
    }

    painter.drawImage(QPointF(target.left() + offsetX, target.top() + offsetY), scaled);
}

} // namespace

QImage PageRenderer::renderPage(const ProjectPage& page, const SolidBackground& background,
                                qreal pageWidthMm, qreal pageHeightMm, int ppi)
{
    const QSize pixelSize(mmToPixels(pageWidthMm, ppi), mmToPixels(pageHeightMm, ppi));
    QImage canvas(pixelSize, QImage::Format_ARGB32);
    canvas.fill(QColor(background.colorHex));

    QPainter painter(&canvas);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    const int slotCount = qMin(page.layout.slotRects.size(), page.slotStates.size());
    for (int i = 0; i < slotCount; ++i) {
        const NormalizedRect& rect = page.layout.slotRects.at(i);
        const QRectF target(rect.x * pixelSize.width(), rect.y * pixelSize.height(),
                            rect.width * pixelSize.width(), rect.height * pixelSize.height());
        const SlotImageState& state = page.slotStates.at(i);

        if (state.imagePath.isEmpty()) {
            drawPlaceholder(painter, target, QStringLiteral("空槽位"));
            continue;
        }

        const ImageLoader::LoadedImage loaded = ImageLoader::loadOriented(state.imagePath);
        if (!loaded.ok) {
            // 读取失败：可见占位 + 不中断整页（可解释错误策略）。
            drawPlaceholder(painter, target, QStringLiteral("读取失败"));
            continue;
        }
        painter.save();
        painter.setClipRect(target);
        drawSlotImage(painter, loaded.image, target, state);
        painter.restore();
    }

    painter.end();
    return canvas;
}

} // namespace liusu::render
