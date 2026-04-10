#include "services/TemplateLayout.h"

namespace pte::layout {

QSizeF physicalSizeMm(int)
{
    return QSizeF(148.0, 100.0);
}

QVector<QRectF> slotRectsNormalized(int templateChoice)
{
    QVector<QRectF> rects;
    const qreal margin = 0.04;
    const qreal gap = 0.02;

    auto grid = [&](int cols, int rows) {
        const qreal w = (1.0 - margin * 2 - gap * (cols - 1)) / cols;
        const qreal h = (1.0 - margin * 2 - gap * (rows - 1)) / rows;
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                rects << QRectF(margin + c * (w + gap), margin + r * (h + gap), w, h);
            }
        }
    };

    if (templateChoice == 2) {
        grid(2, 1); // 左右并排
    } else if (templateChoice == 4) {
        grid(2, 2);
    } else {
        grid(3, 2);
    }
    return rects;
}

QVector<QRectF> slotRectsPixels(int templateChoice, const QSize &canvasSize)
{
    QVector<QRectF> out;
    const auto normalized = slotRectsNormalized(templateChoice);
    out.reserve(normalized.size());
    for (const auto &r : normalized) {
        out << QRectF(r.x() * canvasSize.width(), r.y() * canvasSize.height(),
                      r.width() * canvasSize.width(), r.height() * canvasSize.height());
    }
    return out;
}

} // namespace pte::layout
