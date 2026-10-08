#include "app/AppController.h"
#include "app/AppTheme.h"
#include "app/PagePreviewProvider.h"
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>
#include <QQuickStyle>
#include <QDir>
#include <QtTest/QtTest>

class UiTest final : public QObject
{
    Q_OBJECT
    AppController controller;
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
        QTest::qWait(300);
        QVERIFY(window->grabWindow().save(directory+"/home.png"));
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
    void cleanupTestCase() { if(window) window->close(); }
};
QTEST_MAIN(UiTest)
#include "tst_ui.moc"
