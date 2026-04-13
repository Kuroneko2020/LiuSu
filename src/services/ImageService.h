#pragma once

#include "models/ImageResource.h"

#include <QObject>
#include <QStringList>

namespace pte {

class ImageService : public QObject {
    Q_OBJECT
public:
    explicit ImageService(QObject *parent = nullptr);

    ImageResource normalizeAndCacheFile(const QString &path);
    QList<ImageResource> normalizeAndCacheFiles(const QStringList &paths);
    void setCacheRoot(const QString &cacheRoot);
    [[nodiscard]] QString cacheRoot() const;
    void setPreviewMaxEdge(int edge);
    [[nodiscard]] int previewMaxEdge() const;
    bool clearCache() const;

    [[nodiscard]] QStringList supportedInputFormats() const;
    [[nodiscard]] QString fileDialogFilter() const;
    [[nodiscard]] QString heifSupportNote() const;

private:
    [[nodiscard]] ImageResource normalizeAndCache(const QString &path) const;
    QString m_cacheRoot;
    int m_previewMaxEdge = 2048;
};

} // namespace pte
