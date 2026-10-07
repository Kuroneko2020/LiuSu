#include <QtTest/QtTest>

// 冒烟测试：验证测试框架与构建链路可用。业务测试随子方案 02 起进入各自目录。
class SmokeTest final : public QObject
{
    Q_OBJECT

private slots:
    void testFrameworkBoots();
    void testQtCoreBasics();
};

void SmokeTest::testFrameworkBoots()
{
    QVERIFY(true);
}

void SmokeTest::testQtCoreBasics()
{
    QString s = QStringLiteral("留素");
    QCOMPARE(s.length(), 2);
    QCOMPARE(1 + 1, 2);
}

QTEST_MAIN(SmokeTest)

#include "tst_smoke.moc"
