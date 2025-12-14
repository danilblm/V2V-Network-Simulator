#ifndef SPATIALGRIDMODEL_H
#define SPATIALGRIDMODEL_H

#include <QMap>
#include <QList>
#include <QPair>
#include "VehicleModel.h"

// Structure pour représenter une cellule de grille
struct GridCell {
    int x;
    int y;

    bool operator==(const GridCell& other) const {
        return x == other.x && y == other.y;
    }

    bool operator<(const GridCell& other) const {
        if (x != other.x) return x < other.x;
        return y < other.y;
    }
};

// Hash pour utiliser GridCell comme clé de QMap
inline uint qHash(const GridCell& cell) {
    return qHash(QPair<int, int>(cell.x, cell.y));
}

class SpatialGrid
{
public:
    SpatialGrid(double minLat, double maxLat, double minLon, double maxLon, double cellSizeMeters = 500.0);

    // Réinitialiser la grille et ajouter tous les véhicules
    void updateGrid(const QList<Vehicle*>& vehicles);

    // Obtenir les véhicules voisins d'un véhicule (dans la même cellule et les cellules adjacentes)
    QList<Vehicle*> getNeighbors(const Vehicle* vehicle) const;

    // Obtenir la cellule d'un véhicule
    GridCell getCell(const Vehicle* vehicle) const;

    // Statistiques de debug
    int getCellCount() const { return m_grid.size(); }
    int getMaxVehiclesPerCell() const;
    double getAverageVehiclesPerCell() const;

private:
    // Convertir lat/lon en coordonnées de cellule
    GridCell latLonToCell(double lat, double lon) const;

    // Grille : cellule -> liste de véhicules dans cette cellule
    QMap<GridCell, QList<Vehicle*>> m_grid;

    // Limites de la carte
    double m_minLat;
    double m_maxLat;
    double m_minLon;
    double m_maxLon;

    // Taille d'une cellule en mètres (environ)
    double m_cellSizeMeters;

    // Taille d'une cellule en degrés (calculée)
    double m_cellSizeLat;
    double m_cellSizeLon;
};

#endif // SPATIALGRIDMODEL_H
