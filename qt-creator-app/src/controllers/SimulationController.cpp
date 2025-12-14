#include "SimulationController.h"
#include <QRandomGenerator>
#include <QDateTime>
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
}

void SimulationController::pauseSimulation()
{
    if (!m_isRunning) return;

    m_isRunning = false;
    m_timer->stop();
    emit isRunningChanged();
}

void SimulationController::resetSimulation()
{
    pauseSimulation();
    clearVehicles();
    m_nextVehicleId = 0;
    emit vehicleCountChanged();
    emit vehiclePositionsUpdated(QVariantList());
}

void SimulationController::setTimeScale(double scale)
{
    if (scale < 0.1) scale = 0.1;
    if (scale > 10.0) scale = 10.0;

    m_timeScale = scale;
    emit timeScaleChanged();
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

    for (int i = 0; i < count; ++i) {
        int index = QRandomGenerator::global()->bounded(validStartNodes.size());
        Node* startNode = validStartNodes[index];

        Vehicle* vehicle = new Vehicle(m_nextVehicleId++, startNode, this);
        m_vehicles.append(vehicle);
    }

    emit vehicleCountChanged();
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

    // ✅ Obtenir les ARÊTES DIRECTIONNELLES (seules affichées visuellement)
    QList<DirectedEdge> edges = m_interferenceGraph.getDirectedEdges();

    // Créer un map rapide pour accéder aux véhicules par ID
    QMap<int, Vehicle*> vehicleMap;
    for (Vehicle* v : m_vehicles) {
        vehicleMap[v->getId()] = v;
    }

    // Pour chaque arête, ajouter les positions avec direction
    for (const DirectedEdge& edge : edges) {
        Vehicle* vFrom = vehicleMap.value(edge.fromVehicleId, nullptr);
        Vehicle* vTo = vehicleMap.value(edge.toVehicleId, nullptr);

        if (vFrom && vTo) {
            QVariantMap item;
            item["fromVehicleId"] = edge.fromVehicleId;
            item["toVehicleId"] = edge.toVehicleId;
            item["lat1"] = vFrom->latitude();
            item["lon1"] = vFrom->longitude();
            item["lat2"] = vTo->latitude();
            item["lon2"] = vTo->longitude();
            item["distance"] = edge.distance;
            item["signalStrength"] = edge.signalStrength;
            item["isDirected"] = true;  // Pour dessiner une flèche

            result.append(item);
        }
    }

    return result;
}

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
            if (!positions.isEmpty()) {
                QVariantMap first = positions.first().toMap();
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
