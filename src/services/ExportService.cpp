#include "services/ExportService.h"

#include "models/ProjectState.h"

#include <QDate>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QPainter>
#include <QRegularExpression>

namespace pte {
namespace {

QVector<QRectF> templateRects(int templateChoice, const QSize &canvasSize)
{
    QVector<QRectF> rects;
    const qreal w = canvasSize.width();
    const qreal h = canvasSize.height();
    const qreal gap = qMax<qreal>(4.0, w * 0.005);
    const qreal margin = qMax<qreal>(8.0, w * 0.01);

    auto grid = [&](int cols, int rows) {
        const qreal cellW = (w - margin * 2 - gap * (cols - 1)) / cols;
        const qreal cellH = (h - margin * 2 - gap * (rows - 1)) / rows;
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                rects << QRectF(margin + c * (cellW + gap), margin + r * (cellH + gap), cellW, cellH);
            }
        }
    };

    if (templateChoice == 2) {
        grid(1, 2);
    } else if (templateChoice == 4) {
        grid(2, 2);
    } else {
        grid(3, 2);
    }
    return rects;
}

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

void drawImageInRect(QPainter &painter, const QImage &image, const QRectF &target, bool fillCrop)
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
            src = QRectF((image.width() - width) / 2.0, 0, width, image.height());
        } else {
            const qreal height = image.width() / targetRatio;
            src = QRectF(0, (image.height() - height) / 2.0, image.width(), height);
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

} // namespace

ExportService::ExportService(QObject *parent)
    : QObject(parent)
{
}

ExportService::Result ExportService::exportPages(const ProjectState &project, const Request &request) const
{
    Result result;

    const QDir outDir(request.outputDir);
    if (request.outputDir.isEmpty() || !outDir.exists()) {
        result.message = QStringLiteral("导出路径无效，请先选择有效目录。");
        return result;
    }

    QVector<int> pageIndexes;
    if (request.scope == Scope::CurrentPage) {
        if (project.currentPageIndex() >= 0) {
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
    const QSize exportSize(ppi * 6, ppi * 4);
    const QString extension = request.format.compare("PNG", Qt::CaseInsensitive) == 0 ? "png" : "jpg";

    int sequence = 1;
    for (int pageIndex : pageIndexes) {
        QImage canvas(exportSize, QImage::Format_ARGB32_Premultiplied);
        canvas.fill(Qt::white);

        QPainter painter(&canvas);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

        const int templateChoice = project.pageTemplateChoice(pageIndex);
        const QVector<QRectF> rects = templateRects(templateChoice, exportSize);

        QStringList usedNames;
        for (int slot = 0; slot < rects.size(); ++slot) {
            if (!project.pageSlotHasImage(pageIndex, slot)) {
                continue;
            }
            const QString path = project.pageSlotImagePath(pageIndex, slot);
            const QImage image = transformImage(path, project.pageSlotRotation(pageIndex, slot), project.pageSlotMirrored(pageIndex, slot));
            if (image.isNull()) {
                continue;
            }
            usedNames << QFileInfo(path).completeBaseName();
            drawImageInRect(painter, image, rects.at(slot), project.pageSlotFillCrop(pageIndex, slot));
        }

        if (request.cropMarks) {
            drawCropMarks(painter, exportSize, rects);
        }

        painter.end();

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
