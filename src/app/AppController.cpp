#include "app/AppController.h"
#include <QDir>

#include <QImageReader>
#include <QSettings>
#include <QStandardPaths>
#include <QUrl>
#include "services/AutoLayoutPolicy.h"
#include "services/TemplateLayout.h"

namespace pte {

namespace {
QString defaultPicturesDir()
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    return path.isEmpty() ? QStringLiteral(".") : path;
}

QString toLocalPath(const QString &pathOrUrl)
{
    const QUrl url(pathOrUrl);
    if (url.isLocalFile()) {
        return url.toLocalFile();
    }
    return pathOrUrl;
}

QStringList toLocalPaths(const QVariantList &pathsOrUrls)
{
    QStringList out;
    out.reserve(pathsOrUrls.size());
    for (const QVariant &value : pathsOrUrls) {
        const QString path = toLocalPath(value.toString());
        if (!path.isEmpty()) {
            out << path;
        }
    }
    return out;
}
}

AppController::AppController(QObject *parent)
    : QObject(parent)
{
    loadSettings();
    if (m_exportSettings.path.isEmpty()) {
        m_exportSettings.path = m_settings.defaultPath;
    }

    connect(&m_project, &ProjectState::slotsChanged, this, [this]() {
        if (m_lastThumbnailContentRevision == m_project.contentRevision()) {
            return;
        }
        m_lastThumbnailContentRevision = m_project.contentRevision();
        ++m_pageThumbnailRevision;
        emit thumbnailsChanged();
    });
    connect(&m_project, &ProjectState::pagesChanged, this, [this]() {
        ++m_pageThumbnailRevision;
        m_lastThumbnailContentRevision = m_project.contentRevision();
        emit thumbnailsChanged();
    });
}

ProjectState *AppController::project() { return &m_project; }

void AppController::startManualLayout(int choice)
{
    m_pendingTemplateChoice = choice;
    m_pendingAutoMode = false;
    m_pendingAutoFiles.clear();
    if (m_project.hasValidPages()) {
        emit requestConfirmNewSession();
        return;
    }
    m_project.startNewSession(toTemplateType(choice));
    emit requestNavigateToEditor();
}

void AppController::startAutoLayout(int choice)
{
    Q_UNUSED(choice)
    m_lastExportSuccess = false;
    m_lastExportMessage = QStringLiteral("请在首页使用文件选择对话框后再执行自动布局。");
    emit exportResultChanged();
}

void AppController::startAutoLayoutWithFiles(int choice, const QVariantList &fileUrls)
{
    m_pendingTemplateChoice = choice;
    m_pendingAutoMode = true;
    m_pendingAutoFiles = fileUrls;
    if (m_project.hasValidPages()) {
        emit requestConfirmNewSession();
        return;
    }

    m_project.startNewSession(toTemplateType(choice));

    const auto images = m_imageService.normalizeAndCacheFiles(toLocalPaths(fileUrls));
    if (images.isEmpty()) {
        m_lastExportSuccess = false;
        m_lastExportMessage = QStringLiteral("自动模式已取消：未选择图片。");
        emit exportResultChanged();
        return;
    }

    int imported = 0;
    while (imported < images.size()) {
        int targetSlot = m_project.findNextAvailableSlot();
        if (targetSlot < 0) {
            m_project.createPage(toTemplateType(m_project.currentTemplateChoice()));
            targetSlot = m_project.findNextAvailableSlot();
        }
        if (targetSlot < 0) {
            break;
        }
        const auto &resource = images.at(imported);
        m_project.assignImageToSlot(targetSlot, resource);

        QImageReader reader(resource.exportPath);
        const QSize sz = reader.size();
        const qreal imageAspect = sz.height() > 0 ? static_cast<qreal>(sz.width()) / sz.height() : 1.0;
        const auto decision = AutoLayoutPolicy::decide(m_settings.autoPreset, imageAspect, m_project.slotRectNormalized(targetSlot));
        m_project.configureSlot(targetSlot, decision.fillCrop, decision.rotation, decision.mirrored);
        ++imported;
    }

    if (m_exportSettings.path.isEmpty() || !QDir(m_exportSettings.path).exists()) {
        m_lastExportSuccess = false;
        m_lastExportMessage = QStringLiteral("自动模式失败：请先选择有效导出目录。");
        emit exportResultChanged();
        return;
    }

    m_exportSettings.scope = ExportService::Scope::Queue;
    runExport();
}


void AppController::confirmStartNewSession(bool accepted)
{
    if (!accepted) {
        m_pendingAutoMode = false;
        m_pendingAutoFiles.clear();
        return;
    }

    if (m_pendingAutoMode) {
        const QVariantList pendingFiles = m_pendingAutoFiles;
        m_pendingAutoMode = false;
        m_pendingAutoFiles.clear();
        startAutoLayoutWithFiles(m_pendingTemplateChoice, pendingFiles);
        return;
    } else {
        m_project.startNewSession(toTemplateType(m_pendingTemplateChoice));
        emit requestNavigateToEditor();
    }
    m_pendingAutoMode = false;
    m_pendingAutoFiles.clear();
}

void AppController::createBlankPage(int choice)
{
    m_project.createPage(toTemplateType(choice));
}

void AppController::importToSlot(int slotIndex)
{
    Q_UNUSED(slotIndex)
}

void AppController::importToSlotFromFile(int slotIndex, const QString &fileUrl)
{
    const auto resource = m_imageService.normalizeAndCacheFile(toLocalPath(fileUrl));
    if (resource.exportPath.isEmpty()) {
        return;
    }
    m_project.assignImageToSlot(slotIndex, resource);
}

void AppController::batchImport()
{
}

void AppController::batchImportFromFiles(const QVariantList &fileUrls)
{
    const auto images = m_imageService.normalizeAndCacheFiles(toLocalPaths(fileUrls));
    if (images.isEmpty()) {
        return;
    }

    int imported = 0;
    while (imported < images.size()) {
        int targetSlot = m_project.findNextAvailableSlot();
        if (targetSlot < 0) {
            m_project.createPage(toTemplateType(m_project.currentTemplateChoice()));
            targetSlot = m_project.findNextAvailableSlot();
        }

        if (targetSlot < 0) {
            break;
        }

        m_project.assignImageToSlot(targetSlot, images.at(imported));
        ++imported;
    }
}

void AppController::exportCurrentPage()
{
    m_exportSettings.scope = ExportService::Scope::CurrentPage;
    emit exportSettingsChanged();
    emit requestNavigateToExport();
}

void AppController::exportQueue()
{
    m_exportSettings.scope = ExportService::Scope::Queue;
    emit exportSettingsChanged();
    emit requestNavigateToExport();
}

void AppController::chooseExportPath()
{
}

void AppController::setExportPathFromDialog(const QString &folderUrl)
{
    const QString dir = toLocalPath(folderUrl);
    if (dir.isEmpty()) {
        return;
    }
    setExportPath(dir);
    if (m_settings.rememberPath) {
        setDefaultExportPath(dir);
    }
}

void AppController::runExport()
{
    ExportService::Request req;
    req.outputDir = m_exportSettings.path;
    req.format = m_exportSettings.format;
    req.namingRule = m_exportSettings.naming;
    req.resolutionPreset = m_exportSettings.resolution;
    req.customPpi = m_exportSettings.customPpi;
    req.cropMarks = m_exportSettings.cropMarks;
    req.scope = m_exportSettings.scope;

    const auto result = m_exportService.exportPages(m_project, req);
    m_lastExportSuccess = result.success;
    m_lastExportMessage = result.message;
    emit exportResultChanged();

    if (result.success && m_settings.rememberPath) {
        setDefaultExportPath(m_exportSettings.path);
    }
}

QString AppController::pageThumbnailSource(int pageIndex)
{
    constexpr int thumbWidth = 220;
    const int thumbHeight = qMax(1, qRound(static_cast<qreal>(thumbWidth) / layout::pageAspectRatio(2)));
    const QString path = m_exportService.renderPageThumbnail(m_project, pageIndex, thumbWidth, thumbHeight);
    if (path.isEmpty()) {
        return {};
    }
    return QUrl::fromLocalFile(path).toString() + QStringLiteral("?v=%1").arg(m_pageThumbnailRevision);
}

QString AppController::slotPreviewSource(int slotIndex, int width, int height)
{
    const QString path = m_exportService.renderSlotPreview(m_project, m_project.currentPageIndex(), slotIndex, width, height);
    if (path.isEmpty()) {
        return {};
    }
    return QUrl::fromLocalFile(path).toString();
}

QVariantList AppController::templateSlotRects(int choice) const
{
    QVariantList out;
    const auto rects = layout::slotRectsNormalized(choice);
    out.reserve(rects.size());
    for (const QRectF &r : rects) {
        QVariantMap m;
        m.insert(QStringLiteral("x"), r.x());
        m.insert(QStringLiteral("y"), r.y());
        m.insert(QStringLiteral("width"), r.width());
        m.insert(QStringLiteral("height"), r.height());
        out << m;
    }
    return out;
}

QString AppController::exportPath() const { return m_exportSettings.path; }
void AppController::setExportPath(const QString &value) { if (m_exportSettings.path == value) return; m_exportSettings.path = value; emit exportSettingsChanged(); }
QString AppController::exportFormat() const { return m_exportSettings.format; }
void AppController::setExportFormat(const QString &value) { if (m_exportSettings.format == value) return; m_exportSettings.format = value; emit exportSettingsChanged(); }
QString AppController::namingRule() const { return m_exportSettings.naming; }
void AppController::setNamingRule(const QString &value) { if (m_exportSettings.naming == value) return; m_exportSettings.naming = value; emit exportSettingsChanged(); }
QString AppController::resolutionPreset() const { return m_exportSettings.resolution; }
void AppController::setResolutionPreset(const QString &value) { if (m_exportSettings.resolution == value) return; m_exportSettings.resolution = value; emit exportSettingsChanged(); }
int AppController::customPpi() const { return m_exportSettings.customPpi; }
void AppController::setCustomPpi(int value) { if (m_exportSettings.customPpi == value) return; m_exportSettings.customPpi = value; emit exportSettingsChanged(); }
bool AppController::cropMarks() const { return m_exportSettings.cropMarks; }
void AppController::setCropMarks(bool value) { if (m_exportSettings.cropMarks == value) return; m_exportSettings.cropMarks = value; emit exportSettingsChanged(); }
QString AppController::exportScope() const { return m_exportSettings.scope == ExportService::Scope::Queue ? QStringLiteral("全队列") : QStringLiteral("当前页"); }

QString AppController::lastExportMessage() const { return m_lastExportMessage; }
bool AppController::lastExportSuccess() const { return m_lastExportSuccess; }

QString AppController::autoLayoutPreset() const { return m_settings.autoPreset; }
qreal AppController::pageAspectRatio() const { return layout::pageAspectRatio(2); }
int AppController::pageThumbnailRevision() const { return m_pageThumbnailRevision; }
void AppController::setAutoLayoutPreset(const QString &value) { if (m_settings.autoPreset == value) return; m_settings.autoPreset = value; persistExportDefaults(); emit appSettingsChanged(); }
QString AppController::defaultExportPath() const { return m_settings.defaultPath; }
void AppController::setDefaultExportPath(const QString &value) { if (m_settings.defaultPath == value) return; m_settings.defaultPath = value; persistExportDefaults(); emit appSettingsChanged(); }
bool AppController::rememberLastPath() const { return m_settings.rememberPath; }
void AppController::setRememberLastPath(bool value) { if (m_settings.rememberPath == value) return; m_settings.rememberPath = value; persistExportDefaults(); emit appSettingsChanged(); }
QString AppController::defaultExportFormat() const { return m_settings.defaultFormat; }
void AppController::setDefaultExportFormat(const QString &value) { if (m_settings.defaultFormat == value) return; m_settings.defaultFormat = value; m_exportSettings.format = value; persistExportDefaults(); emit appSettingsChanged(); emit exportSettingsChanged(); }
QString AppController::defaultExportResolution() const { return m_settings.defaultResolution; }
void AppController::setDefaultExportResolution(const QString &value) { if (m_settings.defaultResolution == value) return; m_settings.defaultResolution = value; m_exportSettings.resolution = value; persistExportDefaults(); emit appSettingsChanged(); emit exportSettingsChanged(); }
int AppController::defaultCustomPpi() const { return m_settings.defaultCustomPpi; }
void AppController::setDefaultCustomPpi(int value) { if (m_settings.defaultCustomPpi == value) return; m_settings.defaultCustomPpi = qBound(72, value, 1200); m_exportSettings.customPpi = m_settings.defaultCustomPpi; persistExportDefaults(); emit appSettingsChanged(); emit exportSettingsChanged(); }
bool AppController::defaultCropMarks() const { return m_settings.defaultCrop; }
void AppController::setDefaultCropMarks(bool value) { if (m_settings.defaultCrop == value) return; m_settings.defaultCrop = value; m_exportSettings.cropMarks = value; persistExportDefaults(); emit appSettingsChanged(); emit exportSettingsChanged(); }
QString AppController::themePlaceholder() const { return m_settings.theme; }
void AppController::setThemePlaceholder(const QString &value) { if (m_settings.theme == value) return; m_settings.theme = value; emit appSettingsChanged(); }

TemplateType AppController::toTemplateType(int choice)
{
    if (choice == 4) return TemplateType::FourUp;
    if (choice == 9) return TemplateType::NineUp;
    return TemplateType::TwoUp;
}

void AppController::loadSettings()
{
    QSettings settings(QStringLiteral("PhotoTemplateEditor"), QStringLiteral("PhotoTemplateEditor"));
    m_settings.autoPreset = settings.value(QStringLiteral("auto/preset"), QStringLiteral("均衡填充")).toString();
    m_settings.defaultPath = settings.value(QStringLiteral("export/defaultPath"), defaultPicturesDir()).toString();
    m_settings.rememberPath = settings.value(QStringLiteral("export/rememberLastPath"), true).toBool();
    m_settings.defaultFormat = settings.value(QStringLiteral("export/defaultFormat"), QStringLiteral("JPG")).toString();
    m_settings.defaultResolution = settings.value(QStringLiteral("export/defaultResolution"), QStringLiteral("300 PPI")).toString();
    m_settings.defaultCrop = settings.value(QStringLiteral("export/defaultCropMarks"), false).toBool();
    m_settings.defaultCustomPpi = settings.value(QStringLiteral("export/defaultCustomPpi"), 300).toInt();

    m_exportSettings.path = m_settings.defaultPath;
    m_exportSettings.format = m_settings.defaultFormat;
    m_exportSettings.resolution = m_settings.defaultResolution;
    m_exportSettings.cropMarks = m_settings.defaultCrop;
    m_exportSettings.customPpi = m_settings.defaultCustomPpi;
}

void AppController::persistExportDefaults() const
{
    QSettings settings(QStringLiteral("PhotoTemplateEditor"), QStringLiteral("PhotoTemplateEditor"));
    settings.setValue(QStringLiteral("auto/preset"), m_settings.autoPreset);
    settings.setValue(QStringLiteral("export/defaultPath"), m_settings.defaultPath);
    settings.setValue(QStringLiteral("export/rememberLastPath"), m_settings.rememberPath);
    settings.setValue(QStringLiteral("export/defaultFormat"), m_settings.defaultFormat);
    settings.setValue(QStringLiteral("export/defaultResolution"), m_settings.defaultResolution);
    settings.setValue(QStringLiteral("export/defaultCropMarks"), m_settings.defaultCrop);
    settings.setValue(QStringLiteral("export/defaultCustomPpi"), m_settings.defaultCustomPpi);
}

} // namespace pte
