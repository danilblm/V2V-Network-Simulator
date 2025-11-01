#include "InterferenceGraphModel.h"
#include <QtMath>
#include <QDebug>
#include <QRandomGenerator>

InterferenceGraph::InterferenceGraph(QObject *parent)
    : QObject(parent)
{
}

void InterferenceGraph::updateGraph(const QList<Vehicle*>& vehicles)
{
    // Sauvegarder l'état précédent
    QSet<QPair<int, int>> currentConnections;

    // Réinitialiser le graphe
    m_adjacencyList.clear();
    m_activeConnections.clear();
    m_isolatedVehicles.clear();

    // Assigner des rayons de transmission aléatoires aux nouveaux véhicules
    for (const Vehicle* v : vehicles) {
        int id = v->getId();
        if (!m_transmissionRanges.contains(id)) {
            // Rayon aléatoire entre 100 et 500 mètres
            double range = 100.0 + QRandomGenerator::global()->bounded(401);
            m_transmissionRanges[id] = range;
        }
    }

    // Construire le graphe d'interférences
    for (int i = 0; i < vehicles.size(); ++i) {
        const Vehicle* v1 = vehicles[i];
        int id1 = v1->getId();
        double range1 = m_transmissionRanges.value(id1, DEFAULT_TRANSMISSION_RANGE);

        bool hasConnection = false;

        for (int j = i + 1; j < vehicles.size(); ++j) {
            const Vehicle* v2 = vehicles[j];
            int id2 = v2->getId();
            double range2 = m_transmissionRanges.value(id2, DEFAULT_TRANSMISSION_RANGE);

            // Vérifier si les deux véhicules sont en portée
            if (areInRange(v1, v2, range1, range2)) {
                double distance = calculateDistance(v1, v2);

                // Ajouter la connexion au graphe
                m_adjacencyList[id1].insert(id2);
                m_adjacencyList[id2].insert(id1);

                // Créer la connexion V2V
                V2VConnection conn;
                conn.vehicleId1 = qMin(id1, id2);
                conn.vehicleId2 = qMax(id1, id2);
                conn.distance = distance;
                conn.signalStrength = calculateSignalStrength(distance, qMin(range1, range2));

                m_activeConnections.append(conn);

                // Enregistrer pour détection de changements
                currentConnections.insert(qMakePair(conn.vehicleId1, conn.vehicleId2));

                hasConnection = true;
            }
        }

        // Si le véhicule n'a aucune connexion, il est isolé
        if (!hasConnection && !m_adjacencyList.contains(id1)) {
            m_isolatedVehicles.insert(id1);
        }
    }

    // Détecter les nouvelles connexions
    for (const auto& pair : currentConnections) {
        if (!m_previousConnections.contains(pair)) {
            const V2VConnection* conn = nullptr;
            for (const auto& c : m_activeConnections) {
                if ((c.vehicleId1 == pair.first && c.vehicleId2 == pair.second)) {
                    conn = &c;
                    break;
                }
            }
            if (conn) {
                emit newConnectionEstablished(conn->vehicleId1, conn->vehicleId2, conn->distance);
            }
        }
    }

    // Détecter les connexions perdues
    for (const auto& pair : m_previousConnections) {
        if (!currentConnections.contains(pair)) {
            emit connectionLost(pair.first, pair.second);
        }
    }

    m_previousConnections = currentConnections;
    emit connectionsChanged();
}

void InterferenceGraph::setTransmissionRange(int vehicleId, double range)
{
    // Limiter entre 100 et 500 mètres
    range = qBound(100.0, range, 500.0);
    m_transmissionRanges[vehicleId] = range;
}

QList<int> InterferenceGraph::getNeighbors(int vehicleId) const
{
    if (!m_adjacencyList.contains(vehicleId)) {
        return QList<int>();
    }

    return m_adjacencyList[vehicleId].values();
}

QList<V2VConnection> InterferenceGraph::getActiveConnections() const
{
    return m_activeConnections;
}

double InterferenceGraph::calculateDistance(const Vehicle* v1, const Vehicle* v2) const
{
    const double R = 6371000.0; // Rayon de la Terre en mètres

    double lat1 = qDegreesToRadians(v1->latitude());
    double lon1 = qDegreesToRadians(v1->longitude());
    double lat2 = qDegreesToRadians(v2->latitude());
    double lon2 = qDegreesToRadians(v2->longitude());

    double dLat = lat2 - lat1;
    double dLon = lon2 - lon1;

    double a = qSin(dLat / 2) * qSin(dLat / 2) +
               qCos(lat1) * qCos(lat2) *
                   qSin(dLon / 2) * qSin(dLon / 2);

    double c = 2 * qAtan2(qSqrt(a), qSqrt(1 - a));

    return R * c;
}

double InterferenceGraph::calculateSignalStrength(double distance, double transmissionRange) const
{
    // Modèle de propagation simple: force décroit avec la distance
    // Signal strength = 100% à distance 0, 0% à la limite du rayon
    if (distance >= transmissionRange) {
        return 0.0;
    }

    // Formule logarithmique pour simuler l'atténuation du signal
    double ratio = distance / transmissionRange;
    return 100.0 * (1.0 - ratio * ratio); // Atténuation quadratique
}

bool InterferenceGraph::areInRange(const Vehicle* v1, const Vehicle* v2,
                                   double range1, double range2) const
{
    double distance = calculateDistance(v1, v2);

    // Les véhicules sont en portée si la distance est inférieure
    // au rayon de transmission de l'un OU de l'autre
    return distance <= range1 || distance <= range2;
}

QVariantList InterferenceGraph::getConnectionsForVisualization() const
{
    QVariantList result;

    for (const V2VConnection& conn : m_activeConnections) {
        QVariantMap item;
        item["vehicle1"] = conn.vehicleId1;
        item["vehicle2"] = conn.vehicleId2;
        item["distance"] = QString::number(conn.distance, 'f', 0) + "m";
        item["signalStrength"] = QString::number(conn.signalStrength, 'f', 1) + "%";
        result.append(item);
    }

    return result;
}

QVariantMap InterferenceGraph::getStatistics() const
{
    QVariantMap stats;

    stats["totalConnections"] = m_activeConnections.size();
    stats["connectedVehicles"] = connectedVehicleCount();
    stats["isolatedVehicles"] = isolatedVehicleCount();

    // Calculer le degré moyen (nombre moyen de connexions par véhicule)
    if (!m_adjacencyList.isEmpty()) {
        int totalDegree = 0;
        for (const auto& neighbors : m_adjacencyList) {
            totalDegree += neighbors.size();
        }
        stats["averageDegree"] = QString::number(
            (double)totalDegree / m_adjacencyList.size(), 'f', 2);
    } else {
        stats["averageDegree"] = "0.00";
    }

    // Distance moyenne des connexions
    if (!m_activeConnections.isEmpty()) {
        double totalDistance = 0.0;
        for (const V2VConnection& conn : m_activeConnections) {
            totalDistance += conn.distance;
        }
        stats["averageDistance"] = QString::number(
                                       totalDistance / m_activeConnections.size(), 'f', 0) + "m";
    } else {
        stats["averageDistance"] = "0m";
    }

    return stats;
}
