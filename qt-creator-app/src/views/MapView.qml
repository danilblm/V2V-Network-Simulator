import QtQuick.Controls
import QtLocation
import QtPositioning
import QtQuick

Item {
    id: mapContainer
    width: parent ? parent.width : 800
    height: parent ? parent.height : 600

    property real minLat: 47.70
    property real maxLat: 47.80
    property real minLon: 7.28
    property real maxLon: 7.40
    property var vehicleColors: [
        "#e74c3c", "#3498db", "#2ecc71", "#f1c40f",
        "#9b59b6", "#e67e22", "#1abc9c", "#e84393"
    ]

    signal mapLoaded()

    Map {
        id: map
        anchors.fill: parent
        plugin: Plugin {
            name: "osm"

            PluginParameter {
                name: "osm.mapping.providersrepository.disabled"
                value: true
            }

            PluginParameter {
                name: "osm.mapping.host"
                value: "https://tile.openstreetmap.org/"
            }

            PluginParameter {
                name: "osm.mapping.secure"
                value: true
            }
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
            mapContainer.mapLoaded()
        }

        // ✅ Layer pour les rayons de transmission avec couleurs aléatoires OPAQUES
        MapItemView {
            id: transmissionRangeLayer
            model: ListModel { id: transmissionRangeModel }

            delegate: MapCircle {
                center: QtPositioning.coordinate(model.lat, model.lon)
                radius: model.range  // Rayon en mètres (100-500m)
                color: model.rangeColor  // Couleur aléatoire opaque
                opacity: 0.4  // Semi-opaque pour voir les superpositions
                border.width: 2
                border.color: model.rangeBorderColor
            }
        }

        // ✅ Layer pour les arêtes directionnelles V2V (AVEC DIRECTION)
        MapItemView {
            id: connectionLayer
            model: ListModel { id: connectionModel }

            delegate: MapPolyline {
                line.width: 3
                line.color: model.signalStrength > 80 ? "#2ecc71" : "#e67e22"
                opacity: 0.9
                path: [
                    QtPositioning.coordinate(model.lat1, model.lon1),
                    QtPositioning.coordinate(model.lat2, model.lon2)
                ]
            }
        }

        // ✅ Layer pour les indicateurs de direction (petits cercles)
        MapItemView {
            id: directionIndicatorLayer
            model: connectionModel

            delegate: MapCircle {
                center: QtPositioning.coordinate(model.lat2, model.lon2)
                radius: 5
                color: model.signalStrength > 80 ? "#27ae60" : "#d35400"
                border.width: 1
                border.color: "#ffffff"
                opacity: 0.9
            }
        }

        // ✅ Layer pour les véhicules (DESSUS pour être visibles)
        MapItemView {
            id: vehicleLayer
            model: ListModel { id: vehicleModel }
            delegate: VehicleItem { }
        }
    }

    MouseArea {
        anchors.fill: parent
        property real lastX: 0
        property real lastY: 0
        cursorShape: Qt.OpenHandCursor

        onPressed: (mouse) => {
            cursorShape = Qt.ClosedHandCursor
            lastX = mouse.x
            lastY = mouse.y
            mouse.accepted = true
        }

        onReleased: (mouse) => {
            cursorShape = Qt.OpenHandCursor
        }

        onPositionChanged: (mouse) => {
            if (mouse.buttons & Qt.LeftButton) {
                var dx = mouse.x - lastX
                var dy = mouse.y - lastY

                var latPerPixel = 0.00005 * Math.pow(2, 14 - map.zoomLevel)
                var lonPerPixel = 0.00007 * Math.pow(2, 14 - map.zoomLevel)

                map.center.latitude += dy * latPerPixel
                map.center.longitude -= dx * lonPerPixel

                if (map.center.latitude < minLat) map.center.latitude = minLat
                if (map.center.latitude > maxLat) map.center.latitude = maxLat
                if (map.center.longitude < minLon) map.center.longitude = minLon
                if (map.center.longitude > maxLon) map.center.longitude = maxLon

                lastX = mouse.x
                lastY = mouse.y
            }
        }

        onWheel: (wheel) => {
            if (wheel.angleDelta.y > 0)
                map.zoomLevel = Math.min(map.zoomLevel + 0.5, map.maximumZoomLevel)
            else
                map.zoomLevel = Math.max(map.zoomLevel - 0.5, map.minimumZoomLevel)
        }

        onDoubleClicked: (mouse) => {
            map.zoomLevel = Math.min(map.zoomLevel + 1, map.maximumZoomLevel)
        }
    }

    // 🔄 Réception des routes depuis le C++
    Connections {
        target: mapController
        function onRoadReady(roads) {
            const maxRoutes = Math.min(5000, roads.length)

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
        }
    }

    // ✅ Couleurs aléatoires pour les rayons de transmission (OPAQUES)
    property var transmissionRangeColors: [
        "#FF6B6B", "#4ECDC4", "#45B7D1", "#FFA07A",
        "#98D8C8", "#F7DC6F", "#BB8FCE", "#85C1E2",
        "#F8B195", "#C06C84", "#6C5B7B", "#355C7D",
        "#99B898", "#FECEAB", "#FF847C", "#E84A5F"
    ]

    // Couleurs de bordure assorties
    property var transmissionBorderColors: [
        "#E53935", "#00897B", "#1E88E5", "#FF6F00",
        "#00897B", "#F9A825", "#8E24AA", "#039BE5",
        "#E64A19", "#AD1457", "#4527A0", "#283593",
        "#43A047", "#FB8C00", "#E53935", "#C62828"
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

    // ✅ Mise à jour des rayons de transmission ET des arêtes directionnelles V2V
    Timer {
        interval: 200  // 5 fois par seconde
        running: simulationController.isRunning
        repeat: true
        onTriggered: {
            if (simulationController.interferenceGraph) {
                // 1. Récupérer les positions avec rayons de transmission
                var vehiclesWithRanges = simulationController.getVehiclesWithTransmissionRanges()

                // Mise à jour des rayons de transmission avec couleurs aléatoires
                if (transmissionRangeModel.count !== vehiclesWithRanges.length) {
                    transmissionRangeModel.clear()
                    for (var i = 0; i < vehiclesWithRanges.length; i++) {
                        var v = vehiclesWithRanges[i]
                        // Assigner une couleur aléatoire basée sur l'ID du véhicule
                        var colorIndex = v.id % transmissionRangeColors.length
                        transmissionRangeModel.append({
                            lat: v.lat,
                            lon: v.lon,
                            range: v.transmissionRange,
                            rangeColor: transmissionRangeColors[colorIndex],
                            rangeBorderColor: transmissionBorderColors[colorIndex]
                        })
                    }
                } else {
                    for (var i = 0; i < vehiclesWithRanges.length; i++) {
                        var v = vehiclesWithRanges[i]
                        var colorIndex = v.id % transmissionRangeColors.length
                        transmissionRangeModel.set(i, {
                            lat: v.lat,
                            lon: v.lon,
                            range: v.transmissionRange,
                            rangeColor: transmissionRangeColors[colorIndex],
                            rangeBorderColor: transmissionBorderColors[colorIndex]
                        })
                    }
                }

                // 2. Récupérer les arêtes directionnelles (A→B)
                var edges = simulationController.getV2VConnectionsWithPositions()

                // Mise à jour des arêtes directionnelles
                connectionModel.clear()
                var maxDisplay = Math.min(500, edges.length)
                for (var t = 0; t < maxDisplay; t++) {
                    connectionModel.append(edges[t])
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
