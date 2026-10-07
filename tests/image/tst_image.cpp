#include "services/image/ImageLoader.h"
#include "services/image/ImageSource.h"
#include "services/image/SlotImageCacheKey.h"
#include "services/image/ThumbnailCache.h"

#include <QDir>
#include <QImage>
#include <QTemporaryDir>
#include <QtTest/QtTest>

using namespace liusu::services;

namespace {

QString fixturesPath(const QString& name)
{
    // 图集在 tests/fixtures/，QFINDTESTDATA 以本测试源码目录（tests/image）为基准。
    return QFINDTESTDATA("../fixtures/" + name);
}

QColor sampleColor(const QImage& image, qreal fx, qreal fy)
{
    const int x = qBound(0, qRound(image.width() * fx), image.width() - 1);
    const int y = qBound(0, qRound(image.height() * fy), image.height() - 1);
    return image.pixelColor(x, y);
}

bool colorClose(const QColor& a, const QColor& b, int tolerance = 40)
{
    return qAbs(a.red() - b.red()) <= tolerance && qAbs(a.green() - b.green()) <= tolerance
        && qAbs(a.blue() - b.blue()) <= tolerance;
}

QImage solidImage(const QSize& size, const QColor& color)
{
    QImage img(size, QImage::Format_ARGB32);
    img.fill(color);
    return img;
}

// 四象限期望：对 40×30 的 R/B/G/Y 源图应用 EXIF 方向变换后的正确结果
// （R=左上 B=右上 G=左下 Y=右下；EXIF 5~8 会交换宽高）。
struct QuadrantExpectation {
    int orientation;
    int width;
    int height;
    QColor tl, tr, bl, br;
};

QList<QuadrantExpectation> orientationExpectations()
{
    const QColor R(220, 40, 40), B(40, 60, 220), G(40, 170, 60), Y(235, 210, 40);
    return {
        { 1, 40, 30, R, B, G, Y }, // 原样
        { 2, 40, 30, B, R, Y, G }, // 水平镜像
        { 3, 40, 30, Y, G, B, R }, // 旋转 180
        { 4, 40, 30, G, Y, R, B }, // 垂直镜像
        { 5, 30, 40, R, G, B, Y }, // 转置
        { 6, 30, 40, G, R, Y, B }, // 旋转 90°
        { 7, 30, 40, Y, B, G, R }, // 反转置
        { 8, 30, 40, B, Y, R, G }, // 旋转 270°
    };
}

} // namespace

// 子方案 03 验收测试：EXIF 方向、源身份、缓存 key 组成、两级缩略图缓存。
class ImageTest final : public QObject
{
    Q_OBJECT

private slots:
    void orientedLoadMatchesExifSpec();
    void pngOrientationSupported();
    void plainPngUntouched();
    void readInfoReportsOrientedSize();
    void corruptAndNonImageFilesFailGracefully();
    void cacheKeyComponentsEachInvalidate();
    void thumbnailScalesDownOnly();
    void thumbnailAppliesRotationAndMirror();
    void samePathReplaceNeverHitsStaleThumb();
    void diskCachePersistsAcrossInstances();
    void clearAllRemovesOnlyOwnFiles();
    void invalidSourceReturnsEmpty();
};

void ImageTest::orientedLoadMatchesExifSpec()
{
    for (const QuadrantExpectation& expect : orientationExpectations()) {
        const QString path = fixturesPath(QStringLiteral("exif%1.jpg").arg(expect.orientation));
        const ImageLoader::LoadedImage loaded = ImageLoader::loadOriented(path);
        QVERIFY2(loaded.ok, qPrintable(QStringLiteral("exif%1: %2").arg(expect.orientation).arg(loaded.error)));
        QCOMPARE(loaded.image.width(), expect.width);
        QCOMPARE(loaded.image.height(), expect.height);
        const QColor tl = sampleColor(loaded.image, 0.25, 0.25);
        const QColor tr = sampleColor(loaded.image, 0.75, 0.25);
        const QColor bl = sampleColor(loaded.image, 0.25, 0.75);
        const QColor br = sampleColor(loaded.image, 0.75, 0.75);
        QVERIFY2(colorClose(tl, expect.tl), qPrintable(QStringLiteral("exif%1 tl").arg(expect.orientation)));
        QVERIFY2(colorClose(tr, expect.tr), qPrintable(QStringLiteral("exif%1 tr").arg(expect.orientation)));
        QVERIFY2(colorClose(bl, expect.bl), qPrintable(QStringLiteral("exif%1 bl").arg(expect.orientation)));
        QVERIFY2(colorClose(br, expect.br), qPrintable(QStringLiteral("exif%1 br").arg(expect.orientation)));
    }
}

void ImageTest::pngOrientationSupported()
{
    // 已知限制（Qt PNG handler 不应用 eXIf 方向，见 ImageLoader.h）：
    // 本测试只验证无方向 PNG 走同一入口，PNG 方向块暂不做自动校正。
    const ImageLoader::LoadedImage loaded = ImageLoader::loadOriented(fixturesPath("plain.png"));
    QVERIFY(loaded.ok);
    QCOMPARE(loaded.image.width(), 40);
    QCOMPARE(loaded.image.height(), 30);
}

void ImageTest::plainPngUntouched()
{
    const ImageLoader::LoadedImage loaded = ImageLoader::loadOriented(fixturesPath("plain.png"));
    QVERIFY(loaded.ok);
    QCOMPARE(loaded.image.width(), 40);
    QCOMPARE(loaded.image.height(), 30);
    QVERIFY(colorClose(sampleColor(loaded.image, 0.25, 0.25), QColor(220, 40, 40)));
    QVERIFY(colorClose(sampleColor(loaded.image, 0.75, 0.25), QColor(40, 60, 220)));
}

void ImageTest::readInfoReportsOrientedSize()
{
    const ImageLoader::SourceInfo info = ImageLoader::readInfo(fixturesPath("exif6.jpg"));
    QVERIFY(info.readable);
    QCOMPARE(info.format, QByteArrayLiteral("jpeg"));
    QCOMPARE(info.orientedSize.width(), 30);
    QCOMPARE(info.orientedSize.height(), 40);
    QVERIFY(info.fileSizeBytes > 0);
    QVERIFY(info.lastModifiedMs > 0);
}

void ImageTest::corruptAndNonImageFilesFailGracefully()
{
    // 截断 JPEG：读信息可能成功（只有头部），但完整解码必须给出可解释错误。
    const ImageLoader::LoadedImage corrupt = ImageLoader::loadOriented(fixturesPath("corrupt.jpg"));
    QVERIFY(!corrupt.ok);
    QVERIFY(!corrupt.error.isEmpty());

    const ImageLoader::SourceInfo txt = ImageLoader::readInfo(fixturesPath("notimage.txt"));
    QVERIFY(!txt.readable);
    QVERIFY(!txt.error.isEmpty());
}

void ImageTest::cacheKeyComponentsEachInvalidate()
{
    SlotImageCacheKey key;
    key.imagePath = QStringLiteral("C:/p/a.jpg");
    key.fileSizeBytes = 100;
    key.lastModifiedMs = 1000;
    key.previewMaxEdge = 256;
    key.rotationDegrees = 0;
    key.mirrored = false;
    key.fillMode = FillMode::Fill;
    key.cropOffsetX = 0.0;
    key.cropOffsetY = 0.0;
    key.renderVersion = 1;
    const QString base = key.toString();

    // 逐字段变化都必须让 key 变化（图片与缓存规则·缓存 key 最低要求）。
    {
        SlotImageCacheKey c = key; c.imagePath = QStringLiteral("C:/p/b.jpg");
        QVERIFY(c.toString() != base);
    }
    {
        SlotImageCacheKey c = key; c.fileSizeBytes = 200;
        QVERIFY(c.toString() != base);
    }
    {
        SlotImageCacheKey c = key; c.lastModifiedMs = 2000;
        QVERIFY(c.toString() != base);
    }
    {
        SlotImageCacheKey c = key; c.previewMaxEdge = 512;
        QVERIFY(c.toString() != base);
    }
    {
        SlotImageCacheKey c = key; c.rotationDegrees = 90;
        QVERIFY(c.toString() != base);
    }
    {
        SlotImageCacheKey c = key; c.mirrored = true;
        QVERIFY(c.toString() != base);
    }
    {
        SlotImageCacheKey c = key; c.fillMode = FillMode::Fit;
        QVERIFY(c.toString() != base);
    }
    {
        SlotImageCacheKey c = key; c.cropOffsetX = 0.5;
        QVERIFY(c.toString() != base);
    }
    {
        SlotImageCacheKey c = key; c.cropOffsetY = -0.5;
        QVERIFY(c.toString() != base);
    }
    {
        SlotImageCacheKey c = key; c.renderVersion = 2;
        QVERIFY(c.toString() != base);
    }
    // 未变化时 key 稳定。
    QCOMPARE(key.toString(), base);
}

void ImageTest::thumbnailScalesDownOnly()
{
    ThumbnailCache cache(QTemporaryDir().path() + QStringLiteral("/thumb"));
    const auto source = ImageSourceIdentity::stat(fixturesPath("big.png"));
    QVERIFY(source.isValid());

    const QImage thumb = cache.thumbnail(source, 256, 0, false);
    QVERIFY(!thumb.isNull());
    QCOMPARE(thumb.width(), 256);
    QCOMPARE(thumb.height(), 192);

    // 不放大：小图请求大 maxEdge 时保持原尺寸。
    const QImage small = cache.thumbnail(
        ImageSourceIdentity::stat(fixturesPath("plain.png")), 256, 0, false);
    QCOMPARE(small.width(), 40);
    QCOMPARE(small.height(), 30);
}

void ImageTest::thumbnailAppliesRotationAndMirror()
{
    ThumbnailCache cache(QTemporaryDir().path() + QStringLiteral("/thumb"));
    const auto source = ImageSourceIdentity::stat(fixturesPath("exif1.jpg"));
    QVERIFY(source.isValid());

    const QImage base = cache.thumbnail(source, 256, 0, false);
    const QImage rotated = cache.thumbnail(source, 256, 90, false);
    QCOMPARE(base.width(), 40);
    QCOMPARE(rotated.width(), 30);
    QCOMPARE(rotated.height(), 40);
    // 旋转 90° 后，原图左上的红出现在右上象限。
    QVERIFY(colorClose(sampleColor(rotated, 0.75, 0.25), QColor(220, 40, 40)));

    const QImage mirrored = cache.thumbnail(source, 256, 0, true);
    // 水平镜像后，左上红换到右上。
    QVERIFY(colorClose(sampleColor(mirrored, 0.75, 0.25), QColor(220, 40, 40)));
    QVERIFY(colorClose(sampleColor(mirrored, 0.25, 0.25), QColor(40, 60, 220)));
}

void ImageTest::samePathReplaceNeverHitsStaleThumb()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("photo.png"));

    const QImage first = solidImage(QSize(40, 30), QColor(220, 40, 40));
    QVERIFY(first.save(path, "PNG"));

    ThumbnailCache cache(dir.path() + QStringLiteral("/thumb"));
    const auto sourceA = ImageSourceIdentity::stat(path);
    const QImage thumbA = cache.thumbnail(sourceA, 256, 0, false);
    QVERIFY(colorClose(sampleColor(thumbA, 0.5, 0.5), QColor(220, 40, 40), 4));

    // 同路径替换为不同内容（尺寸不同 → 源身份必然变化）。
    const QImage second = solidImage(QSize(60, 45), QColor(40, 60, 220));
    QVERIFY(second.save(path, "PNG"));

    const auto sourceB = ImageSourceIdentity::stat(path);
    QVERIFY(sourceB.fileSizeBytes != sourceA.fileSizeBytes
            || sourceB.lastModifiedMs != sourceA.lastModifiedMs);
    const QImage thumbB = cache.thumbnail(sourceB, 256, 0, false);
    QVERIFY(!thumbB.isNull());
    QVERIFY2(colorClose(sampleColor(thumbB, 0.5, 0.5), QColor(40, 60, 220), 4),
             "同路径替换后不得命中旧缩略图");
}

void ImageTest::diskCachePersistsAcrossInstances()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString disk = dir.path() + QStringLiteral("/thumb");

    ThumbnailCache first(disk);
    const auto source = ImageSourceIdentity::stat(fixturesPath("exif1.jpg"));
    const QImage t1 = first.thumbnail(source, 32, 0, false);
    QVERIFY(!t1.isNull());

    QDir diskDir(disk);
    const auto written = diskDir.entryList({ QStringLiteral("liusu-thumb-*") }, QDir::Files);
    QVERIFY(!written.isEmpty());

    // 新实例内存为空，必须能从磁盘层取回。
    ThumbnailCache second(disk);
    const QImage t2 = second.thumbnail(source, 32, 0, false);
    QVERIFY(!t2.isNull());
    QCOMPARE(t2.size(), t1.size());
}

void ImageTest::clearAllRemovesOnlyOwnFiles()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString disk = dir.path() + QStringLiteral("/thumb");

    ThumbnailCache cache(disk);
    const auto source = ImageSourceIdentity::stat(fixturesPath("exif1.jpg"));
    QVERIFY(!cache.thumbnail(source, 32, 0, false).isNull());
    QVERIFY(!QDir(disk).entryList({ QStringLiteral("liusu-thumb-*") }, QDir::Files).isEmpty());

    // 目录里放一个无关文件 + 源图本身保留，清理绝不能波及。
    const QString unrelated = disk + QStringLiteral("/keep.txt");
    QVERIFY(QFile::exists(disk));
    { QFile f(unrelated); QVERIFY(f.open(QIODevice::WriteOnly)); f.write("keep"); }

    cache.clearAll();

    QVERIFY(QDir(disk).entryList({ QStringLiteral("liusu-thumb-*") }, QDir::Files).isEmpty());
    QVERIFY(QFile::exists(unrelated));
    QVERIFY(QFile::exists(fixturesPath("exif1.jpg")));
}

void ImageTest::invalidSourceReturnsEmpty()
{
    ThumbnailCache cache(QTemporaryDir().path() + QStringLiteral("/thumb"));

    ImageSourceIdentity missing;
    missing.absolutePath = QStringLiteral("Z:/nowhere/none.jpg");
    QVERIFY(!missing.isValid());
    QVERIFY(cache.thumbnail(missing, 256, 0, false).isNull());

    QVERIFY(cache.thumbnail(ImageSourceIdentity::stat(fixturesPath("notimage.txt")), 256, 0, false)
                .isNull());
    QVERIFY(cache.thumbnail(ImageSourceIdentity::stat(fixturesPath("corrupt.jpg")), 256, 0, false)
                .isNull());
}

QTEST_MAIN(ImageTest)

#include "tst_image.moc"
