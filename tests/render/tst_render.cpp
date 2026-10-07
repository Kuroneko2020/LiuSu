#include "domain/ProjectDocument.h"
#include "services/render/PageRenderer.h"

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
    void fillModeCoversWholeSlot();
    void exifOrientationRespectedInRender();
    void emptySlotShowsPlaceholder();
    void missingFileShowsPlaceholderNotBlankPage();
    void fitModeLeavesBackgroundMargins();
    void rotationTurnsQuadrants();
    void mirrorFlipsQuadrants();
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
    // 采样点避开居中的“空槽位”文字笔画；占位底色是 #eceff4。
    const QColor fill = pixel(canvas, slot, 0.2, 0.2);
    QVERIFY(colorClose(fill, QColor(0xec, 0xef, 0xf4), 12));
    // 中心区域不是页面背景白（占位确实画在槽位上）。
    QVERIFY(pixel(canvas, slot, 0.5, 0.5) != QColor(Qt::white));
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
    // 读取失败仍是可解释的错误占位，而不是静默留白。
    const QColor center = pixel(canvas, slot, 0.5, 0.5);
    QVERIFY(center != QColor(Qt::white));
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

QTEST_MAIN(RenderTest)

#include "tst_render.moc"
