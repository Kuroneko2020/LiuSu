#pragma once

#include <QRectF>
#include <QString>

namespace pte {

struct AutoLayoutDecision {
    bool fillCrop = true;
    int rotation = 0;
    bool mirrored = false;
};

class AutoLayoutPolicy {
public:
    static AutoLayoutDecision decide(const QString &preset, qreal imageAspect, const QRectF &slotRect);
};

} // namespace pte
