#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QVariant>
#include <QVariantMap>
#include <QVariantList>
#include "controllers/MapController.h"
#include "SimulationController.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // ✅ Enregistrer les types pour Qt Meta-Object System
    qRegisterMetaType<QVariantMap>("QVariantMap");
    qRegisterMetaType<QVariantList>("QVariantList");

    QQmlApplicationEngine engine;
    qDebug() << "🚀 Application V2V Project démarrée";

    // Contrôleur de carte
    MapController mapController;
    engine.rootContext()->setContextProperty("mapController", &mapController);

    // Contrôleur de simulation avec graphe d'interférences
    SimulationController simulationController(mapController.getGraph());
    engine.rootContext()->setContextProperty("simulationController", &simulationController);

    engine.load(QUrl::fromLocalFile(R"(C:\Users\DELL\OneDrive\Bureau\Master-IM\projet-reseau-mobile\V2V-Network-Simulator\qt-creator-app\src\MainUI.qml)"));

    if (engine.rootObjects().isEmpty()) {
        qCritical() << "❌ Erreur : impossible de charger l'interface QML";
        return -1;
    }

    qDebug() << "✅ Interface QML chargée avec succès";
    qDebug() << "📡 Graphe d'interférences V2V activé";

    return app.exec();
}
