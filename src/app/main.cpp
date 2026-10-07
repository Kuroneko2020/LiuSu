#include "AppController.h"
#include "AppTheme.h"
#include "PagePreviewProvider.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("LiuSu"));
    QCoreApplication::setApplicationName(QStringLiteral("LiuSu"));

    QQuickStyle::setStyle(QStringLiteral("Basic"));

    AppController controller;
    // 界面主题单例（C++ 注册；QML 侧用法 AppTheme.xxx 不变）。
    // 实例必须先于引擎存在并在引擎整个生命周期内有效。
    AppTheme theme;
    qmlRegisterSingletonInstance("LiuSu", 1, 0, "AppTheme", &theme);

    QQmlApplicationEngine engine;
    engine.addImageProvider(QStringLiteral("liusu"), new PagePreviewProvider(&controller));
    engine.rootContext()->setContextProperty(QStringLiteral("app"), &controller);
    // QML 加载失败直接退出非零码，避免半启动状态被误判为可用。
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.loadFromModule("LiuSu", "Main");

    return app.exec();
}
