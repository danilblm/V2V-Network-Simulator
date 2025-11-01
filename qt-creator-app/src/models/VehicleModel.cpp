#include "VehicleModel.h"
#include <QRandomGenerator>
#include <QtMath>
#include <QDebug>

Vehicle::Vehicle(int id, Node* startNode, QObject *parent)
    : QObject(parent)
    , m_id(id)
    , m_currentNode(startNode)
    , m_currentEdge(nullptr)
    , m_targetNode(nullptr)
    , m_previousNode(nullptr)  // ✅ Initialisé à nullptr
    , m_currentLat(startNode->lat)
    , m_currentLon(startNode->lon)
    , m_speed(0.0)
    , m_progressOnEdge(0.0)
    , m_stuckCounter(0)
{
    selectNextEdge();
}

void Vehicle::update(double deltaTime)
{
    if (!m_currentEdge || !m_targetNode) {
        m_stuckCounter++;
        if (m_stuckCounter > 10) {
            qDebug() << "❌ Véhicule" << m_id << "bloqué pendant" << m_stuckCounter << "frames au nœud"
                     << (m_currentNode ? m_currentNode->id : -1);
        }
        selectNextEdge();
        if (!m_currentEdge || !m_targetNode) return;
    }

    m_stuckCounter = 0; // Reset si on a une arête valide

    // Distance parcourue pendant deltaTime
    double distanceTraveled = m_speed * deltaTime;

    // Mise à jour de la progression sur l'arête
    if (m_currentEdge->length > 0) {
        double progressIncrement = distanceTraveled / m_currentEdge->length;
        m_progressOnEdge += progressIncrement;
    }

    // ✅ Interpolation linéaire entre currentNode et targetNode
    double lat1 = m_currentNode->lat;
    double lon1 = m_currentNode->lon;
    double lat2 = m_targetNode->lat;
    double lon2 = m_targetNode->lon;

    // Clamp la progression entre 0 et 1
    double clampedProgress = qBound(0.0, m_progressOnEdge, 1.0);

    m_currentLat = lat1 + (lat2 - lat1) * clampedProgress;
    m_currentLon = lon1 + (lon2 - lon1) * clampedProgress;

    // Si on a atteint la fin de l'arête
    if (m_progressOnEdge >= 1.0) {
        m_progressOnEdge = 0.0;
        m_previousNode = m_currentNode;  // ✅ Sauvegarder d'où on vient
        m_currentNode = m_targetNode;
        selectNextEdge();
    }

    emit positionChanged();
}

void Vehicle::selectNextEdge()
{
    if (!m_currentNode || m_currentNode->edges.isEmpty()) {
        m_currentEdge = nullptr;
        m_targetNode = nullptr;
        m_speed = 0.0;

        // ✅ Si bloqué trop longtemps, réinitialiser l'historique
        if (m_stuckCounter > 30) {
            qDebug() << "🔄 Reset historique du véhicule" << m_id;
            m_previousNode = nullptr;
            m_stuckCounter = 0;
        }
        return;
    }

    // ✅ Construction de la liste des arêtes disponibles avec leur direction
    struct EdgeOption {
        Edge* edge;
        Node* destination;
    };

    QList<EdgeOption> availableEdges;

    for (Edge* edge : m_currentNode->edges) {
        Node* destination = nullptr;

        if (edge->from == m_currentNode) {
            destination = edge->to;
        } else if (edge->to == m_currentNode) {
            destination = edge->from;
        }

        // ✅ INTERDIRE de revenir au nœud précédent (sauf si aucune autre option)
        if (destination && destination != m_previousNode) {
            availableEdges.append({edge, destination});
        }
    }

    // ✅ Si aucune arête disponible (on est dans une impasse), permettre le demi-tour
    if (availableEdges.isEmpty()) {
        qDebug() << "⚠️ Véhicule" << m_id << "en impasse au nœud" << m_currentNode->id
                 << "- demi-tour forcé";

        // Reconstruire la liste EN AUTORISANT le retour arrière cette fois
        for (Edge* edge : m_currentNode->edges) {
            Node* destination = nullptr;

            if (edge->from == m_currentNode) {
                destination = edge->to;
            } else if (edge->to == m_currentNode) {
                destination = edge->from;
            }

            if (destination) {
                availableEdges.append({edge, destination});
            }
        }

        // Si vraiment aucune option, on est bloqué
        if (availableEdges.isEmpty()) {
            m_currentEdge = nullptr;
            m_targetNode = nullptr;
            m_speed = 0.0;
            return;
        }
    }

    // Sélection aléatoire parmi les arêtes qui NE retournent PAS en arrière
    int index = QRandomGenerator::global()->bounded(availableEdges.size());
    EdgeOption selected = availableEdges[index];

    m_currentEdge = selected.edge;
    m_targetNode = selected.destination;

    // ✅ Vérification de sécurité
    if (!m_targetNode || m_targetNode == m_currentNode) {
        qDebug() << "❌ Destination invalide pour véhicule" << m_id;
        m_currentEdge = nullptr;
        m_targetNode = nullptr;
        m_speed = 0.0;
        return;
    }

    // Calcul de la vitesse selon le type de route
    m_speed = calculateSpeed(m_currentEdge->type);
    emit speedChanged();
}

double Vehicle::calculateSpeed(const QString& roadType)
{
    // Vitesses moyennes en m/s selon le type de route
    if (roadType == "motorway") return 33.3;     // 120 km/h
    if (roadType == "trunk") return 27.8;        // 100 km/h
    if (roadType == "primary") return 22.2;      // 80 km/h
    if (roadType == "secondary") return 16.7;    // 60 km/h
    if (roadType == "tertiary") return 13.9;     // 50 km/h
    if (roadType == "residential") return 8.3;   // 30 km/h
    if (roadType == "service") return 5.6;       // 20 km/h

    return 11.1; // Défaut: 40 km/h
}
