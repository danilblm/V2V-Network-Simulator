#ifndef INTERFERENCEGRAPH_H
#define INTERFERENCEGRAPH_H

#include <QObject>
#include <QMap>
#include <QSet>
#include <QVector>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QString>
#include <QPair>
#include "VehicleModel.h"
#include "spatialgrid.h"

// ✅ ARÊTE DIRECTIONNELLE : A peut communiquer avec B
struct DirectedEdge {
    int fromVehicleId;  // Véhicule émetteur
    int toVehicleId;    // Véhicule dans le rayon
    double distance;
    double signalStrength;

    bool operator==(const DirectedEdge& other) const {
        return fromVehicleId == other.fromVehicleId && toVehicleId == other.toVehicleId;
    }
};

// ✅ CONNEXION POTENTIELLE : Les rayons se chevauchent (stockée, pas affichée)
struct PotentialConnection {
    int vehicleId1;
    int vehicleId2;
    double distance;
    bool rangesOverlap;  // Les cercles se touchent

    bool operator==(const PotentialConnection& other) const {
        return (vehicleId1 == other.vehicleId1 && vehicleId2 == other.vehicleId2) ||
               (vehicleId1 == other.vehicleId2 && vehicleId2 == other.vehicleId1);
    }
};

class InterferenceGraph : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int directedEdgeCount READ directedEdgeCount NOTIFY graphChanged)
    Q_PROPERTY(int potentialConnectionCount READ potentialConnectionCount NOTIFY graphChanged)
    Q_PROPERTY(int isolatedVehicleCount READ isolatedVehicleCount NOTIFY graphChanged)

public:
    explicit InterferenceGraph(QObject *parent = nullptr);

    // Initialiser la grille spatiale
    void initializeSpatialGrid(double minLat, double maxLat, double minLon, double maxLon);

    // ✅ Mise à jour du graphe d'interférences (OPTIMISÉ avec SpatialGrid)
    void updateGraph(const QList<Vehicle*>& vehicles);

    // Définir le rayon de transmission (en mètres)
    void setTransmissionRange(int vehicleId, double range);

    // Obtenir le rayon de transmission
    double getTransmissionRange(int vehicleId) const;

    // Obtenir les voisins accessibles depuis un véhicule (arêtes sortantes)
    QList<int> getReachableVehicles(int vehicleId) const;

    // Obtenir toutes les arêtes directionnelles (pour affichage visuel)
    QList<DirectedEdge> getDirectedEdges() const;

    // Obtenir toutes les connexions potentielles (chevauchement)
    QList<PotentialConnection> getPotentialConnections() const;

    // Statistiques
    int directedEdgeCount() const { return m_directedEdges.size(); }
    int potentialConnectionCount() const { return m_potentialConnections.size(); }
    int isolatedVehicleCount() const { return m_isolatedVehicles.size(); }

    // Export pour visualisation QML
    Q_INVOKABLE QVariantList getDirectedEdgesForVisualization() const;
    Q_INVOKABLE QVariantList getPotentialConnectionsForVisualization() const;
    Q_INVOKABLE QVariantMap getStatistics() const;

signals:
    void graphChanged();
    void newDirectedEdgeEstablished(int fromVehicleId, int toVehicleId, double distance);
    void directedEdgeLost(int fromVehicleId, int toVehicleId);
    void newPotentialConnection(int vehicleId1, int vehicleId2);
    void potentialConnectionLost(int vehicleId1, int vehicleId2);

private:
    double calculateDistance(const Vehicle* v1, const Vehicle* v2) const;
    double calculateSignalStrength(double distance, double transmissionRange) const;

    // ✅ NOUVEAU: Algorithme de fallback O(n²) si la grille n'est pas disponible
    void updateGraphBruteForce(const QList<Vehicle*>& vehicles);

    // ✅ ARÊTES DIRECTIONNELLES : from → to (affichées visuellement)
    QList<DirectedEdge> m_directedEdges;

    // Liste d'adjacence : vehicleId → liste des véhicules accessibles
    QMap<int, QSet<int>> m_adjacencyList;

    // ✅ CONNEXIONS POTENTIELLES : chevauchement de rayons (stockées uniquement)
    QList<PotentialConnection> m_potentialConnections;

    // Rayons de transmission par véhicule (100-500m)
    QMap<int, double> m_transmissionRanges;

    QSet<int> m_isolatedVehicles;

    // État précédent pour détection de changements
    QSet<QPair<int, int>> m_previousDirectedEdges;
    QSet<QPair<int, int>> m_previousPotentialConnections;

    // ✅ Grille spatiale pour optimisation O(n) au lieu de O(n²)
    SpatialGrid* m_spatialGrid;

    const double DEFAULT_TRANSMISSION_RANGE = 300.0;
};

#endif // INTERFERENCEGRAPH_H
