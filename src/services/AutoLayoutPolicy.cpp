#include "services/AutoLayoutPolicy.h"

#include <QtMath>

namespace pte {

namespace {
constexpr qreal kPortraitRotateThreshold = 1.05;
constexpr qreal kBalancedMismatchThreshold = 1.5;
}

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
        if (imageAspect > kPortraitRotateThreshold) {
            d.rotation = 90;
        }
        return d;
    }

    const qreal mismatch = qMax(imageAspect / slotAspect, slotAspect / imageAspect);
    d.fillCrop = mismatch < kBalancedMismatchThreshold;
    return d;
}

} // namespace pte
