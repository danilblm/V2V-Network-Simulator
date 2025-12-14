#include "InterferenceGraphModel.h"
#include <QtMath>
#include <QRandomGenerator>

InterferenceGraph::InterferenceGraph(QObject *parent)
    : QObject(parent)
    , m_spatialGrid(nullptr)
{
}

void InterferenceGraph::initializeSpatialGrid(double minLat, double maxLat, double minLon, double maxLon)
{
    if (m_spatialGrid) {
        delete m_spatialGrid;
        m_spatialGrid = nullptr;
    }

    // Taille de cellule = 500m (suffisant pour couvrir les rayons de transmission max)
    m_spatialGrid = new SpatialGrid(minLat, maxLat, minLon, maxLon, 500.0);
}

void InterferenceGraph::updateGraph(const QList<Vehicle*>& vehicles)
{
    // Vérifier que la grille spatiale est initialisée
    if (!m_spatialGrid) {
        qWarning() << "⚠️ SpatialGrid non initialisée, utilisation de l'algorithme O(n²)";
        updateGraphBruteForce(vehicles);
        return;
    }

    // ✅ ÉTAPE 1: Mettre à jour la grille spatiale
    m_spatialGrid->updateGrid(vehicles);

    // Sauvegarder l'état précédent
    QSet<QPair<int, int>> currentDirectedEdges;
    QSet<QPair<int, int>> currentPotentialConnections;

    // Réinitialiser le graphe
    m_adjacencyList.clear();
    m_directedEdges.clear();
    m_potentialConnections.clear();
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

    // ✅ ÉTAPE 2: Pour chaque véhicule, ne vérifier QUE ses voisins dans la grille
    for (const Vehicle* v1 : vehicles) {
        int id1 = v1->getId();
        double range1 = m_transmissionRanges.value(id1, DEFAULT_TRANSMISSION_RANGE);

        // ✅ Obtenir uniquement les voisins proches (9 cellules max au lieu de tous les véhicules)
        QList<Vehicle*> neighbors = m_spatialGrid->getNeighbors(v1);

        for (const Vehicle* v2 : neighbors) {
            int id2 = v2->getId();
            double range2 = m_transmissionRanges.value(id2, DEFAULT_TRANSMISSION_RANGE);

            double distance = calculateDistance(v1, v2);

            // ✅ CAS 1: v2 est DANS le rayon de v1 → Arête directionnelle v1→v2
            if (distance <= range1) {
                // Créer l'arête directionnelle
                DirectedEdge edge;
                edge.fromVehicleId = id1;
                edge.toVehicleId = id2;
                edge.distance = distance;
                edge.signalStrength = calculateSignalStrength(distance, range1);

                m_directedEdges.append(edge);
                m_adjacencyList[id1].insert(id2);

                // Enregistrer pour détection de changements
                currentDirectedEdges.insert(qMakePair(id1, id2));
            }

            // ✅ CAS 2: Les rayons se chevauchent
            // Éviter les doublons en ne traitant que id1 < id2
            if (id1 < id2 && distance <= (range1 + range2)) {
                PotentialConnection conn;
                conn.vehicleId1 = id1;
                conn.vehicleId2 = id2;
                conn.distance = distance;
                conn.rangesOverlap = true;

                m_potentialConnections.append(conn);

                // Enregistrer pour détection de changements
                currentPotentialConnections.insert(qMakePair(id1, id2));
            }
        }
    }

    // Détecter les véhicules isolés (aucune arête sortante)
    for (const Vehicle* v : vehicles) {
        int id = v->getId();
        if (!m_adjacencyList.contains(id) || m_adjacencyList[id].isEmpty()) {
            m_isolatedVehicles.insert(id);
        }
    }

    // ✅ Détecter les nouvelles arêtes directionnelles
    for (const auto& pair : currentDirectedEdges) {
        if (!m_previousDirectedEdges.contains(pair)) {
            emit newDirectedEdgeEstablished(pair.first, pair.second, 0);
        }
    }

    // ✅ Détecter les arêtes perdues
    for (const auto& pair : m_previousDirectedEdges) {
        if (!currentDirectedEdges.contains(pair)) {
            emit directedEdgeLost(pair.first, pair.second);
        }
    }

    // ✅ Détecter les nouvelles connexions potentielles
    for (const auto& pair : currentPotentialConnections) {
        if (!m_previousPotentialConnections.contains(pair)) {
            emit newPotentialConnection(pair.first, pair.second);
        }
    }

    // ✅ Détecter les connexions potentielles perdues
    for (const auto& pair : m_previousPotentialConnections) {
        if (!currentPotentialConnections.contains(pair)) {
            emit potentialConnectionLost(pair.first, pair.second);
        }
    }

    m_previousDirectedEdges = currentDirectedEdges;
    m_previousPotentialConnections = currentPotentialConnections;

    emit graphChanged();
}

// ✅ FALLBACK: Algorithme O(n²) si la grille n'est pas disponible
void InterferenceGraph::updateGraphBruteForce(const QList<Vehicle*>& vehicles)
{
    // Sauvegarder l'état précédent
    QSet<QPair<int, int>> currentDirectedEdges;
    QSet<QPair<int, int>> currentPotentialConnections;

    // Réinitialiser le graphe
    m_adjacencyList.clear();
    m_directedEdges.clear();
    m_potentialConnections.clear();
    m_isolatedVehicles.clear();

    // Assigner des rayons de transmission aléatoires aux nouveaux véhicules
    for (const Vehicle* v : vehicles) {
        int id = v->getId();
        if (!m_transmissionRanges.contains(id)) {
            double range = 100.0 + QRandomGenerator::global()->bounded(401);
            m_transmissionRanges[id] = range;
        }
    }

    // Double boucle O(n²)
    for (int i = 0; i < vehicles.size(); ++i) {
        const Vehicle* v1 = vehicles[i];
        int id1 = v1->getId();
        double range1 = m_transmissionRanges.value(id1, DEFAULT_TRANSMISSION_RANGE);

        for (int j = 0; j < vehicles.size(); ++j) {
            if (i == j) continue;

            const Vehicle* v2 = vehicles[j];
            int id2 = v2->getId();
            double range2 = m_transmissionRanges.value(id2, DEFAULT_TRANSMISSION_RANGE);

            double distance = calculateDistance(v1, v2);

            // CAS 1: Arête directionnelle
            if (distance <= range1) {
                DirectedEdge edge;
                edge.fromVehicleId = id1;
                edge.toVehicleId = id2;
                edge.distance = distance;
                edge.signalStrength = calculateSignalStrength(distance, range1);

                m_directedEdges.append(edge);
                m_adjacencyList[id1].insert(id2);
                currentDirectedEdges.insert(qMakePair(id1, id2));
            }

            // CAS 2: Connexion potentielle
            if (i < j && distance <= (range1 + range2)) {
                PotentialConnection conn;
                conn.vehicleId1 = qMin(id1, id2);
                conn.vehicleId2 = qMax(id1, id2);
                conn.distance = distance;
                conn.rangesOverlap = true;

                m_potentialConnections.append(conn);
                currentPotentialConnections.insert(qMakePair(conn.vehicleId1, conn.vehicleId2));
            }
        }
    }

    // Détecter les véhicules isolés
    for (const Vehicle* v : vehicles) {
        int id = v->getId();
        if (!m_adjacencyList.contains(id) || m_adjacencyList[id].isEmpty()) {
            m_isolatedVehicles.insert(id);
        }
    }

    // Détection de changements
    for (const auto& pair : currentDirectedEdges) {
        if (!m_previousDirectedEdges.contains(pair)) {
            emit newDirectedEdgeEstablished(pair.first, pair.second, 0);
        }
    }

    for (const auto& pair : m_previousDirectedEdges) {
        if (!currentDirectedEdges.contains(pair)) {
            emit directedEdgeLost(pair.first, pair.second);
        }
    }

    for (const auto& pair : currentPotentialConnections) {
        if (!m_previousPotentialConnections.contains(pair)) {
            emit newPotentialConnection(pair.first, pair.second);
        }
    }

    for (const auto& pair : m_previousPotentialConnections) {
        if (!currentPotentialConnections.contains(pair)) {
            emit potentialConnectionLost(pair.first, pair.second);
        }
    }

    m_previousDirectedEdges = currentDirectedEdges;
    m_previousPotentialConnections = currentPotentialConnections;

    emit graphChanged();
}

void InterferenceGraph::setTransmissionRange(int vehicleId, double range)
{
    range = qBound(100.0, range, 500.0);
    m_transmissionRanges[vehicleId] = range;
}

double InterferenceGraph::getTransmissionRange(int vehicleId) const
{
    return m_transmissionRanges.value(vehicleId, DEFAULT_TRANSMISSION_RANGE);
}

QList<int> InterferenceGraph::getReachableVehicles(int vehicleId) const
{
    if (!m_adjacencyList.contains(vehicleId)) {
        return QList<int>();
    }
    return m_adjacencyList[vehicleId].values();
}

QList<DirectedEdge> InterferenceGraph::getDirectedEdges() const
{
    return m_directedEdges;
}

QList<PotentialConnection> InterferenceGraph::getPotentialConnections() const
{
    return m_potentialConnections;
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
    if (distance >= transmissionRange) {
        return 0.0;
    }

    double ratio = distance / transmissionRange;
    return 100.0 * (1.0 - ratio * ratio);
}

QVariantList InterferenceGraph::getDirectedEdgesForVisualization() const
{
    QVariantList result;

    for (const DirectedEdge& edge : m_directedEdges) {
        QVariantMap item;
        item["fromVehicle"] = edge.fromVehicleId;
        item["toVehicle"] = edge.toVehicleId;
        item["distance"] = QString::number(edge.distance, 'f', 0) + "m";
        item["signalStrength"] = QString::number(edge.signalStrength, 'f', 1) + "%";
        result.append(item);
    }

    return result;
}

QVariantList InterferenceGraph::getPotentialConnectionsForVisualization() const
{
    QVariantList result;

    for (const PotentialConnection& conn : m_potentialConnections) {
        QVariantMap item;
        item["vehicle1"] = conn.vehicleId1;
        item["vehicle2"] = conn.vehicleId2;
        item["distance"] = QString::number(conn.distance, 'f', 0) + "m";
        item["rangesOverlap"] = conn.rangesOverlap;
        result.append(item);
    }

    return result;
}

QVariantMap InterferenceGraph::getStatistics() const
{
    QVariantMap stats;

    stats["directedEdges"] = m_directedEdges.size();
    stats["potentialConnections"] = m_potentialConnections.size();
    stats["isolatedVehicles"] = isolatedVehicleCount();

    // Degré moyen sortant
    if (!m_adjacencyList.isEmpty()) {
        int totalOutDegree = 0;
        for (const auto& neighbors : m_adjacencyList) {
            totalOutDegree += neighbors.size();
        }
        stats["averageOutDegree"] = QString::number(
            (double)totalOutDegree / m_adjacencyList.size(), 'f', 2);
    } else {
        stats["averageOutDegree"] = "0.00";
    }

    // Distance moyenne des arêtes
    if (!m_directedEdges.isEmpty()) {
        double totalDistance = 0.0;
        for (const DirectedEdge& edge : m_directedEdges) {
            totalDistance += edge.distance;
        }
        stats["averageDistance"] = QString::number(
                                       totalDistance / m_directedEdges.size(), 'f', 0) + "m";
    } else {
        stats["averageDistance"] = "0m";
    }

    // ✅ Statistiques de la grille spatiale
    if (m_spatialGrid) {
        stats["gridCells"] = m_spatialGrid->getCellCount();
        stats["maxVehiclesPerCell"] = m_spatialGrid->getMaxVehiclesPerCell();
        stats["avgVehiclesPerCell"] = QString::number(
            m_spatialGrid->getAverageVehiclesPerCell(), 'f', 1);
    }

    return stats;
}
