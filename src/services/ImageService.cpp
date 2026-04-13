#include <algorithm>
#include "services/ImageService.h"

#include <QCollator>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QImageWriter>
#include <QStandardPaths>
#include <QTransform>

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
    cleanupCacheDir(m_cacheRoot + QStringLiteral("/transformed"), 14);
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

ImageResource ImageService::refreshResource(const ImageResource &resource) const
{
    if (!resource.originalPath.isEmpty()) {
        return normalizeAndCache(resource.originalPath);
    }
    return {};
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
    resource.originalLastModifiedMs = info.lastModified().toMSecsSinceEpoch();

    QImage image = reader.read();
    if (image.isNull()) {
        resource.previewPath = info.absoluteFilePath();
        resource.previewWidth = 0;
        resource.previewHeight = 0;
        return resource;
    }

    if (image.width() > m_previewMaxEdge || image.height() > m_previewMaxEdge) {
        image = image.scaled(m_previewMaxEdge, m_previewMaxEdge, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    const QString cacheDir = previewCacheDir();
    const QByteArray key = QStringLiteral("%1|%2|%3|%4|%5")
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
    resource.previewWidth = image.width();
    resource.previewHeight = image.height();
    return resource;
}

QString ImageService::transformedPreviewPath(const ImageResource &resource, int rotation, bool mirrored) const
{
    const QString basePath = resource.previewPath.isEmpty() ? resource.exportPath : resource.previewPath;
    if (basePath.isEmpty()) {
        return {};
    }
    const int normalizedRotation = ((rotation % 360) + 360) % 360;
    if (normalizedRotation == 0 && !mirrored) {
        return basePath;
    }

    const QFileInfo info(basePath);
    const qint64 stamp = resource.originalLastModifiedMs > 0
        ? resource.originalLastModifiedMs
        : info.lastModified().toMSecsSinceEpoch();
    const QByteArray key = QStringLiteral("%1|%2|%3|%4|%5")
                               .arg(resource.originalPath.isEmpty() ? info.absoluteFilePath() : resource.originalPath)
                               .arg(stamp)
                               .arg(m_previewMaxEdge)
                               .arg(normalizedRotation)
                               .arg(mirrored ? 1 : 0)
                               .toUtf8();
    const QString digest = QString::fromUtf8(QCryptographicHash::hash(key, QCryptographicHash::Sha1).toHex());
    const QString outPath = transformedCacheDir() + QStringLiteral("/tx_%1.png").arg(digest);
    if (QFileInfo::exists(outPath)) {
        return outPath;
    }

    QImageReader reader(basePath);
    reader.setAutoTransform(true);
    QImage image = reader.read();
    if (image.isNull()) {
        return basePath;
    }
    QTransform transform;
    if (mirrored) {
        transform.scale(-1, 1);
    }
    if (normalizedRotation != 0) {
        transform.rotate(normalizedRotation);
    }
    if (!transform.isIdentity()) {
        image = image.transformed(transform, Qt::SmoothTransformation);
    }
    image.save(outPath, "PNG");
    return outPath;
}

QSize ImageService::transformedPreviewSize(const ImageResource &resource, int rotation) const
{
    QSize size(resource.previewWidth, resource.previewHeight);
    if (size.isEmpty()) {
        QImageReader reader(resource.previewPath.isEmpty() ? resource.exportPath : resource.previewPath);
        size = reader.size();
    }
    if ((((rotation % 360) + 360) % 180) == 90) {
        size.transpose();
    }
    return size;
}

void ImageService::setCacheRoot(const QString &cacheRoot)
{
    if (cacheRoot.isEmpty() || m_cacheRoot == cacheRoot) {
        return;
    }
    m_cacheRoot = cacheRoot;
    QDir().mkpath(previewCacheDir());
    QDir().mkpath(transformedCacheDir());
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
        m_cacheRoot + QStringLiteral("/transformed"),
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

QString ImageService::transformedCacheDir() const
{
    const QString dir = m_cacheRoot + QStringLiteral("/transformed");
    QDir().mkpath(dir);
    return dir;
}

QString ImageService::previewCacheDir() const
{
    const QString dir = m_cacheRoot + QStringLiteral("/cache");
    QDir().mkpath(dir);
    return dir;
}

} // namespace pte
