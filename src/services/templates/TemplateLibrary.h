#pragma once
#include "domain/TemplateCatalog.h"
#include <QUrl>

namespace liusu::services {
// 仅负责模板目录的读取和持久化；坐标校验由领域目录完成。
class TemplateLibrary {
public:
    explicit TemplateLibrary(const QString& directory = QString());
    const domain::TemplateCatalog& catalog() const { return m_catalog; }
    bool importCatalog(const QUrl& fileUrl,QString* error);
    QString loadError() const { return m_loadError; }
private:
    QString m_directory;
    QString m_loadError;
    domain::TemplateCatalog m_catalog;
};
}
