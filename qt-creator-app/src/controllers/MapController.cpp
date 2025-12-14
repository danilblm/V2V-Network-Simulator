#include "MapController.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QSet>

MapController::MapController(QObject *parent)
    : QObject(parent)
{
}

void MapController::loadOSMData(const QString &path)
{
    QString basePath = QCoreApplication::applicationDirPath();
    QString fullPath = basePath + "/../" + path;

    QFile file(fullPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "❌ Impossible d'ouvrir le fichier:" << fullPath;
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    QJsonArray elements = doc.object()["elements"].toArray();
    QVariantList roadList;

    // ✅ Types de routes autorisées pour les véhicules (ROUTES CARROSSABLES UNIQUEMENT)
    QSet<QString> allowedRoadTypes = {
        "motorway",           // Autoroute
        "motorway_link",      // Bretelle d'autoroute
        "trunk",              // Route nationale
        "trunk_link",         // Bretelle de nationale
        "primary",            // Route principale
        "primary_link",       // Bretelle de route principale
        "secondary",          // Route secondaire
        "secondary_link",     // Bretelle de route secondaire
        "tertiary",           // Route tertiaire
        "tertiary_link",      // Bretelle de route tertiaire
        "unclassified",       // Route non classée (mais carrossable)
        "residential",        // Route résidentielle
        "living_street",      // Zone de rencontre
        "service"             // Route de service (parking, etc.)
    };

    // ✅ Types INTERDITS (chemins piétons, vélos, etc.)
    QSet<QString> forbiddenTypes = {
        "footway",            // Chemin piéton
        "path",               // Sentier
        "pedestrian",         // Zone piétonne
        "steps",              // Escaliers
        "cycleway",           // Piste cyclable
        "bridleway",          // Chemin cavalier
        "track",              // Chemin agricole/forestier
        "corridor",           // Couloir intérieur
        "construction"        // Route en construction
    };

    int edgeCount = 0;
    int filteredCount = 0;

    for (auto el : elements) {
        QJsonObject obj = el.toObject();
        if (obj["type"] != "way") continue;

        QJsonArray geometry = obj["geometry"].toArray();
        if (geometry.size() < 2) continue;

        QString highwayType = obj["tags"].toObject()["highway"].toString();

        // ✅ FILTRE 1: Ignorer si pas de type highway
        if (highwayType.isEmpty()) {
            continue;
        }

        // ✅ FILTRE 2: Ignorer les types interdits
        if (forbiddenTypes.contains(highwayType)) {
            filteredCount++;
            continue;
        }

        // ✅ FILTRE 3: N'accepter QUE les types autorisés
        if (!allowedRoadTypes.contains(highwayType)) {
            filteredCount++;
            continue;
        }

        // ✅ FILTRE 4: Ignorer les routes piétonnes explicites
        QJsonObject tags = obj["tags"].toObject();
        QString foot = tags["foot"].toString();
        QString access = tags["access"].toString();

        if (foot == "designated" || access == "no") {
            filteredCount++;
            continue;
        }

        // ✅ Ajouter les segments au graphe
        for (int i = 0; i < geometry.size() - 1; ++i) {
            QJsonObject g1 = geometry[i].toObject();
            QJsonObject g2 = geometry[i + 1].toObject();

            double lat1 = g1["lat"].toDouble();
            double lon1 = g1["lon"].toDouble();
            double lat2 = g2["lat"].toDouble();
            double lon2 = g2["lon"].toDouble();

            graph.addEdge(lat1, lon1, lat2, lon2, highwayType);
        }

        // Pour l'affichage QML (polyline) avec métadonnées
        QVariantList coords;
        for (auto g : geometry) {
            auto geo = g.toObject();
            coords << QVariant::fromValue(QGeoCoordinate(
                geo["lat"].toDouble(),
                geo["lon"].toDouble()
                ));
        }

        // ✅ Créer un objet avec les coordonnées ET le type de route
        QVariantMap roadData;
        roadData["path"] = coords;
        roadData["type"] = highwayType;
        roadData["color"] = getRoadColor(highwayType);
        roadData["width"] = getRoadWidth(highwayType);

        roadList << roadData;
        edgeCount++;
    }
    emit roadReady(roadList);
}

// ✅ Obtenir la couleur selon le type de route
QString MapController::getRoadColor(const QString &roadType)
{
    if (roadType.startsWith("motorway")) return "#e74c3c";    // Rouge (autoroute)
    if (roadType.startsWith("trunk")) return "#e67e22";        // Orange (nationale)
    if (roadType.startsWith("primary")) return "#f39c12";      // Orange clair
    if (roadType.startsWith("secondary")) return "#f1c40f";    // Jaune
    if (roadType.startsWith("tertiary")) return "#3498db";     // Bleu
    if (roadType == "residential") return "#95a5a6";           // Gris
    if (roadType == "service") return "#bdc3c7";               // Gris clair
    if (roadType == "living_street") return "#2ecc71";         // Vert
    return "#7f8c8d"; // Gris par défaut
}

// ✅ Obtenir la largeur selon le type de route
int MapController::getRoadWidth(const QString &roadType)
{
    if (roadType.startsWith("motorway")) return 4;
    if (roadType.startsWith("trunk")) return 3;
    if (roadType.startsWith("primary")) return 3;
    if (roadType.startsWith("secondary")) return 2;
    return 2; // Défaut
}
