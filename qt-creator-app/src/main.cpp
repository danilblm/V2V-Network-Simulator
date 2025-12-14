#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QVariant>
#include <QVariantMap>
#include <QVariantList>
#include "controllers/MapController.h"
#include "SimulationController.h"
#include <QDir>
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
    // Dans main.cpp, après avoir créé simulationController
    SimulationController simulationController(mapController.getGraph());

    // ✅ Initialiser la grille spatiale (limites de Mulhouse)
    simulationController.interferenceGraph()->initializeSpatialGrid(
        47.70, 47.80,  // minLat, maxLat
        7.28, 7.40     // minLon, maxLon
        );

    engine.rootContext()->setContextProperty("simulationController", &simulationController);

    const QUrl url = QUrl::fromLocalFile(QDir::current().filePath("../../src/MainUI.qml"));
    engine.load(url);

    if (engine.rootObjects().isEmpty()) {
        qCritical() << "❌ Erreur : impossible de charger l'interface QML";
        return -1;
    }

    qDebug() << "✅ Interface QML chargée avec succès";
    qDebug() << "📡 Graphe d'interférences V2V activé";

    return app.exec();
}
