#ifndef GRAPH_H
#define GRAPH_H

#include <QList>
#include <QMap>
#include <QString>
#include <QtMath>

class Node;
class Edge;

class Node {
public:
    int id;
    double lat;
    double lon;
    QList<Edge*> edges;

    Node(int id_, double lat_, double lon_) : id(id_), lat(lat_), lon(lon_) {}
};

class Edge {
public:
    Node* from;
    Node* to;
    double length;   // en mètres
    QString type;    // type de route (highway, trunk...)

    Edge(Node* a, Node* b, double len, const QString &t)
        : from(a), to(b), length(len), type(t) {}
};

class Graph {
public:
    QList<Node*> nodes;
    QList<Edge*> edges;

    Graph();
    ~Graph();

    Node* getOrCreateNode(double lat, double lon);
    void addEdge(double lat1, double lon1, double lat2, double lon2, const QString &type);
    double distance(double lat1, double lon1, double lat2, double lon2) const;

private:
    QMap<QString, Node*> nodeMap; // clé = "lat,lon"
    int nextId;
};

#endif // GRAPH_H
