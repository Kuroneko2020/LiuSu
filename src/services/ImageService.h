#pragma once

#include "models/ImageResource.h"

#include <QObject>
#include <QSize>
#include <QStringList>

namespace pte {

class ImageService : public QObject {
    Q_OBJECT
public:
    explicit ImageService(QObject *parent = nullptr);

    ImageResource normalizeAndCacheFile(const QString &path);
    QList<ImageResource> normalizeAndCacheFiles(const QStringList &paths);
    ImageResource refreshResource(const ImageResource &resource) const;
    [[nodiscard]] QString transformedPreviewPath(const ImageResource &resource, int rotation, bool mirrored) const;
    [[nodiscard]] QSize transformedPreviewSize(const ImageResource &resource, int rotation) const;
    void setCacheRoot(const QString &cacheRoot);
    [[nodiscard]] QString cacheRoot() const;
    void setPreviewMaxEdge(int edge);
    [[nodiscard]] int previewMaxEdge() const;
    bool clearCache() const;
    void cleanupExpiredCache(int retentionDays) const;

    [[nodiscard]] QStringList supportedInputFormats() const;
    [[nodiscard]] QString fileDialogFilter() const;
    [[nodiscard]] QString heifSupportNote() const;

private:
    [[nodiscard]] ImageResource normalizeAndCache(const QString &path) const;
    [[nodiscard]] QString transformedCacheDir() const;
    [[nodiscard]] QString previewCacheDir() const;
    QString m_cacheRoot;
    int m_previewMaxEdge = 2048;
};

} // namespace pte
