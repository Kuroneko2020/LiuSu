#include "ProjectDocument.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

namespace liusu::domain {

namespace {

constexpr int kMinPpi = 72;
constexpr int kMaxPpi = 1200;

const QRegularExpression& colorHexPattern()
{
    static const QRegularExpression pattern(QStringLiteral("^#[0-9a-f]{6}$"));
    return pattern;
}

bool isValidRotation(int degrees)
{
    return degrees == 0 || degrees == 90 || degrees == 180 || degrees == 270;
}

bool isValidCropOffset(qreal value)
{
    return value >= -1.0 && value <= 1.0;
}

bool isValidColorHex(const QString& hex)
{
    return colorHexPattern().match(hex).hasMatch();
}

QJsonObject serializeLayout(const LayoutModel& layout)
{
    QJsonArray slotRectArray;
    for (const NormalizedRect& rect : layout.slotRects) {
        slotRectArray.append(QJsonObject{ { QStringLiteral("x"), rect.x },
                                          { QStringLiteral("y"), rect.y },
                                          { QStringLiteral("width"), rect.width },
                                          { QStringLiteral("height"), rect.height } });
    }
    return QJsonObject{ { QStringLiteral("slots"), slotRectArray } };
}

QJsonObject serializeSlot(const SlotImageState& state)
{
    QJsonObject obj;
    obj.insert(QStringLiteral("imagePath"), state.imagePath);
    obj.insert(QStringLiteral("rotation"), state.rotationDegrees);
    obj.insert(QStringLiteral("mirrored"), state.mirrored);
    obj.insert(QStringLiteral("fillMode"),
               state.fillMode == FillMode::Fit ? QStringLiteral("fit") : QStringLiteral("fill"));
    obj.insert(QStringLiteral("cropX"), state.cropOffsetX);
    obj.insert(QStringLiteral("cropY"), state.cropOffsetY);
    return obj;
}

std::optional<SlotImageState> parseSlot(const QJsonObject& obj, QString* error)
{
    SlotImageState state;
    state.imagePath = obj.value(QStringLiteral("imagePath")).toString();

    // 类型必须精确匹配：toInt 的默认值语义会把 "abc" 变成 0，
    // 那等于静默改写用户数据，这里必须显式失败。
    const QJsonValue rotationValue = obj.value(QStringLiteral("rotation"));
    if (!rotationValue.isDouble())
        return std::nullopt;
    state.rotationDegrees = static_cast<int>(rotationValue.toDouble());
    if (!isValidRotation(state.rotationDegrees)) {
        *error = QStringLiteral("槽位旋转角度非法: %1").arg(state.rotationDegrees);
        return std::nullopt;
    }

    const QJsonValue mirroredValue = obj.value(QStringLiteral("mirrored"));
    if (!mirroredValue.isBool())
        return std::nullopt;
    state.mirrored = mirroredValue.toBool();

    const QString fillMode = obj.value(QStringLiteral("fillMode")).toString();
    if (fillMode == QStringLiteral("fit"))
        state.fillMode = FillMode::Fit;
    else if (fillMode == QStringLiteral("fill"))
        state.fillMode = FillMode::Fill;
    else
        return std::nullopt;

    const QJsonValue cropX = obj.value(QStringLiteral("cropX"));
    const QJsonValue cropY = obj.value(QStringLiteral("cropY"));
    if (!cropX.isDouble() || !cropY.isDouble())
        return std::nullopt;
    state.cropOffsetX = cropX.toDouble();
    state.cropOffsetY = cropY.toDouble();
    if (!isValidCropOffset(state.cropOffsetX) || !isValidCropOffset(state.cropOffsetY)) {
        *error = QStringLiteral("裁切偏移超出 [-1, 1]");
        return std::nullopt;
    }

    return state;
}

std::optional<LayoutModel> parseLayout(const QJsonObject& layoutObj, QString* error)
{
    const QJsonArray slotRects = layoutObj.value(QStringLiteral("slots")).toArray();
    if (slotRects.isEmpty()) {
        *error = QStringLiteral("布局至少需要一个槽位");
        return std::nullopt;
    }

    LayoutModel model;
    for (const QJsonValue& value : slotRects) {
        if (!value.isObject())
            return std::nullopt;
        const QJsonObject rectObj = value.toObject();
        NormalizedRect rect;
        const QJsonValue x = rectObj.value(QStringLiteral("x"));
        const QJsonValue y = rectObj.value(QStringLiteral("y"));
        const QJsonValue w = rectObj.value(QStringLiteral("width"));
        const QJsonValue h = rectObj.value(QStringLiteral("height"));
        if (!x.isDouble() || !y.isDouble() || !w.isDouble() || !h.isDouble())
            return std::nullopt;
        rect.x = x.toDouble();
        rect.y = y.toDouble();
        rect.width = w.toDouble();
        rect.height = h.toDouble();
        if (!rect.isValid()) {
            *error = QStringLiteral("槽位矩形非法（越界或非正宽高）");
            return std::nullopt;
        }
        model.slotRects.append(rect);
    }
    return model;
}

std::optional<ProjectPage> parsePage(const QJsonObject& pageObj, QString* error)
{
    const QJsonValue layoutValue = pageObj.value(QStringLiteral("layout"));
    if (!layoutValue.isObject())
        return std::nullopt;

    ProjectPage page;
    const std::optional<LayoutModel> layout = parseLayout(layoutValue.toObject(), error);
    if (!layout)
        return std::nullopt;
    page.layout = *layout;

    const QJsonArray slotStates = pageObj.value(QStringLiteral("slotStates")).toArray();
    if (slotStates.size() != page.layout.slotRects.size()) {
        *error = QStringLiteral("槽位图片状态数量(%1)与布局槽位数量(%2)不一致")
                     .arg(slotStates.size()).arg(page.layout.slotRects.size());
        return std::nullopt;
    }
    for (const QJsonValue& value : slotStates) {
        if (!value.isObject())
            return std::nullopt;
        const std::optional<SlotImageState> state = parseSlot(value.toObject(), error);
        if (!state)
            return std::nullopt;
        page.slotStates.append(*state);
    }
    return page;
}

} // namespace

bool ExportSettings::operator==(const ExportSettings& other) const
{
    return ppi == other.ppi && jpegFormat == other.jpegFormat
        && jpegQuality == other.jpegQuality && originalMode == other.originalMode;
}

bool ProjectPage::isValid() const
{
    if (!layout.isValid())
        return false;
    if (slotStates.size() != layout.slotRects.size())
        return false;
    for (const SlotImageState& state : slotStates) {
        if (!isValidRotation(state.rotationDegrees))
            return false;
        if (!isValidCropOffset(state.cropOffsetX) || !isValidCropOffset(state.cropOffsetY))
            return false;
    }
    return true;
}

bool ProjectPage::operator==(const ProjectPage& other) const
{
    return layout == other.layout && slotStates == other.slotStates;
}

ProjectDocument ProjectDocument::createDefault()
{
    ProjectDocument doc;
    ProjectPage page;
    page.layout = LayoutPresets::create(QStringLiteral("single"));
    page.slotStates = QList<SlotImageState>{ SlotImageState{} };
    doc.pages = QList<ProjectPage>{ page };
    return doc;
}

bool ProjectDocument::isValid() const
{
    if (pages.isEmpty())
        return false;
    for (const ProjectPage& page : pages) {
        if (!page.isValid())
            return false;
    }
    if (!isValidColorHex(background.colorHex))
        return false;
    if (exportSettings.ppi < kMinPpi || exportSettings.ppi > kMaxPpi)
        return false;
    if (exportSettings.jpegQuality < 1 || exportSettings.jpegQuality > 100)
        return false;
    return true;
}

bool ProjectDocument::operator==(const ProjectDocument& other) const
{
    return version == other.version && pageProfileId == other.pageProfileId
        && printerProfileId == other.printerProfileId && pages == other.pages
        && background == other.background && exportSettings == other.exportSettings;
}

QByteArray serializeProject(const ProjectDocument& document)
{
    QJsonArray pagesArray;
    for (const ProjectPage& page : document.pages) {
        QJsonArray slotStateArray;
        for (const SlotImageState& state : page.slotStates)
            slotStateArray.append(serializeSlot(state));
        QJsonObject pageObj;
        pageObj.insert(QStringLiteral("layout"), serializeLayout(page.layout));
        pageObj.insert(QStringLiteral("slotStates"), slotStateArray);
        pagesArray.append(pageObj);
    }

    QJsonObject exportObj;
    exportObj.insert(QStringLiteral("ppi"), document.exportSettings.ppi);
    exportObj.insert(QStringLiteral("format"),
                     document.exportSettings.jpegFormat ? QStringLiteral("JPEG")
                                                        : QStringLiteral("PNG"));
    exportObj.insert(QStringLiteral("jpegQuality"), document.exportSettings.jpegQuality);
    exportObj.insert(QStringLiteral("originalMode"), document.exportSettings.originalMode);

    QJsonObject root;
    root.insert(QStringLiteral("format"), QStringLiteral("liusu-project"));
    root.insert(QStringLiteral("version"), document.version);
    root.insert(QStringLiteral("page"), document.pageProfileId);
    root.insert(QStringLiteral("printer"), document.printerProfileId);
    root.insert(QStringLiteral("pages"), pagesArray);
    root.insert(QStringLiteral("background"), document.background.colorHex);
    root.insert(QStringLiteral("export"), exportObj);

    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

ProjectParseResult parseProject(const QByteArray& json)
{
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(json, &parseError);
    if (doc.isNull()) {
        return { {}, QStringLiteral("JSON 解析失败: %1").arg(parseError.errorString()), false };
    }
    if (!doc.isObject())
        return { {}, QStringLiteral("项目文件顶层必须是对象"), false };

    const QJsonObject root = doc.object();

    // 魔数与版本先于一切字段校验：防止把其它 JSON 文件误当项目文件打开。
    if (root.value(QStringLiteral("format")).toString() != QStringLiteral("liusu-project"))
        return { {}, QStringLiteral("不是留素项目文件"), false };

    const QJsonValue versionValue = root.value(QStringLiteral("version"));
    if (!versionValue.isDouble())
        return { {}, QStringLiteral("缺少版本号"), false };
    const int version = static_cast<int>(versionValue.toDouble());
    if (version != kProjectFileVersion)
        return { {},
                 QStringLiteral("不支持的项目文件版本: %1（当前支持 %2）")
                     .arg(version).arg(kProjectFileVersion),
                 false };

    ProjectDocument document;
    document.version = version;
    document.pageProfileId = root.value(QStringLiteral("page")).toString();
    document.printerProfileId = root.value(QStringLiteral("printer")).toString();
    if (document.pageProfileId.isEmpty() || document.printerProfileId.isEmpty())
        return { {}, QStringLiteral("缺少页面或打印机档案 id"), false };

    const QJsonValue pagesValue = root.value(QStringLiteral("pages"));
    if (!pagesValue.isArray())
        return { {}, QStringLiteral("缺少页队列"), false };
    const QJsonArray pages = pagesValue.toArray();
    if (pages.isEmpty())
        return { {}, QStringLiteral("页队列至少需要一页"), false };

    for (const QJsonValue& value : pages) {
        if (!value.isObject())
            return { {}, QStringLiteral("页必须是对象"), false };
        QString error;
        const std::optional<ProjectPage> page = parsePage(value.toObject(), &error);
        if (!page)
            return { {}, error.isEmpty() ? QStringLiteral("页字段非法") : error, false };
        document.pages.append(*page);
    }

    const QString background = root.value(QStringLiteral("background")).toString();
    if (!isValidColorHex(background))
        return { {}, QStringLiteral("背景颜色非法: %1").arg(background), false };
    document.background.colorHex = background;

    const QJsonValue exportValue = root.value(QStringLiteral("export"));
    if (!exportValue.isObject())
        return { {}, QStringLiteral("缺少导出设置"), false };
    const QJsonObject exportObj = exportValue.toObject();
    const QJsonValue ppi = exportObj.value(QStringLiteral("ppi"));
    const QJsonValue quality = exportObj.value(QStringLiteral("jpegQuality"));
    const QJsonValue originalMode = exportObj.value(QStringLiteral("originalMode"));
    const QString format = exportObj.value(QStringLiteral("format")).toString();
    if (!ppi.isDouble() || !quality.isDouble() || !originalMode.isBool())
        return { {}, QStringLiteral("导出设置字段缺失或类型错误"), false };
    document.exportSettings.ppi = static_cast<int>(ppi.toDouble());
    document.exportSettings.jpegQuality = static_cast<int>(quality.toDouble());
    document.exportSettings.originalMode = originalMode.toBool();
    if (format == QStringLiteral("JPEG"))
        document.exportSettings.jpegFormat = true;
    else if (format == QStringLiteral("PNG"))
        document.exportSettings.jpegFormat = false;
    else
        return { {}, QStringLiteral("导出格式非法: %1").arg(format), false };
    if (!document.isValid())
        return { {}, QStringLiteral("导出设置数值越界"), false };

    return { document, {}, true };
}

} // namespace liusu::domain
