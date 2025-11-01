import QtQuick
import QtQuick.Controls
import QtLocation
import QtPositioning

Item {
    id: mapContainer
    width: parent ? parent.width : 800
    height: parent ? parent.height : 600

    property real minLat: 47.70
    property real maxLat: 47.80
    property real minLon: 7.28
    property real maxLon: 7.40

    signal mapLoaded()

    Map {
        id: map
        anchors.fill: parent

        plugin: Plugin {
            name: "osm"
            PluginParameter { name: "osm.mapping.providersrepository.disabled"; value: true }
            PluginParameter { name: "osm.mapping.host"; value: "https://tile.openstreetmap.org/" }
            PluginParameter { name: "osm.mapping.secure"; value: true }
        }

        center: QtPositioning.coordinate(47.7508, 7.3359)
        zoomLevel: 14
        minimumZoomLevel: 12
        maximumZoomLevel: 18

        onCenterChanged: {
            if (center.latitude < minLat) center.latitude = minLat
            if (center.latitude > maxLat) center.latitude = maxLat
            if (center.longitude < minLon) center.longitude = minLon
            if (center.longitude > maxLon) center.longitude = maxLon
        }

        Component.onCompleted: {
            console.log("🗺️ Carte initialisée")
            mapContainer.mapLoaded()
        }

        // ✅ Layer pour les connexions V2V (DOIT être AVANT les véhicules pour apparaître dessous)
        MapItemView {
            id: connectionLayer
            model: ListModel { id: connectionModel }

            delegate: MapPolyline {
                line.width: 2
                line.color: model.signalStrength > 80 ? "#2ecc71" : "#e67e22"
                opacity: 0.6
                path: [
                    QtPositioning.coordinate(model.lat1, model.lon1),
                    QtPositioning.coordinate(model.lat2, model.lon2)
                ]
            }
        }

        // ✅ Layer Canvas pour dessiner tous les véhicules
        MapItemView {
            id: vehicleLayer
            model: ListModel { id: vehicleModel }

            delegate: MapCircle {
                center: QtPositioning.coordinate(model.lat, model.lon)
                radius: 8
                color: model.color || "#ff0000"
                border.width: 2
                border.color: "#ffffff"
                opacity: 0.9
            }
        }
    }

    // 🔄 Réception des routes depuis le C++
    Connections {
        target: mapController
        function onRoadReady(roads) {
            console.log("✅ Nombre total de routes reçues :", roads.length)
            const maxRoutes = Math.min(5000, roads.length)
            console.log("🟦 Affichage de", maxRoutes, "routes")

            for (let i = 0; i < maxRoutes; ++i) {
                let roadData = roads[i]
                let path = roadData.path

                if (!path || path.length < 2) continue

                let polyline = Qt.createQmlObject(`
                    import QtLocation 6.5;
                    MapPolyline {
                        line.width: ${roadData.width || 2};
                        line.color: "${roadData.color || '#3498db'}";
                        opacity: 0.8;
                    }
                `, map)

                polyline.path = path
                map.addMapItem(polyline)
            }

            console.log("✅ Affichage terminé - Routes carrossables uniquement")
        }
    }

    // ✅ Couleurs pour les véhicules
    property var vehicleColors: [
        "#e74c3c", "#3498db", "#2ecc71", "#f39c12",
        "#9b59b6", "#1abc9c", "#e67e22", "#95a5a6"
    ]

    // ✅ Mise à jour des positions des véhicules
    Connections {
        target: simulationController
        function onVehiclePositionsUpdated(positions) {
            if (positions.length > 0) {
                // Mise à jour ultra-rapide du modèle
                if (vehicleModel.count !== positions.length) {
                    vehicleModel.clear()
                    for (let i = 0; i < positions.length; ++i) {
                        let pos = positions[i]
                        vehicleModel.append({
                            lat: pos.lat,
                            lon: pos.lon,
                            color: vehicleColors[pos.id % vehicleColors.length]
                        })
                    }
                } else {
                    // Mise à jour en place (plus rapide)
                    for (let i = 0; i < positions.length; ++i) {
                        let pos = positions[i]
                        vehicleModel.set(i, {
                            lat: pos.lat,
                            lon: pos.lon,
                            color: vehicleColors[pos.id % vehicleColors.length]
                        })
                    }
                }
            }
        }
    }

    // ✅ NOUVEAU: Mise à jour des connexions V2V
    Timer {
        interval: 200  // Mise à jour 5 fois par seconde (pour ne pas surcharger)
        running: simulationController.isRunning
        repeat: true
        onTriggered: {
            if (simulationController.interferenceGraph) {
                // Récupérer les connexions avec les positions
                var connections = simulationController.getV2VConnectionsWithPositions()

                // Mettre à jour le modèle
                connectionModel.clear()

                // Limiter le nombre de connexions affichées pour les performances
                var maxDisplay = Math.min(500, connections.length)
                for (var i = 0; i < maxDisplay; i++) {
                    connectionModel.append(connections[i])
                }

                if (connections.length > 0 && frameCounter++ % 50 === 0) {
                    console.log("📡 Affichage de", maxDisplay, "connexions V2V sur", connections.length)
                }
            }
        }

        property int frameCounter: 0
    }

    // 🕹️ Fonctions de contrôle
    function zoomIn()  { map.zoomLevel = Math.min(map.zoomLevel + 1, map.maximumZoomLevel) }
    function zoomOut() { map.zoomLevel = Math.max(map.zoomLevel - 1, map.minimumZoomLevel) }
    function centerMulhouse() {
        map.center = QtPositioning.coordinate(47.7508, 7.3359)
        map.zoomLevel = 14
    }
}
