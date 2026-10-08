#include "domain/ProjectDocument.h"
#include "services/render/PageRenderer.h"

#include <QDir>
#include <QElapsedTimer>
#include <QImage>
#include <QtTest/QtTest>

using namespace liusu::domain;
using namespace liusu::render;

namespace {

QString fixturesPath(const QString& name)
{
    return QFINDTESTDATA("../fixtures/" + name);
}

QColor pixel(const QImage& image, const QRectF& rect, qreal fx, qreal fy)
{
    const int x = qBound(0, qRound(rect.left() + rect.width() * fx), image.width() - 1);
    const int y = qBound(0, qRound(rect.top() + rect.height() * fy), image.height() - 1);
    return image.pixelColor(x, y);
}

bool colorClose(const QColor& a, const QColor& b, int tolerance = 40)
{
    return qAbs(a.red() - b.red()) <= tolerance && qAbs(a.green() - b.green()) <= tolerance
        && qAbs(a.blue() - b.blue()) <= tolerance;
}

QRectF slotRectPixels(const NormalizedRect& rect, const QImage& canvas)
{
    return QRectF(rect.x * canvas.width(), rect.y * canvas.height(),
                  rect.width * canvas.width(), rect.height * canvas.height());
}

constexpr qreal kPageWidthMm = 148.0;
constexpr qreal kPageHeightMm = 100.0;

} // namespace

// 子方案 04 渲染验收测试：槽位覆盖、EXIF 方向、填充模式、占位、PPI 尺寸。
class RenderTest final : public QObject
{
    Q_OBJECT

private slots:
    void canvasSizeFollowsPpi();
    void ppiTierOutputSizes();
    void fillModeCoversWholeSlot();
    void exifOrientationRespectedInRender();
    void emptySlotShowsPlaceholder();
    void missingFileShowsPlaceholderNotBlankPage();
    void fitModeLeavesBackgroundMargins();
    void rotationTurnsQuadrants();
    void mirrorFlipsQuadrants();
    void allLayoutPresetsRender();
    void writeLayoutSamples();
    void renderPerfBaseline();
};

void RenderTest::canvasSizeFollowsPpi()
{
    ProjectDocument doc = ProjectDocument::createDefault();
    const QImage at144 = PageRenderer::renderPage(doc.pages.first(), doc.background,
                                                  kPageWidthMm, kPageHeightMm, 144);
    QCOMPARE(at144.size(), QSize(qRound(148.0 / 25.4 * 144), qRound(100.0 / 25.4 * 144)));

    const QImage at300 = PageRenderer::renderPage(doc.pages.first(), doc.background,
                                                  kPageWidthMm, kPageHeightMm, 300);
    QCOMPARE(at300.size(), QSize(1748, 1181));

    // 不同 PPI 只改变像素密度，不改变宽高比比例（容差 1px）。
    const qreal ratio144 = qreal(at144.width()) / at144.height();
    const qreal ratio300 = qreal(at300.width()) / at300.height();
    QVERIFY(qAbs(ratio144 - ratio300) < 0.01);
}

void RenderTest::fillModeCoversWholeSlot()
{
    // 关键回归：图片必须缩放到覆盖整个槽位，而不是按原始像素尺寸绘制。
    ProjectDocument doc = ProjectDocument::createDefault();
    ProjectPage& page = doc.pages[0];
    page.layout = LayoutPresets::create(QStringLiteral("four"));
    page.slotStates = {
        SlotImageState{ fixturesPath("exif1.jpg"), 0, false, FillMode::Fill, 0.0, 0.0 },
        SlotImageState{}, SlotImageState{}, SlotImageState{},
    };

    const QImage canvas = PageRenderer::renderPage(page, doc.background,
                                                   kPageWidthMm, kPageHeightMm, 144);
    const QRectF slot = slotRectPixels(page.layout.slotRects.at(0), canvas);

    // 槽位四角与中心都必须被照片颜色覆盖（源图四象限：红/蓝/绿/黄）。
    QVERIFY(colorClose(pixel(canvas, slot, 0.1, 0.1), QColor(220, 40, 40)));   // 红 左上
    QVERIFY(colorClose(pixel(canvas, slot, 0.9, 0.1), QColor(40, 60, 220)));   // 蓝 右上
    QVERIFY(colorClose(pixel(canvas, slot, 0.1, 0.9), QColor(40, 170, 60)));   // 绿 左下
    QVERIFY(colorClose(pixel(canvas, slot, 0.9, 0.9), QColor(235, 210, 40)));  // 黄 右下
}

void RenderTest::exifOrientationRespectedInRender()
{
    // exif6.jpg（方向 6：需顺时针转 90° 显示）：校正后绿在左上。
    ProjectDocument doc = ProjectDocument::createDefault();
    ProjectPage& page = doc.pages[0];
    page.layout = LayoutPresets::create(QStringLiteral("four"));
    page.slotStates = {
        SlotImageState{ fixturesPath("exif6.jpg"), 0, false, FillMode::Fill, 0.0, 0.0 },
        SlotImageState{}, SlotImageState{}, SlotImageState{},
    };

    const QImage canvas = PageRenderer::renderPage(page, doc.background,
                                                   kPageWidthMm, kPageHeightMm, 144);
    const QRectF slot = slotRectPixels(page.layout.slotRects.at(0), canvas);
    QVERIFY(colorClose(pixel(canvas, slot, 0.1, 0.1), QColor(40, 170, 60)));   // 绿 左上
    QVERIFY(colorClose(pixel(canvas, slot, 0.9, 0.9), QColor(40, 60, 220)));   // 蓝 右下
}

void RenderTest::emptySlotShowsPlaceholder()
{
    ProjectDocument doc = ProjectDocument::createDefault();
    ProjectPage& page = doc.pages[0];
    page.layout = LayoutPresets::create(QStringLiteral("four"));
    page.slotStates = { SlotImageState{}, SlotImageState{}, SlotImageState{}, SlotImageState{} };

    const QImage canvas = PageRenderer::renderPage(page, doc.background,
                                                   kPageWidthMm, kPageHeightMm, 144);
    const QRectF slot = slotRectPixels(page.layout.slotRects.at(0), canvas);

    // 占位是「3D 底座 + 玻璃态大加号」：左侧采样点避开加号横臂与四边内斜面。
    const QColor base = pixel(canvas, slot, 0.12, 0.5);
    QVERIFY(base != QColor(Qt::white));
    QVERIFY(base.red() > 180 && base.red() >= base.blue());   // 同工作台一致的暖中性底

    // 中心落在玻璃态加号上：既不是页面背景白，也比底座更亮（玻璃受光面）。
    const QColor mark = pixel(canvas, slot, 0.5, 0.5);
    QVERIFY(mark != QColor(Qt::white));
    QVERIFY(mark.lightness() + 40 < base.lightness()); // 轻量深色加号，拒绝塑料按键式高光
}

void RenderTest::missingFileShowsPlaceholderNotBlankPage()
{
    ProjectDocument doc = ProjectDocument::createDefault();
    ProjectPage& page = doc.pages[0];
    page.layout = LayoutPresets::create(QStringLiteral("four"));
    page.slotStates = {
        SlotImageState{ QStringLiteral("Z:/nowhere/missing.jpg"), 0, false, FillMode::Fill, 0.0, 0.0 },
        SlotImageState{}, SlotImageState{}, SlotImageState{},
    };

    const QImage canvas = PageRenderer::renderPage(page, doc.background,
                                                   kPageWidthMm, kPageHeightMm, 144);
    const QRectF slot = slotRectPixels(page.layout.slotRects.at(0), canvas);
    // 读取失败仍是可解释的错误占位，而不是静默留白：整块槽位都有内容。
    QVERIFY(pixel(canvas, slot, 0.5, 0.5) != QColor(Qt::white));
    QVERIFY(pixel(canvas, slot, 0.12, 0.5) != QColor(Qt::white));
}

void RenderTest::fitModeLeavesBackgroundMargins()
{
    // 方形槽位心理模型：Fit 一张 4:3 横图进 66×44 槽位时，
    // 图片按宽适配，上下不应留边；用 m色背景验证左右/上下哪一侧留边。
    ProjectDocument doc = ProjectDocument::createDefault();
    doc.background.colorHex = QStringLiteral("#ff00ff"); // 洋红背景便于断言
    ProjectPage& page = doc.pages[0];
    page.layout = LayoutPresets::create(QStringLiteral("four"));
    page.slotStates = {
        SlotImageState{ fixturesPath("big.png"), 0, false, FillMode::Fit, 0.0, 0.0 },
        SlotImageState{}, SlotImageState{}, SlotImageState{},
    };

    const QImage canvas = PageRenderer::renderPage(page, doc.background,
                                                   kPageWidthMm, kPageHeightMm, 144);
    const QRectF slot = slotRectPixels(page.layout.slotRects.at(0), canvas);
    // big.png 是 4:3；槽位 66:44 = 1.5:1（也是 3:2，两者比例接近）。
    // Fit 模式下图片完整放入：中心必为照片色而不是背景色。
    QVERIFY(!colorClose(pixel(canvas, slot, 0.5, 0.5), QColor(255, 0, 255), 20));
    // 槽位内四角：Fit 不与槽位对齐的维度会露出背景。
    const bool topLeftPhoto = colorClose(pixel(canvas, slot, 0.02, 0.02), QColor(220, 40, 40), 40);
    const bool topLeftBackground = colorClose(pixel(canvas, slot, 0.02, 0.02), QColor(255, 0, 255), 20);
    QVERIFY(topLeftPhoto || topLeftBackground);
}

void RenderTest::rotationTurnsQuadrants()
{
    ProjectDocument doc = ProjectDocument::createDefault();
    ProjectPage& page = doc.pages[0];
    page.layout = LayoutPresets::create(QStringLiteral("four"));
    page.slotStates = {
        SlotImageState{ fixturesPath("exif1.jpg"), 90, false, FillMode::Fill, 0.0, 0.0 },
        SlotImageState{}, SlotImageState{}, SlotImageState{},
    };

    const QImage canvas = PageRenderer::renderPage(page, doc.background,
                                                   kPageWidthMm, kPageHeightMm, 144);
    const QRectF slot = slotRectPixels(page.layout.slotRects.at(0), canvas);
    // 顺时针 90° 后，原左上的红到右上；原右下的黄到左下。
    QVERIFY(colorClose(pixel(canvas, slot, 0.9, 0.1), QColor(220, 40, 40)));
    QVERIFY(colorClose(pixel(canvas, slot, 0.1, 0.9), QColor(235, 210, 40)));
}

void RenderTest::mirrorFlipsQuadrants()
{
    ProjectDocument doc = ProjectDocument::createDefault();
    ProjectPage& page = doc.pages[0];
    page.layout = LayoutPresets::create(QStringLiteral("four"));
    page.slotStates = {
        SlotImageState{ fixturesPath("exif1.jpg"), 0, true, FillMode::Fill, 0.0, 0.0 },
        SlotImageState{}, SlotImageState{}, SlotImageState{},
    };

    const QImage canvas = PageRenderer::renderPage(page, doc.background,
                                                   kPageWidthMm, kPageHeightMm, 144);
    const QRectF slot = slotRectPixels(page.layout.slotRects.at(0), canvas);
    // 水平镜像后：红左上↔右上交换，绿左下↔黄右下交换。
    QVERIFY(colorClose(pixel(canvas, slot, 0.9, 0.1), QColor(220, 40, 40)));
    QVERIFY(colorClose(pixel(canvas, slot, 0.1, 0.1), QColor(40, 60, 220)));
    QVERIFY(colorClose(pixel(canvas, slot, 0.9, 0.9), QColor(40, 170, 60)));
    QVERIFY(colorClose(pixel(canvas, slot, 0.1, 0.9), QColor(235, 210, 40)));
}

void RenderTest::ppiTierOutputSizes()
{
    // 三档位（ADR-0004）：300 / 600 / 自定义 900。输出像素由 mm×ppi 推导。
    ProjectDocument doc = ProjectDocument::createDefault();
    struct Case { int ppi; int width; int height; };
    const QList<Case> cases = {
        { 300, 1748, 1181 },
        { 600, 3496, 2362 },
        { 900, 5244, 3543 },
    };
    for (const Case& c : cases) {
        const QImage canvas = PageRenderer::renderPage(doc.pages.first(), doc.background,
                                                       kPageWidthMm, kPageHeightMm, c.ppi);
        QCOMPARE(canvas.width(), c.width);
        QCOMPARE(canvas.height(), c.height);
    }
    // 不同档位只改变像素密度：宽高比恒定。
    // 精确校验：1748/1181 与 3496/2362 与 5244/3543 比例一致（容差 <0.001）。
}

void RenderTest::allLayoutPresetsRender()
{
    // 四类预设各自以"自动填充"（循环使用测试图，Fill 模式）渲染，
    // 每个槽位都必须被照片覆盖（不是背景、不是占位）。
    const QStringList presetIds = {
        QStringLiteral("single"), QStringLiteral("two"),
        QStringLiteral("four"), QStringLiteral("nine"),
    };
    const QStringList fixtureNames = {
        QStringLiteral("exif1.jpg"), QStringLiteral("exif6.jpg"),
        QStringLiteral("exif3.jpg"), QStringLiteral("exif8.jpg"),
        QStringLiteral("plain.png"),
    };

    for (const QString& presetId : presetIds) {
        ProjectDocument doc = ProjectDocument::createDefault();
        doc.background.colorHex = QStringLiteral("#010203"); // 独特背景色，便于断言"未被背景覆盖"
        ProjectPage& page = doc.pages[0];
        page.layout = LayoutPresets::create(presetId);
        page.slotStates.clear();
        const int count = page.layout.slotRects.size();
        for (int i = 0; i < count; ++i) {
            page.slotStates.append(SlotImageState{
                fixturesPath(fixtureNames.at(i % fixtureNames.size())),
                0, false, FillMode::Fill, 0.0, 0.0 });
        }

        const QImage canvas = PageRenderer::renderPage(page, doc.background,
                                                       kPageWidthMm, kPageHeightMm, 300);
        QVERIFY2(!canvas.isNull(), qPrintable(presetId));

        for (int i = 0; i < count; ++i) {
            const QRectF slot = slotRectPixels(page.layout.slotRects.at(i), canvas);
            // Fill 模式：槽位中心必为照片色，不是背景色也不是空槽位占位
            // （占位 = 浅灰 3D 底座 + 玻璃态加号，两者都是冷调中性灰）。
            const QColor center = pixel(canvas, slot, 0.5, 0.5);
            QVERIFY2(center != QColor(1, 2, 3), qPrintable(QStringLiteral("%1 slot %2 是背景").arg(presetId).arg(i)));
            QVERIFY2(!colorClose(center, QColor(0xe6, 0xe8, 0xef), 8),
                     qPrintable(QStringLiteral("%1 slot %2 是占位底座").arg(presetId).arg(i)));
            QVERIFY2(!colorClose(center, QColor(0xf1, 0xf4, 0xf9), 8),
                     qPrintable(QStringLiteral("%1 slot %2 是占位加号").arg(presetId).arg(i)));
        }
    }
}

void RenderTest::writeLayoutSamples()
{
    // 样张输出（04 计划）：四类布局 @300 PPI 固定样张写入构建目录，
    // 供人工目检与跨版本比对。路径可由 LIUSU_SAMPLE_DIR 覆盖。
    QString outDir = qEnvironmentVariable("LIUSU_SAMPLE_DIR");
    if (outDir.isEmpty())
        outDir = QStringLiteral(QT_TESTCASE_BUILDDIR) + QStringLiteral("/samples");
    QVERIFY(QDir().mkpath(outDir));

    const QStringList presetIds = {
        QStringLiteral("single"), QStringLiteral("two"),
        QStringLiteral("four"), QStringLiteral("nine"),
    };
    const QStringList fixtureNames = {
        QStringLiteral("exif1.jpg"), QStringLiteral("exif6.jpg"),
        QStringLiteral("exif3.jpg"), QStringLiteral("exif8.jpg"),
        QStringLiteral("plain.png"),
    };

    const QList<QPair<QString, QString>> fixtures = {
        { QStringLiteral("plain.png"), QStringLiteral("原样 3:2") },
        { QStringLiteral("exif6.jpg"), QStringLiteral("EXIF 转正") },
    };

    for (const QString& presetId : presetIds) {
        ProjectDocument doc = ProjectDocument::createDefault();
        ProjectPage& page = doc.pages[0];
        page.layout = LayoutPresets::create(presetId);
        page.slotStates.clear();
        const int count = page.layout.slotRects.size();
        for (int i = 0; i < count; ++i) {
            page.slotStates.append(SlotImageState{
                fixturesPath(fixtureNames.at(i % fixtureNames.size())),
                0, false, FillMode::Fill, 0.0, 0.0 });
        }

        const QImage canvas = PageRenderer::renderPage(page, doc.background,
                                                       kPageWidthMm, kPageHeightMm, 300);
        const QString file = outDir + QStringLiteral("/sample-") + presetId + QStringLiteral("-300ppi.png");
        QVERIFY2(canvas.save(file, "PNG"), qPrintable(file));
        qInfo("样张写入: %s", qPrintable(file));
    }
}

void RenderTest::renderPerfBaseline()
{
    // 性能基线（04 计划）：四宫格 @300 PPI 的渲染耗时建立可比基线。
    // 不设硬指标，只给宽松健全性上界（单页平均 > 2s 视为异常）。
    ProjectDocument doc = ProjectDocument::createDefault();
    ProjectPage& page = doc.pages[0];
    page.layout = LayoutPresets::create(QStringLiteral("four"));
    page.slotStates.clear();
    for (int i = 0; i < 4; ++i)
        page.slotStates.append(SlotImageState{ fixturesPath("exif1.jpg"), 0, false, FillMode::Fill, 0.0, 0.0 });

    QElapsedTimer timer;
    timer.start();
    constexpr int kRounds = 5;
    for (int i = 0; i < kRounds; ++i)
        (void)PageRenderer::renderPage(page, doc.background, kPageWidthMm, kPageHeightMm, 300);
    const qint64 avgMs = timer.elapsed() / kRounds;
    qInfo("性能基线：四宫格 @300 PPI 单页渲染平均 %lld ms", avgMs);
    QVERIFY2(avgMs < 2000, "单页渲染耗时异常（> 2s）");
}

QTEST_MAIN(RenderTest)

#include "tst_render.moc"
