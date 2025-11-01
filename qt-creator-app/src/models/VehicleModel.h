#ifndef VEHICLE_H
#define VEHICLE_H

#include <QObject>
#include <QGeoCoordinate>
#include "GraphModel.h"

class Vehicle : public QObject {
    Q_OBJECT
    Q_PROPERTY(double latitude READ latitude NOTIFY positionChanged)
    Q_PROPERTY(double longitude READ longitude NOTIFY positionChanged)
    Q_PROPERTY(double speed READ speed NOTIFY speedChanged)

public:
    explicit Vehicle(int id, Node* startNode, QObject *parent = nullptr);

    int getId() const { return m_id; }
    double latitude() const { return m_currentLat; }
    double longitude() const { return m_currentLon; }
    double speed() const { return m_speed; }

    void update(double deltaTime);
    void selectNextEdge();

signals:
    void positionChanged();
    void speedChanged();

private:
    int m_id;
    Node* m_currentNode;
    Edge* m_currentEdge;

    // ✅ Pour gérer le sens de déplacement
    Node* m_targetNode;  // Nouveau: nœud de destination sur l'arête actuelle
    Node* m_previousNode; // ✅ NOUVEAU: pour éviter les demi-tours

    double m_currentLat;
    double m_currentLon;
    double m_speed;
    double m_progressOnEdge;  // 0.0 à 1.0

    // ✅ Debug: compteur de blocages
    int m_stuckCounter;

    double calculateSpeed(const QString& roadType);
};

#endif // VEHICLE_H
