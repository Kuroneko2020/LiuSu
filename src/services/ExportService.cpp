#include "services/ExportService.h"

#include "models/ProjectState.h"
#include "services/TemplateLayout.h"

#include <QDate>
#include <QDateTime>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QPainter>
#include <QPointF>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUrl>
#include <cmath>
#include <limits>

namespace pte {
namespace {

QString sanitizeName(const QString &input)
{
    QString out = input;
    out.replace(QRegularExpression(QStringLiteral("[\\\\/:*?\"<>|]+")), QStringLiteral("_"));
    out.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral("_"));
    out = out.trimmed();
    if (out.isEmpty()) {
        out = QStringLiteral("export");
    }
    if (out.length() > 80) {
        out = out.left(80);
    }
    return out;
}

QString uniquePath(const QDir &dir, const QString &baseName, const QString &ext)
{
    QString candidate = dir.filePath(baseName + "." + ext);
    int idx = 1;
    while (QFileInfo::exists(candidate)) {
        candidate = dir.filePath(QStringLiteral("%1_%2.%3").arg(baseName).arg(idx++).arg(ext));
    }
    return candidate;
}

bool isWritableDirectory(const QDir &dir)
{
    const QFileInfo info(dir.absolutePath());
    return info.exists() && info.isDir() && info.isWritable();
}

QSize readOrientedImageSize(const QString &path)
{
    QImageReader reader(path);
    reader.setAutoTransform(true);
    const QImage image = reader.read();
    return image.isNull() ? QSize{} : image.size();
}

void applyPhysicalResolution(QImage &image, int templateChoice)
{
    const QSizeF mm = layout::physicalSizeMm(templateChoice);
    if (image.isNull() || mm.width() <= 0.0 || mm.height() <= 0.0) {
        return;
    }
    image.setDotsPerMeterX(qMax(1, qRound(image.width() * 1000.0 / mm.width())));
    image.setDotsPerMeterY(qMax(1, qRound(image.height() * 1000.0 / mm.height())));
}

QString pageThumbnailCacheKey(const ProjectState &project, int pageIndex, int width, int height)
{
    QString key = QStringLiteral("%1|%2x%3|%4")
                      .arg(pageIndex)
                      .arg(width)
                      .arg(height)
                      .arg(project.pageTemplateChoice(pageIndex));
    key += QStringLiteral("|bg:%1|%2|%3")
               .arg(project.pageBackgroundMode(pageIndex))
               .arg(project.pageBackgroundColor(pageIndex).name(QColor::HexArgb))
               .arg(project.pageBackgroundTexturePath(pageIndex));
    const int slotCountOnPage = project.pageSlotCount(pageIndex);
    for (int slot = 0; slot < slotCountOnPage; ++slot) {
        key += QStringLiteral("|%1:%2:%3:%4:%5:%6:%7")
                   .arg(slot)
                   .arg(project.pageSlotHasImage(pageIndex, slot) ? 1 : 0)
                   .arg(project.pageSlotPreviewPath(pageIndex, slot))
                   .arg(project.pageSlotRotation(pageIndex, slot))
                   .arg(project.pageSlotMirrored(pageIndex, slot) ? 1 : 0)
                   .arg(project.pageSlotFillCrop(pageIndex, slot) ? 1 : 0)
                   .arg(QStringLiteral("%1,%2").arg(project.pageSlotOffset(pageIndex, slot).x(), 0, 'f', 4).arg(project.pageSlotOffset(pageIndex, slot).y(), 0, 'f', 4));
    }
    return key;
}

QImage transformImage(const QString &path, int rotation, bool mirrored)
{
    QImageReader reader(path);
    reader.setAutoTransform(true);
    QImage image = reader.read();
    if (image.isNull()) {
        return image;
    }

    QTransform transform;
    if (mirrored) {
        transform.scale(-1, 1);
    }
    if (rotation != 0) {
        transform.rotate(rotation);
    }

    if (!transform.isIdentity()) {
        image = image.transformed(transform, Qt::SmoothTransformation);
    }
    return image;
}

void drawImageInRect(QPainter &painter, const QImage &image, const QRectF &target, bool fillCrop, const QPointF &offset)
{
    if (image.isNull()) {
        return;
    }

    if (fillCrop) {
        const qreal targetRatio = target.width() / target.height();
        const qreal srcRatio = static_cast<qreal>(image.width()) / image.height();
        QRectF src;
        if (srcRatio > targetRatio) {
            const qreal width = image.height() * targetRatio;
            const qreal range = image.width() - width;
            const qreal shift = -qBound(-1.0, offset.x(), 1.0) * range * 0.5;
            src = QRectF((range / 2.0) + shift, 0, width, image.height());
        } else {
            const qreal height = image.width() / targetRatio;
            const qreal range = image.height() - height;
            const qreal shift = -qBound(-1.0, offset.y(), 1.0) * range * 0.5;
            src = QRectF(0, (range / 2.0) + shift, image.width(), height);
        }
        painter.drawImage(target, image, src);
    } else {
        QSizeF scaled = image.size();
        scaled.scale(target.size(), Qt::KeepAspectRatio);
        const QRectF fitted(target.x() + (target.width() - scaled.width()) / 2.0,
                            target.y() + (target.height() - scaled.height()) / 2.0,
                            scaled.width(),
                            scaled.height());
        painter.drawImage(fitted, image);
    }
}

void drawCropMarks(QPainter &painter, const QSize &size, const QVector<QRectF> &slotRects)
{
    QPen pen(QColor("#333333"));
    pen.setWidthF(1.5);
    painter.setPen(pen);

    const qreal mark = qMin(size.width(), size.height()) * 0.02;
    const QRectF pageRect(0, 0, size.width(), size.height());

    auto corner = [&](qreal x, qreal y, qreal dx, qreal dy) {
        painter.drawLine(QPointF(x, y), QPointF(x + dx * mark, y));
        painter.drawLine(QPointF(x, y), QPointF(x, y + dy * mark));
    };

    corner(pageRect.left(), pageRect.top(), 1, 1);
    corner(pageRect.right(), pageRect.top(), -1, 1);
    corner(pageRect.left(), pageRect.bottom(), 1, -1);
    corner(pageRect.right(), pageRect.bottom(), -1, -1);

    for (const auto &r : slotRects) {
        painter.drawLine(QPointF(r.left(), r.top()), QPointF(r.left() + mark * 0.5, r.top()));
        painter.drawLine(QPointF(r.right(), r.top()), QPointF(r.right() - mark * 0.5, r.top()));
        painter.drawLine(QPointF(r.left(), r.bottom()), QPointF(r.left() + mark * 0.5, r.bottom()));
        painter.drawLine(QPointF(r.right(), r.bottom()), QPointF(r.right() - mark * 0.5, r.bottom()));
    }
}

QImage renderPageImage(const ProjectState &project, int pageIndex, const QSize &size, bool cropMarks, bool usePreviewCache)
{
    QImage canvas(size, QImage::Format_ARGB32_Premultiplied);
    const QString bgMode = project.pageBackgroundMode(pageIndex);
    const QColor bgColor = project.pageBackgroundColor(pageIndex).isValid() ? project.pageBackgroundColor(pageIndex) : QColor(QStringLiteral("#FFFFFF"));
    canvas.fill(bgMode == QStringLiteral("color") ? bgColor : Qt::white);

    QPainter painter(&canvas);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const int templateChoice = project.pageTemplateChoice(pageIndex);
    const QVector<QRectF> rects = layout::slotRectsPixels(templateChoice, size);

    if (bgMode == QStringLiteral("texture")) {
        QString texturePath = project.pageBackgroundTexturePath(pageIndex);
        const QUrl textureUrl(texturePath);
        if (textureUrl.isLocalFile()) {
            texturePath = textureUrl.toLocalFile();
        }
        QImage texture(texturePath);
        if (!texture.isNull()) {
            painter.drawImage(QRectF(0, 0, size.width(), size.height()), texture);
        } else {
            painter.fillRect(QRectF(0, 0, size.width(), size.height()), bgColor);
        }
    }

    for (int slot = 0; slot < rects.size(); ++slot) {
        if (!project.pageSlotHasImage(pageIndex, slot)) {
            continue;
        }
        const QString path = usePreviewCache
            ? project.pageSlotPreviewPath(pageIndex, slot)
            : project.pageSlotImagePath(pageIndex, slot);
        const QImage image = usePreviewCache
            ? transformImage(path, 0, false)
            : transformImage(path, project.pageSlotRotation(pageIndex, slot), project.pageSlotMirrored(pageIndex, slot));
        if (image.isNull()) {
            continue;
        }
        drawImageInRect(painter, image, rects.at(slot), project.pageSlotFillCrop(pageIndex, slot), project.pageSlotOffset(pageIndex, slot));
    }

    if (cropMarks) {
        drawCropMarks(painter, size, rects);
    }

    painter.end();
    return canvas;
}

QSize resolveOriginalQualityPageSize(const ProjectState &project, int pageIndex, const QSize &fallback)
{
    const int templateChoice = project.pageTemplateChoice(pageIndex);
    const auto normalizedRects = layout::slotRectsNormalized(templateChoice);
    if (normalizedRects.isEmpty()) {
        return fallback;
    }

    qreal maxScale = std::numeric_limits<qreal>::max();
    bool hasImage = false;
    const qreal pageAspect = layout::pageAspectRatio(templateChoice);

    for (int slot = 0; slot < normalizedRects.size(); ++slot) {
        if (!project.pageSlotHasImage(pageIndex, slot)) {
            continue;
        }
        hasImage = true;

        QSize srcSize = project.pageSlotOrientedImageSize(pageIndex, slot);
        if (srcSize.isEmpty()) {
            srcSize = readOrientedImageSize(project.pageSlotImagePath(pageIndex, slot));
        }
        if (srcSize.isEmpty()) {
            continue;
        }
        const int rot = ((project.pageSlotRotation(pageIndex, slot) % 360) + 360) % 360;
        if (rot == 90 || rot == 270) {
            srcSize.transpose();
        }

        const QRectF &slotRect = normalizedRects.at(slot);
        const qreal targetRatio = layout::slotAspectRatio(templateChoice, slotRect);
        const qreal srcRatio = srcSize.height() > 0 ? (static_cast<qreal>(srcSize.width()) / srcSize.height()) : targetRatio;
        qreal usedW = srcSize.width();
        qreal usedH = srcSize.height();
        if (project.pageSlotFillCrop(pageIndex, slot)) {
            if (srcRatio > targetRatio) {
                usedW = srcSize.height() * targetRatio;
            } else {
                usedH = srcSize.width() / targetRatio;
            }
        }
        const qreal scaleX = usedW / qMax(1e-6, slotRect.width() * pageAspect);
        const qreal scaleY = usedH / qMax(1e-6, slotRect.height());
        const qreal slotMaxScale = qMax(1.0, qMin(scaleX, scaleY));
        maxScale = qMin(maxScale, slotMaxScale);
    }

    if (!hasImage || !std::isfinite(maxScale)) {
        return fallback;
    }

    const int pageH = qMax(1, qFloor(maxScale));
    const int pageW = qMax(1, qRound(pageH * pageAspect));
    return QSize(pageW, pageH);
}

} // namespace

ExportService::ExportService(QObject *parent)
    : QObject(parent)
{
    m_cacheRoot = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QStringLiteral("/liusu");
}

ExportService::Result ExportService::exportPages(const ProjectState &project, const Request &request) const
{
    Result result;

    const QDir outDir(request.outputDir);
    if (request.outputDir.isEmpty() || !outDir.exists()) {
        result.message = QStringLiteral("导出路径无效，请先选择有效目录。");
        return result;
    }
    if (!isWritableDirectory(outDir)) {
        result.message = QStringLiteral("导出路径不可写，请选择有写入权限的目录。");
        return result;
    }

    QVector<int> pageIndexes;
    if (request.scope == Scope::CurrentPage) {
        if (project.currentPageIndex() >= 0 && project.isPageValid(project.currentPageIndex())) {
            pageIndexes << project.currentPageIndex();
        }
    } else {
        for (int i = 0; i < project.pageCount(); ++i) {
            if (project.isPageValid(i)) {
                pageIndexes << i;
            }
        }
    }

    if (pageIndexes.isEmpty()) {
        result.message = QStringLiteral("没有可导出的有效页面。");
        return result;
    }

    const int ppi = resolvePpi(request);
    const QSizeF mm = layout::physicalSizeMm(2);
    const QSize fallbackSize(qRound(mm.width() / 25.4 * ppi), qRound(mm.height() / 25.4 * ppi));
    const QString extension = request.originalQuality ? QStringLiteral("png")
        : (request.format.compare("PNG", Qt::CaseInsensitive) == 0 ? QStringLiteral("png") : QStringLiteral("jpg"));

    int sequence = 1;
    for (int pageIndex : pageIndexes) {
        const QSize exportSize = request.originalQuality
            ? resolveOriginalQualityPageSize(project, pageIndex, fallbackSize)
            : fallbackSize;
        QImage canvas = renderPageImage(project, pageIndex, exportSize, request.cropMarks, false);
        applyPhysicalResolution(canvas, project.pageTemplateChoice(pageIndex));

        QStringList usedNames;
        for (int slot = 0; slot < project.pageSlotCount(pageIndex); ++slot) {
            if (project.pageSlotHasImage(pageIndex, slot)) {
                const QString rawName = project.pageSlotOriginalBaseName(pageIndex, slot);
                if (!rawName.isEmpty()) {
                    usedNames << rawName;
                } else {
                    usedNames << QFileInfo(project.pageSlotImagePath(pageIndex, slot)).completeBaseName();
                }
            }
        }

        QString baseName;
        if (request.namingRule == QStringLiteral("日期-序号")) {
            baseName = QStringLiteral("%1_%2").arg(QDate::currentDate().toString("yyyyMMdd")).arg(sequence, 3, 10, QLatin1Char('0'));
        } else {
            baseName = sanitizeName(usedNames.isEmpty() ? QStringLiteral("page_%1").arg(pageIndex + 1) : usedNames.join("_"));
        }

        const QString target = uniquePath(outDir, baseName, extension);
        const bool saved = canvas.save(target, extension.toUpper().toUtf8().constData(), extension == "jpg" ? 95 : -1);
        if (!saved) {
            result.message = QStringLiteral("导出失败：无法写入 %1").arg(target);
            return result;
        }

        result.exportedFiles << target;
        ++sequence;
    }

    result.success = true;
    result.message = QStringLiteral("导出成功，共 %1 个文件。").arg(result.exportedFiles.size());
    return result;
}

QString ExportService::renderPageThumbnail(const ProjectState &project, int pageIndex, int width, int height) const
{
    if (pageIndex < 0 || pageIndex >= project.pageCount()) {
        return {};
    }
    const QString key = pageThumbnailCacheKey(project, pageIndex, width, height);
    if (m_pageThumbnailCacheByKey.contains(key)) {
        const QString cachedPath = m_pageThumbnailCacheByKey.value(key);
        if (QFileInfo::exists(cachedPath)) {
            return cachedPath;
        }
    }

    const QImage image = renderPageImage(project, pageIndex, QSize(width, height), false, true);

    const QString dir = thumbsCacheDir();
    QDir().mkpath(dir);
    const QString digest = QString::fromUtf8(QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Sha1).toHex());
    const QString path = dir + QStringLiteral("/page_%1_%2.png").arg(pageIndex).arg(digest);
    image.save(path, "PNG");
    m_pageThumbnailCacheByKey.insert(key, path);
    return path;
}

QString ExportService::renderSlotPreview(const ProjectState &project, int pageIndex, int slotIndex, int width, int height) const
{
    if (pageIndex < 0 || pageIndex >= project.pageCount() || !project.pageSlotHasImage(pageIndex, slotIndex)) {
        return {};
    }

    const int w = qMax(16, width);
    const int h = qMax(16, height);
    const QString path = project.pageSlotPreviewPath(pageIndex, slotIndex);
    const int rotation = project.pageSlotRotation(pageIndex, slotIndex);
    const bool mirrored = project.pageSlotMirrored(pageIndex, slotIndex);
    const bool fillCrop = project.pageSlotFillCrop(pageIndex, slotIndex);
    const QPointF offset = project.pageSlotOffset(pageIndex, slotIndex);
    const QFileInfo sourceInfo(path);
    const QByteArray key = QStringLiteral("%1|%2|%3|%4|%5|%6|%7|%8|%9|%10|%11")
                               .arg(path)
                               .arg(sourceInfo.size())
                               .arg(sourceInfo.lastModified().toMSecsSinceEpoch())
                               .arg(pageIndex)
                               .arg(slotIndex)
                               .arg(rotation)
                               .arg(mirrored ? 1 : 0)
                               .arg(fillCrop ? 1 : 0)
                               .arg(offset.x(), 0, 'f', 4)
                               .arg(offset.y(), 0, 'f', 4)
                               .arg(QStringLiteral("%1x%2").arg(w).arg(h))
                               .toUtf8();
    const QString digest = QString::fromUtf8(QCryptographicHash::hash(key, QCryptographicHash::Sha1).toHex());

    const QString dir = slotPreviewCacheDir();
    QDir().mkpath(dir);
    const QString previewPath = dir + QStringLiteral("/%1.png").arg(digest);
    if (QFileInfo::exists(previewPath)) {
        return previewPath;
    }

    QImage canvas(QSize(w, h), QImage::Format_ARGB32_Premultiplied);
    canvas.fill(Qt::white);
    QPainter painter(&canvas);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    const QImage image = transformImage(path, 0, false);
    drawImageInRect(painter, image, QRectF(0, 0, w, h), fillCrop, offset);
    painter.end();
    canvas.save(previewPath, "PNG");
    return previewPath;
}

void ExportService::clearThumbnailCache()
{
    m_pageThumbnailCacheByKey.clear();
    const QStringList dirs{thumbsCacheDir(), slotPreviewCacheDir()};
    for (const QString &dirPath : dirs) {
        QDir dir(dirPath);
        if (!dir.exists()) {
            continue;
        }
        const auto files = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
        for (const QFileInfo &fi : files) {
            QFile::remove(fi.absoluteFilePath());
        }
    }
}

void ExportService::setCacheRoot(const QString &cacheRoot)
{
    if (cacheRoot.isEmpty() || m_cacheRoot == cacheRoot) {
        return;
    }
    m_cacheRoot = cacheRoot;
    QDir().mkpath(thumbsCacheDir());
    QDir().mkpath(slotPreviewCacheDir());
}

QString ExportService::cacheRoot() const
{
    return m_cacheRoot;
}

QString ExportService::thumbsCacheDir() const
{
    const QString dir = m_cacheRoot + QStringLiteral("/thumbs");
    QDir().mkpath(dir);
    return dir;
}

QString ExportService::slotPreviewCacheDir() const
{
    const QString dir = m_cacheRoot + QStringLiteral("/slot-previews");
    QDir().mkpath(dir);
    return dir;
}

int ExportService::resolvePpi(const Request &request) const
{
    if (request.resolutionPreset.contains(QStringLiteral("600"))) {
        return 600;
    }
    if (request.resolutionPreset.contains(QStringLiteral("自定义"))) {
        return qBound(72, request.customPpi, 1200);
    }
    return 300;
}

} // namespace pte
