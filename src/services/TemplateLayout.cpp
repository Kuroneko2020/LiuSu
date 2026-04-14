#include "services/TemplateLayout.h"

namespace pte::layout {

QSizeF physicalSizeMm(int)
{
    return QSizeF(148.0, 100.0);
}

qreal pageAspectRatio(int templateChoice)
{
    const QSizeF mm = physicalSizeMm(templateChoice);
    return mm.height() > 0 ? mm.width() / mm.height() : 1.0;
}

QVector<QRectF> slotRectsNormalized(int templateChoice)
{
    const QSizeF paperMm = physicalSizeMm(templateChoice);
    const qreal pw = paperMm.width();
    const qreal ph = paperMm.height();
    auto mm = [&](qreal x, qreal y, qreal w, qreal h) {
        return QRectF(x / pw, y / ph, w / pw, h / ph);
    };

    // Two-up: fixed reference layout
    // page: 148x100 mm, margins: L/R=7 mm, T/B=5 mm, gutter=14 mm
    // slots: left(7,5,60,90), right(81,5,60,90)
    if (templateChoice == 2) {
        return {
            mm(7.0, 5.0, 60.0, 90.0),
            mm(81.0, 5.0, 60.0, 90.0)
        };
    }
    if (templateChoice == 4) {
        return {
            mm(4.0, 3.0, 66.0, 44.0),
            mm(78.0, 3.0, 66.0, 44.0),
            mm(4.0, 53.0, 66.0, 44.0),
            mm(78.0, 53.0, 66.0, 44.0)
        };
    }
    if (templateChoice == 9) {
        return {
            mm(0.6666667, 0.6666667, 48.0, 32.0),
            mm(50.0, 0.6666667, 48.0, 32.0),
            mm(99.3333333, 0.6666667, 48.0, 32.0),
            mm(0.6666667, 34.0, 48.0, 32.0),
            mm(50.0, 34.0, 48.0, 32.0),
            mm(99.3333333, 34.0, 48.0, 32.0),
            mm(0.6666667, 67.3333333, 48.0, 32.0),
            mm(50.0, 67.3333333, 48.0, 32.0),
            mm(99.3333333, 67.3333333, 48.0, 32.0)
        };
    }
    return {
        mm(7.0, 5.0, 60.0, 90.0),
        mm(81.0, 5.0, 60.0, 90.0)
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
