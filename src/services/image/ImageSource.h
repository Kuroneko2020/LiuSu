#pragma once

#include <QFileInfo>
#include <QString>

namespace liusu::services {

// 图片源身份：缓存 key 的文件侧输入。
// 同一路径的文件被替换（内容或修改时间变化）后身份必然不同，
// 因此所有以身份参与的缓存 key 天然不会误命中旧图（图片与缓存规则）。
struct ImageSourceIdentity {
    QString absolutePath;
    qint64 fileSizeBytes = -1;
    qint64 lastModifiedMs = -1;

    bool isValid() const { return fileSizeBytes >= 0 && lastModifiedMs >= 0; }

    // 对不存在 / 不可访问的路径返回 isValid()==false 的身份。
    static ImageSourceIdentity stat(const QString& absolutePath)
    {
        ImageSourceIdentity identity;
        const QFileInfo info(absolutePath);
        if (!info.exists() || !info.isFile())
            return identity;
        identity.absolutePath = info.absoluteFilePath();
        identity.fileSizeBytes = info.size();
        identity.lastModifiedMs = info.lastModified().toMSecsSinceEpoch();
        return identity;
    }
};

} // namespace liusu::services
