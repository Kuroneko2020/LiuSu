#include "PreviewController.h"

#include "domain/Units.h"
#include "services/render/PageRenderer.h"

#include <QDir>
#include <QImage>
#include <QStandardPaths>
#include <QUrl>

liusu::domain::ProjectDocument PreviewController::buildDemoProject()
{
    using liusu::domain::FillMode;
    using liusu::domain::ProjectPage;
    using liusu::domain::SlotImageState;

    // 演示页：四宫格 + 4 张不同 EXIF 方向的测试图，
    // 分别展示旋转 / 镜像 / 铺满裁切 / 完整放入等编辑状态。
    liusu::domain::ProjectDocument doc = liusu::domain::ProjectDocument::createDefault();
    ProjectPage& page = doc.pages[0];
    page.layout = liusu::domain::LayoutPresets::create(QStringLiteral("four"));
    page.slotStates = {
        SlotImageState{ QStringLiteral(":/images/exif1.jpg"), 0, false, FillMode::Fill, 0.0, 0.0 },
        SlotImageState{ QStringLiteral(":/images/exif6.jpg"), 0, false, FillMode::Fill, 0.0, 0.0 },
        SlotImageState{ QStringLiteral(":/images/exif3.jpg"), 90, false, FillMode::Fit, 0.0, 0.0 },
        SlotImageState{ QStringLiteral(":/images/exif8.jpg"), 0, true, FillMode::Fill, -0.5, 0.0 },
    };
    return doc;
}

QString PreviewController::writePreviewPng(const QImage& image)
{
    const QDir base(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    if (!base.mkpath(QStringLiteral(".")))
        return {};
    const QString path = base.filePath(QStringLiteral("preview-page.png"));
    if (!image.save(path, "PNG"))
        return {};
    return QUrl::fromLocalFile(path).toString();
}

PreviewController::PreviewController(QObject* parent)
    : QObject(parent)
{
    const liusu::domain::ProjectDocument doc = buildDemoProject();
    // 预览用 144 PPI（屏幕观感密度）；导出档位 300/600 在子方案 04/06 走同一入口。
    const QImage page = liusu::render::PageRenderer::renderPage(
        doc.pages.first(), doc.background, 148.0, 100.0, 144);
    m_previewUrl = writePreviewPng(page);
}
