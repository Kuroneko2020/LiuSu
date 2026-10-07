#include "Page.h"

#include <QtGlobal>

namespace liusu::domain {

bool SlotImageState::operator==(const SlotImageState& other) const
{
    return imagePath == other.imagePath && rotationDegrees == other.rotationDegrees
        && mirrored == other.mirrored && fillMode == other.fillMode
        && qFuzzyCompare(cropOffsetX, other.cropOffsetX)
        && qFuzzyCompare(cropOffsetY, other.cropOffsetY);
}

bool SolidBackground::operator==(const SolidBackground& other) const
{
    return colorHex == other.colorHex;
}

bool PageProfile::operator==(const PageProfile& other) const
{
    return id == other.id && displayName == other.displayName
        && qFuzzyCompare(widthMm, other.widthMm) && qFuzzyCompare(heightMm, other.heightMm);
}

bool PrinterProfile::operator==(const PrinterProfile& other) const
{
    return id == other.id && displayName == other.displayName
        && pageProfileId == other.pageProfileId
        && qFuzzyCompare(printableMarginMm, other.printableMarginMm)
        && recommendedPpi == other.recommendedPpi;
}

namespace Profiles {

QList<PageProfile> builtinPageProfiles()
{
    return {
        // 6 英寸相纸（小米 1S 的 6 寸纸）：148 x 100 mm，横版为默认方向（ADR-0010）。
        // 几何与方向沿用旧版 TemplateLayout 的验证值；如需调整只改此处数据，
        // 并同步 ADR-0003 / ADR-0010 的核定结论。
        { QStringLiteral("6in-148x100"), QStringLiteral("6 英寸相纸"), 148.0, 100.0 },
    };
}

QList<PrinterProfile> builtinPrinterProfiles()
{
    return {
        { QStringLiteral("xiaomi-1s"),
          QStringLiteral("小米米家照片打印机 1S"),
          QStringLiteral("6in-148x100"),
          0.0,   // 可打印留白待核定，先按整页可打印近似（ADR-0003）
          300 }, // 热升华输出密度 300 dpi
    };
}

std::optional<PageProfile> findPageProfile(const QString& id)
{
    for (const PageProfile& profile : builtinPageProfiles()) {
        if (profile.id == id)
            return profile;
    }
    return std::nullopt;
}

std::optional<PrinterProfile> findPrinterProfile(const QString& id)
{
    for (const PrinterProfile& profile : builtinPrinterProfiles()) {
        if (profile.id == id)
            return profile;
    }
    return std::nullopt;
}

} // namespace Profiles
} // namespace liusu::domain
