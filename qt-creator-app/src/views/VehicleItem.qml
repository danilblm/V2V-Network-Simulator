import QtQuick
import QtLocation
import QtPositioning

MapQuickItem {
    id: vehicleItem
    coordinate: QtPositioning.coordinate(model.lat, model.lon)
    anchorPoint.x: carImage.width / 2
    anchorPoint.y: carImage.height / 2

    // Icône de voiture
    sourceItem: Image {
        id: carImage
        source: "../assets/car.png"     // ← ton image locale
        width: 32
        height: 32
        smooth: true
        antialiasing: true
        transformOrigin: Item.Center
        rotation: model.angle || 0   // Rotation basée sur l'angle du véhicule
    }
}
