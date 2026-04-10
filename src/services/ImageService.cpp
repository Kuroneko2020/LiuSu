#include <algorithm>
#include "services/ImageService.h"

#include <QCollator>
#include <QCryptographicHash>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QImageReader>
#include <QStandardPaths>

namespace pte {

ImageService::ImageService(QObject *parent)
    : QObject(parent)
{
}

ImageResource ImageService::importSingleImage()
{
    const QString path = QFileDialog::getOpenFileName(nullptr, QStringLiteral("选择一张图片"), QString(), fileDialogFilter());
    if (path.isEmpty()) {
        return {};
    }
    return normalizeAndCache(path);
}

QList<ImageResource> ImageService::importMultipleImages()
{
    QStringList paths = QFileDialog::getOpenFileNames(nullptr, QStringLiteral("批量导入图片"), QString(), fileDialogFilter());
    if (paths.isEmpty()) {
        return {};
    }

    QCollator collator;
    collator.setNumericMode(true);
    std::sort(paths.begin(), paths.end(), [&collator](const QString &a, const QString &b) {
        return collator.compare(QFileInfo(a).fileName(), QFileInfo(b).fileName()) < 0;
    });

    QList<ImageResource> valid;
    for (const auto &path : paths) {
        const ImageResource resource = normalizeAndCache(path);
        if (!resource.cachePath.isEmpty()) {
            valid << resource;
        }
    }
    return valid;
}

QStringList ImageService::supportedInputFormats() const
{
    return {
        QStringLiteral("JPG"), QStringLiteral("JPEG"), QStringLiteral("PNG"), QStringLiteral("WebP"),
        QStringLiteral("BMP"), QStringLiteral("TIFF"), QStringLiteral("HEIC"), QStringLiteral("HEIF")
    };
}

QString ImageService::fileDialogFilter() const
{
    return QStringLiteral("Images (*.jpg *.jpeg *.png *.webp *.bmp *.tif *.tiff *.heic *.heif)");
}

QString ImageService::heifSupportNote() const
{
    return QStringLiteral("HEIC/HEIF 依赖系统解码插件，若当前平台无插件会导入失败。");
}

ImageResource ImageService::normalizeAndCache(const QString &path) const
{
    QImageReader reader(path);
    reader.setAutoTransform(true);
    if (!reader.canRead()) {
        return {};
    }

    const QImage image = reader.read();
    if (image.isNull()) {
        return {};
    }

    QFileInfo info(path);
    const QString cacheKey = QStringLiteral("%1|%2|%3")
        .arg(info.absoluteFilePath())
        .arg(info.size())
        .arg(info.lastModified().toMSecsSinceEpoch());

    const QString baseDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QStringLiteral("/photo-template-editor/cache");
    QDir().mkpath(baseDir);

    const QByteArray digest = QCryptographicHash::hash(cacheKey.toUtf8(), QCryptographicHash::Sha1).toHex();
    const QString outPath = baseDir + QStringLiteral("/%1.png").arg(QString::fromUtf8(digest));
    image.save(outPath, "PNG");

    ImageResource resource;
    resource.originalPath = info.absoluteFilePath();
    resource.originalBaseName = info.completeBaseName();
    resource.cachePath = outPath;
    return resource;
}

} // namespace pte
