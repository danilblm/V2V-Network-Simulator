#ifndef INTERFERENCEGRAPHMODELH_H
#define INTERFERENCEGRAPHMODELH_H

#include <QObject>
#include <QMap>
#include <QSet>
#include <QVector>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QString>
#include <QPair>

#include "models/VehicleModel.h"

// Structure représentant une connexion V2V entre deux véhicules
struct V2VConnection {
    int vehicleId1;
    int vehicleId2;
    double distance;        // Distance entre les deux véhicules
    double signalStrength;  // Force du signal (basée sur la distance)

    bool operator==(const V2VConnection& other) const {
        return (vehicleId1 == other.vehicleId1 && vehicleId2 == other.vehicleId2) ||
               (vehicleId1 == other.vehicleId2 && vehicleId2 == other.vehicleId1);
    }
};

class InterferenceGraph : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int connectionCount READ connectionCount NOTIFY connectionsChanged)
    Q_PROPERTY(int isolatedVehicleCount READ isolatedVehicleCount NOTIFY connectionsChanged)

public:
    explicit InterferenceGraph(QObject *parent = nullptr);

    // Mise à jour du graphe d'interférences
    void updateGraph(const QList<Vehicle*>& vehicles);

    // Définir le rayon de transmission (en mètres)
    void setTransmissionRange(int vehicleId, double range);

    // Obtenir les voisins d'un véhicule
    QList<int> getNeighbors(int vehicleId) const;

    // Obtenir toutes les connexions actives
    QList<V2VConnection> getActiveConnections() const;

    // Statistiques
    int connectionCount() const { return m_activeConnections.size(); }
    int isolatedVehicleCount() const { return m_isolatedVehicles.size(); }
    int connectedVehicleCount() const { return m_adjacencyList.size(); }

    double getTransmissionRange(int vehicleId) const;

    // Export pour visualisation QML
    Q_INVOKABLE QVariantList getConnectionsForVisualization() const;
    Q_INVOKABLE QVariantMap getStatistics() const;

signals:
    void connectionsChanged();
    void newConnectionEstablished(int vehicleId1, int vehicleId2, double distance);
    void connectionLost(int vehicleId1, int vehicleId2);

private:
    // Calcul de distance entre deux véhicules
    double calculateDistance(const Vehicle* v1, const Vehicle* v2) const;

    // Calcul de la force du signal
    double calculateSignalStrength(double distance, double transmissionRange) const;

    // Vérifier si deux véhicules sont en portée
    bool areInRange(const Vehicle* v1, const Vehicle* v2, double range1, double range2) const;

    // Liste d'adjacence du graphe: vehicleId -> liste des voisins
    QMap<int, QSet<int>> m_adjacencyList;

    // Connexions actives
    QList<V2VConnection> m_activeConnections;

    // Rayons de transmission par véhicule (100-500m)
    QMap<int, double> m_transmissionRanges;

    // Véhicules isolés (sans connexion)
    QSet<int> m_isolatedVehicles;

    // Connexions de la frame précédente (pour détecter changements)
    QSet<QPair<int, int>> m_previousConnections;

    // Rayon de transmission par défaut
    const double DEFAULT_TRANSMISSION_RANGE = 300.0; // 300 mètres
};

#endif // INTERFERENCEGRAPHMODELH_H
