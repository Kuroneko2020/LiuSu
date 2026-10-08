#include "domain/TemplateCatalog.h"
#include "domain/ExportOptions.h"
#include <QtTest/QtTest>

class CatalogTest final : public QObject {
    Q_OBJECT
private slots:
    void acceptsManyTemplatesWithMetadata() {
        liusu::domain::TemplateCatalog catalog;
        const int initial=catalog.entries().size();
        QByteArray entries;
        for(int i=0;i<40;++i) {
            if(i) entries+=',';
            entries+=QString(R"({"id":"extra-%1","name":"模板 %1","category":"实验","caption":"CONTACT","slots":[{"x":0,"y":0,"width":1,"height":1}]})").arg(i).toUtf8();
        }
        QString error;
        QVERIFY2(catalog.appendFromJson("{\"templates\":["+entries+"]}",&error),qPrintable(error));
        QCOMPARE(catalog.entries().size(),initial+40);
        const auto* definition=catalog.find("extra-39");
        QVERIFY(definition);
        QCOMPARE(definition->category,QString("实验"));
        QVERIFY(definition->layout.isValid());
    }
    void invalidPackIsAtomic() {
        liusu::domain::TemplateCatalog catalog;
        const int initial=catalog.entries().size();
        QString error;
        QVERIFY(!catalog.appendFromJson(R"({"templates":[{"id":"valid","name":"有效","slots":[{"x":0,"y":0,"width":1,"height":1}]},{"id":"bad","name":"无效","slots":[{"x":0,"y":0,"width":2,"height":1}]}]})",&error));
        QCOMPARE(catalog.entries().size(),initial);
        QVERIFY(!catalog.find("valid"));
        QVERIFY(!error.isEmpty());
        QVERIFY(!catalog.appendFromJson(R"({"templates":[{"id":"four","name":"冲突","slots":[{"x":0,"y":0,"width":1,"height":1}]}]})",&error));
    }
    void exportDefinitionsValidateValues() {
        using namespace liusu::domain;
        ExportSettings settings;
        QVERIFY(!exportOptionDefinitions().isEmpty());
        QString error;
        QVERIFY(setExportOption(settings,"ppi",450,&error));
        QVERIFY(setExportOption(settings,"format",QString("png"),&error));
        QVERIFY(setExportOption(settings,"jpegQuality",88,&error));
        QCOMPARE(settings.ppi,450);
        QVERIFY(!settings.jpegFormat);
        QCOMPARE(settings.jpegQuality,88);
        const auto before=settings;
        QVERIFY(!setExportOption(settings,"ppi",30,&error));
        QVERIFY(!setExportOption(settings,"format",QString("unimplemented"),&error));
        QVERIFY(!setExportOption(settings,"future-setting",true,&error));
        QVERIFY(!setExportOption(settings,"ppi",true,&error));
        QVERIFY(settings==before);
    }
    void rejectsOccludedPhotoSlots() {
        liusu::domain::TemplateCatalog catalog;
        QString error;
        QVERIFY(!catalog.appendFromJson(R"({"templates":[{"id":"overlap","name":"重叠","slots":[{"x":0,"y":0,"width":1,"height":1},{"x":0,"y":0,"width":1,"height":1}]}]})",&error));
        QVERIFY(!catalog.find("overlap"));
        QVERIFY(catalog.appendFromJson(R"({"templates":[{"id":"adjacent","name":"相邻","slots":[{"x":0,"y":0,"width":0.5,"height":1},{"x":0.5,"y":0,"width":0.5,"height":1}]}]})",&error));
    }
};
QTEST_GUILESS_MAIN(CatalogTest)
#include "tst_catalog.moc"
