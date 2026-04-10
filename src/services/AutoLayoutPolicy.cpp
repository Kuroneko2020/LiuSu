#include "services/AutoLayoutPolicy.h"

#include <QtMath>

namespace pte {

AutoLayoutDecision AutoLayoutPolicy::decide(const QString &preset, qreal imageAspect, const QRectF &slotRect)
{
    const qreal slotAspect = slotRect.width() > 0 && slotRect.height() > 0 ? slotRect.width() / slotRect.height() : 1.0;

    AutoLayoutDecision d;
    if (preset == QStringLiteral("证件照优先")) {
        d.fillCrop = false;
        return d;
    }

    if (preset == QStringLiteral("人像优先")) {
        d.fillCrop = true;
        if (imageAspect > 1.05) {
            d.rotation = 90;
        }
        return d;
    }

    const qreal mismatch = qMax(imageAspect / slotAspect, slotAspect / imageAspect);
    d.fillCrop = mismatch < 1.5;
    return d;
}

} // namespace pte
