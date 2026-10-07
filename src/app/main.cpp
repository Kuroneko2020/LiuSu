#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // 界面风格稿选定前先用 Basic 风格占位；后续在 05 子方案按选定风格调整。
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    QQmlApplicationEngine engine;
    // QML 加载失败直接退出非零码，避免半启动状态被误判为可用。
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.loadFromModule("LiuSu", "Main");

    return app.exec();
}
