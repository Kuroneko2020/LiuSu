#include "services/ImageService.h"

namespace pte {

ImageService::ImageService(QObject *parent)
    : QObject(parent)
{
}

QString ImageService::importSinglePlaceholder()
{
    return QStringLiteral("placeholder://image");
}

QStringList ImageService::importBatchPlaceholder(int count)
{
    QStringList list;
    for (int i = 0; i < count; ++i) {
        list << QStringLiteral("placeholder://batch/%1").arg(i + 1);
    }
    return list;
}

QStringList ImageService::supportedInputFormats() const
{
    return {
        QStringLiteral("JPG"), QStringLiteral("JPEG"), QStringLiteral("PNG"), QStringLiteral("WebP"),
        QStringLiteral("BMP"), QStringLiteral("TIFF"), QStringLiteral("HEIC"), QStringLiteral("HEIF")
    };
}

} // namespace pte
