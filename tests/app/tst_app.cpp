#include "app/AppController.h"
#include "domain/ProjectDocument.h"

#include <QFile>
#include <QTemporaryDir>
#include <QtTest/QtTest>
#include <limits>

class AppTest final : public QObject
{
    Q_OBJECT
private:
    liusu::domain::ProjectDocument saved(AppController& app, const QString& path)
    {
        if (!app.saveProject(QUrl::fromLocalFile(path))) return {};
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) return {};
        return liusu::domain::parseProject(file.readAll()).document;
    }
private slots:
    void tenPagesHaveIndependentTemplates();
    void reducingTemplatePreservesOverflowAndEdits();
    void importingFillsMixedPagesAndKeepsEveryPhoto();
    void cropAndExportSettingsSurviveReopen();
    void invalidCommandsLeaveDocumentUnchanged();
    void deletedPageCannotReuseOldPreview();
    void mixedPagesExportAtConfiguredSize();
    void repeatedExportsKeepExistingFiles();
    void invalidFileUrlsAndExportSettingsAreRejected();
    void emptyProjectEditsStillNeedReplacementProtection();
    void templateEditsKeepOtherPagePreviews();
    void importedTemplatesPersistAndWorkPerPage();
    void exportOptionsDrivePersistentSettings();
    void templateIdentitySurvivesMissingCatalog();
};

void AppTest::tenPagesHaveIndependentTemplates()
{
    AppController app;
    app.startManual("four");
    app.addPages("four", 9);
    QCOMPARE(app.pageCount(), 10);
    app.setCurrentPage(0);
    app.changeCurrentLayout("two");
    QCOMPARE(app.currentSlotCount(), 2);
    app.setCurrentPage(3);
    app.changeCurrentLayout("nine");
    app.setCurrentPage(8);
    app.changeCurrentLayout("single");
    QTemporaryDir dir;
    const auto document = saved(app, dir.filePath("mixed.liusu"));
    QVERIFY(document.isValid());
    QCOMPARE(document.pages.size(), 10);
    QCOMPARE(document.pages[0].slotStates.size(), 2);
    QCOMPARE(document.pages[3].slotStates.size(), 9);
    QCOMPARE(document.pages[8].slotStates.size(), 1);
    QCOMPARE(document.pages[2].slotStates.size(), 4);
}

void AppTest::reducingTemplatePreservesOverflowAndEdits()
{
    AppController app;
    app.startManual("nine");
    for (int i = 0; i < 9; ++i)
        app.assignFileToSlot(i, QUrl::fromLocalFile(QString("/photo-%1.jpg").arg(i)));
    app.rotateSlot(8);
    app.mirrorSlot(8);
    app.setCropOffset(8, 0.4, -0.7);
    app.addPages("two", 1);
    app.assignFileToSlot(0, QUrl::fromLocalFile("/existing-last.jpg"));
    app.setCurrentPage(0);
    app.changeCurrentLayout("four");
    QCOMPARE(app.pageCount(), 4); // 4 + 4 + 1 photos, then original second page.
    QTemporaryDir dir;
    const auto document = saved(app, dir.filePath("overflow.liusu"));
    QStringList paths;
    for (const auto& page : document.pages)
        for (const auto& state : page.slotStates)
            if (!state.imagePath.isEmpty()) paths.append(state.imagePath);
    QCOMPARE(paths.size(), 10);
    for (int i = 0; i < 9; ++i) QCOMPARE(paths[i], QString("/photo-%1.jpg").arg(i));
    QCOMPARE(paths.last(), QString("/existing-last.jpg"));
    const auto& moved = document.pages[2].slotStates[0];
    QCOMPARE(moved.rotationDegrees, 90);
    QVERIFY(moved.mirrored);
    QCOMPARE(moved.cropOffsetX, 0.4);
    QCOMPARE(moved.cropOffsetY, -0.7);
    QCOMPARE(document.pages.last().slotStates.size(), 2);
}

void AppTest::importingFillsMixedPagesAndKeepsEveryPhoto()
{
    AppController app;
    app.startManual("two");
    app.addPages("four", 1);
    app.setCurrentPage(0);
    QVariantList urls;
    for (int i = 0; i < 10; ++i) urls.append(QUrl::fromLocalFile(QString("/batch-%1.jpg").arg(i)));
    app.importPhotos(urls);
    QTemporaryDir dir;
    const auto document = saved(app, dir.filePath("import.liusu"));
    QStringList paths;
    for (const auto& page : document.pages)
        for (const auto& state : page.slotStates)
            if (!state.imagePath.isEmpty()) paths.append(state.imagePath);
    QCOMPARE(paths.size(), 10);
    for (int i = 0; i < 10; ++i) QCOMPARE(paths[i], QString("/batch-%1.jpg").arg(i));
    QCOMPARE(document.pages[1].slotStates.size(), 4);
}

void AppTest::cropAndExportSettingsSurviveReopen()
{
    AppController app;
    app.startManual("two");
    app.assignFileToSlot(0, QUrl::fromLocalFile("/photo.jpg"));
    app.setCropOffset(0, 8.0, -8.0);
    app.setExportSettings(600, false, 89);
    QTemporaryDir dir;
    const auto document = saved(app, dir.filePath("settings.liusu"));
    QCOMPARE(document.pages[0].slotStates[0].cropOffsetX, 1.0);
    QCOMPARE(document.pages[0].slotStates[0].cropOffsetY, -1.0);
    AppController reopened;
    QVERIFY(reopened.openProject(QUrl::fromLocalFile(dir.filePath("settings.liusu"))));
    QCOMPARE(reopened.exportPpi(), 600);
    QCOMPARE(reopened.exportJpeg(), false);
    QCOMPARE(reopened.exportQuality(), 89);
    reopened.selectSlotAt(0.1, 0.1);
    QCOMPARE(reopened.selectedSlotState().value("cropX").toDouble(), 1.0);
}

void AppTest::invalidCommandsLeaveDocumentUnchanged()
{
    AppController app;
    const auto url = app.previewUrl();
    app.addPages("unknown", 10);
    app.addPages("four", 0);
    app.addPages("four", 101);
    app.changeCurrentLayout("unknown");
    app.setCropOffset(0, std::numeric_limits<double>::quiet_NaN(), 0);
    app.setExportSettings(0, false, -1);
    QCOMPARE(app.pageCount(), 1);
    QCOMPARE(app.previewUrl(), url);
    QCOMPARE(app.exportPpi(), 300);
}

void AppTest::deletedPageCannotReuseOldPreview()
{
    AppController app;
    app.startManual("single");
    app.assignFileToSlot(0, QUrl::fromLocalFile(QFINDTESTDATA("../fixtures/exif1.jpg")));
    app.renderPageForProvider(0, QSize(300, 203));
    app.addPages("four", 1);
    const auto before = app.pageThumbnailUrl(0);
    app.setCurrentPage(0);
    app.deleteCurrentPage();
    QVERIFY(app.pageThumbnailUrl(0) != before);
    QCOMPARE(app.currentSlotCount(), 4);
}

void AppTest::mixedPagesExportAtConfiguredSize()
{
    AppController app;
    app.startManual("two");
    app.addPages("nine", 1);
    app.addPages("four", 1);
    QTemporaryDir dir;
    QVERIFY(app.exportAllPages(QUrl::fromLocalFile(dir.path()), 300, false, 95));
    for (int i = 1; i <= 3; ++i) {
        const QImage image(dir.filePath(QString("liusu-PG-%1.png").arg(i, 3, 10, QLatin1Char('0'))));
        QCOMPARE(image.size(), QSize(1748, 1181));
    }
}

void AppTest::repeatedExportsKeepExistingFiles()
{
    AppController app;
    app.startManual("single");
    QTemporaryDir dir;
    QFile existing(dir.filePath("liusu-PG-001.png"));
    QVERIFY(existing.open(QIODevice::WriteOnly));
    QCOMPARE(existing.write("keep this existing file"),qint64(23));
    existing.close();
    QVERIFY(app.exportAllPages(QUrl::fromLocalFile(dir.path()),72,false,95));
    QVERIFY(existing.open(QIODevice::ReadOnly));
    QCOMPARE(existing.readAll(),QByteArray("keep this existing file"));
    const QImage image(dir.filePath("liusu-2-PG-001.png"));
    QVERIFY(!image.isNull());
}

void AppTest::invalidFileUrlsAndExportSettingsAreRejected()
{
    AppController app;
    app.startManual("single");
    app.assignFileToSlot(0,QUrl::fromLocalFile("/existing.jpg"));
    const QString before = app.previewUrl();
    app.assignFileToSlot(0,QUrl("https://example.invalid/photo.jpg"));
    QCOMPARE(app.previewUrl(),before);
    app.importPhotos({QUrl("https://example.invalid/photo.jpg")});
    QCOMPARE(app.previewUrl(),before);
    QTemporaryDir dir;
    QVERIFY(!app.exportCurrentPage(QUrl::fromLocalFile(dir.filePath("invalid.png")),0,false,95));
    QVERIFY(!QFile::exists(dir.filePath("invalid.png")));
    QVERIFY(!app.exportAllPages(QUrl(),300,false,95));
}

void AppTest::emptyProjectEditsStillNeedReplacementProtection()
{
    AppController app;
    app.startManual("four");
    app.setBackground("#e6e9e0");
    app.setExportSettings(600,false,90);
    QVERIFY(app.hasContent());
    QTemporaryDir dir;
    const auto document = saved(app,dir.filePath("empty-edits.liusu"));
    QVERIFY(document.isValid());
    AppController reopened;
    QVERIFY(reopened.openProject(QUrl::fromLocalFile(dir.filePath("empty-edits.liusu"))));
    QVERIFY(reopened.hasContent());
}

void AppTest::templateEditsKeepOtherPagePreviews()
{
    AppController app;
    app.startManual("four");
    app.addPages("two",9);
    app.setCurrentPage(0);
    const auto other = app.pageThumbnailUrl(1);
    const auto current = app.previewUrl();
    app.changeCurrentLayout("nine");
    QCOMPARE(app.pageThumbnailUrl(1),other);
    QVERIFY(app.previewUrl()!=current);
}

void AppTest::importedTemplatesPersistAndWorkPerPage()
{
    QTemporaryDir storage;
    QTemporaryDir input;
    QFile file(input.filePath("pack.json"));
    QVERIFY(file.open(QIODevice::WriteOnly));
    const QByteArray pack=R"({"templates":[{"id":"triptych","name":"三联","category":"留存","caption":"TRIPTYCH","slots":[{"x":0,"y":0,"width":0.3,"height":1},{"x":0.35,"y":0,"width":0.3,"height":1},{"x":0.7,"y":0,"width":0.3,"height":1}]}]})";
    file.write(pack); file.close();
    AppController app(nullptr,storage.path());
    QVERIFY(app.importTemplateCatalog(QUrl::fromLocalFile(file.fileName())));
    app.startManual("triptych");
    QCOMPARE(app.currentSlotCount(),3);
    QCOMPARE(app.currentLayoutId(),QString("triptych"));
    app.addPages("nine",9);
    app.changeCurrentLayout("triptych");
    QCOMPARE(app.pageCount(),10);
    AppController reopened(nullptr,storage.path());
    reopened.startManual("triptych");
    QCOMPARE(reopened.currentSlotCount(),3);
    QVERIFY(!reopened.importTemplateCatalog(QUrl::fromLocalFile(file.fileName())));
    QCOMPARE(reopened.layoutPresets().size(),5);
}

void AppTest::exportOptionsDrivePersistentSettings()
{
    AppController app;
    app.startManual("four");
    QVERIFY(app.setExportOption("ppi",450));
    QVERIFY(app.setExportOption("format",QString("png")));
    QVERIFY(!app.setExportOption("future-unsupported",42));
    QCOMPARE(app.exportOptionValues().value("ppi").toInt(),450);
    QTemporaryDir dir;
    saved(app,dir.filePath("options.liusu"));
    AppController reopened;
    QVERIFY(reopened.openProject(QUrl::fromLocalFile(dir.filePath("options.liusu"))));
    QCOMPARE(reopened.exportOptionValues().value("format").toString(),QString("png"));
    QCOMPARE(reopened.exportOptionValues().value("ppi").toInt(),450);
}

void AppTest::templateIdentitySurvivesMissingCatalog()
{
    QTemporaryDir storage, absent, input;
    QFile pack(input.filePath("identity.json"));
    QVERIFY(pack.open(QIODevice::WriteOnly));
    pack.write(R"({"templates":[{"id":"alternate","name":"另一张整幅","slots":[{"x":0,"y":0,"width":1,"height":1}]}]})"); pack.close();
    AppController app(nullptr,storage.path());
    QVERIFY(app.importTemplateCatalog(QUrl::fromLocalFile(pack.fileName())));
    app.startManual("single");
    app.changeCurrentLayout("alternate");
    QCOMPARE(app.currentLayoutId(),QString("alternate"));
    const auto document=saved(app,input.filePath("identity.liusu"));
    QCOMPARE(document.pages.first().templateId,QString("alternate"));
    AppController reopened(nullptr,absent.path());
    QVERIFY(reopened.openProject(QUrl::fromLocalFile(input.filePath("identity.liusu"))));
    QCOMPARE(reopened.currentLayoutId(),QString("alternate"));
    QCOMPARE(reopened.currentSlotCount(),1);
    QVERIFY(reopened.exportCurrentPage(QUrl::fromLocalFile(input.filePath("missing-catalog.png")),72,false,90));
}

QTEST_MAIN(AppTest)
#include "tst_app.moc"
