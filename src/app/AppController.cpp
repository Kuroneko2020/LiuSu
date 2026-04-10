#include "app/AppController.h"
#include <QDir>

#include <QFileDialog>
#include <QSettings>
#include <QStandardPaths>

namespace pte {

namespace {
QString defaultPicturesDir()
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    return path.isEmpty() ? QStringLiteral(".") : path;
}
}

AppController::AppController(QObject *parent)
    : QObject(parent)
{
    loadSettings();
    if (m_exportSettings.path.isEmpty()) {
        m_exportSettings.path = m_settings.defaultPath;
    }

    connect(&m_project, &ProjectState::slotsChanged, this, &AppController::thumbnailsChanged);
    connect(&m_project, &ProjectState::pagesChanged, this, &AppController::thumbnailsChanged);
}

ProjectState *AppController::project() { return &m_project; }

void AppController::startManualLayout(int choice)
{
    m_project.ensureInitialPage(toTemplateType(choice));
    emit requestNavigateToEditor();
}

void AppController::startAutoLayout(int choice)
{
    m_project.ensureInitialPage(toTemplateType(choice));

    const QStringList images = m_imageService.importMultipleImages();
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
        m_project.assignImageToSlot(targetSlot, images.at(imported));
        ++imported;
    }

    if (m_exportSettings.path.isEmpty() || !QDir(m_exportSettings.path).exists()) {
        chooseExportPath();
        if (m_exportSettings.path.isEmpty() || !QDir(m_exportSettings.path).exists()) {
            m_lastExportSuccess = false;
            m_lastExportMessage = QStringLiteral("自动模式失败：导出目录无效。");
            emit exportResultChanged();
            return;
        }
    }

    m_exportSettings.scope = ExportService::Scope::Queue;
    runExport();
}

void AppController::createBlankPage(int choice)
{
    m_project.createPage(toTemplateType(choice));
}

void AppController::importToSlot(int slotIndex)
{
    const QString path = m_imageService.importSingleImage();
    if (path.isEmpty()) {
        return;
    }
    m_project.assignImageToSlot(slotIndex, path);
}

void AppController::batchImport()
{
    const QStringList images = m_imageService.importMultipleImages();
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
    const QString dir = QFileDialog::getExistingDirectory(nullptr, QStringLiteral("选择导出目录"), m_exportSettings.path);
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
    return m_exportService.renderPageThumbnail(m_project, pageIndex, 220, 140);
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
void AppController::setAutoLayoutPreset(const QString &value) { if (m_settings.autoPreset == value) return; m_settings.autoPreset = value; persistExportDefaults(); emit appSettingsChanged(); }
QString AppController::defaultExportPath() const { return m_settings.defaultPath; }
void AppController::setDefaultExportPath(const QString &value) { if (m_settings.defaultPath == value) return; m_settings.defaultPath = value; persistExportDefaults(); emit appSettingsChanged(); }
bool AppController::rememberLastPath() const { return m_settings.rememberPath; }
void AppController::setRememberLastPath(bool value) { if (m_settings.rememberPath == value) return; m_settings.rememberPath = value; persistExportDefaults(); emit appSettingsChanged(); }
QString AppController::defaultExportFormat() const { return m_settings.defaultFormat; }
void AppController::setDefaultExportFormat(const QString &value) { if (m_settings.defaultFormat == value) return; m_settings.defaultFormat = value; m_exportSettings.format = value; persistExportDefaults(); emit appSettingsChanged(); emit exportSettingsChanged(); }
QString AppController::defaultExportResolution() const { return m_settings.defaultResolution; }
void AppController::setDefaultExportResolution(const QString &value) { if (m_settings.defaultResolution == value) return; m_settings.defaultResolution = value; m_exportSettings.resolution = value; persistExportDefaults(); emit appSettingsChanged(); emit exportSettingsChanged(); }
bool AppController::defaultCropMarks() const { return m_settings.defaultCrop; }
void AppController::setDefaultCropMarks(bool value) { if (m_settings.defaultCrop == value) return; m_settings.defaultCrop = value; m_exportSettings.cropMarks = value; persistExportDefaults(); emit appSettingsChanged(); emit exportSettingsChanged(); }
QString AppController::themePlaceholder() const { return m_settings.theme; }
void AppController::setThemePlaceholder(const QString &value) { if (m_settings.theme == value) return; m_settings.theme = value; emit appSettingsChanged(); }

TemplateType AppController::toTemplateType(int choice)
{
    if (choice == 4) return TemplateType::FourUp;
    if (choice == 6) return TemplateType::SixUp;
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

    m_exportSettings.path = m_settings.defaultPath;
    m_exportSettings.format = m_settings.defaultFormat;
    m_exportSettings.resolution = m_settings.defaultResolution;
    m_exportSettings.cropMarks = m_settings.defaultCrop;
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
}

} // namespace pte
