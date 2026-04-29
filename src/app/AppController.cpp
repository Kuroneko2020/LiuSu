#include "app/AppController.h"
#include <QDir>

#include <QDesktopServices>
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

QString ensureDir(const QString &path)
{
    if (path.isEmpty()) {
        return path;
    }
    QDir().mkpath(path);
    return path;
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

QString ppiPresetForValue(int ppi)
{
    if (ppi == 600) {
        return QStringLiteral("600 PPI");
    }
    if (ppi == 300) {
        return QStringLiteral("300 PPI");
    }
    return QStringLiteral("自定义 PPI");
}
}

AppController::AppController(QObject *parent)
    : QObject(parent)
{
    loadSettings();
    m_project.setImageService(&m_imageService);
    if (m_exportSettings.path.isEmpty()) {
        m_exportSettings.path = m_settings.defaultPath;
    }
    m_thumbnailEmitTimer.setSingleShot(true);
    m_thumbnailEmitTimer.setInterval(120);
    connect(&m_thumbnailEmitTimer, &QTimer::timeout, this, [this]() {
        emit thumbnailsChanged();
    });

    connect(&m_project, &ProjectState::slotsChanged, this, [this]() {
        markPageThumbnailDirty(m_project.currentPageIndex());
    });
    connect(&m_project, &ProjectState::pagesChanged, this, [this]() { handlePagesChanged(); });
    connect(&m_project, &ProjectState::backgroundChanged, this, [this]() {
        m_pageThumbnailCache.clear();
        m_dirtyThumbnailPages.clear();
        for (int i = 0; i < m_project.pageCount(); ++i) {
            markPageThumbnailDirty(i);
        }
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
    if (m_project.hasValidPages()) {
        m_pendingTemplateChoice = choice;
        m_pendingAutoMode = true;
        m_pendingAutoFiles = fileUrls;
        emit requestConfirmNewSession();
        return;
    }

    m_pendingAutoMode = false;
    m_pendingAutoFiles.clear();
    executeAutoLayoutWithFiles(choice, fileUrls);
}

void AppController::executeAutoLayoutWithFiles(int choice, const QVariantList &fileUrls)
{
    m_project.startNewSession(toTemplateType(choice));

    const auto images = m_imageService.normalizeAndCacheFiles(toLocalPaths(fileUrls));
    if (images.isEmpty()) {
        m_lastExportSuccess = false;
        m_lastExportMessage = QStringLiteral("自动模式已取消：未选择图片。");
        emit exportResultChanged();
        return;
    }

    m_deferThumbnailSignals = true;
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
        const qreal slotAspect = layout::slotAspectRatio(m_project.currentTemplateChoice(), m_project.slotRectNormalized(targetSlot));
        const auto decision = AutoLayoutPolicy::decide(m_settings.autoPreset, m_settings.autoFill, m_settings.autoOrientation, imageAspect, slotAspect);
        m_project.configureSlot(targetSlot, decision.fillCrop, decision.rotation, decision.mirrored);
        ++imported;
    }
    m_deferThumbnailSignals = false;
    emit thumbnailsChanged();

    if (m_exportSettings.path.isEmpty() || !QDir(m_exportSettings.path).exists()) {
        m_lastExportSuccess = false;
        m_lastExportMessage = QStringLiteral("自动模式失败：请先选择有效导出目录。");
        emit exportResultChanged();
        return;
    }

    m_exportSettings.scope = ExportService::Scope::Queue;
    m_exportSettings.naming = m_settings.autoNamingRule;
    m_exportSettings.cropMarks = m_settings.autoCropMarks;
    m_exportSettings.format = m_settings.defaultFormat;
    m_exportSettings.resolution = ppiPresetForValue(m_settings.autoDefaultPpi);
    m_exportSettings.customPpi = m_settings.autoDefaultPpi;
    m_exportSettings.originalQuality = m_settings.autoOriginalQuality;
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
        const int pendingChoice = m_pendingTemplateChoice;
        const QVariantList pendingFiles = m_pendingAutoFiles;
        m_pendingAutoMode = false;
        m_pendingAutoFiles.clear();
        executeAutoLayoutWithFiles(pendingChoice, pendingFiles);
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

void AppController::createBlankPageForCurrentTemplate()
{
    int choice = m_project.currentTemplateChoice();
    if (choice != 2 && choice != 4 && choice != 9) {
        choice = 2;
    }
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

    m_deferThumbnailSignals = true;
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
    m_deferThumbnailSignals = false;
    emit thumbnailsChanged();
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

void AppController::setDefaultExportPathFromDialog(const QString &folderUrl)
{
    const QString dir = toLocalPath(folderUrl);
    if (dir.isEmpty()) {
        return;
    }
    setDefaultExportPath(dir);
}

void AppController::setCacheDirectoryFromDialog(const QString &folderUrl)
{
    const QString dir = toLocalPath(folderUrl);
    if (dir.isEmpty()) {
        return;
    }
    setCacheDirectory(dir);
}

void AppController::setTextureDirectoryFromDialog(const QString &folderUrl)
{
    const QString dir = toLocalPath(folderUrl);
    if (dir.isEmpty()) {
        return;
    }
    setTextureDirectory(dir);
}

QVariantList AppController::availableTextures() const
{
    QVariantList out;
    QDir dir(m_settings.textureDir);
    if (!dir.exists()) {
        return out;
    }
    const QStringList filters{QStringLiteral("*.png"), QStringLiteral("*.jpg"), QStringLiteral("*.jpeg"), QStringLiteral("*.webp"), QStringLiteral("*.bmp")};
    const QFileInfoList files = dir.entryInfoList(filters, QDir::Files | QDir::Readable, QDir::Name);
    out.reserve(files.size());
    for (const QFileInfo &fi : files) {
        out << QUrl::fromLocalFile(fi.absoluteFilePath()).toString();
    }
    return out;
}

void AppController::openTextureDirectory()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_settings.textureDir));
}

void AppController::refreshTextures()
{
    ++m_textureListRevision;
    emit appSettingsChanged();
}

bool AppController::clearPreviewCache()
{
    const bool ok = m_imageService.clearCache();
    m_exportService.clearThumbnailCache();
    if (ok) {
        m_pageThumbnailCache.clear();
        m_dirtyThumbnailPages.clear();
        for (int i = 0; i < m_project.pageCount(); ++i) {
            m_dirtyThumbnailPages.insert(i);
            m_pageThumbnailRevisions[i] = m_pageThumbnailRevisions.value(i, 0) + 1;
        }
        ++m_thumbnailListRevision;
        emit thumbnailsChanged();
    }
    return ok;
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
    req.originalQuality = m_exportSettings.originalQuality;
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
    if (pageIndex < 0 || pageIndex >= m_project.pageCount()) {
        return {};
    }
    if (!m_dirtyThumbnailPages.contains(pageIndex) && m_pageThumbnailCache.contains(pageIndex)) {
        return m_pageThumbnailCache.value(pageIndex);
    }

    constexpr int thumbWidth = 220;
    const int thumbHeight = qMax(1, qRound(static_cast<qreal>(thumbWidth) / layout::pageAspectRatio(2)));
    const QString path = m_exportService.renderPageThumbnail(m_project, pageIndex, thumbWidth, thumbHeight);
    if (path.isEmpty()) {
        return {};
    }
    const QString source = QUrl::fromLocalFile(path).toString()
                           + QStringLiteral("?v=%1").arg(m_pageThumbnailRevisions.value(pageIndex, 0));
    m_pageThumbnailCache.insert(pageIndex, source);
    m_dirtyThumbnailPages.remove(pageIndex);
    return source;
}

int AppController::pageThumbnailRevisionAt(int pageIndex) const
{
    return m_pageThumbnailRevisions.value(pageIndex, 0);
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
bool AppController::originalQualityExport() const { return m_exportSettings.originalQuality; }
void AppController::setOriginalQualityExport(bool value) { if (m_exportSettings.originalQuality == value) return; m_exportSettings.originalQuality = value; emit exportSettingsChanged(); }
QString AppController::exportScope() const { return m_exportSettings.scope == ExportService::Scope::Queue ? QStringLiteral("全队列") : QStringLiteral("当前页"); }

QString AppController::lastExportMessage() const { return m_lastExportMessage; }
bool AppController::lastExportSuccess() const { return m_lastExportSuccess; }

QString AppController::autoLayoutPreset() const { return m_settings.autoPreset; }
qreal AppController::pageAspectRatio() const { return layout::pageAspectRatio(2); }
int AppController::thumbnailListRevision() const { return m_thumbnailListRevision; }
void AppController::setAutoLayoutPreset(const QString &value) { if (m_settings.autoPreset == value) return; m_settings.autoPreset = value; persistExportDefaults(); emit appSettingsChanged(); }
int AppController::autoDefaultPpi() const { return m_settings.autoDefaultPpi; }
void AppController::setAutoDefaultPpi(int value) { const int clamped = qBound(72, value, 1200); if (m_settings.autoDefaultPpi == clamped) return; m_settings.autoDefaultPpi = clamped; persistExportDefaults(); emit appSettingsChanged(); }
QString AppController::autoFillStrategy() const { return m_settings.autoFill; }
void AppController::setAutoFillStrategy(const QString &value) { if (m_settings.autoFill == value) return; m_settings.autoFill = value; persistExportDefaults(); emit appSettingsChanged(); }
QString AppController::autoOrientationPolicy() const { return m_settings.autoOrientation; }
void AppController::setAutoOrientationPolicy(const QString &value) { if (m_settings.autoOrientation == value) return; m_settings.autoOrientation = value; persistExportDefaults(); emit appSettingsChanged(); }
QString AppController::autoNamingRule() const { return m_settings.autoNamingRule; }
void AppController::setAutoNamingRule(const QString &value) { if (m_settings.autoNamingRule == value) return; m_settings.autoNamingRule = value; persistExportDefaults(); emit appSettingsChanged(); }
bool AppController::autoCropMarks() const { return m_settings.autoCropMarks; }
void AppController::setAutoCropMarks(bool value) { if (m_settings.autoCropMarks == value) return; m_settings.autoCropMarks = value; persistExportDefaults(); emit appSettingsChanged(); }
QString AppController::defaultExportPath() const { return m_settings.defaultPath; }
void AppController::setDefaultExportPath(const QString &value) { const QString normalized = ensureDir(value); if (m_settings.defaultPath == normalized) return; m_settings.defaultPath = normalized; persistExportDefaults(); emit appSettingsChanged(); }
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
QString AppController::cacheDirectory() const { return m_settings.cacheDir; }
void AppController::setCacheDirectory(const QString &value) {
    if (value.isEmpty() || m_settings.cacheDir == value) return;
    m_settings.cacheDir = ensureDir(value);
    m_imageService.setCacheRoot(value);
    m_exportService.setCacheRoot(value);
    m_project.refreshSlotPreviewResources();
    clearPreviewCache();
    persistExportDefaults();
    emit appSettingsChanged();
}
int AppController::cacheRetentionDays() const { return m_settings.cacheRetentionDays; }
void AppController::setCacheRetentionDays(int days)
{
    int normalized = 30;
    if (days <= 1) {
        normalized = 1;
    } else if (days <= 7) {
        normalized = 7;
    }
    if (m_settings.cacheRetentionDays == normalized) return;
    m_settings.cacheRetentionDays = normalized;
    persistExportDefaults();
    emit appSettingsChanged();
}
int AppController::previewMaxEdge() const { return m_settings.previewMaxEdge; }
void AppController::setPreviewMaxEdge(int value) {
    const int clamped = qBound(1600, value, 2560);
    if (m_settings.previewMaxEdge == clamped) return;
    m_settings.previewMaxEdge = clamped;
    m_imageService.setPreviewMaxEdge(clamped);
    m_project.refreshSlotPreviewResources();
    clearPreviewCache();
    persistExportDefaults();
    emit appSettingsChanged();
}
bool AppController::autoOriginalQualityExport() const { return m_settings.autoOriginalQuality; }
void AppController::setAutoOriginalQualityExport(bool value)
{
    if (m_settings.autoOriginalQuality == value) return;
    m_settings.autoOriginalQuality = value;
    persistExportDefaults();
    emit appSettingsChanged();
}
QString AppController::textureDirectory() const { return m_settings.textureDir; }
void AppController::setTextureDirectory(const QString &value)
{
    const QString normalized = ensureDir(value);
    if (normalized.isEmpty() || m_settings.textureDir == normalized) return;
    m_settings.textureDir = normalized;
    persistExportDefaults();
    refreshTextures();
    emit appSettingsChanged();
}
int AppController::textureListRevision() const { return m_textureListRevision; }

TemplateType AppController::toTemplateType(int choice)
{
    if (choice == 4) return TemplateType::FourUp;
    if (choice == 9) return TemplateType::NineUp;
    return TemplateType::TwoUp;
}

void AppController::loadSettings()
{
    QSettings settings(QStringLiteral("Liusu"), QStringLiteral("Liusu"));
    m_settings.autoPreset = settings.value(QStringLiteral("auto/preset"), QStringLiteral("均衡填充")).toString();
    m_settings.autoDefaultPpi = settings.value(QStringLiteral("auto/defaultPpi"), 300).toInt();
    m_settings.autoFill = settings.value(QStringLiteral("auto/defaultFill"), QStringLiteral("原图完整放入")).toString();
    m_settings.autoOrientation = settings.value(QStringLiteral("auto/orientationPolicy"), QStringLiteral("自动右转 90°")).toString();
    m_settings.autoNamingRule = settings.value(QStringLiteral("auto/namingRule"), QStringLiteral("组合命名")).toString();
    m_settings.autoCropMarks = settings.value(QStringLiteral("auto/cropMarks"), false).toBool();
    const QString defaultExportRoot = ensureDir(defaultPicturesDir() + QStringLiteral("/Liusu/Exports"));
    const QString defaultCacheRoot = ensureDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QStringLiteral("/preview"));
    const QString defaultTextureRoot = ensureDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + QStringLiteral("/textures"));

    m_settings.defaultPath = settings.value(QStringLiteral("export/defaultPath"), defaultExportRoot).toString();
    m_settings.rememberPath = settings.value(QStringLiteral("export/rememberLastPath"), true).toBool();
    m_settings.defaultFormat = settings.value(QStringLiteral("export/defaultFormat"), QStringLiteral("JPG")).toString();
    m_settings.defaultResolution = settings.value(QStringLiteral("export/defaultResolution"), QStringLiteral("300 PPI")).toString();
    m_settings.defaultCrop = settings.value(QStringLiteral("export/defaultCropMarks"), false).toBool();
    m_settings.defaultCustomPpi = settings.value(QStringLiteral("export/defaultCustomPpi"), 300).toInt();
    m_settings.cacheDir = settings.value(QStringLiteral("cache/dir"), defaultCacheRoot).toString();
    m_settings.cacheRetentionDays = settings.value(QStringLiteral("cache/retentionDays"), 30).toInt();
    m_settings.previewMaxEdge = settings.value(QStringLiteral("cache/previewMaxEdge"), 1600).toInt();
    m_settings.textureDir = settings.value(QStringLiteral("texture/dir"), defaultTextureRoot).toString();
    m_settings.autoOriginalQuality = settings.value(QStringLiteral("auto/originalQuality"), false).toBool();
    ensureDir(m_settings.defaultPath);
    ensureDir(m_settings.cacheDir);
    ensureDir(m_settings.textureDir);

    m_exportSettings.path = m_settings.defaultPath;
    m_exportSettings.format = m_settings.defaultFormat;
    m_exportSettings.resolution = m_settings.defaultResolution;
    m_exportSettings.cropMarks = m_settings.defaultCrop;
    m_exportSettings.customPpi = m_settings.defaultCustomPpi;
    m_exportSettings.originalQuality = false;
    m_imageService.setCacheRoot(m_settings.cacheDir);
    m_exportService.setCacheRoot(m_settings.cacheDir);
    m_imageService.setPreviewMaxEdge(m_settings.previewMaxEdge);
    m_imageService.cleanupExpiredCache(m_settings.cacheRetentionDays);
}

void AppController::persistExportDefaults() const
{
    QSettings settings(QStringLiteral("Liusu"), QStringLiteral("Liusu"));
    settings.setValue(QStringLiteral("auto/preset"), m_settings.autoPreset);
    settings.setValue(QStringLiteral("auto/defaultPpi"), m_settings.autoDefaultPpi);
    settings.setValue(QStringLiteral("auto/defaultFill"), m_settings.autoFill);
    settings.setValue(QStringLiteral("auto/orientationPolicy"), m_settings.autoOrientation);
    settings.setValue(QStringLiteral("auto/namingRule"), m_settings.autoNamingRule);
    settings.setValue(QStringLiteral("auto/cropMarks"), m_settings.autoCropMarks);
    settings.setValue(QStringLiteral("export/defaultPath"), m_settings.defaultPath);
    settings.setValue(QStringLiteral("export/rememberLastPath"), m_settings.rememberPath);
    settings.setValue(QStringLiteral("export/defaultFormat"), m_settings.defaultFormat);
    settings.setValue(QStringLiteral("export/defaultResolution"), m_settings.defaultResolution);
    settings.setValue(QStringLiteral("export/defaultCropMarks"), m_settings.defaultCrop);
    settings.setValue(QStringLiteral("export/defaultCustomPpi"), m_settings.defaultCustomPpi);
    settings.setValue(QStringLiteral("cache/dir"), m_settings.cacheDir);
    settings.setValue(QStringLiteral("cache/retentionDays"), m_settings.cacheRetentionDays);
    settings.setValue(QStringLiteral("cache/previewMaxEdge"), m_settings.previewMaxEdge);
    settings.setValue(QStringLiteral("texture/dir"), m_settings.textureDir);
    settings.setValue(QStringLiteral("auto/originalQuality"), m_settings.autoOriginalQuality);
}

void AppController::markPageThumbnailDirty(int pageIndex)
{
    if (pageIndex < 0 || pageIndex >= m_project.pageCount()) {
        return;
    }
    m_dirtyThumbnailPages.insert(pageIndex);
    m_pageThumbnailRevisions[pageIndex] = m_pageThumbnailRevisions.value(pageIndex, 0) + 1;
    m_pageThumbnailCache.remove(pageIndex);
    if (!m_deferThumbnailSignals) {
        scheduleThumbnailSignal();
    }
}

void AppController::scheduleThumbnailSignal()
{
    if (!m_thumbnailEmitTimer.isActive()) {
        m_thumbnailEmitTimer.start();
    } else {
        m_thumbnailEmitTimer.start();
    }
}

void AppController::handlePagesChanged()
{
    QHash<int, int> normalizedRevisions;
    QHash<int, QString> normalizedCache;
    QSet<int> normalizedDirty;
    for (int i = 0; i < m_project.pageCount(); ++i) {
        normalizedRevisions.insert(i, m_pageThumbnailRevisions.value(i, 0));
        if (m_pageThumbnailCache.contains(i)) {
            normalizedCache.insert(i, m_pageThumbnailCache.value(i));
        }
        if (m_dirtyThumbnailPages.contains(i)) {
            normalizedDirty.insert(i);
        }
    }

    m_pageThumbnailRevisions = normalizedRevisions;
    m_pageThumbnailCache = normalizedCache;
    m_dirtyThumbnailPages = normalizedDirty;
    for (int i = 0; i < m_project.pageCount(); ++i) {
        m_dirtyThumbnailPages.insert(i);
        m_pageThumbnailRevisions[i] = m_pageThumbnailRevisions.value(i, 0) + 1;
        m_pageThumbnailCache.remove(i);
    }

    ++m_thumbnailListRevision;
    emit thumbnailsChanged();
}

} // namespace pte
