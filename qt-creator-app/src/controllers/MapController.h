#ifndef MAPCONTROLLER_H
#define MAPCONTROLLER_H

#include <QObject>
#include <QVariantList>
#include <QGeoCoordinate>
#include "../models/GraphModel.h"

class MapController : public QObject {
    Q_OBJECT

public:
    explicit MapController(QObject *parent = nullptr);

    Q_INVOKABLE void loadOSMData(const QString &path);

    // ✅ Permet à SimulationController d'accéder au graphe
    Graph* getGraph() { return &graph; }

signals:
    void roadReady(const QVariantList &roads);

private:
    Graph graph;

    // ✅ Nouveau: obtenir la couleur selon le type de route
    QString getRoadColor(const QString &roadType);
    int getRoadWidth(const QString &roadType);
};

#endif // MAPCONTROLLER_H
