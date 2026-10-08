#include "TemplateCatalog.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSet>
#include <algorithm>

namespace liusu::domain {
TemplateCatalog::TemplateCatalog()
{
    for (const auto& info : LayoutPresets::builtinPresets())
        m_entries.append({info.id,info.displayName,QStringLiteral("基础拼版"),info.id.toUpper(),LayoutPresets::create(info.id)});
}
const TemplateDefinition* TemplateCatalog::find(const QString& id) const
{
    for (const auto& entry : m_entries) if (entry.id == id) return &entry;
    return nullptr;
}
bool TemplateCatalog::appendFromJson(const QByteArray& json, QString* error)
{
    auto fail = [&](const QString& message) { if(error) *error=message; return false; };
    QJsonParseError parseError;
    const auto document=QJsonDocument::fromJson(json,&parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) return fail(QStringLiteral("模板目录不是有效 JSON 对象"));
    const auto items=document.object().value(QStringLiteral("templates"));
    if (!items.isArray() || items.toArray().isEmpty()) return fail(QStringLiteral("模板目录需要非空 templates 数组"));
    QSet<QString> identities;
    for(const auto& entry:m_entries) identities.insert(entry.id);
    QList<TemplateDefinition> pending;
    static const QRegularExpression validId(QStringLiteral("^[a-zA-Z0-9][a-zA-Z0-9._-]{0,127}$"));
    for(const auto& item:items.toArray()) {
        if(!item.isObject()) return fail(QStringLiteral("模板条目必须是对象"));
        const auto object=item.toObject();
        TemplateDefinition definition;
        definition.id=object.value("id").toString();
        definition.name=object.value("name").toString().trimmed();
        if(!validId.match(definition.id).hasMatch() || definition.name.isEmpty()) return fail(QStringLiteral("模板 ID 或名称无效"));
        if(identities.contains(definition.id)) return fail(QStringLiteral("模板 ID 重复：%1").arg(definition.id));
        definition.category=object.value("category").toString(QStringLiteral("导入模板"));
        definition.caption=object.value("caption").toString(definition.id.toUpper());
        const auto rectangles=object.value("slots");
        if(!rectangles.isArray()) return fail(QStringLiteral("模板 %1 缺少槽位数组").arg(definition.id));
        for(const auto& value:rectangles.toArray()) {
            if(!value.isObject()) return fail(QStringLiteral("槽位必须是矩形对象"));
            const auto rect=value.toObject();
            for(const auto* key: {"x","y","width","height"})
                if(!rect.value(QLatin1String(key)).isDouble()) return fail(QStringLiteral("槽位坐标必须是数值"));
            definition.layout.slotRects.append({rect.value("x").toDouble(),rect.value("y").toDouble(),rect.value("width").toDouble(),rect.value("height").toDouble()});
        }
        if(!definition.layout.isValid()) return fail(QStringLiteral("模板 %1 的槽位超出页面或尺寸无效").arg(definition.id));
        // 当前编辑合同是每个槽位都能直接选中；重叠会让底层照片被遮住。
        // 边缘相接允许，只有面积相交才拒绝，待层级编辑能力独立实现后再扩展。
        const auto& rectanglesInLayout=definition.layout.slotRects;
        for(qsizetype i=0;i<rectanglesInLayout.size();++i)
            for(qsizetype j=i+1;j<rectanglesInLayout.size();++j) {
                const auto& a=rectanglesInLayout[i];
                const auto& b=rectanglesInLayout[j];
                const auto width=std::min(a.x+a.width,b.x+b.width)-std::max(a.x,b.x);
                const auto height=std::min(a.y+a.height,b.y+b.height)-std::max(a.y,b.y);
                if(width>1e-9 && height>1e-9)
                    return fail(QStringLiteral("模板 %1 的槽位重叠，当前编辑器暂不支持层叠照片").arg(definition.id));
            }
        identities.insert(definition.id);
        pending.append(definition);
    }
    m_entries.append(pending);
    if(error) error->clear();
    return true;
}
}
