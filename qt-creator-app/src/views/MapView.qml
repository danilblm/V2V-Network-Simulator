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

        // ✅ Layer pour les rayons de transmission (SOUS les connexions)
        MapItemView {
            id: transmissionRangeLayer
            model: ListModel { id: transmissionRangeModel }

            delegate: MapCircle {
                center: QtPositioning.coordinate(model.lat, model.lon)
                radius: model.range  // Rayon en mètres (100-500m)
                color: model.color
                opacity: 0.15  // Très transparent pour ne pas surcharger
                border.width: 1
                border.color: model.borderColor
            }
        }

        // ✅ Layer pour les connexions V2V
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

        // ✅ Layer pour les véhicules (DESSUS pour être visibles)
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

    // Couleurs pour les rayons de transmission (plus transparentes)
    property var rangeColors: [
        "#e74c3c40", "#3498db40", "#2ecc7140", "#f39c1240",
        "#9b59b640", "#1abc9c40", "#e67e2240", "#95a5a640"
    ]

    property var rangeBorderColors: [
        "#e74c3c", "#3498db", "#2ecc71", "#f39c12",
        "#9b59b6", "#1abc9c", "#e67e22", "#95a5a6"
    ]

    // ✅ Mise à jour des positions des véhicules
    Connections {
        target: simulationController
        function onVehiclePositionsUpdated(positions) {
            if (positions.length > 0) {
                // Mise à jour des véhicules
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

    // ✅ Mise à jour des rayons de transmission ET des connexions V2V
    Timer {
        interval: 200  // 5 fois par seconde
        running: simulationController.isRunning
        repeat: true
        onTriggered: {
            if (simulationController.interferenceGraph) {
                // 1. Récupérer les positions avec rayons de transmission
                var vehiclesWithRanges = simulationController.getVehiclesWithTransmissionRanges()

                // Mise à jour des rayons de transmission
                if (transmissionRangeModel.count !== vehiclesWithRanges.length) {
                    transmissionRangeModel.clear()
                    for (var i = 0; i < vehiclesWithRanges.length; i++) {
                        var v = vehiclesWithRanges[i]
                        transmissionRangeModel.append({
                            lat: v.lat,
                            lon: v.lon,
                            range: v.transmissionRange,
                            color: rangeColors[v.id % rangeColors.length],
                            borderColor: rangeBorderColors[v.id % rangeBorderColors.length]
                        })
                    }
                } else {
                    for (var i = 0; i < vehiclesWithRanges.length; i++) {
                        var v = vehiclesWithRanges[i]
                        transmissionRangeModel.set(i, {
                            lat: v.lat,
                            lon: v.lon,
                            range: v.transmissionRange,
                            color: rangeColors[v.id % rangeColors.length],
                            borderColor: rangeBorderColors[v.id % rangeBorderColors.length]
                        })
                    }
                }

                // 2. Récupérer les connexions
                var connections = simulationController.getV2VConnectionsWithPositions()

                // Mise à jour des connexions
                connectionModel.clear()
                var maxDisplay = Math.min(500, connections.length)
                for (var t = 0; t < maxDisplay; t++) {
                    connectionModel.append(connections[t])
                }

                if (connections.length > 0 && frameCounter++ % 50 === 0) {
                    console.log("📡", vehiclesWithRanges.length, "véhicules,",
                               maxDisplay, "connexions affichées")
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
