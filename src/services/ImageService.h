#pragma once

#include "models/ImageResource.h"

#include <QObject>
#include <QStringList>

namespace pte {

class ImageService : public QObject {
    Q_OBJECT
public:
    explicit ImageService(QObject *parent = nullptr);

    ImageResource normalizeAndCacheFile(const QString &path) const;
    QList<ImageResource> normalizeAndCacheFiles(const QStringList &paths) const;

    [[nodiscard]] QStringList supportedInputFormats() const;
    [[nodiscard]] QString fileDialogFilter() const;
    [[nodiscard]] QString heifSupportNote() const;

private:
    [[nodiscard]] ImageResource normalizeAndCache(const QString &path) const;
};

} // namespace pte
