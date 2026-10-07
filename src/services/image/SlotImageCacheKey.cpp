#include "SlotImageCacheKey.h"

#include "services/image/ImageLoader.h"

#include <QStringList>

namespace liusu::services {

QString SlotImageCacheKey::toString() const
{
    QStringList parts;
    parts << QStringLiteral("v%1").arg(renderVersion > 0 ? renderVersion
                                                         : ImageLoader::kRenderAlgorithmVersion)
          << imagePath
          << QString::number(fileSizeBytes)
          << QString::number(lastModifiedMs)
          << QString::number(previewMaxEdge)
          << QString::number(rotationDegrees)
          << (mirrored ? QStringLiteral("m1") : QStringLiteral("m0"))
          << (fillMode == FillMode::Fit ? QStringLiteral("fit") : QStringLiteral("fill"))
          << QString::number(cropOffsetX, 'f', 4)
          << QString::number(cropOffsetY, 'f', 4);
    return parts.join(QLatin1Char('|'));
}

} // namespace liusu::services
