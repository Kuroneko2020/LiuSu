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

QString ImageService::importSingleImage()
{
    const QString path = QFileDialog::getOpenFileName(nullptr, QStringLiteral("选择一张图片"), QString(), fileDialogFilter());
    if (path.isEmpty()) {
        return {};
    }
    return normalizeAndCache(path);
}

QStringList ImageService::importMultipleImages()
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

    QStringList validPaths;
    for (const auto &path : paths) {
        const QString normalized = normalizeAndCache(path);
        if (!normalized.isEmpty()) {
            validPaths << normalized;
        }
    }
    return validPaths;
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

QString ImageService::normalizeAndCache(const QString &path) const
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

    const QString baseDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QStringLiteral("/photo-template-editor/cache");
    QDir().mkpath(baseDir);

    const QByteArray digest = QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Sha1).toHex();
    const QString outPath = baseDir + QStringLiteral("/%1.png").arg(QString::fromUtf8(digest));
    image.save(outPath, "PNG");
    return outPath;
}

} // namespace pte
