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

// 子方案 02 验收测试（含 ADR-0010 对齐）：坐标换算、旧版预设几何、
// 档案注册表、多页项目文件序列化。
class DomainTest final : public QObject
{
    Q_OBJECT

private slots:
    void mmPixelConversion();
    void normalizedRectValidation();
    void presetGeometryOldValues();
    void unknownPresetRejected();
    void profileRegistry();
    void defaultProjectIsValid();
    void multiPageSerializationRoundtrip();
    void parseRejectsForeignFile();
    void parseRejectsUnknownVersion();
    void parseRejectsCorruptedJson();
    void parseRejectsSlotCountMismatch();
    void parseRejectsEmptyPages();
    void parseRejectsInvalidSlotFields();
    void parseRejectsInvalidExportSettings();
    void parseIgnoresUnknownFields();
};

void DomainTest::mmPixelConversion()
{
    // 1 英寸的锚点换算必须精确。
    QCOMPARE(mmToPixels(25.4, 300), 300);
    QCOMPARE(mmToPixels(25.4, 600), 600);

    // 6 寸纸（横版 148×100）300 PPI 输出的整页像素尺寸。
    QCOMPARE(mmToPixels(148.0, 300), 1748); // 148/25.4*300 = 1748.03
    QCOMPARE(mmToPixels(100.0, 300), 1181); // 100/25.4*300 = 1181.10

    // 逆换算回到毫米（允许四舍五入误差 < 0.01mm）。
    QVERIFY(qAbs(pixelsToMm(mmToPixels(148.0, 300), 300) - 148.0) < 0.01);
    QVERIFY(qAbs(pixelsToMm(mmToPixels(100.0, 600), 600) - 100.0) < 0.01);
}

void DomainTest::normalizedRectValidation()
{
    QVERIFY(rect(0.0, 0.0, 1.0, 1.0).isValid());
    QVERIFY(!rect(0.0, 0.0, 0.0, 1.0).isValid());   // 零宽
    QVERIFY(!rect(-0.1, 0.0, 0.5, 0.5).isValid());  // 负坐标
    QVERIFY(!rect(0.6, 0.0, 0.5, 0.5).isValid());   // 越界
    QVERIFY(!rect(0.0, 0.6, 0.5, 0.5).isValid());

    // 3 等分累加的浮点误差必须被容差吸收。
    const qreal third = 1.0 / 3.0;
    QVERIFY(rect(2 * third, 0.0, third, 1.0).isValid());
}

void DomainTest::presetGeometryOldValues()
{
    // 预设几何 = 旧版 TemplateLayout.cpp 的验证值（ADR-0010）。
    const LayoutModel single = LayoutPresets::create(QStringLiteral("single"));
    QCOMPARE(single.slotRects.size(), 1);
    QCOMPARE(single.slotRects.first(), rect(0.0, 0.0, 1.0, 1.0));

    // 二宫格：竖照双联 60×90，边距 L/R 7、T/B 5，中缝 14。
    const LayoutModel two = LayoutPresets::create(QStringLiteral("two"));
    QCOMPARE(two.slotRects.size(), 2);
    QCOMPARE(two.slotRects.at(0), rect(7.0 / 148.0, 0.05, 60.0 / 148.0, 0.90));
    QCOMPARE(two.slotRects.at(1), rect(81.0 / 148.0, 0.05, 60.0 / 148.0, 0.90));

    // 四宫格：66×44（3:2 横照）。
    const LayoutModel four = LayoutPresets::create(QStringLiteral("four"));
    QCOMPARE(four.slotRects.size(), 4);
    QCOMPARE(four.slotRects.at(0), rect(4.0 / 148.0, 0.03, 66.0 / 148.0, 0.44));
    QCOMPARE(four.slotRects.at(3), rect(78.0 / 148.0, 0.53, 66.0 / 148.0, 0.44));

    // 九宫格：48×32（3:2 横照）。
    const LayoutModel nine = LayoutPresets::create(QStringLiteral("nine"));
    QCOMPARE(nine.slotRects.size(), 9);
    QVERIFY(nine.isValid());
    QCOMPARE(nine.slotRects.at(8),
             rect(99.333 / 148.0, 67.333 / 100.0, 48.0 / 148.0, 32.0 / 100.0));
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
    QCOMPARE(pages.first().id, QStringLiteral("6in-148x100"));
    QCOMPARE(pages.first().widthMm, 148.0);
    QCOMPARE(pages.first().heightMm, 100.0);

    const auto printers = Profiles::builtinPrinterProfiles();
    QCOMPARE(printers.size(), 1);
    QCOMPARE(printers.first().id, QStringLiteral("xiaomi-1s"));
    QCOMPARE(printers.first().recommendedPpi, 300);
    QCOMPARE(printers.first().pageProfileId, QStringLiteral("6in-148x100"));

    QVERIFY(Profiles::findPageProfile(QStringLiteral("6in-148x100")).has_value());
    QVERIFY(!Profiles::findPageProfile(QStringLiteral("nope")).has_value());
    QVERIFY(Profiles::findPrinterProfile(QStringLiteral("xiaomi-1s")).has_value());
}

void DomainTest::defaultProjectIsValid()
{
    const ProjectDocument doc = ProjectDocument::createDefault();
    QVERIFY(doc.isValid());
    QVERIFY(doc == doc);
    QCOMPARE(doc.version, kProjectFileVersion);
    QCOMPARE(doc.pages.size(), 1);
    QCOMPARE(doc.pages.first().layout.slotRects.size(), 1); // 默认单张全幅
    QCOMPARE(doc.exportSettings.ppi, 300);
}

void DomainTest::multiPageSerializationRoundtrip()
{
    // 两页队列，覆盖全部字段。
    ProjectDocument doc;
    doc.pageProfileId = QStringLiteral("6in-148x100");
    doc.printerProfileId = QStringLiteral("xiaomi-1s");

    ProjectPage page1;
    page1.layout = LayoutPresets::create(QStringLiteral("four"));
    page1.slotStates = {
        SlotImageState{ QStringLiteral("C:/photos/a.jpg"), 90, true, FillMode::Fit, 0.0, 0.0 },
        SlotImageState{ QStringLiteral("C:/photos/b.png"), 0, false, FillMode::Fill, -1.0, 0.5 },
        SlotImageState{},
        SlotImageState{ QStringLiteral("C:/photos/c.jpg"), 270, false, FillMode::Fill, 1.0, -1.0 },
    };

    ProjectPage page2;
    page2.layout = LayoutPresets::create(QStringLiteral("two"));
    page2.slotStates = {
        SlotImageState{ QStringLiteral("C:/photos/d.jpg"), 180, false, FillMode::Fill, 0.25, -0.25 },
        SlotImageState{},
    };

    doc.pages = { page1, page2 };
    doc.background.colorHex = QStringLiteral("#f5f0e6");
    doc.exportSettings = ExportSettings{ 600, false, 100, false };

    const ProjectParseResult result = parseProject(serializeProject(doc));
    QVERIFY2(result.ok, qPrintable(result.errorMessage));
    QVERIFY(result.document == doc);
    QCOMPARE(result.document.pages.size(), 2);
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
    ProjectDocument doc = ProjectDocument::createDefault();
    doc.pages[0].layout = LayoutPresets::create(QStringLiteral("two")); // 2 槽位
    doc.pages[0].slotStates = { SlotImageState{} };                     // 只留 1 个状态
    const ProjectParseResult result = parseProject(serializeProject(doc));
    QVERIFY(!result.ok);
    QVERIFY(result.errorMessage.contains(QStringLiteral("不一致")));
}

void DomainTest::parseRejectsEmptyPages()
{
    ProjectDocument doc = ProjectDocument::createDefault();
    doc.pages.clear();
    const ProjectParseResult result = parseProject(serializeProject(doc));
    QVERIFY(!result.ok);
    QVERIFY(result.errorMessage.contains(QStringLiteral("页队列")));
}

void DomainTest::parseRejectsInvalidSlotFields()
{
    // 非法旋转角。
    {
        ProjectDocument doc = ProjectDocument::createDefault();
        doc.pages[0].slotStates[0].rotationDegrees = 45;
        QVERIFY(!parseProject(serializeProject(doc)).ok);
    }
    // 非法裁切偏移。
    {
        ProjectDocument doc = ProjectDocument::createDefault();
        doc.pages[0].slotStates[0].cropOffsetY = 1.5;
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
        doc.pages[0].layout.slotRects[0].width = 1.5;
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
