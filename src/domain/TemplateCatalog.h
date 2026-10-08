#pragma once
#include "Layout.h"
#include <QByteArray>

namespace liusu::domain {
struct TemplateDefinition {
    QString id;
    QString name;
    QString category;
    QString caption;
    LayoutModel layout;
};

// 目录负责可扩展的模板身份和展示元数据；项目仍保存实际槽位几何。
// 页面可选记录目录 ID；实际几何独立保存，删除目录也不破坏已经保存的页面。
class TemplateCatalog {
public:
    TemplateCatalog();
    const QList<TemplateDefinition>& entries() const { return m_entries; }
    const TemplateDefinition* find(const QString& id) const;
    // 一个目录包全量校验后才加入，错误或 ID 冲突不产生部分导入。
    bool appendFromJson(const QByteArray& json, QString* error);
private:
    QList<TemplateDefinition> m_entries;
};
}
