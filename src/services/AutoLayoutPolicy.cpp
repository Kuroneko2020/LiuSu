#include "services/AutoLayoutPolicy.h"

#include <QtMath>

namespace pte {

namespace {
constexpr qreal kPortraitRotateThreshold = 1.05;
constexpr qreal kBalancedMismatchThreshold = 1.5;
}

AutoLayoutDecision AutoLayoutPolicy::decide(const QString &preset, const QString &fillStrategy, const QString &orientationPolicy, qreal imageAspect, const QRectF &slotRect)
{
    const qreal slotAspect = slotRect.width() > 0 && slotRect.height() > 0 ? slotRect.width() / slotRect.height() : 1.0;

    AutoLayoutDecision d;
    if (fillStrategy == QStringLiteral("原图完整放入") || preset == QStringLiteral("证件照优先")) {
        d.fillCrop = false;
    } else {
        d.fillCrop = true;
    }

    const qreal mismatch = qMax(imageAspect / slotAspect, slotAspect / imageAspect);
    if (preset == QStringLiteral("均衡填充")) {
        d.fillCrop = mismatch < kBalancedMismatchThreshold;
    }
    if (orientationPolicy == QStringLiteral("自动右转 90°")) {
        const bool imageLandscape = imageAspect >= 1.0;
        const bool slotLandscape = slotAspect >= 1.0;
        if (imageLandscape != slotLandscape || (preset == QStringLiteral("人像优先") && imageAspect > kPortraitRotateThreshold)) {
            d.rotation = 90;
        }
    }
    return d;
}

} // namespace pte
