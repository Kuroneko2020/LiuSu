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
    m_cacheRoot = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QStringLiteral("/photo-template-editor");
    cleanupCacheDir(m_cacheRoot + QStringLiteral("/cache"), 14);
    cleanupCacheDir(m_cacheRoot + QStringLiteral("/thumbs"), 14);
    cleanupCacheDir(m_cacheRoot + QStringLiteral("/slot-images"), 14);
    cleanupCacheDir(m_cacheRoot + QStringLiteral("/slot-previews"), 14);
}

ImageResource ImageService::normalizeAndCacheFile(const QString &path)
{
    if (path.isEmpty()) {
        return {};
    }
    return normalizeAndCache(path);
}

QList<ImageResource> ImageService::normalizeAndCacheFiles(const QStringList &paths)
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

    if (image.width() > m_previewMaxEdge || image.height() > m_previewMaxEdge) {
        image = image.scaled(m_previewMaxEdge, m_previewMaxEdge, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    const QString cacheDir = m_cacheRoot + QStringLiteral("/cache");
    QDir().mkpath(cacheDir);
    const QByteArray key = QStringLiteral("%1|%2|%3|%4")
                               .arg(resource.originalPath)
                               .arg(info.lastModified().toMSecsSinceEpoch())
                               .arg(image.width())
                               .arg(image.height())
                               .arg(m_previewMaxEdge)
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

void ImageService::setCacheRoot(const QString &cacheRoot)
{
    if (cacheRoot.isEmpty() || m_cacheRoot == cacheRoot) {
        return;
    }
    m_cacheRoot = cacheRoot;
}

QString ImageService::cacheRoot() const
{
    return m_cacheRoot;
}

void ImageService::setPreviewMaxEdge(int edge)
{
    m_previewMaxEdge = qBound(1024, edge, 4096);
}

int ImageService::previewMaxEdge() const
{
    return m_previewMaxEdge;
}

bool ImageService::clearCache() const
{
    bool ok = true;
    const QStringList dirs{
        m_cacheRoot + QStringLiteral("/cache"),
        m_cacheRoot + QStringLiteral("/thumbs"),
        m_cacheRoot + QStringLiteral("/slot-images"),
        m_cacheRoot + QStringLiteral("/slot-previews")
    };
    for (const QString &dirPath : dirs) {
        QDir dir(dirPath);
        if (!dir.exists()) {
            continue;
        }
        const auto files = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
        for (const QFileInfo &fi : files) {
            ok = QFile::remove(fi.absoluteFilePath()) && ok;
        }
    }
    return ok;
}

} // namespace pte
