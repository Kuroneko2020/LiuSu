#include "ThumbnailCache.h"

#include "services/image/ImageLoader.h"
#include "services/image/SlotImageCacheKey.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QImageReader>
#include <QImageWriter>
#include <QSaveFile>
#include <QTransform>

namespace liusu::services {

ThumbnailCache::ThumbnailCache(QString diskDir, qint64 maxMemoryBytes)
    : m_diskDir(std::move(diskDir))
{
    m_mem.setMaxCost(static_cast<int>(qMin<long long>(maxMemoryBytes, 1024LL * 1024 * 1024)));
}

void ThumbnailCache::setDiskDir(const QString& dir)
{
    if (m_diskDir == dir)
        return;
    m_diskDir = dir;
    m_mem.clear();
}

QString ThumbnailCache::makeKey(const ImageSourceIdentity& source, int previewMaxEdge,
                                int rotationDegrees, bool mirrored) const
{
    // 缩略图是槽位图的前段（不含填充/裁切，那是页面合成阶段的输入，子方案 04）。
    SlotImageCacheKey key;
    key.imagePath = source.absolutePath;
    key.fileSizeBytes = source.fileSizeBytes;
    key.lastModifiedMs = source.lastModifiedMs;
    key.previewMaxEdge = previewMaxEdge;
    key.rotationDegrees = rotationDegrees;
    key.mirrored = mirrored;
    // fillMode / cropOffset 不影响缩略图像素，取默认值即可（结构体字段仍被
    // 完整测试覆盖，供 04 的槽位合成 key 复用）。
    return QStringLiteral("thumb|") + key.toString();
}

QString ThumbnailCache::diskPathForKey(const QString& key) const
{
    const QByteArray hash =
        QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Sha1).toHex();
    return m_diskDir + QLatin1Char('/') + QString::fromLatin1(kDiskFilePrefix) + QLatin1String(hash);
}

QImage ThumbnailCache::loadFromDisk(const QString& key) const
{
    const QString path = diskPathForKey(key);
    if (!QFileInfo::exists(path))
        return {};
    QImageReader reader(path);
    reader.setAutoTransform(false); // 缓存文件由本类写出，无方向元数据
    return reader.read();
}

void ThumbnailCache::saveToDisk(const QString& key, const QImage& image) const
{
    if (m_diskDir.isEmpty() || image.isNull())
        return;
    QDir dir;
    if (!dir.mkpath(m_diskDir))
        return;
    QSaveFile file(diskPathForKey(key));
    if (!file.open(QIODevice::WriteOnly))
        return;
    QImageWriter writer(&file, "PNG");
    if (!writer.write(image) || !file.commit())
        return; // 缓存写入失败是最佳努力，不影响主流程
}

QImage ThumbnailCache::thumbnail(const ImageSourceIdentity& source, int previewMaxEdge,
                                 int rotationDegrees, bool mirrored)
{
    if (!source.isValid() || previewMaxEdge <= 0)
        return {};

    const QString key = makeKey(source, previewMaxEdge, rotationDegrees, mirrored);

    if (const QImage* hit = m_mem.object(key))
        return *hit;
    const QImage fromDisk = loadFromDisk(key);
    if (!fromDisk.isNull()) {
        const qint64 cost = fromDisk.sizeInBytes();
        m_mem.insert(key, new QImage(fromDisk), static_cast<int>(qMin<qint64>(cost, 1024 * 1024 * 1024)));
        return fromDisk;
    }

    const ImageLoader::LoadedImage loaded = ImageLoader::loadOriented(source.absolutePath);
    if (!loaded.ok)
        return {};

    QImage img = loaded.image;
    if (rotationDegrees != 0) {
        QTransform transform;
        transform.rotate(rotationDegrees);
        img = img.transformed(transform, Qt::SmoothTransformation);
    }
    if (mirrored)
        img = img.mirrored(true, false);

    // 只缩小不放大；最长边约束 previewMaxEdge。
    const int maxEdge = qMax(img.width(), img.height());
    if (maxEdge > previewMaxEdge) {
        const qreal scale = static_cast<qreal>(previewMaxEdge) / static_cast<qreal>(maxEdge);
        img = img.scaled(qRound(img.width() * scale), qRound(img.height() * scale),
                         Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    const qint64 cost = img.sizeInBytes();
    m_mem.insert(key, new QImage(img), static_cast<int>(qMin<qint64>(cost, 1024 * 1024 * 1024)));
    saveToDisk(key, img);
    return img;
}

void ThumbnailCache::clearAll()
{
    m_mem.clear();
    if (m_diskDir.isEmpty())
        return;
    const QDir dir(m_diskDir);
    if (!dir.exists())
        return;
    const auto entries = dir.entryList(QStringList{ QString::fromLatin1(kDiskFilePrefix) +QLatin1String("*") },
                                       QDir::Files);
    for (const QString& entry : entries)
        QFile::remove(dir.filePath(entry));
}

} // namespace liusu::services
