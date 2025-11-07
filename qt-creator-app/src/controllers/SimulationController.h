#ifndef SIMULATIONCONTROLLER_H
#define SIMULATIONCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QList>
#include <QVariant>

#include "../models/VehicleModel.h"
#include "../models/GraphModel.h"
#include "../models/InterferenceGraphModel.h"

class SimulationController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningChanged)
    Q_PROPERTY(int vehicleCount READ vehicleCount NOTIFY vehicleCountChanged)
    Q_PROPERTY(double timeScale READ timeScale WRITE setTimeScale NOTIFY timeScaleChanged)
    Q_PROPERTY(InterferenceGraph* interferenceGraph READ interferenceGraph CONSTANT)

public:
    explicit SimulationController(Graph* graph, QObject *parent = nullptr);
    ~SimulationController();

    bool isRunning() const { return m_isRunning; }
    int vehicleCount() const { return m_vehicles.size(); }
    double timeScale() const { return m_timeScale; }
    InterferenceGraph* interferenceGraph() { return &m_interferenceGraph; }

    Q_INVOKABLE void startSimulation();
    Q_INVOKABLE void pauseSimulation();
    Q_INVOKABLE void resetSimulation();
    Q_INVOKABLE void spawnVehicles(int count);
    Q_INVOKABLE QVariantList getVehiclePositions();
    Q_INVOKABLE QVariantList getVehiclesWithTransmissionRanges();

    // ✅ NOUVEAU: Obtenir les connexions V2V avec positions pour affichage
    Q_INVOKABLE QVariantList getV2VConnectionsWithPositions();

public slots:
    void setTimeScale(double scale);

signals:
    void isRunningChanged();
    void vehicleCountChanged();
    void timeScaleChanged();
    void vehiclePositionsUpdated(QVariantList positions);

private slots:
    void updateSimulation();

private:
    Node* getRandomStartNode();
    void clearVehicles();

    Graph* m_graph;
    QList<Vehicle*> m_vehicles;
    QTimer* m_timer;
    bool m_isRunning;
    double m_timeScale;
    qint64 m_lastUpdateTime;
    int m_nextVehicleId;

    // Graphe d'interférences V2V
    InterferenceGraph m_interferenceGraph;
};

#endif // SIMULATIONCONTROLLER_H
