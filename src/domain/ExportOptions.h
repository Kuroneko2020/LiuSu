#pragma once
#include "ProjectDocument.h"
#include <QVariantList>
#include <QVariantMap>

namespace liusu::domain {
// 已实现能力的 UI 描述。选项数量与呈现次序不进入 QML；新增能力同时更新
// 模型/序列化/执行器与此定义，不能靠显示一个控件宣称导出能力已实现。
QVariantList exportOptionDefinitions();
QVariantMap exportOptionValues(const ExportSettings& settings);
bool setExportOption(ExportSettings& settings,const QString& id,const QVariant& value,QString* error);
}
