#include "AppController.h"
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
