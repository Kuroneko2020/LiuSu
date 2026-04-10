#pragma once

#include <QString>

namespace pte {

enum class TemplateType {
    TwoUp = 2,
    FourUp = 4,
    SixUp = 6
};

enum class FillMode {
    FitInside,
    FillCrop
};

inline QString templateDisplayName(TemplateType type)
{
    switch (type) {
    case TemplateType::TwoUp:
        return QStringLiteral("两张拼图");
    case TemplateType::FourUp:
        return QStringLiteral("四张拼图");
    case TemplateType::SixUp:
        return QStringLiteral("六张拼图");
    }
    return QStringLiteral("未知模板");
}

inline int slotCount(TemplateType type)
{
    return static_cast<int>(type);
}

} // namespace pte
