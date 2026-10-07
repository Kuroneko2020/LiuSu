#include "domain/Layout.h"
#include "domain/Page.h"
#include "domain/ProjectDocument.h"
#include "domain/Units.h"

#include <QtTest/QtTest>

using namespace liusu::domain;

namespace {
// 测试辅助：QVERIFY / QCOMPARE 是宏，花括号初始化里的逗号会被预处理器
// 当成宏参数分隔符；经由函数调用传参可避开。
NormalizedRect rect(qreal x, qreal y, qreal width, qreal height)
{
    return NormalizedRect{ x, y, width, height };
}
} // namespace

// 子方案 02 验收测试：坐标换算、预设几何、档案注册表、项目文件序列化。
class DomainTest final : public QObject
{
    Q_OBJECT

private slots:
    void mmPixelConversion();
    void normalizedRectValidation();
    void presetGeometry();
    void unknownPresetRejected();
    void profileRegistry();
    void defaultProjectIsValid();
    void serializationRoundtrip();
    void parseRejectsForeignFile();
    void parseRejectsUnknownVersion();
    void parseRejectsCorruptedJson();
    void parseRejectsSlotCountMismatch();
    void parseRejectsInvalidSlotFields();
    void parseRejectsInvalidExportSettings();
    void parseIgnoresUnknownFields();
};

void DomainTest::mmPixelConversion()
{
    // 1 英寸的锚点换算必须精确。
    QCOMPARE(mmToPixels(25.4, 300), 300);
    QCOMPARE(mmToPixels(25.4, 600), 600);

    // 6 寸纸 300 PPI 输出的整页像素尺寸。
    QCOMPARE(mmToPixels(100.0, 300), 1181); // 100/25.4*300 = 1181.10
    QCOMPARE(mmToPixels(148.0, 300), 1748); // 148/25.4*300 = 1748.03

    // 逆换算回到毫米（允许四舍五入误差 < 0.01mm）。
    QVERIFY(qAbs(pixelsToMm(mmToPixels(100.0, 300), 300) - 100.0) < 0.01);
    QVERIFY(qAbs(pixelsToMm(mmToPixels(148.0, 600), 600) - 148.0) < 0.01);
}

void DomainTest::normalizedRectValidation()
{
    QVERIFY(rect(0.0, 0.0, 1.0, 1.0).isValid());
    QVERIFY(!rect(0.0, 0.0, 0.0, 1.0).isValid());     // 零宽
    QVERIFY(!rect(-0.1, 0.0, 0.5, 0.5).isValid());    // 负坐标
    QVERIFY(!rect(0.6, 0.0, 0.5, 0.5).isValid());     // 越界
    QVERIFY(!rect(0.0, 0.6, 0.5, 0.5).isValid());

    // 3 等分累加的浮点误差必须被容差吸收。
    const qreal third = 1.0 / 3.0;
    QVERIFY(rect(2 * third, 0.0, third, 1.0).isValid());
}

void DomainTest::presetGeometry()
{
    const LayoutModel single = LayoutPresets::create(QStringLiteral("single"));
    QCOMPARE(single.slotRects.size(), 1);
    QCOMPARE(single.slotRects.first(), rect(0.0, 0.0, 1.0, 1.0));

    const LayoutModel two = LayoutPresets::create(QStringLiteral("two"));
    QCOMPARE(two.slotRects.size(), 2);
    QCOMPARE(two.slotRects.at(0), rect(0.0, 0.0, 1.0, 0.5)); // 竖向双拼：上半
    QCOMPARE(two.slotRects.at(1), rect(0.0, 0.5, 1.0, 0.5));

    const LayoutModel four = LayoutPresets::create(QStringLiteral("four"));
    QCOMPARE(four.slotRects.size(), 4);
    QCOMPARE(four.slotRects.at(0), rect(0.0, 0.0, 0.5, 0.5));
    QCOMPARE(four.slotRects.at(3), rect(0.5, 0.5, 0.5, 0.5));

    const LayoutModel nine = LayoutPresets::create(QStringLiteral("nine"));
    QCOMPARE(nine.slotRects.size(), 9);
    // 3x3 网格全部合法且覆盖整页。
    QVERIFY(nine.isValid());
    const qreal third = 1.0 / 3.0;
    QCOMPARE(nine.slotRects.at(8), rect(2 * third, 2 * third, third, third));
}

void DomainTest::unknownPresetRejected()
{
    bool ok = true;
    const LayoutModel model = LayoutPresets::create(QStringLiteral("nope"), &ok);
    QVERIFY(!ok);
    QVERIFY(model.slotRects.isEmpty());
}

void DomainTest::profileRegistry()
{
    const auto pages = Profiles::builtinPageProfiles();
    QCOMPARE(pages.size(), 1);
    QCOMPARE(pages.first().id, QStringLiteral("6in-100x148"));
    QCOMPARE(pages.first().widthMm, 100.0);
    QCOMPARE(pages.first().heightMm, 148.0);

    const auto printers = Profiles::builtinPrinterProfiles();
    QCOMPARE(printers.size(), 1);
    QCOMPARE(printers.first().id, QStringLiteral("xiaomi-1s"));
    QCOMPARE(printers.first().recommendedPpi, 300);
    QCOMPARE(printers.first().pageProfileId, QStringLiteral("6in-100x148"));

    QVERIFY(Profiles::findPageProfile(QStringLiteral("6in-100x148")).has_value());
    QVERIFY(!Profiles::findPageProfile(QStringLiteral("nope")).has_value());
    QVERIFY(Profiles::findPrinterProfile(QStringLiteral("xiaomi-1s")).has_value());
}

void DomainTest::defaultProjectIsValid()
{
    const ProjectDocument doc = ProjectDocument::createDefault();
    QVERIFY(doc.isValid());
    QVERIFY(doc == doc);
    QCOMPARE(doc.version, kProjectFileVersion);
    QCOMPARE(doc.exportSettings.ppi, 300);
}

void DomainTest::serializationRoundtrip()
{
    // 构造一个与默认值不同、覆盖全部字段的工程。
    ProjectDocument doc;
    doc.pageProfileId = QStringLiteral("6in-100x148");
    doc.printerProfileId = QStringLiteral("xiaomi-1s");
    doc.layout = LayoutPresets::create(QStringLiteral("four"));
    doc.slotStates = {
        SlotImageState{ QStringLiteral("C:/photos/a.jpg"), 90, true, FillMode::Fit, 0.0, 0.0 },
        SlotImageState{ QStringLiteral("C:/photos/b.png"), 0, false, FillMode::Fill, -1.0, 0.5 },
        SlotImageState{},
        SlotImageState{ QStringLiteral("C:/photos/c.jpg"), 270, false, FillMode::Fill, 1.0, -1.0 },
    };
    doc.background.colorHex = QStringLiteral("#f5f0e6");
    doc.exportSettings = ExportSettings{ 600, false, 100, false };

    const ProjectParseResult result = parseProject(serializeProject(doc));
    QVERIFY2(result.ok, qPrintable(result.errorMessage));
    QVERIFY(result.document == doc);
}

void DomainTest::parseRejectsForeignFile()
{
    const QByteArray json = QByteArrayLiteral("{\"hello\": \"world\"}");
    const ProjectParseResult result = parseProject(json);
    QVERIFY(!result.ok);
    QVERIFY(!result.errorMessage.isEmpty());
}

void DomainTest::parseRejectsUnknownVersion()
{
    ProjectDocument doc = ProjectDocument::createDefault();
    QByteArray json = serializeProject(doc);
    json.replace("\"version\":1", "\"version\":99");
    const ProjectParseResult result = parseProject(json);
    QVERIFY(!result.ok);
    QVERIFY(result.errorMessage.contains(QStringLiteral("版本")));
}

void DomainTest::parseRejectsCorruptedJson()
{
    QVERIFY(!parseProject(QByteArrayLiteral("{ not json")).ok);
    QVERIFY(!parseProject(QByteArrayLiteral("[]")).ok);
    QVERIFY(!parseProject(QByteArrayLiteral("")).ok);
}

void DomainTest::parseRejectsSlotCountMismatch()
{
    ProjectDocument doc = ProjectDocument::createDefault(); // single：1 槽位
    doc.layout = LayoutPresets::create(QStringLiteral("two")); // 2 槽位
    doc.slotStates = { SlotImageState{} }; // 只留 1 个状态
    const ProjectParseResult result = parseProject(serializeProject(doc));
    QVERIFY(!result.ok);
    QVERIFY(result.errorMessage.contains(QStringLiteral("不一致")));
}

void DomainTest::parseRejectsInvalidSlotFields()
{
    // 非法旋转角。
    {
        ProjectDocument doc = ProjectDocument::createDefault();
        doc.slotStates[0].rotationDegrees = 45;
        QVERIFY(!parseProject(serializeProject(doc)).ok);
    }
    // 非法裁切偏移。
    {
        ProjectDocument doc = ProjectDocument::createDefault();
        doc.slotStates[0].cropOffsetY = 1.5;
        QVERIFY(!parseProject(serializeProject(doc)).ok);
    }
    // 非法背景色。
    {
        ProjectDocument doc = ProjectDocument::createDefault();
        doc.background.colorHex = QStringLiteral("white");
        QVERIFY(!parseProject(serializeProject(doc)).ok);
    }
    // 非法槽位矩形：越界。
    {
        ProjectDocument doc = ProjectDocument::createDefault();
        doc.layout.slotRects[0].width = 1.5;
        QVERIFY(!parseProject(serializeProject(doc)).ok);
    }
}

void DomainTest::parseRejectsInvalidExportSettings()
{
    // PPI 越界（档位合法范围 [72,1200]，ADR-0004）。
    {
        ProjectDocument doc = ProjectDocument::createDefault();
        doc.exportSettings.ppi = 2000;
        QVERIFY(!parseProject(serializeProject(doc)).ok);
    }
    {
        ProjectDocument doc = ProjectDocument::createDefault();
        doc.exportSettings.ppi = 10;
        QVERIFY(!parseProject(serializeProject(doc)).ok);
    }
    // JPEG 质量越界。
    {
        ProjectDocument doc = ProjectDocument::createDefault();
        doc.exportSettings.jpegQuality = 0;
        QVERIFY(!parseProject(serializeProject(doc)).ok);
    }
}

void DomainTest::parseIgnoresUnknownFields()
{
    // 向前兼容：未来版本新增字段，本版解析仍须成功且数据不变。
    ProjectDocument doc = ProjectDocument::createDefault();
    QByteArray json = serializeProject(doc);
    json.replace("{\"format\":", "{\"futureField\":42,\"format\":");
    const ProjectParseResult result = parseProject(json);
    QVERIFY2(result.ok, qPrintable(result.errorMessage));
    QVERIFY(result.document == doc);
}

QTEST_MAIN(DomainTest)

#include "tst_domain.moc"
