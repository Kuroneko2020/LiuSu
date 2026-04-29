#pragma once

#include <QString>
#include <QtGlobal>

namespace pte {

struct AutoLayoutDecision {
    bool fillCrop = true;
    int rotation = 0;
    bool mirrored = false;
};

class AutoLayoutPolicy {
public:
    static AutoLayoutDecision decide(const QString &preset, const QString &fillStrategy, const QString &orientationPolicy, qreal imageAspect, qreal slotAspect);
};

} // namespace pte
