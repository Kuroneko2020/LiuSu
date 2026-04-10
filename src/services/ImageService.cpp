#include <algorithm>
#include "services/ImageService.h"

#include <QCollator>
#include <QFileDialog>
#include <QFileInfo>
#include <QImageReader>

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

    QImageReader reader(path);
    reader.setAutoTransform(true);
    if (!reader.canRead()) {
        return {};
    }

    // 通过 QImageReader 的自动变换保证 EXIF 方向已在此阶段修正。
    if (reader.read().isNull()) {
        return {};
    }
    return path;
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
    validPaths.reserve(paths.size());
    for (const auto &path : paths) {
        QImageReader reader(path);
        reader.setAutoTransform(true);
        if (!reader.canRead()) {
            continue;
        }
        if (!reader.read().isNull()) {
            validPaths << path;
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

} // namespace pte
