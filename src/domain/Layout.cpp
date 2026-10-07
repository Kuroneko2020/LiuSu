#include "Layout.h"

#include <QtGlobal>

namespace liusu::domain {

bool LayoutModel::isValid() const
{
    if (slotRects.isEmpty())
        return false;
    for (const NormalizedRect& rect : slotRects) {
        if (!rect.isValid())
            return false;
    }
    return true;
}

bool LayoutModel::operator==(const LayoutModel& other) const
{
    return slotRects == other.slotRects;
}

namespace LayoutPresets {

QList<PresetInfo> builtinPresets()
{
    return {
        { QStringLiteral("single"), QStringLiteral("单张全幅") },
        { QStringLiteral("two"), QStringLiteral("二宫格") },
        { QStringLiteral("four"), QStringLiteral("四宫格") },
        { QStringLiteral("nine"), QStringLiteral("九宫格") },
    };
}

bool isBuiltin(const QString& presetId)
{
    const auto presets = builtinPresets();
    for (const PresetInfo& info : presets) {
        if (info.id == presetId)
            return true;
    }
    return false;
}

namespace {

// 旧版验证过的几何（ADR-0010）：页面 148×100mm 横向，
// 归一化 = mm / 148 或 mm / 100。数值来自 references 旧版 TemplateLayout.cpp。
NormalizedRect mmRect(qreal xMm, qreal yMm, qreal wMm, qreal hMm)
{
    return NormalizedRect{ xMm / 148.0, yMm / 100.0, wMm / 148.0, hMm / 100.0 };
}

} // namespace

LayoutModel create(const QString& presetId, bool* ok)
{
    if (ok)
        *ok = true;

    if (presetId == QStringLiteral("single")) {
        // 单张全幅：整页一张，配合"铺满裁切"即整页无边距照片。
        return { { mmRect(0.0, 0.0, 148.0, 100.0) } };
    }
    if (presetId == QStringLiteral("two")) {
        // 二宫格：竖照双联——两张 60×90（证件照式），边距 L/R 7、T/B 5，中缝 14。
        return { { mmRect(7.0, 5.0, 60.0, 90.0), mmRect(81.0, 5.0, 60.0, 90.0) } };
    }
    if (presetId == QStringLiteral("four")) {
        // 四宫格：66×44（3:2 横照）2×2，边距 L/R 4、T/B 3。
        return { { mmRect(4.0, 3.0, 66.0, 44.0),
                  mmRect(78.0, 3.0, 66.0, 44.0),
                  mmRect(4.0, 53.0, 66.0, 44.0),
                  mmRect(78.0, 53.0, 66.0, 44.0) } };
    }
    if (presetId == QStringLiteral("nine")) {
        // 九宫格：48×32（3:2 横照）3×3，起点 0.667/50/99.333 × 0.667/34/67.333。
        return { { mmRect(0.667, 0.667, 48.0, 32.0),
                  mmRect(50.0, 0.667, 48.0, 32.0),
                  mmRect(99.333, 0.667, 48.0, 32.0),
                  mmRect(0.667, 34.0, 48.0, 32.0),
                  mmRect(50.0, 34.0, 48.0, 32.0),
                  mmRect(99.333, 34.0, 48.0, 32.0),
                  mmRect(0.667, 67.333, 48.0, 32.0),
                  mmRect(50.0, 67.333, 48.0, 32.0),
                  mmRect(99.333, 67.333, 48.0, 32.0) } };
    }

    if (ok)
        *ok = false;
    return {};
}

} // namespace LayoutPresets
} // namespace liusu::domain
