#include "GraphModel.h"
#include <QDebug>

Graph::Graph() : nextId(0) {}

Graph::~Graph() {
    for (auto n : nodes) delete n;
    for (auto e : edges) delete e;
}

Node* Graph::getOrCreateNode(double lat, double lon) {
    QString key = QString("%1,%2").arg(lat, 0, 'f', 6).arg(lon, 0, 'f', 6);
    if (nodeMap.contains(key))
        return nodeMap[key];

    Node* n = new Node(nextId++, lat, lon);
    nodeMap[key] = n;
    nodes.append(n);
    return n;
}

void Graph::addEdge(double lat1, double lon1, double lat2, double lon2, const QString &type) {
    Node* n1 = getOrCreateNode(lat1, lon1);
    Node* n2 = getOrCreateNode(lat2, lon2);

    // ✅ Vérifier que les deux nœuds sont différents
    if (n1 == n2) {
        return; // Pas d'arête vers soi-même
    }

    double len = distance(lat1, lon1, lat2, lon2);

    // ✅ Créer une seule arête bidirectionnelle
    Edge* e = new Edge(n1, n2, len, type);
    edges.append(e);

    // ✅ IMPORTANT: Ajouter l'arête aux deux nœuds pour permettre la bidirectionnalité
    // Mais l'arête elle-même n'est créée qu'une fois
    n1->edges.append(e);

    // ✅ Vérifier qu'on n'ajoute pas l'arête deux fois au même nœud
    if (!n2->edges.contains(e)) {
        n2->edges.append(e);
    }
}

double Graph::distance(double lat1, double lon1, double lat2, double lon2) const {
    const double R = 6371000.0; // rayon Terre en mètres
    double dLat = qDegreesToRadians(lat2 - lat1);
    double dLon = qDegreesToRadians(lon2 - lon1);
    double a = qSin(dLat / 2) * qSin(dLat / 2) +
               qCos(qDegreesToRadians(lat1)) * qCos(qDegreesToRadians(lat2)) *
                   qSin(dLon / 2) * qSin(dLon / 2);
    double c = 2 * qAtan2(qSqrt(a), qSqrt(1 - a));
    return R * c;
}
