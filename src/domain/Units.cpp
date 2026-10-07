#include "Units.h"

#include <QtMath>

namespace liusu::domain {

namespace {
// 归一化坐标的浮点比较容差。3x3 预设这类 1/3 步长值累加后
// 可能与 1.0 产生最低位误差，比较时必须放行这个量级。
constexpr qreal kNormEpsilon = 1e-9;
} // namespace

bool NormalizedRect::isValid() const
{
    if (x < -kNormEpsilon || y < -kNormEpsilon)
        return false;
    if (width <= 0.0 || height <= 0.0)
        return false;
    if (x + width > 1.0 + kNormEpsilon || y + height > 1.0 + kNormEpsilon)
        return false;
    return true;
}

bool NormalizedRect::operator==(const NormalizedRect& other) const
{
    return qFuzzyCompare(x, other.x) && qFuzzyCompare(y, other.y)
        && qFuzzyCompare(width, other.width) && qFuzzyCompare(height, other.height);
}

qint64 mmToPixels(qreal mm, int ppi)
{
    Q_ASSERT(ppi > 0);
    return qRound64(mm / 25.4 * static_cast<qreal>(ppi));
}

qreal pixelsToMm(qint64 px, int ppi)
{
    Q_ASSERT(ppi > 0);
    return static_cast<qreal>(px) * 25.4 / static_cast<qreal>(ppi);
}

} // namespace liusu::domain
