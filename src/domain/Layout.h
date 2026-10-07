#pragma once

#include "Units.h"

#include <QList>
#include <QString>

namespace liusu::domain {

// 布局模型：页面内一组照片槽位（归一化矩形）。
//
// 契约（ADR-0006）：
// - 预设不是模型的组成部分：本结构不含预设 id，其它代码不得按槽位数量
//   （2/4/9）写分支；槽位数量差异由循环自然处理。
// - slotRects 与项目文件中的槽位图片状态按下标一一对应，两表长度必须一致。
// - 边框层（ADR-0001）未来接入时在此模型之外，不修改槽位矩形。
// - 成员不得命名为 slots：那是 Qt 的关键字宏，会被预处理器静默吞掉。
struct LayoutModel {
    QList<NormalizedRect> slotRects;

    // 至少一个槽位，且每个槽位合法。
    bool isValid() const;

    bool operator==(const LayoutModel& other) const;
};

// 内置布局预设生成器。新增预设 = 增加一个注册项，不改布局模型与渲染。
namespace LayoutPresets {

struct PresetInfo {
    QString id;          // "single" / "two" / "four" / "nine"
    QString displayName; // 界面显示名（中文）
};

QList<PresetInfo> builtinPresets();

bool isBuiltin(const QString& presetId);

// 未知 presetId 时返回空布局（isValid() 为 false）。
// ok 非空时接收成功与否；为 nullptr 表示调用方不关心，不参与断言。
// 预设几何沿用旧版 TemplateLayout 的验证值（ADR-0010，横版 148×100 页面）：
// 二宫格为 60×90 竖照双联（证件照式，边距 7/5mm、中缝 14mm）；
// 四宫格 66×44、九宫格 48×32 均为 3:2 横照。归一化 = mm/148 或 mm/100。
LayoutModel create(const QString& presetId, bool* ok = nullptr);

} // namespace LayoutPresets
} // namespace liusu::domain
