#pragma once

#include "Layout.h"
#include "Page.h"

#include <QByteArray>
#include <QList>
#include <QString>

namespace liusu::domain {

// 项目文件 schema 版本。任何破坏性字段变更必须递增，并在解析端写迁移逻辑；
// 当前只识别 kProjectFileVersion，其它版本一律拒绝（不静默猜测）。
constexpr int kProjectFileVersion = 1;

// 导出设置随项目保存：保证"重开项目后导出与关闭前一致"（00-基准验收口径）。
struct ExportSettings {
    int ppi{300};            // 档位语义见 ADR-0004（300/600/自定义），合法范围 [72, 1200]
    bool jpegFormat{true};   // true = JPEG，false = PNG
    int jpegQuality{95};     // 仅 JPEG 有效，[1, 100]
    bool originalMode{false};// 原图导出模式：按原图像素导出 PNG，忽略 ppi

    bool operator==(const ExportSettings& other) const;
};

// 项目文档：一个可保存 / 可打开的排版工程。
// 不含运行时视觉状态（当前选中槽位、预览缩放等）——那些不属于项目文件。
struct ProjectDocument {
    int version{kProjectFileVersion};
    QString pageProfileId{QStringLiteral("6in-100x148")};
    QString printerProfileId{QStringLiteral("xiaomi-1s")};
    LayoutModel layout;              // 槽位几何
    QList<SlotImageState> slotStates; // 与 layout.slotRects 按下标一一对应
    SolidBackground background;
    ExportSettings exportSettings;

    static ProjectDocument createDefault();

    // 结构合法性：布局有效、图片状态与槽位一一对应、数值在合法范围。
    // 不校验档案 id 是否存在于注册表（注册表可演进），也不涉及文件系统。
    // 注意成员名避开 Qt 关键字宏（slots/signals/emits），否则声明被静默吞掉。
    bool isValid() const;

    bool operator==(const ProjectDocument& other) const;
};

struct ProjectParseResult {
    ProjectDocument document;
    QString errorMessage; // ok == false 时非空
    bool ok{false};
};

// 序列化契约：
// - 顶层含 "format":"liusu-project" 魔数与 "version"；version 决定解析方式。
// - 未知字段一律忽略（向前兼容），未知版本拒绝。
// - 字段缺失、类型错误、数值越界都返回失败，不静默补默认值——
//   宁可打开失败也不产出与用户记忆不符的页面。
QByteArray serializeProject(const ProjectDocument& document);
ProjectParseResult parseProject(const QByteArray& json);

} // namespace liusu::domain
