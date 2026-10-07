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

LayoutModel singleLayout()
{
    // 单张全幅：整页一个槽位。配合"铺满裁切"即整页无边距照片。
    return { { NormalizedRect{ 0.0, 0.0, 1.0, 1.0 } } };
}

// rows x cols 等分网格，行优先排列。
LayoutModel gridLayout(int rows, int cols)
{
    Q_ASSERT(rows > 0 && cols > 0);
    LayoutModel model;
    const qreal w = 1.0 / static_cast<qreal>(cols);
    const qreal h = 1.0 / static_cast<qreal>(rows);
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            model.slotRects.append(NormalizedRect{ col * w, row * h, w, h });
        }
    }
    return model;
}

} // namespace

LayoutModel create(const QString& presetId, bool* ok)
{
    if (ok)
        *ok = true;

    // 二宫格为竖向双拼：每张约 100x74mm，即 6 寸纸最常用的 3 寸双拼裁法。
    if (presetId == QStringLiteral("single"))
        return singleLayout();
    if (presetId == QStringLiteral("two"))
        return gridLayout(2, 1);
    if (presetId == QStringLiteral("four"))
        return gridLayout(2, 2);
    if (presetId == QStringLiteral("nine"))
        return gridLayout(3, 3);

    if (ok)
        *ok = false;
    return {};
}

} // namespace LayoutPresets
} // namespace liusu::domain
