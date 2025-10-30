#include "MapController.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

MapController::MapController(QObject *parent)
    : QObject(parent)
{
}

void MapController::loadOSMData(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "❌ Impossible d'ouvrir" << path;
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    QJsonArray elements = doc.object()["elements"].toArray();
    QVariantList roadList;

    for (auto el : elements) {
        QJsonObject obj = el.toObject();
        if (obj["type"] != "way") continue;

        QJsonArray geometry = obj["geometry"].toArray();
        QVariantList coords;

        for (auto g : geometry) {
            auto geo = g.toObject();
            coords << QVariant::fromValue(QGeoCoordinate(
                geo["lat"].toDouble(),
                geo["lon"].toDouble()
            ));
        }

        roadList << QVariant::fromValue(coords);
    }

    qDebug() << "✅ Routes chargées:" << roadList.size();
    emit roadReady(roadList);
}
