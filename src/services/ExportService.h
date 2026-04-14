#pragma once

#include <QObject>
#include <QHash>
#include <QStringList>

namespace pte {

class ProjectState;

class ExportService : public QObject {
    Q_OBJECT
public:
    enum class Scope { CurrentPage, Queue };
    Q_ENUM(Scope)

    struct Request {
        QString outputDir;
        QString format;          // JPG/PNG
        QString namingRule;      // 组合命名/日期-序号
        QString resolutionPreset; // 300 PPI / 600 PPI / 自定义 PPI
        int customPpi = 300;
        bool cropMarks = false;
        bool originalQuality = false;
        Scope scope = Scope::CurrentPage;
    };

    struct Result {
        bool success = false;
        QString message;
        QStringList exportedFiles;
    };

    explicit ExportService(QObject *parent = nullptr);

    Result exportPages(const ProjectState &project, const Request &request) const;
    QString renderPageThumbnail(const ProjectState &project, int pageIndex, int width = 240, int height = 160) const;
    QString renderSlotPreview(const ProjectState &project, int pageIndex, int slotIndex, int width, int height) const;
    void clearThumbnailCache();
    void setCacheRoot(const QString &cacheRoot);
    [[nodiscard]] QString cacheRoot() const;

private:
    [[nodiscard]] int resolvePpi(const Request &request) const;
    [[nodiscard]] QString thumbsCacheDir() const;
    [[nodiscard]] QString slotPreviewCacheDir() const;
    mutable QHash<QString, QString> m_pageThumbnailCacheByKey;
    QString m_cacheRoot;
};

} // namespace pte
