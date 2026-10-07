#pragma once

#include <QList>
#include <QString>
#include <optional>

namespace liusu::domain {

// 槽位填充模式。渲染端语义：
// - Fit（完整放入）：图片整张缩放进槽位，必要时留白，不裁切图片。
// - Fill（铺满裁切）：图片等比放大到覆盖槽位，超出部分被裁掉，
//   裁掉哪一侧由裁切偏移（SlotImageState::cropOffsetX/Y）决定。
enum class FillMode {
    Fit,
    Fill,
};

// 单个槽位的图片编辑状态。只存用户可编辑的持久状态；
// 文件缺失 / 读取失败是运行时结论，由服务层（子方案 03）标注，不写进模型。
struct SlotImageState {
    QString imagePath;      // 空 = 空槽位
    int rotationDegrees{0}; // 仅允许 0 / 90 / 180 / 270，顺时针
    bool mirrored{false};
    FillMode fillMode{FillMode::Fill};
    // 裁切偏移（仅 Fill 模式有意义）：-1 = 取景贴图片左/上边，
    // 0 = 居中，+1 = 贴右/下边；横竖方向独立。合法范围 [-1, 1]。
    // 渲染实现必须按此语义取景，不得私自改成像素偏移。
    qreal cropOffsetX{0.0};
    qreal cropOffsetY{0.0};

    bool operator==(const SlotImageState& other) const;
};

// 页面纯色背景，#rrggbb 小写十六进制。首版无透明度；校验在项目文件解析处。
// 花纹 / 图片背景是未来方向（ADR-0008），届时在此扩展为变体而非加字段。
struct SolidBackground {
    QString colorHex{QStringLiteral("#ffffff")};

    bool operator==(const SolidBackground& other) const;
};

// 页面档案：一种相纸的物理尺寸定义。
// 宽高按纸张纵向记录（短边为宽）；导出方向旋转属于导出参数，不改档案。
struct PageProfile {
    QString id;
    QString displayName;
    qreal widthMm{100.0};
    qreal heightMm{148.0};

    bool operator==(const PageProfile& other) const;
};

// 打印机档案：一台机型的输出特性（ADR-0003）。
// 数据不写死在代码逻辑里：修改/新增机型只动 Profiles 注册表的数据。
struct PrinterProfile {
    QString id;
    QString displayName;
    QString pageProfileId; // 关联的页面档案
    // 四边不可打印留白。小米 1S：待官方手册或实测核定（ADR-0003），
    // 当前为 0 占位，按整页可打印近似；核定后只改这里的数据并注明来源。
    qreal printableMarginMm{0.0};
    int recommendedPpi{300};

    bool operator==(const PrinterProfile& other) const;
};

// 内置档案注册表：页面尺寸与打印机的统一事实来源（尺寸与渲染规则）。
// 首个机型为小米米家照片打印机 1S（ADR-0003）。
namespace Profiles {

QList<PageProfile> builtinPageProfiles();
QList<PrinterProfile> builtinPrinterProfiles();

std::optional<PageProfile> findPageProfile(const QString& id);
std::optional<PrinterProfile> findPrinterProfile(const QString& id);

} // namespace Profiles
} // namespace liusu::domain
