#include "PageRenderer.h"

#include "domain/Units.h"
#include "services/image/ImageLoader.h"

#include <QFont>
#include <QPainter>
#include <QPainterPath>

namespace liusu::render {

using liusu::domain::FillMode;
using liusu::domain::NormalizedRect;
using liusu::domain::SlotImageState;
using liusu::domain::mmToPixels;
using liusu::services::ImageLoader;

namespace {

// ---- 槽位占位（空槽位 / 读取失败）------------------------------------------
//
// 为什么画在渲染层而不是编辑页 QML：占位是页面内容的一部分，预览、缩略图与
// 导出必须是同一份结果（尺寸与渲染规则）。只在编辑页叠装饰会让导出与屏幕
// 显示不一致。
//
// 纵深做法对照 RhineLabUI：不用假倾斜，用"下暗上亮"的垂直渐变 + 受光边/背光边
// 做厚度与凹凸。所有尺寸按槽位短边等比推导，换 PPI 只改像素密度、不变形。
const QColor kBaseTop(0xf0, 0xf1, 0xea);
const QColor kBaseBottom(0xe5, 0xe8, 0xdd);
const QColor kMarkTop(0x92, 0x9c, 0x87);
const QColor kMarkBottom(0x92, 0x9c, 0x87);
const QColor kMarkTopError(0xf6, 0xe0, 0xd8);
const QColor kMarkBottomError(0xcf, 0xa0, 0x96);

// 槽位底座：上亮下暗的渐变 + 四边内斜面（左上受光、右下背光）。
void drawSlotBase(QPainter& painter, const QRectF& target)
{
    const qreal unit = qMin(target.width(), target.height());
    const qreal bevel = qMax(1.0, unit * 0.003);

    QLinearGradient base(target.topLeft(), target.bottomLeft());
    base.setColorAt(0.0, kBaseTop);
    base.setColorAt(1.0, kBaseBottom);
    painter.fillRect(target, base);

    painter.setPen(Qt::NoPen);
    painter.fillRect(QRectF(target.left(), target.top(), target.width(), bevel),
                     QColor(255, 255, 255, 220));
    painter.fillRect(QRectF(target.left(), target.top(), bevel, target.height()),
                     QColor(255, 255, 255, 180));
    painter.fillRect(QRectF(target.left(), target.bottom() - bevel, target.width(), bevel),
                     QColor(28, 25, 18, 62));
    painter.fillRect(QRectF(target.right() - bevel, target.top(), bevel, target.height()),
                     QColor(28, 25, 18, 50));

    painter.setPen(QColor(28, 25, 18, 31));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(target.adjusted(0, 0, -1, -1));
}

// 玻璃态符号：主体渐变 + 挤出暗侧 + 轮廓发丝线，做成一片薄玻璃。
// unit 取槽位短边，加号与叹号共用同一份厚度/边缘比例，两个符号才像一套。
//
// 为什么不做内斜面高光月牙：十字是非凸轮廓，"原轮廓减去平移后的轮廓"这条月牙
// 会在凹角处跨过两臂，留下一道横穿的接缝条纹，读起来像两根条叠在一起。
// 改用整圈发丝线给边缘定义（RhineLabUI 的精密仪器语汇），不留接缝。
void drawGlassMark(QPainter& painter, const QPainterPath& mark, qreal unit,
                   const QColor& top, const QColor& bottom)
{
    const qreal extrude = qMax(1.0, unit * 0.014);

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);

    // 挤出暗侧：向右下偏移的极浅副本，只给一点点厚度，不做投影。
    painter.translate(extrude, extrude);
    painter.fillPath(mark, QColor(28, 25, 18, 26));
    painter.translate(-extrude, -extrude);

    // 主体：下暗上亮的渐变（对照 RhineLabUI 的面 AO）。
    const QRectF box = mark.boundingRect();
    QLinearGradient body(box.topLeft(), box.bottomLeft());
    body.setColorAt(0.0, top);
    body.setColorAt(1.0, bottom);
    painter.fillPath(mark, body);

    // 轮廓发丝线：先把两臂并成一条闭合轮廓再描边——直接描原始路径会把两臂
    // 交叉处的内部线条也画出来。
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor(28, 25, 18, 42), qMax(1.0, unit * 0.005),
                        Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.strokePath(mark.simplified(), painter.pen());
    painter.restore();
}

// 加号：两条细臂，中心即槽位中心。臂长/臂宽约 13:1，端头只做小圆角——
// 圆角开到半宽会变成胶囊，读起来像医疗十字，不像"添加"。
// 必须用 WindingFill：默认的 OddEvenFill 会把两臂重叠的中心挖成空洞，
// 加号只剩四段残臂（本函数最易踩的坑）。
QPainterPath plusMark(const QRectF& target, qreal unit)
{
    const QPointF c = target.center();
    const qreal arm = unit * 0.16;
    const qreal thick = unit * 0.012;
    const qreal r = thick * 0.35;

    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addRoundedRect(QRectF(c.x() - arm / 2, c.y() - thick / 2, arm, thick), r, r);
    path.addRoundedRect(QRectF(c.x() - thick / 2, c.y() - arm / 2, thick, arm), r, r);
    return path;
}

// 叹号：竖臂 + 下方圆点（读取失败用，与加号同一套玻璃语汇）。
// 整体上移，把槽位下半部让给"读取失败"说明，圆点不得与文字相碰。
QPainterPath bangMark(const QRectF& target, qreal unit)
{
    const QPointF c = target.center();
    const qreal thick = unit * 0.064;
    const qreal r = thick * 0.35;

    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addRoundedRect(QRectF(c.x() - thick / 2, c.y() - unit * 0.25, thick, unit * 0.19), r, r);
    const qreal dot = unit * 0.062;
    path.addRoundedRect(QRectF(c.x() - dot / 2, c.y() + unit * 0.005, dot, dot), dot / 2, dot / 2);
    return path;
}

// 空槽位：暖灰底座 + 克制的细加号，与照片工作台保持同一色温。
void drawEmptySlot(QPainter& painter, const QRectF& target)
{
    const qreal unit = qMin(target.width(), target.height());
    painter.save();
    drawSlotBase(painter, target);
    drawGlassMark(painter, plusMark(target, unit), unit, kMarkTop, kMarkBottom);
    painter.restore();
}

// 读取失败：同一套 3D 底座 + 玻璃态叹号，加一行可解释说明（可解释错误策略）。
void drawErrorSlot(QPainter& painter, const QRectF& target, const QString& label)
{
    const qreal unit = qMin(target.width(), target.height());
    painter.save();
    drawSlotBase(painter, target);
    drawGlassMark(painter, bangMark(target, unit), unit, kMarkTopError, kMarkBottomError);

    if (!label.isEmpty()) {
        QFont font = painter.font();
        font.setPixelSize(qMax(9.0, unit * 0.095));
        font.setBold(true);
        painter.setFont(font);
        painter.setPen(QColor(0xb3, 0x40, 0x2e));
        // 说明落在叹号下方留白里，与圆点保持间距（圆点底在 +0.073·unit）。
        painter.drawText(QRectF(target.left(), target.center().y() + unit * 0.13,
                                target.width(), unit * 0.20),
                         Qt::AlignHCenter | Qt::AlignTop, label);
    }
    painter.restore();
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
            // 空槽位仍走共享渲染管线；只降低装饰权重，不改槽位几何。
            drawEmptySlot(painter, target);
            continue;
        }

        const ImageLoader::LoadedImage loaded = ImageLoader::loadOriented(state.imagePath);
        if (!loaded.ok) {
            // 读取失败：可见占位 + 不中断整页（可解释错误策略）。
            drawErrorSlot(painter, target, QStringLiteral("读取失败"));
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
