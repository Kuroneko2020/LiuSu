#pragma once

#include "Layout.h"
#include "Page.h"

#include <QByteArray>
#include <QList>
#include <QString>

namespace liusu::domain {

// 项目文件 schema 版本。
// v1 最终形态（ADR-0010）：多页队列。该格式从未对外发布，重定义无迁移成本；
// 任何破坏性字段变更仍须递增版本并在解析端写迁移逻辑。
constexpr int kProjectFileVersion = 1;

// 导出设置随项目保存：保证"重开项目后导出与关闭前一致"（00-基准验收口径）。
struct ExportSettings {
    int ppi{300};            // 档位语义见 ADR-0004（300/600/自定义），合法范围 [72, 1200]
    bool jpegFormat{true};   // true = JPEG，false = PNG
    int jpegQuality{95};     // 仅 JPEG 有效，[1, 100]
    bool originalMode{false};// 原图导出模式：按原图像素导出 PNG，忽略 ppi

    bool operator==(const ExportSettings& other) const;
};

// 项目中的一页：布局 + 各槽位图片状态（两表按下标一一对应）。
struct ProjectPage {
    LayoutModel layout;
    QList<SlotImageState> slotStates;

    bool isValid() const;
    bool operator==(const ProjectPage& other) const;
};

// 项目文档：一个可保存 / 可打开的排版工程（页队列）。
// 背景与导出设置为项目级（旧版同款）；不含运行时视觉状态。
struct ProjectDocument {
    int version{kProjectFileVersion};
    QString pageProfileId{QStringLiteral("6in-148x100")};
    QString printerProfileId{QStringLiteral("xiaomi-1s")};
    QList<ProjectPage> pages;    // 页队列（胶片栏），至少一页
    SolidBackground background;
    ExportSettings exportSettings;

    static ProjectDocument createDefault();

    // 结构合法性：至少一页、每页有效、数值在合法范围。
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
