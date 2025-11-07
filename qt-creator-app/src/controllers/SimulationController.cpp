#include "SimulationController.h"
#include <QRandomGenerator>
#include <QDateTime>
#include <QDebug>
#include <QVariant>

SimulationController::SimulationController(Graph* graph, QObject *parent)
    : QObject(parent)
    , m_graph(graph)
    , m_timer(new QTimer(this))
    , m_isRunning(false)
    , m_timeScale(1.0)
    , m_lastUpdateTime(0)
    , m_nextVehicleId(0)
    , m_interferenceGraph(this)
{
    m_timer->setInterval(33); // ~30 Hz
    connect(m_timer, &QTimer::timeout, this, &SimulationController::updateSimulation);
}

SimulationController::~SimulationController()
{
    clearVehicles();
}

void SimulationController::startSimulation()
{
    if (m_isRunning) return;

    m_isRunning = true;
    m_lastUpdateTime = QDateTime::currentMSecsSinceEpoch();
    m_timer->start();
    emit isRunningChanged();
    qDebug() << "✅ Simulation démarrée avec" << m_vehicles.size() << "véhicules";
}

void SimulationController::pauseSimulation()
{
    if (!m_isRunning) return;

    m_isRunning = false;
    m_timer->stop();
    emit isRunningChanged();
    qDebug() << "⏸️ Simulation en pause";
}

void SimulationController::resetSimulation()
{
    pauseSimulation();
    clearVehicles();
    m_nextVehicleId = 0;
    emit vehicleCountChanged();
    emit vehiclePositionsUpdated(QVariantList());
    qDebug() << "🔄 Simulation réinitialisée";
}

void SimulationController::setTimeScale(double scale)
{
    if (scale < 0.1) scale = 0.1;
    if (scale > 10.0) scale = 10.0;

    m_timeScale = scale;
    emit timeScaleChanged();
    qDebug() << "⏱️ Échelle de temps:" << m_timeScale << "x";
}

void SimulationController::spawnVehicles(int count)
{
    if (!m_graph || m_graph->nodes.isEmpty()) {
        qWarning() << "❌ Impossible de créer des véhicules : graphe vide";
        return;
    }

    QList<Node*> validStartNodes;
    for (Node* node : m_graph->nodes) {
        if (!node->edges.isEmpty()) {
            validStartNodes.append(node);
        }
    }

    if (validStartNodes.isEmpty()) {
        qWarning() << "❌ Aucun nœud valide trouvé";
        return;
    }

    qDebug() << "🚗 Création de" << count << "véhicules sur" << validStartNodes.size() << "nœuds valides";

    for (int i = 0; i < count; ++i) {
        int index = QRandomGenerator::global()->bounded(validStartNodes.size());
        Node* startNode = validStartNodes[index];

        Vehicle* vehicle = new Vehicle(m_nextVehicleId++, startNode, this);
        m_vehicles.append(vehicle);
    }

    emit vehicleCountChanged();
    qDebug() << "✅ Total véhicules:" << m_vehicles.size();
}

QVariantList SimulationController::getVehiclePositions()
{
    QVariantList positions;
    positions.reserve(m_vehicles.size());

    for (Vehicle* vehicle : m_vehicles) {
        QVariantMap pos;
        pos["id"] = vehicle->getId();
        pos["lat"] = vehicle->latitude();
        pos["lon"] = vehicle->longitude();
        pos["speed"] = vehicle->speed();
        positions.append(pos);
    }

    return positions;
}

QVariantList SimulationController::getV2VConnectionsWithPositions()
{
    QVariantList result;

    // Obtenir toutes les connexions actives
    QList<V2VConnection> connections = m_interferenceGraph.getActiveConnections();

    // Créer un map rapide pour accéder aux véhicules par ID
    QMap<int, Vehicle*> vehicleMap;
    for (Vehicle* v : m_vehicles) {
        vehicleMap[v->getId()] = v;
    }

    // Pour chaque connexion, ajouter les positions des deux véhicules
    for (const V2VConnection& conn : connections) {
        Vehicle* v1 = vehicleMap.value(conn.vehicleId1, nullptr);
        Vehicle* v2 = vehicleMap.value(conn.vehicleId2, nullptr);

        // Vérifier que les deux véhicules existent
        if (v1 && v2) {
            QVariantMap item;
            item["vehicleId1"] = conn.vehicleId1;
            item["vehicleId2"] = conn.vehicleId2;
            item["lat1"] = v1->latitude();
            item["lon1"] = v1->longitude();
            item["lat2"] = v2->latitude();
            item["lon2"] = v2->longitude();
            item["distance"] = conn.distance;
            item["signalStrength"] = conn.signalStrength;

            result.append(item);
        }
    }

    return result;
}

void SimulationController::updateSimulation()
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    double deltaTime = (currentTime - m_lastUpdateTime) / 1000.0;
    m_lastUpdateTime = currentTime;

    deltaTime = qMin(deltaTime, 0.1);
    deltaTime *= m_timeScale;

    // Mise à jour des véhicules
    for (Vehicle* vehicle : m_vehicles) {
        vehicle->update(deltaTime);
    }

    // Mise à jour du graphe d'interférences V2V
    m_interferenceGraph.updateGraph(m_vehicles);

    // Envoi au QML
    static int frameCounter = 0;
    int updateFrequency = (m_vehicles.size() > 1000) ? 2 : 1;

    if (++frameCounter >= updateFrequency) {
        frameCounter = 0;
        QVariantList positions = getVehiclePositions();

        // DEBUG: Log toutes les 30 frames
        static int logCounter = 0;
        if (++logCounter >= 30) {
            logCounter = 0;
            qDebug() << "📍 Envoi de" << positions.size() << "véhicules au QML";
            if (!positions.isEmpty()) {
                QVariantMap first = positions.first().toMap();
                qDebug() << "   Premier véhicule: lat=" << first["lat"].toDouble()
                         << "lon=" << first["lon"].toDouble();
            }
        }

        emit vehiclePositionsUpdated(positions);
    }
}

Node* SimulationController::getRandomStartNode()
{
    if (m_graph->nodes.isEmpty()) return nullptr;

    int index = QRandomGenerator::global()->bounded(m_graph->nodes.size());
    return m_graph->nodes[index];
}

void SimulationController::clearVehicles()
{
    for (Vehicle* vehicle : m_vehicles) {
        delete vehicle;
    }
    m_vehicles.clear();
}

// Ajouter cette méthode dans SimulationController.cpp

QVariantList SimulationController::getVehiclesWithTransmissionRanges()
{
    QVariantList result;
    result.reserve(m_vehicles.size());

    for (Vehicle* vehicle : m_vehicles) {
        int vehicleId = vehicle->getId();

        // Récupérer le rayon de transmission depuis InterferenceGraph
        double transmissionRange = m_interferenceGraph.getTransmissionRange(vehicleId);

        QVariantMap item;
        item["id"] = vehicleId;
        item["lat"] = vehicle->latitude();
        item["lon"] = vehicle->longitude();
        item["transmissionRange"] = transmissionRange;  // En mètres

        result.append(item);
    }

    return result;
}
