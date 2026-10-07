#pragma once

#include "services/image/ImageSource.h"

#include <QCache>
#include <QImage>
#include <QString>

namespace liusu::services {

// 缩略图缓存：内存 + 磁盘两级（图片与缓存规则·缓存分层）。
//
// - key 由源身份 + previewMaxEdge + 旋转/镜像 + 渲染算法版本构成；
//   同路径替换文件后源身份变化，自动不命中旧缩略图；
// - 内存层有字节上限（cost = QImage::sizeInBytes()）；
// - 磁盘层写入固定前缀 `liusu-thumb-` 的 PNG；清理只删除本前缀文件，
//   绝不触碰用户原图、项目文件或目录内其它内容；
// - 不放大：源图短于 previewMaxEdge 时返回原尺寸。
class ThumbnailCache {
public:
    explicit ThumbnailCache(QString diskDir, qint64 maxMemoryBytes = 64 * 1024 * 1024);

    void setDiskDir(const QString& dir);   // 切换目录会清空内存层
    QString diskDir() const { return m_diskDir; }

    // 取缩略图（已应用 EXIF 方向 + 旋转 + 镜像，最长边不超过 previewMaxEdge）。
    // 源身份无效或解码失败时返回空 QImage。
    QImage thumbnail(const ImageSourceIdentity& source, int previewMaxEdge,
                     int rotationDegrees, bool mirrored);

    // 清空两级缓存；只删除磁盘目录内本缓存写入的 `liusu-thumb-` 前缀文件。
    void clearAll();

    static constexpr const char* kDiskFilePrefix = "liusu-thumb-";

private:
    QString diskPathForKey(const QString& key) const;
    QString makeKey(const ImageSourceIdentity& source, int previewMaxEdge,
                    int rotationDegrees, bool mirrored) const;
    QImage loadFromDisk(const QString& key) const;
    void saveToDisk(const QString& key, const QImage& image) const;

    QString m_diskDir;
    QCache<QString, QImage> m_mem;
};

} // namespace liusu::services
