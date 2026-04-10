#include "app/AppController.h"

namespace pte {

AppController::AppController(QObject *parent)
    : QObject(parent)
{
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
    batchImport();
    emit requestNavigateToEditor();
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

void AppController::exportCurrentPage() { m_exportService.exportCurrentPagePlaceholder(); }
void AppController::exportQueue() { m_exportService.exportQueuePlaceholder(); }

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
QString AppController::autoLayoutPreset() const { return m_settings.autoPreset; }
void AppController::setAutoLayoutPreset(const QString &value) { if (m_settings.autoPreset == value) return; m_settings.autoPreset = value; emit appSettingsChanged(); }
QString AppController::defaultExportPath() const { return m_settings.defaultPath; }
void AppController::setDefaultExportPath(const QString &value) { if (m_settings.defaultPath == value) return; m_settings.defaultPath = value; emit appSettingsChanged(); }
bool AppController::rememberLastPath() const { return m_settings.rememberPath; }
void AppController::setRememberLastPath(bool value) { if (m_settings.rememberPath == value) return; m_settings.rememberPath = value; emit appSettingsChanged(); }
QString AppController::defaultExportFormat() const { return m_settings.defaultFormat; }
void AppController::setDefaultExportFormat(const QString &value) { if (m_settings.defaultFormat == value) return; m_settings.defaultFormat = value; emit appSettingsChanged(); }
QString AppController::defaultExportResolution() const { return m_settings.defaultResolution; }
void AppController::setDefaultExportResolution(const QString &value) { if (m_settings.defaultResolution == value) return; m_settings.defaultResolution = value; emit appSettingsChanged(); }
bool AppController::defaultCropMarks() const { return m_settings.defaultCrop; }
void AppController::setDefaultCropMarks(bool value) { if (m_settings.defaultCrop == value) return; m_settings.defaultCrop = value; emit appSettingsChanged(); }
QString AppController::themePlaceholder() const { return m_settings.theme; }
void AppController::setThemePlaceholder(const QString &value) { if (m_settings.theme == value) return; m_settings.theme = value; emit appSettingsChanged(); }

TemplateType AppController::toTemplateType(int choice)
{
    if (choice == 4) return TemplateType::FourUp;
    if (choice == 6) return TemplateType::SixUp;
    return TemplateType::TwoUp;
}

} // namespace pte
