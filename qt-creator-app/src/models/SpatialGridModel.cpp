#include "SpatialGridModel.h"
#include <QtMath>

SpatialGrid::SpatialGrid(double minLat, double maxLat, double minLon, double maxLon, double cellSizeMeters)
    : m_minLat(minLat)
    , m_maxLat(maxLat)
    , m_minLon(minLon)
    , m_maxLon(maxLon)
    , m_cellSizeMeters(cellSizeMeters)
{
    // Approximation : 1 degré de latitude ≈ 111 km
    m_cellSizeLat = cellSizeMeters / 111000.0;

    // Approximation : 1 degré de longitude ≈ 111 km * cos(latitude)
    // On utilise la latitude moyenne
    double avgLat = (minLat + maxLat) / 2.0;
    double latRadians = qDegreesToRadians(avgLat);
    m_cellSizeLon = cellSizeMeters / (111000.0 * qCos(latRadians));
}

void SpatialGrid::updateGrid(const QList<Vehicle*>& vehicles)
{
    // Réinitialiser la grille
    m_grid.clear();

    // Placer chaque véhicule dans sa cellule
    for (Vehicle* vehicle : vehicles) {
        GridCell cell = getCell(vehicle);
        m_grid[cell].append(vehicle);
    }
}

QList<Vehicle*> SpatialGrid::getNeighbors(const Vehicle* vehicle) const
{
    QList<Vehicle*> neighbors;
    GridCell centerCell = getCell(vehicle);

    // Parcourir la cellule centrale et les 8 cellules adjacentes (voisinage de Moore)
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            GridCell neighborCell;
            neighborCell.x = centerCell.x + dx;
            neighborCell.y = centerCell.y + dy;

            // Si la cellule existe dans la grille
            if (m_grid.contains(neighborCell)) {
                const QList<Vehicle*>& cellVehicles = m_grid[neighborCell];
                for (Vehicle* v : cellVehicles) {
                    // Ne pas inclure le véhicule lui-même
                    if (v != vehicle) {
                        neighbors.append(v);
                    }
                }
            }
        }
    }

    return neighbors;
}

GridCell SpatialGrid::getCell(const Vehicle* vehicle) const
{
    return latLonToCell(vehicle->latitude(), vehicle->longitude());
}

GridCell SpatialGrid::latLonToCell(double lat, double lon) const
{
    GridCell cell;

    // Convertir les coordonnées géographiques en indices de cellule
    cell.x = static_cast<int>((lon - m_minLon) / m_cellSizeLon);
    cell.y = static_cast<int>((lat - m_minLat) / m_cellSizeLat);

    return cell;
}

int SpatialGrid::getMaxVehiclesPerCell() const
{
    int maxCount = 0;
    for (const QList<Vehicle*>& cellVehicles : m_grid) {
        if (cellVehicles.size() > maxCount) {
            maxCount = cellVehicles.size();
        }
    }
    return maxCount;
}

double SpatialGrid::getAverageVehiclesPerCell() const
{
    if (m_grid.isEmpty()) return 0.0;

    int totalVehicles = 0;
    for (const QList<Vehicle*>& cellVehicles : m_grid) {
        totalVehicles += cellVehicles.size();
    }

    return static_cast<double>(totalVehicles) / m_grid.size();
}
