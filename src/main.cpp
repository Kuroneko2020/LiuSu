#include "app/AppController.h"

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("PhotoTemplateEditor"));
    QCoreApplication::setApplicationName(QStringLiteral("PhotoTemplateEditor"));
    QCoreApplication::setApplicationDisplayName(QStringLiteral("Photo Template Editor"));

    pte::AppController controller;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("appController"), &controller);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, []() {
        QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.loadFromModule("PhotoTemplateEditor", "Main");

    return app.exec();
}
