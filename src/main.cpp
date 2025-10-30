#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "controllers/MapController.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    MapController mapController;
    engine.rootContext()->setContextProperty("mapController", &mapController);

    engine.load(QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/../qml/MainUI.qml"));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
