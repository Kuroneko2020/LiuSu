#include "app/AppController.h"
#include "app/AppTheme.h"
#include "app/PagePreviewProvider.h"
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>
#include <QQuickStyle>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtTest/QtTest>

class UiTest final : public QObject
{
    Q_OBJECT
    QTemporaryDir templateStorage;
    AppController controller{nullptr,templateStorage.path()};
    AppTheme theme;
    QQmlApplicationEngine engine;
    QQuickWindow* window = nullptr;
    QStringList warnings;
    QQuickItem* findItem(QQuickItem* parent, const QString& name)
    {
        if (parent->objectName() == name) return parent;
        for (auto* child : parent->childItems())
            if (auto* match = findItem(child,name)) return match;
        return nullptr;
    }
private slots:
    void initTestCase()
    {
        QQuickStyle::setStyle("Basic");
        qmlRegisterSingletonInstance("LiuSu",1,0,"AppTheme",&theme);
        engine.rootContext()->setContextProperty("app",&controller);
        engine.addImageProvider("liusu",new PagePreviewProvider(&controller));
        connect(&engine,&QQmlEngine::warnings,this,[this](const QList<QQmlError>& errors) {
            for (const auto& error : errors) warnings.append(error.toString());
        });
        engine.load(QUrl::fromLocalFile(QStringLiteral(LIUSU_SOURCE_DIR) + "/qml/Main.qml"));
        QVERIFY2(!engine.rootObjects().isEmpty(),qPrintable(warnings.join('\n')));
        window = qobject_cast<QQuickWindow*>(engine.rootObjects().first());
        QVERIFY(window);
        QTest::qWait(100);
    }
    void spatialCardsRespondWithoutChangingTheProject()
    {
        window->resize(1440,900);
        window->setProperty("editing",false);
        QTest::qWait(100);
        auto* card=findItem(window->contentItem(),"homeTemplate-two");
        QVERIFY(card);
        auto* surface=findItem(card,"cardSurface");
        QVERIFY(surface);
        const auto preview=controller.previewUrl();
        QTest::mouseMove(window,card->mapToScene(QPointF(card->width()*0.8,card->height()*0.25)).toPoint());
        QTRY_VERIFY_WITH_TIMEOUT(surface->property("altitude").toDouble()>12,2000);
        QTRY_VERIFY_WITH_TIMEOUT(qAbs(surface->property("angleY").toDouble())>4,2000);
        QCOMPARE(controller.previewUrl(),preview);
        QTest::mouseMove(window,QPoint(20,20));
        QTest::qWait(600);
        QVERIFY(qAbs(surface->property("altitude").toDouble())<0.5);
    }
    void homeTemplatesNavigateToEditor()
    {
        for (const QString& id : {"single","two","four","nine"}) {
            window->setProperty("editing",false);
            auto* button = findItem(window->contentItem(),"homeTemplate-"+id);
            QVERIFY(button);
            QVERIFY(QMetaObject::invokeMethod(button,"clicked"));
            QCOMPARE(controller.currentLayoutId(),id);
            QVERIFY(window->property("editing").toBool());
        }
    }
    void templatePreviewsHaveVisibleSlots()
    {
        for (const QString& id : {"single","two","four","nine"}) {
            auto* card = findItem(window->contentItem(),"homeTemplate-"+id);
            QVERIFY(card);
            auto* mini = findItem(card,"templateMini");
            QVERIFY(mini);
            int count = 0;
            for (auto* child : mini->childItems()) {
                if (child->objectName() != "templateMiniSlot") continue;
                ++count;
                QVERIFY(child->width() > 0 && child->height() > 0);
                QVERIFY(child->x()+child->width() <= mini->width()+0.01);
                QVERIFY(child->y()+child->height() <= mini->height()+0.01);
            }
            QCOMPARE(count,controller.presetSlots(id).size());
        }
    }
    void resizedPaperStaysInsideWorkspace()
    {
        window->setProperty("editing",true);
        auto* canvas = window->findChild<QQuickItem*>("pageCanvas");
        auto* paper = window->findChild<QQuickItem*>("pagePaper");
        QVERIFY(canvas && paper);
        for (const auto& size : {QSize(960,640),QSize(1080,680),QSize(1440,900)}) {
            window->resize(size);
            QTest::qWait(80);
            QVERIFY(paper->width() > 100);
            QVERIFY(paper->x() >= 0 && paper->y() >= 0);
            QVERIFY(paper->x()+paper->width() <= canvas->width());
            QVERIFY(paper->y()+paper->height() <= canvas->height());
            QVERIFY(qAbs(paper->width()/paper->height()-1.48) < 0.01);
        }
    }
    void resizedHomeAndBatchAddRemainUsable()
    {
        window->setProperty("editing",false);
        for (const auto& size : {QSize(960,640),QSize(1440,640),QSize(1440,900)}) {
            window->resize(size);
            QTest::qWait(80);
            auto* card = findItem(window->contentItem(),"homeTemplate-four");
            auto* preview = findItem(card,"templateMini");
            auto* title = findItem(card,"templateCardTitle");
            QVERIFY(preview->y()+preview->height() < title->y());
        }
        controller.startManual("four");
        window->setProperty("editing",true);
        auto* editor = window->findChild<QObject*>("editorPage");
        QVERIFY(QMetaObject::invokeMethod(editor,"addPages"));
        auto* input = window->findChild<QObject*>("pageCountInput");
        auto* dialog = window->findChild<QObject*>("addPagesDialog");
        QVERIFY(input && dialog);
        QVERIFY(input->setProperty("value",9));
        QTest::qWait(60);
        auto* accept = window->findChild<QQuickItem*>("addPagesAccept");
        QVERIFY(accept);
        const QPoint click = accept->mapToScene(QPointF(accept->width()/2,accept->height()/2)).toPoint();
        QTest::mouseClick(window,Qt::LeftButton,Qt::NoModifier,click);
        QCOMPARE(controller.pageCount(),10);
        QVERIFY(QMetaObject::invokeMethod(editor,"showExport"));
        QTest::qWait(80);
        auto* scroll = window->findChild<QQuickItem*>("inspectorScroll");
        auto* output = window->findChild<QQuickItem*>("outputSection");
        const QPointF point = output->mapToItem(scroll,QPointF(0,0));
        QVERIFY(point.y() >= -1 && point.y() < scroll->height());
        QVERIFY2(warnings.isEmpty(),qPrintable(warnings.join('\n')));
    }
    void mixedPagesAndSelectionBindingsStayValid()
    {
        controller.startManual("four");
        controller.addPages("two",9);
        controller.setCurrentPage(0);
        controller.assignFileToSlot(0,QUrl::fromLocalFile(QStringLiteral(LIUSU_SOURCE_DIR)+"/assets/photos/coast.png"));
        controller.selectSlotAt(0.1,0.1);
        controller.setCropOffset(0,0.5,-0.4);
        controller.changeCurrentLayout("nine");
        controller.setCurrentPage(9);
        controller.changeCurrentLayout("single");
        QTest::qWait(100);
        QVERIFY2(warnings.isEmpty(),qPrintable(warnings.join('\n')));
    }
    void renderReviewImages()
    {
        const QString directory = qEnvironmentVariable("LIUSU_REVIEW_DIR");
        if (directory.isEmpty()) QSKIP("Set LIUSU_REVIEW_DIR to capture QML rendering for visual review.");
        QVERIFY(QDir().mkpath(directory));
        window->resize(1440,900);
        window->setProperty("editing",false);
        controller.clearStatus();
        QTest::mouseMove(window,QPoint(20,20));
        QTest::qWait(300);
        QVERIFY(window->grabWindow().save(directory+"/home.png"));
        auto* card=findItem(window->contentItem(),"homeTemplate-two");
        QVERIFY(card);
        QTest::mouseMove(window,card->mapToScene(QPointF(card->width()*0.9,card->height()*0.2)).toPoint());
        for(int frame=0;frame<14;++frame) {
            QTest::qWait(40);
            QVERIFY(window->grabWindow().save(directory+QString("/motion-%1.png").arg(frame,2,10,QLatin1Char('0'))));
        }
        QVERIFY(window->grabWindow().save(directory+"/home-hover.png"));
        QTest::mouseMove(window,QPoint(20,20));
        for(int frame=14;frame<28;++frame) {
            QTest::qWait(40);
            QVERIFY(window->grabWindow().save(directory+QString("/motion-%1.png").arg(frame,2,10,QLatin1Char('0'))));
        }
        window->resize(1440,640);
        QTest::qWait(200);
        QVERIFY(window->grabWindow().save(directory+"/home-short.png"));
        window->resize(1440,900);
        controller.startManual("four");
        const QUrl photograph = QUrl::fromLocalFile(QStringLiteral(LIUSU_SOURCE_DIR)+"/assets/photos/coast.png");
        for (int slot=0;slot<4;++slot) controller.assignFileToSlot(slot,photograph);
        controller.addPages("two",9);
        controller.setCurrentPage(0);
        controller.selectSlotAt(0.1,0.1);
        window->setProperty("editing",true);
        controller.clearStatus();
        QTest::qWait(300);
        QTRY_COMPARE(window->findChild<QQuickItem*>("pagePaper")->property("arrival").toDouble(),1.0);
        QVERIFY(window->grabWindow().save(directory+"/editor.png"));
        window->resize(960,640);
        QTest::qWait(300);
        QVERIFY(window->grabWindow().save(directory+"/editor-small.png"));
    }
    void draggingBackToStartRestoresCrop()
    {
        window->setProperty("editing",true);
        window->resize(1440,900);
        controller.startManual("two");
        controller.assignFileToSlot(0,QUrl::fromLocalFile(QStringLiteral(LIUSU_SOURCE_DIR)+"/assets/photos/coast.png"));
        QTest::qWait(100);
        auto* paper = window->findChild<QQuickItem*>("pagePaper");
        const QPoint origin = paper->mapToScene(QPointF(paper->width()*0.2,paper->height()*0.5)).toPoint();
        QTest::mousePress(window,Qt::LeftButton,Qt::NoModifier,origin);
        QTest::mouseMove(window,origin+QPoint(30,0));
        QTest::qWait(60);
        QVERIFY(controller.selectedSlotState().value("cropX").toDouble()!=0);
        QTest::mouseMove(window,origin);
        QTest::mouseRelease(window,Qt::LeftButton,Qt::NoModifier,origin);
        QCOMPARE(controller.selectedSlotState().value("cropX").toDouble(),0.0);
    }
    void catalogSearchAndGeneratedExportControls()
    {
        QJsonArray entries;
        const QJsonArray rectangles{QJsonObject{{"x",0},{"y",0},{"width",1},{"height",1}}};
        for(int i=0;i<40;++i)
            entries.append(QJsonObject{{"id",QString("extra-%1").arg(i)},
                {"name",QString("扩展模板 %1").arg(i)},{"category","测试目录"},{"slots",rectangles}});
        QTemporaryDir input;
        QFile file(input.filePath("many.json"));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(QJsonDocument(QJsonObject{{"templates",entries}}).toJson()); file.close();
        QVERIFY(controller.importTemplateCatalog(QUrl::fromLocalFile(file.fileName())));
        window->setProperty("editing",false);
        auto* search=findItem(window->contentItem(),"templateSearch");
        QVERIFY(search);
        search->setProperty("text","extra-39");
        QTest::qWait(100);
        auto* card=findItem(window->contentItem(),"homeTemplate-extra-39");
        QVERIFY(card);
        QTest::mouseClick(window,Qt::LeftButton,Qt::NoModifier,card->mapToScene(QPointF(card->width()/2,card->height()/2)).toPoint());
        QCOMPARE(controller.currentLayoutId(),QString("extra-39"));
        auto* ppi=findItem(window->contentItem(),"exportOption-ppi");
        QVERIFY(ppi);
        ppi->setProperty("value",450);
        QVERIFY(QMetaObject::invokeMethod(ppi,"valueModified"));
        QCOMPARE(controller.exportPpi(),450);
        auto* panel=window->findChild<QObject*>("exportSettingsPanel");
        QVERIFY(panel);
        panel->setProperty("expanded",true);
        QVERIFY(controller.setExportOption("format",QString("png")));
        QTest::qWait(50);
        auto* quality=findItem(window->contentItem(),"exportOption-jpegQuality");
        QVERIFY(quality && !quality->isVisible());
        QVERIFY(controller.setExportOption("format",QString("jpeg")));
        QTest::qWait(50);
        QVERIFY(quality->isVisible());
        search->setProperty("text","");
        QVERIFY2(warnings.isEmpty(),qPrintable(warnings.join('\n')));
    }
    void templatePopupEscapePreservesAddDialog()
    {
        auto* editor=window->findChild<QObject*>("editorPage");
        QVERIFY(QMetaObject::invokeMethod(editor,"addPages"));
        QTest::qWait(200);
        auto* add=window->findChild<QObject*>("addPagesDialog");
        QVERIFY(add && add->property("visible").toBool());
        auto* popup=add->findChild<QObject*>("templateLibraryPopup");
        QVERIFY(popup);
        QVERIFY(QMetaObject::invokeMethod(popup,"open"));
        QTest::qWait(350);
        QTest::keyClick(window,Qt::Key_Escape);
        QTRY_VERIFY(!popup->property("visible").toBool());
        QVERIFY(add->property("visible").toBool());
        QVERIFY(QMetaObject::invokeMethod(add,"close"));
        QVERIFY2(warnings.isEmpty(),qPrintable(warnings.join('\n')));
    }
    void cleanupTestCase() { if(window) window->close(); }
};
QTEST_MAIN(UiTest)
#include "tst_ui.moc"
