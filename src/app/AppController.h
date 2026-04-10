#pragma once

#include "models/ProjectState.h"
#include "services/ExportService.h"
#include "services/ImageService.h"

#include <QObject>

namespace pte {

class AppController : public QObject {
    Q_OBJECT
    Q_PROPERTY(pte::ProjectState* project READ project CONSTANT)
    Q_PROPERTY(QString exportPath READ exportPath WRITE setExportPath NOTIFY exportSettingsChanged)
    Q_PROPERTY(QString exportFormat READ exportFormat WRITE setExportFormat NOTIFY exportSettingsChanged)
    Q_PROPERTY(QString namingRule READ namingRule WRITE setNamingRule NOTIFY exportSettingsChanged)
    Q_PROPERTY(QString resolutionPreset READ resolutionPreset WRITE setResolutionPreset NOTIFY exportSettingsChanged)
    Q_PROPERTY(int customPpi READ customPpi WRITE setCustomPpi NOTIFY exportSettingsChanged)
    Q_PROPERTY(bool cropMarks READ cropMarks WRITE setCropMarks NOTIFY exportSettingsChanged)
    Q_PROPERTY(QString exportScope READ exportScope NOTIFY exportSettingsChanged)
    Q_PROPERTY(QString lastExportMessage READ lastExportMessage NOTIFY exportResultChanged)
    Q_PROPERTY(bool lastExportSuccess READ lastExportSuccess NOTIFY exportResultChanged)

    Q_PROPERTY(QString autoLayoutPreset READ autoLayoutPreset WRITE setAutoLayoutPreset NOTIFY appSettingsChanged)
    Q_PROPERTY(QString defaultExportPath READ defaultExportPath WRITE setDefaultExportPath NOTIFY appSettingsChanged)
    Q_PROPERTY(bool rememberLastPath READ rememberLastPath WRITE setRememberLastPath NOTIFY appSettingsChanged)
    Q_PROPERTY(QString defaultExportFormat READ defaultExportFormat WRITE setDefaultExportFormat NOTIFY appSettingsChanged)
    Q_PROPERTY(QString defaultExportResolution READ defaultExportResolution WRITE setDefaultExportResolution NOTIFY appSettingsChanged)
    Q_PROPERTY(bool defaultCropMarks READ defaultCropMarks WRITE setDefaultCropMarks NOTIFY appSettingsChanged)
    Q_PROPERTY(QString themePlaceholder READ themePlaceholder WRITE setThemePlaceholder NOTIFY appSettingsChanged)

public:
    explicit AppController(QObject *parent = nullptr);

    [[nodiscard]] ProjectState *project();

    Q_INVOKABLE void startManualLayout(int choice);
    Q_INVOKABLE void startAutoLayout(int choice);
    Q_INVOKABLE void createBlankPage(int choice);
    Q_INVOKABLE void importToSlot(int slotIndex);
    Q_INVOKABLE void batchImport();
    Q_INVOKABLE void exportCurrentPage();
    Q_INVOKABLE void exportQueue();
    Q_INVOKABLE void chooseExportPath();
    Q_INVOKABLE void runExport();
    Q_INVOKABLE QString pageThumbnailSource(int pageIndex);

    [[nodiscard]] QString exportPath() const;
    void setExportPath(const QString &value);
    [[nodiscard]] QString exportFormat() const;
    void setExportFormat(const QString &value);
    [[nodiscard]] QString namingRule() const;
    void setNamingRule(const QString &value);
    [[nodiscard]] QString resolutionPreset() const;
    void setResolutionPreset(const QString &value);
    [[nodiscard]] int customPpi() const;
    void setCustomPpi(int value);
    [[nodiscard]] bool cropMarks() const;
    void setCropMarks(bool value);
    [[nodiscard]] QString exportScope() const;

    [[nodiscard]] QString lastExportMessage() const;
    [[nodiscard]] bool lastExportSuccess() const;

    [[nodiscard]] QString autoLayoutPreset() const;
    void setAutoLayoutPreset(const QString &value);
    [[nodiscard]] QString defaultExportPath() const;
    void setDefaultExportPath(const QString &value);
    [[nodiscard]] bool rememberLastPath() const;
    void setRememberLastPath(bool value);
    [[nodiscard]] QString defaultExportFormat() const;
    void setDefaultExportFormat(const QString &value);
    [[nodiscard]] QString defaultExportResolution() const;
    void setDefaultExportResolution(const QString &value);
    [[nodiscard]] bool defaultCropMarks() const;
    void setDefaultCropMarks(bool value);
    [[nodiscard]] QString themePlaceholder() const;
    void setThemePlaceholder(const QString &value);

signals:
    void requestNavigateToEditor();
    void requestNavigateToExport();
    void exportSettingsChanged();
    void exportResultChanged();
    void thumbnailsChanged();
    void appSettingsChanged();

private:
    struct ExportSettings { QString path; QString format{"JPG"}; QString naming{"组合命名"}; QString resolution{"300 PPI"}; int customPpi{300}; bool cropMarks{false}; ExportService::Scope scope{ExportService::Scope::CurrentPage}; };
    struct SettingsModel { QString autoPreset{"均衡填充"}; QString defaultPath; bool rememberPath{true}; QString defaultFormat{"JPG"}; QString defaultResolution{"300 PPI"}; bool defaultCrop{false}; QString theme{"系统"}; };

    [[nodiscard]] static TemplateType toTemplateType(int choice);
    void loadSettings();
    void persistExportDefaults() const;

    ProjectState m_project;
    ImageService m_imageService;
    ExportService m_exportService;
    ExportSettings m_exportSettings;
    SettingsModel m_settings;

    QString m_lastExportMessage;
    bool m_lastExportSuccess = false;
    int m_thumbnailVersion = 0;
};

} // namespace pte
