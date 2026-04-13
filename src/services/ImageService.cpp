#include <algorithm>
#include "services/ImageService.h"

#include <QCollator>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QImageWriter>
#include <QStandardPaths>

namespace pte {

namespace {
void cleanupCacheDir(const QString &dirPath, int days)
{
    QDir dir(dirPath);
    if (!dir.exists()) {
        return;
    }
    const QDateTime threshold = QDateTime::currentDateTimeUtc().addDays(-days);
    const auto files = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    for (const auto &fi : files) {
        if (fi.lastModified().toUTC() < threshold) {
            QFile::remove(fi.absoluteFilePath());
        }
    }
}
}

ImageService::ImageService(QObject *parent)
    : QObject(parent)
{
    const QString tempRoot = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QStringLiteral("/photo-template-editor");
    cleanupCacheDir(tempRoot + QStringLiteral("/cache"), 14);
    cleanupCacheDir(tempRoot + QStringLiteral("/thumbs"), 14);
}

ImageResource ImageService::normalizeAndCacheFile(const QString &path) const
{
    if (path.isEmpty()) {
        return {};
    }
    return normalizeAndCache(path);
}

QList<ImageResource> ImageService::normalizeAndCacheFiles(const QStringList &paths) const
{
    if (paths.isEmpty()) {
        return {};
    }

    QStringList orderedPaths = paths;
    QCollator collator;
    collator.setNumericMode(true);
    std::sort(orderedPaths.begin(), orderedPaths.end(), [&collator](const QString &a, const QString &b) {
        return collator.compare(QFileInfo(a).fileName(), QFileInfo(b).fileName()) < 0;
    });

    QList<ImageResource> valid;
    for (const auto &path : orderedPaths) {
        const ImageResource resource = normalizeAndCache(path);
        if (!resource.exportPath.isEmpty()) {
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

    QFileInfo info(path);

    ImageResource resource;
    resource.originalPath = info.absoluteFilePath();
    resource.originalBaseName = info.completeBaseName();
    resource.exportPath = info.absoluteFilePath();

    QImage image = reader.read();
    if (image.isNull()) {
        resource.previewPath = info.absoluteFilePath();
        return resource;
    }

    const int maxPreviewEdge = 2200;
    if (image.width() > maxPreviewEdge || image.height() > maxPreviewEdge) {
        image = image.scaled(maxPreviewEdge, maxPreviewEdge, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    const QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QStringLiteral("/photo-template-editor/cache");
    QDir().mkpath(cacheDir);
    const QByteArray key = QStringLiteral("%1|%2|%3|%4")
                               .arg(resource.originalPath)
                               .arg(info.lastModified().toMSecsSinceEpoch())
                               .arg(image.width())
                               .arg(image.height())
                               .toUtf8();
    const QString digest = QString::fromUtf8(QCryptographicHash::hash(key, QCryptographicHash::Sha1).toHex());
    const QString previewPath = cacheDir + QStringLiteral("/preview_%1.jpg").arg(digest);
    if (!QFileInfo::exists(previewPath)) {
        QImageWriter writer(previewPath, "JPG");
        writer.setQuality(92);
        if (!writer.write(image)) {
            resource.previewPath = info.absoluteFilePath();
            return resource;
        }
    }
    resource.previewPath = previewPath;
    return resource;
}

} // namespace pte
