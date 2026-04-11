#include "services/TemplateLayout.h"

namespace pte::layout {

QSizeF physicalSizeMm(int)
{
    return QSizeF(148.0, 100.0);
}

QVector<QRectF> slotRectsNormalized(int templateChoice)
{
    const QSizeF paperMm = physicalSizeMm(templateChoice);
    const qreal pw = paperMm.width();
    const qreal ph = paperMm.height();
    auto mm = [&](qreal x, qreal y, qreal w, qreal h) {
        return QRectF(x / pw, y / ph, w / pw, h / ph);
    };

    if (templateChoice == 2) {
        return {
            mm(11.667, 4.0, 61.333, 92.0),
            mm(75.0, 4.0, 61.333, 92.0)
        };
    }
    if (templateChoice == 4) {
        return {
            mm(5.5, 4.0, 67.5, 45.0),
            mm(75.0, 4.0, 67.5, 45.0),
            mm(5.5, 51.0, 67.5, 45.0),
            mm(75.0, 51.0, 67.5, 45.0)
        };
    }
    return {
        mm(27.0, 4.0, 30.0, 45.0),
        mm(59.0, 4.0, 30.0, 45.0),
        mm(91.0, 4.0, 30.0, 45.0),
        mm(27.0, 51.0, 30.0, 45.0),
        mm(59.0, 51.0, 30.0, 45.0),
        mm(91.0, 51.0, 30.0, 45.0)
    };
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
